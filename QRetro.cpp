#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QGamepad>
#include <QKeyEvent>
#include <QPainter>
#include <QThread>

#include <iostream>
#ifndef _WIN32
#include <pthread.h>
#endif

#include "math.h"

#include "QRetro.h"
#include "QRetroCommon.h"
#include "QRetroEnvironment.h"

using namespace std;
using namespace std::chrono;

static QGamepad gamepad;
static int16_t keys[4][RETRO_DEVICE_ID_JOYPAD_R3 + 1];
static int16_t sticks[4][2][2];
static int mousewheel[2];

long long unsigned QRetro::getCurrentFramebuffer()
{
  /* Framebuffer hasn't been initialized */
  if (!m_OpenGlFbo)
    m_OpenGlFbo = new QOpenGLFramebufferObject(QSize(
      static_cast<int>(m_Core.av_info.geometry.base_width),
      static_cast<int>(m_Core.av_info.geometry.base_height)));

  if (!m_ImageDrawing && m_OpenGlFbo->isValid() && !m_OpenGlFbo->isBound())
  {
    m_ImageRendering = true;
    auto a = m_OpenGlFbo->toImage();
    m_Image = a.copy();
    if (!m_Core.hw_render.bottom_left_origin)
      m_Image = m_Image.mirrored();
    m_ImageRendering = false;
  }

  return m_OpenGlFbo->handle();
}

void QRetro::updateScaling()
{
  double x = static_cast<double>(size().height()) / m_BaseRect.height();
  double y = static_cast<double>(size().width())  / m_BaseRect.width();
  double mult = x > y ? y : x;

  if (m_IntegerScaling && mult > 0)
    mult = floor(mult);

  m_Rect.setSize(QSize(
    static_cast<int>(m_BaseRect.width()  * mult),
    static_cast<int>(m_BaseRect.height() * mult)
  ));

  /* Center the rect in the available screenspace */
  m_Rect.moveLeft((size().width() - m_Rect.width())  / 2);
  m_Rect.moveTop((size().height() - m_Rect.height()) / 2);

  m_ScalingFactor = mult;
}

void QRetro::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event)
  updateScaling();
}

void QRetro::exposeEvent(QExposeEvent *event)
{
  Q_UNUSED(event)
}

void QRetro::setGeometry(const unsigned width, const unsigned height)
{
  m_BaseRect = QRect(0, 0, static_cast<int>(width), static_cast<int>(height));
  updateScaling();
}

void QRetro::setupPainter(QPainter *painter)
{
  /* Apply a basic filter if requested. */
  if (m_BilinearFilter)
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

  /* Apply rotation if requested. */
  /* TODO: Make scaling work properly on rotation */
  if (m_Rotation)
  {
    int x = m_Rect.left() + m_Rect.width()  / 2;
    int y = m_Rect.top()  + m_Rect.height() / 2;

    painter->translate(x, y);
    painter->rotate(m_Rotation);
    painter->translate(-x, -y);
  }
}

bool QRetro::event(QEvent *ev)
{
  switch (ev->type())
  {
  case QEvent::UpdateRequest:
    {
      QPainter painter;

      if (surfaceType() == QSurface::RasterSurface)
      {
        /* Cancel repaint if a HW accelerated core is still copying its FBO */
        if (m_ImageRendering || m_ImageDrawing)
          return false;

        /* Remake the buffer if the window dimensions have changed */
        bool dirty = false;
        if (size() != m_BackingStore->size())
        {
          m_BackingStore->resize(size());
          dirty = true;
        }

        m_ImageDrawing = true;
        m_BackingStore->beginPaint(m_Image.rect());
        painter.begin(m_BackingStore->paintDevice());
        setupPainter(&painter);

        painter.fillRect(0, 0, size().width(), size().height(), Qt::black);
        painter.drawImage(m_Rect, m_Image);
        painter.setPen(Qt::red);
        painter.drawText(16, size().height() - 16, "Software");
        painter.end();

        m_BackingStore->endPaint();
        m_BackingStore->flush(dirty ? QRect(0, 0, size().width(), size().height()) : m_Rect);
        m_ImageDrawing = false;
      }
      else if (surfaceType() == QSurface::OpenGLSurface)
      {
        /* We cancel repainting the screen if...
         * ...an FBO copy is still being performed... */
        if (m_ImageRendering)
          return false;
        /* ...the screen is already painting... */
        if (m_OpenGlDevice && m_OpenGlDevice->paintingActive())
          return false;
        /* ... or the OpenGL context is unavailable. */
        if (!m_OpenGlContext->makeCurrent(this))
          return false;

        /* Remake the viewport if the window dimensions have changed */
        if (!m_OpenGlDevice ||  m_OpenGlDevice->size() != size())
          m_OpenGlDevice = new QOpenGLPaintDevice(size());

        m_ImageDrawing = true;
        painter.begin(m_OpenGlDevice);
        setupPainter(&painter);

        painter.fillRect(0, 0, size().width(), size().height(), Qt::black);
        painter.drawImage(m_Rect, m_Image);
        painter.setPen(Qt::blue);
        painter.drawText(16, size().height() - 16, "OpenGL");

        m_OpenGlContext->swapBuffers(this);
        m_ImageDrawing = false;
      }
    }

    return true;
  default:
    return QWindow::event(ev);
  }
}

void QRetro::setImagePtr(const void *data, unsigned width, unsigned height,
  unsigned pitch)
{
  if (!isExposed())
    return;

  /* RETRO_HW_FRAME_BUFFER_VALID generates a warning, so use this instead. */
  if (data && data != reinterpret_cast<void*>(-1))
  {
    m_Image = QImage(
      reinterpret_cast<const uchar*>(data),
      static_cast<int>(width),
      static_cast<int>(height),
      static_cast<int>(pitch),
      m_PixelFormat
    );
  }
  requestUpdate();
}

static inline int16_t qt2lr_analog(double qt)
{
  return static_cast<int16_t>(qt * 0x7FFF);
}

static void core_input_poll(void)
{
  auto _this = _qrthis();

  /* If a custom input poll handler has been installed, use that instead */
  if (_this && _this->hasInputPollHandler())
  {
    _this->runInputPollHandler();
    return;
  }

  _this->core()->retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);

  /* Run the built-in input poll handler */
  keys[0][RETRO_DEVICE_ID_JOYPAD_A] = gamepad.buttonB();
  keys[0][RETRO_DEVICE_ID_JOYPAD_B] = gamepad.buttonA();
  keys[0][RETRO_DEVICE_ID_JOYPAD_X] = gamepad.buttonY();
  keys[0][RETRO_DEVICE_ID_JOYPAD_Y] = gamepad.buttonX();
  keys[0][RETRO_DEVICE_ID_JOYPAD_START] = gamepad.buttonStart();
  keys[0][RETRO_DEVICE_ID_JOYPAD_SELECT] = gamepad.buttonSelect();

  keys[0][RETRO_DEVICE_ID_JOYPAD_L] = gamepad.buttonL1();
  keys[0][RETRO_DEVICE_ID_JOYPAD_R] = gamepad.buttonR1();
  keys[0][RETRO_DEVICE_ID_JOYPAD_L2] = qt2lr_analog(gamepad.buttonL2());
  keys[0][RETRO_DEVICE_ID_JOYPAD_R2] = qt2lr_analog(gamepad.buttonR2());
  keys[0][RETRO_DEVICE_ID_JOYPAD_L3] = gamepad.buttonL3();
  keys[0][RETRO_DEVICE_ID_JOYPAD_R3] = gamepad.buttonR3();

  keys[0][RETRO_DEVICE_ID_JOYPAD_UP] = gamepad.buttonUp();
  keys[0][RETRO_DEVICE_ID_JOYPAD_DOWN] = gamepad.buttonDown();
  keys[0][RETRO_DEVICE_ID_JOYPAD_LEFT] = gamepad.buttonLeft();
  keys[0][RETRO_DEVICE_ID_JOYPAD_RIGHT] = gamepad.buttonRight();

  sticks[0][0][RETRO_DEVICE_ID_ANALOG_X] = qt2lr_analog(gamepad.axisLeftX());
  sticks[0][0][RETRO_DEVICE_ID_ANALOG_Y] = qt2lr_analog(gamepad.axisLeftY());
  sticks[0][1][RETRO_DEVICE_ID_ANALOG_X] = qt2lr_analog(gamepad.axisRightX());
  sticks[0][1][RETRO_DEVICE_ID_ANALOG_Y] = qt2lr_analog(gamepad.axisRightY());

  if (_this)
    _this->updateMouse();
}

void QRetro::updateMouse(void)
{
  /* Update RETRO_DEVICE_MOUSE */
  auto new_pos = mapFromGlobal(QCursor::pos());

  m_MouseDelta = QPoint(
    static_cast<int>((new_pos.x() - m_MousePosition.x()) / m_ScalingFactor),
    static_cast<int>((new_pos.y() - m_MousePosition.y()) / m_ScalingFactor));
  m_MousePosition = new_pos;

  /* Update RETRO_DEVICE_POINTER */
  new_pos -= QPoint(m_Rect.left(), m_Rect.top());
  double x = ((static_cast<double>(new_pos.x()) / m_Rect.width()  * 2.0) - 1.0) * 0x7FFF;
  double y = ((static_cast<double>(new_pos.y()) / m_Rect.height() * 2.0) - 1.0) * 0x7FFF;

  if (fabs(x) > 0x8000 || fabs(y) > 0x8000)
    m_PointerValid = false;
  else
  {
    m_PointerPosition = QPoint(static_cast<int>(x), static_cast<int>(y));
    m_PointerValid = true;
  }
}

void QRetro::setRotation(const unsigned degrees)
{
  m_Rotation = degrees % 360;
}

static int16_t core_input_state(unsigned port, unsigned device, unsigned index,
  unsigned id)
{
  auto _this = _qrthis();

  if (!_this)
    return 0;

  if (_this->hasInputStateHandler())
    return _this->runInputStateHandler(port, device, index, id);

  if (device == RETRO_DEVICE_JOYPAD)
  {
    if (id == RETRO_DEVICE_ID_JOYPAD_MASK && _this->supportsInputBitmasks())
    {
      uint16_t bitmask = 0;

      for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
        bitmask |= keys[port][i] ? (1 << i) : 0;

      return static_cast<int16_t>(bitmask);
    }
    else if (port <= 4 && id <= RETRO_DEVICE_ID_JOYPAD_R3)
      return keys[port][id];
    else
      return 0;
  }
  else if (device == RETRO_DEVICE_ANALOG)
  {
    if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON)
      return keys[port][id];
    else
      return sticks[port][index][id];
  }
  else if (device == RETRO_DEVICE_POINTER)
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_POINTER_X:
      return static_cast<short>(_this->pointerPosition().x());
    case RETRO_DEVICE_ID_POINTER_Y:
      return static_cast<short>(_this->pointerPosition().y());
    case RETRO_DEVICE_ID_POINTER_PRESSED:
      return _this->pointerValid() && QApplication::mouseButtons().testFlag(Qt::LeftButton);
    /* TODO: Support multitouch, if ever needed */
    case RETRO_DEVICE_ID_POINTER_COUNT:
      return 1;
    }
  }
  else if (device == RETRO_DEVICE_MOUSE)
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_MOUSE_X:
      return static_cast<short>(_this->mouseDelta().x());
    case RETRO_DEVICE_ID_MOUSE_Y:
      return static_cast<short>(_this->mouseDelta().y());
    case RETRO_DEVICE_ID_MOUSE_LEFT:
      return QApplication::mouseButtons().testFlag(Qt::LeftButton);
    case RETRO_DEVICE_ID_MOUSE_RIGHT:
      return QApplication::mouseButtons().testFlag(Qt::RightButton);
    case RETRO_DEVICE_ID_MOUSE_MIDDLE:
      return QApplication::mouseButtons().testFlag(Qt::MiddleButton);
    case RETRO_DEVICE_ID_MOUSE_BUTTON_4:
      return QApplication::mouseButtons().testFlag(Qt::ExtraButton1);
    case RETRO_DEVICE_ID_MOUSE_BUTTON_5:
      return QApplication::mouseButtons().testFlag(Qt::ExtraButton2);
    case RETRO_DEVICE_ID_MOUSE_WHEELUP:
      return mousewheel[0] > 0;
    case RETRO_DEVICE_ID_MOUSE_WHEELDOWN:
      return mousewheel[0] < 0;
    case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP:
      return mousewheel[1] > 0;
    case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:
      return mousewheel[1] < 0;
    default:
      return 0;
    }
  }

  return 0;
}

bool load_function(void *func_ptr, QRETRO_LIBRARY_T library, const char *name)
{
  if (!library)
    return false;
  else
  {
    QRETRO_FUNCTION_T new_func = nullptr;

#if defined(_WIN32)
    new_func = reinterpret_cast<QRETRO_FUNCTION_T>(GetProcAddress(library, name));
#else
    new_func = reinterpret_cast<QRETRO_FUNCTION_T>(dlsym(library, name));
#endif

    if (!new_func)
    {
      printf("Could not load %s from dynamic library.\n", name);
      return false;
    }
    else
    {
      memcpy(func_ptr, &new_func, sizeof(QRETRO_FUNCTION_T));
      printf("%p - Loaded %s from dynamic library.\n", func_ptr, name);
      return true;
    }
  }
}

bool QRetro::coreSerialize(void *data, size_t size)
{
  return (data && size) ? m_Core.retro_serialize(data, size) : false;
}

bool QRetro::coreUnserialize(void *data, size_t size)
{
  return (data && size) ? m_Core.retro_unserialize(data, size) : false;
}

#define _load_function(a) success &= load_function(&core->a, library, #a)
bool load_library(retro_core_t *core, QRETRO_LIBRARY_T library)
{
  bool success = true;

  memset(core, 0, sizeof(retro_core_t));
  _load_function(retro_api_version);
  _load_function(retro_cheat_reset);
  _load_function(retro_cheat_set);
  _load_function(retro_deinit);
  _load_function(retro_get_memory_data);
  _load_function(retro_get_memory_size);
  _load_function(retro_get_region);
  _load_function(retro_get_system_av_info);
  _load_function(retro_get_system_info);
  _load_function(retro_init);
  _load_function(retro_load_game);
  _load_function(retro_load_game_special);
  _load_function(retro_reset);
  _load_function(retro_run);
  _load_function(retro_serialize);
  _load_function(retro_serialize_size);
  _load_function(retro_set_audio_sample);
  _load_function(retro_set_audio_sample_batch);
  _load_function(retro_set_controller_port_device);
  _load_function(retro_set_environment);
  _load_function(retro_set_input_poll);
  _load_function(retro_set_input_state);
  _load_function(retro_set_video_refresh);
  _load_function(retro_unload_game);
  _load_function(retro_unserialize);

  if (success)
  {
    core->symbols_inited = true;
    return true;
  }
  return false;
}

QRetro::QRetro(QWindow *parent, retro_hw_context_type format)
{
  setParent(parent);

  initVideo(format);

  m_Location = new QRetroLocation(this);

  /* Default value initialization */
  setAutosaveInterval(5);
  setLanguage(RETRO_LANGUAGE_ENGLISH);
  setLogLevel(RETRO_LOG_ERROR);
  setPreferredRenderer(format);

  /* Initialize member variables */
  m_Active = false;
  memset(&m_Core, 0, sizeof(m_Core));
  m_InputPollHandler = nullptr;
  m_InputStateHandler = nullptr;
  m_Library = nullptr;
  memset(m_SupportedEnvCallbacks, true, sizeof(m_SupportedEnvCallbacks));
}

QRetro::~QRetro()
{
  /* Stop processing the core */
  m_Active = false;

  /* Cancel active threads */
  if (m_ThreadSaving)
    m_ThreadSaving->quit();
  if (m_ThreadTiming)
    m_ThreadTiming->quit();

  /* Let the core run its deconstructors */
  if (m_Core.inited)
  {
    m_Core.retro_unload_game();
    m_Core.retro_deinit();
  }

  /* Delete member variables on heap */
  delete m_Audio;
  delete m_Location;

  /* Free the dynamic library */
  if (m_Library)
  {
#ifdef _WIN32
    FreeLibrary(m_Library);
#else
    dlclose(m_Library);
#endif
  }

  /* Remove the object from our static thread map */
  _qrdelete(this);
}

void QRetro::timing()
{
  auto next = steady_clock::now();

  /* Map our emulation thread to this instance of a QRetro object */
  _qrnew(this_thread::get_id(), this);

  /* Refresh core info, as it may have changed after retro_init */
  m_Core.retro_get_system_info(&m_Core.system_info);
  //m_Core.retro_get_system_av_info(&m_Core.av_info);

  m_Options.setCoreName(m_Core.system_info.library_name);

  /* Environment callbacks can get passed here, before even "retro_init" */
  m_Core.retro_set_environment(core_environment);
  m_Core.retro_set_video_refresh(core_video_refresh);
  m_Core.retro_set_input_poll(core_input_poll);
  m_Core.retro_set_input_state(core_input_state);
  m_Core.retro_set_audio_sample(core_audio_sample);
  m_Core.retro_set_audio_sample_batch(core_audio_sample_batch);

  m_Core.retro_init();
  m_Core.inited = true;

  if (!m_Core.retro_load_game(&m_Core.game_info))
  {
    emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
      "Function retro_load_game failed for an unknown reason:\n%1").arg(m_Core.game_info.path)));
  }

  if (m_Core.hw_render.context_reset)
    m_Core.hw_render.context_reset();

  emit onCoreStart();

  /* Sleep until the core has started running */
  while (!m_Active)
    this_thread::sleep_for(seconds(1));

  m_Core.retro_get_system_info(&m_Core.system_info);
  m_Core.retro_get_system_av_info(&m_Core.av_info);

  setGeometry(m_Core.av_info.geometry.base_width, m_Core.av_info.geometry.base_height);
  QWindow::setGeometry(m_BaseRect);

  m_Audio = new QRetroAudio(m_Core.av_info.timing.sample_rate, 60.0, m_TargetRefreshRate);
  m_Audio->start();

  /* Notify core to begin sending audio, if using audio callback */
  if (m_Core.audio_callback.set_state)
    m_Core.audio_callback.set_state(true);

#ifdef _WIN32
  if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
    printf("Unable to change thread priority.\n");
#else
  int policy;
  int error;
  sched_param param;

  error = pthread_getschedparam(pthread_self(), &policy, &param);
  if (error)
    printf("Unable to get thread priority (%u).\n", error);
  else
  {
    param.sched_priority = sched_get_priority_max(policy);
    error = pthread_setschedparam(pthread_self(), policy, &param);
    if (error)
      printf("Unable to set thread priority (%u).\n", error);
  }
#endif

  while (m_Active)
  {
    if (inputReady() && // stall if waiting for input (netplay)
        isVisible() && // stall if window is not available in context
        !m_Paused && // stall if content is paused
        !m_Audio->excessFramesInBuffer()) // stall to play the audio queue
    {
      if (m_Core.frame_time_callback.callback)
        m_Core.frame_time_callback.callback(m_Core.frame_time_callback.reference);
      m_Core.retro_run();
      m_Frames++;
    }

    m_Core.retro_get_system_av_info(&m_Core.av_info);
    m_BaseRect.setSize(QSize(m_Core.av_info.geometry.base_height * (m_UseAspectRatio ? m_Core.av_info.geometry.aspect_ratio : 1), m_Core.av_info.geometry.base_height));
    updateScaling();

    if (m_Core.audio_callback.callback)
      m_Core.audio_callback.callback();
    m_Audio->playFrame();

    auto time = static_cast<unsigned>(1000000000 / m_TargetRefreshRate);
    if (m_FastForwarding && m_FastForwardRatio > 1.0f)
      time = static_cast<unsigned>(static_cast<float>(time) / m_FastForwardRatio);

    if (m_FastForwarding && m_FastForwardRatio <= 1.0f)
      continue;

    this_thread::sleep_until(next - milliseconds(2));
    while (steady_clock::now() < next);
    next += nanoseconds(time);
  }
}

/* TODO: Support flags, test */
bool QRetro::getCurrentSoftwareFramebuffer(retro_framebuffer *fb)
{
  auto width = static_cast<int>(fb->width);
  auto height = static_cast<int>(fb->height);
  auto format = lr2qt_pixel(fb->format);

  /* Free our buffer if it's being reconstructed to a new size */
  if (m_SoftwareFbBuffer && (m_Image.width() != width ||
                             m_Image.height() != height ||
                             m_Image.format() != format))
  {
    free(m_SoftwareFbBuffer);
    m_SoftwareFbBuffer = nullptr;
  }

  if (!m_SoftwareFbBuffer)
  {
    /* Allocate buffer as black canvas */
    m_SoftwareFbBuffer = reinterpret_cast<uchar*>(
      calloc(fb->width * fb->height * fb->pitch, sizeof(uchar)));

    /* Update our renderable */
    m_Image = QImage(m_SoftwareFbBuffer, width, height, format);
  }

  /* Finalize data to send back to core */
  fb->data = m_SoftwareFbBuffer;
  fb->memory_flags = RETRO_MEMORY_TYPE_CACHED;

  return m_Image.bits();
}

void QRetro::setFastForwardingOverride(retro_fastforwarding_override* ffo)
{
  m_Core.fastforwarding_override = *ffo;

  setFastForwarding(ffo->fastforward);
  setFastForwardingRatio(ffo->ratio);
}

void QRetro::keyPressEvent(QKeyEvent *event)
{
  if (m_Core.keyboard.callback)
    m_Core.keyboard.callback
    (
      event->type() == QEvent::KeyPress,
      qt2lr_keyboard(event->key()),
      event->nativeVirtualKey(),
      qt2lr_keymod(event->modifiers())
    );

  if (event->modifiers().testFlag(Qt::ShiftModifier))
  {
  switch (event->key())
  {
    case Qt::Key_A:
      m_UseAspectRatio = !m_UseAspectRatio;
      updateScaling();
      break;
    case Qt::Key_B:
      m_BilinearFilter = !m_BilinearFilter;
      break;
    case Qt::Key_F:
      showFullScreen();
      break;
    case Qt::Key_I:
      m_IntegerScaling = !m_IntegerScaling;
      updateScaling();
      break;
    case Qt::Key_Escape:
      showNormal();
      break;
    case Qt::Key_O:
      initVideo(RETRO_HW_CONTEXT_OPENGL);
      break;
    case Qt::Key_P:
      m_Paused = !m_Paused;
      break;
    case Qt::Key_R:
      m_Core.retro_reset();
      break;
    case Qt::Key_S:
      initVideo(RETRO_HW_CONTEXT_NONE);
      break;
    case Qt::Key_W:
      setFastForwarding(!m_FastForwarding);
      break;
    case Qt::Key_9:
      setRotation(m_Rotation + 90);
      break;
    case Qt::Key_F1:
      options()->update();
      options()->show();
      break;
    }
  }
}

void QRetro::setFastForwarding(bool enabled)
{
  if (!m_Core.fastforwarding_override.inhibit_toggle)
    m_FastForwarding = enabled;
  else
    emit onCoreLog(RETRO_LOG_WARN,
      "Fastforward toggling is currently being inhibited by the core.");
}

void QRetro::saving()
{
  /* Wait until the first frame */
  while (!m_Frames);

  auto data = m_Core.retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
  auto size = m_Core.retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
  QByteArray hash;
  QCryptographicHash hasher(QCryptographicHash::Md5);
  QFile save_file(QString("%1/%2.sav").arg(
    m_Directories.get(QRetroDirectories::Save),
    QFileInfo(contentPath()).baseName())
  );

  /* Initial load for save file */
  if (save_file.open(QIODevice::ReadOnly) && data && size)
  {
    save_file.read(reinterpret_cast<char*>(data), static_cast<qint64>(size));
    hasher.addData(reinterpret_cast<char*>(data), static_cast<int>(size));
    hash = hasher.result();
  }
  save_file.close();

  while (m_Active)
  {
    QThread::sleep(m_AutosaveInterval);

    /* Request save data from core */
    data = m_Core.retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    size = m_Core.retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);

    /* No save data */
    if (!data || !size)
      continue;

    /* Has the save data changed? If not, don't access the file. */
    hasher.reset();
    hasher.addData(reinterpret_cast<const char*>(data), static_cast<int>(size));
    auto result = hasher.result();
    if (result == hash)
      continue;
    hash = result;

    /* Flush save data to file */
    save_file.open(QIODevice::ReadWrite);
    save_file.write(static_cast<const char*>(data), static_cast<qint64>(size));
    save_file.close();

    emit onSave();
  }
}

bool QRetro::startCore(bool force)
{
  if (!m_Core.symbols_inited)
  {
    emit onCoreLog(RETRO_LOG_ERROR,
      "The core attempted to start without first being initialized.");
    return false;
  }
  else if (!m_Core.supports_no_game && !m_Core.content_loaded && !force)
  {
    emit onCoreLog(RETRO_LOG_ERROR,
      "The core attempted to start without content, but there is no hint that "
      "it doing so is supported.");
    return false;
  }
  else
  {
    m_Active = true;
    m_ThreadSaving = QThread::create([this]{saving();});
    m_ThreadSaving->start();
    gamepad.setDeviceId(0);

    m_ThreadTiming = QThread::create([this]{timing();});
    m_ThreadTiming->start();

    return true;
  }
}

bool QRetro::loadContent(const char *path, const char *meta)
{
  if (!m_Core.symbols_inited)
    return false;
  else
  {
    retro_game_info info;
    QFile rom(path);
    QFileInfo fileinfo(rom);

    /* TODO: Free game data here if it already exists? */

    if (!fileinfo.exists() || !fileinfo.isFile())
    {
      emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
        "The content file specified does not exist:\n%1").arg(path)));
      return false;
    }
    else if (!rom.open(QIODevice::ReadOnly))
    {
      emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
        "The content file specified could not be opened:\n%1").arg(path)));
      return false;
    }
    else
    {
      auto size = static_cast<size_t>(rom.size());

      if (size < 256 * 1024 * 1024) // TODO: Magic number limit
      {
        auto *rom_temp = reinterpret_cast<char*>(malloc(size));
        rom.read(rom_temp, rom.size());
        info.data = rom_temp;
      }
      else
        info.data = nullptr;

      m_ContentPath = path;

      info.meta = meta;
      info.path = m_ContentPath.c_str();
      info.size = size;
      m_Core.game_info = info;

      m_Core.content_loaded = true;

      return true;
    }
  }
}

bool QRetro::loadCore(const char *path)
{
  QFileInfo fileinfo(path);

  if (m_Library)
  {
    emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
      "Tried to load a new core without unloading the previous one.")));
    return false;
  }
  else if (!fileinfo.exists() || !fileinfo.isFile())
  {
    emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
      "The core file specified does not exist:\n%1").arg(path)));
    return false;
  }

#ifdef _WIN32
  /* TODO: Will this work on paths with unicode characters? */
  m_Library = LoadLibraryA(path);
#else
  m_Library = dlopen(path, RTLD_LAZY)
#endif

  if (!m_Library)
  {
#ifdef _WIN32
    emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
      "Unable to load dynamic library from:\n%1\n\nError code: %2").arg(
      path, static_cast<int>(GetLastError()))));
#else
    emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
      "Unable to load dynamic library from:\n%1\n\nError: %2").arg(
      path, dlerror())));
#endif
    return false;
  }

  if (!load_library(&m_Core, m_Library))
    return false;

  m_CorePath = fileinfo.absolutePath().toStdString();

  return true;
}

void* QRetro::getProcAddress(QThread *caller, const char *symbol)
{
  if (!m_OpenGlContextCore)
  {
    m_OpenGlContextCore = new QOpenGLContext();
    m_OpenGlContextCore->moveToThread(caller);
    m_OpenGlContextCore->setFormat(requestedFormat());
    m_OpenGlContextCore->create();
    m_OpenGlContextCore->makeCurrent(this);
    initializeOpenGLFunctions();
  }

  if (m_OpenGlContextCore)
  {
    void *ptr = nullptr;

    ptr = reinterpret_cast<void*>(m_OpenGlContextCore->getProcAddress(symbol));

    return ptr;
  }

  return nullptr;
}

bool QRetro::initVideo(retro_hw_context_type format)
{
  QSurfaceFormat settings;
  settings.setSwapInterval(1);
  setFormat(settings);

  switch (format)
  {
  case RETRO_HW_CONTEXT_NONE:
    setSurfaceType(QSurface::RasterSurface);
    create();

    m_BackingStore = new QBackingStore(this);

    break;
  case RETRO_HW_CONTEXT_OPENGL:
  case RETRO_HW_CONTEXT_OPENGL_CORE:
    setSurfaceType(QSurface::OpenGLSurface);
    create();

    if (!m_OpenGlContext)
    {
      m_OpenGlContext = new QOpenGLContext(this);
      m_OpenGlContext->setFormat(requestedFormat());
      m_OpenGlContext->create();
    }

    break;
  default:
    printf("INIT VIDEO ERROR!");
    return false;
  }

  connect(this, SIGNAL(onVideoRefresh(const void*, unsigned, unsigned, unsigned)), this, SLOT(setImagePtr(const void*, unsigned, unsigned, unsigned)));

  return true;
}

void QRetro::setPixelFormat(retro_pixel_format format)
{
  setPixelFormat(lr2qt_pixel(format));
}

void QRetro::setPixelFormat(QImage::Format format)
{
  const uchar *ptr = m_Image.bits();
  QSize size = m_Image.size();

  m_PixelFormat = format;
  m_Image = QImage(ptr, size.width(), size.height(), m_PixelFormat);
}

void QRetro::wheelEvent(QWheelEvent *event)
{
  mousewheel[0] = event->angleDelta().y() / 8;
  mousewheel[1] = event->angleDelta().x() / 8;
}
