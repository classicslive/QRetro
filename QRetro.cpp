#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QPainter>
#include <QMutexLocker>
#include <QThread>
#include <QUuid>

#ifndef _WIN32
#include <dlfcn.h>
#include <pthread.h>
#endif

#include "math.h"

#include "QRetro.h"
#include "QRetroCommon.h"
#include "QRetroEnvironment.h"
#if QRETRO_HAVE_SDL3
#include "QRetroInputBackendSDL3.h"
#elif QRETRO_HAVE_GAMEPAD
#include "QRetroInputBackendQGamepad.h"
#endif

#if QRETRO_HAVE_ZIP
#include <private/qzipreader_p.h>
#include <private/qzipwriter_p.h>
#endif

using namespace std;
using namespace std::chrono;

long long unsigned QRetro::glGetCurrentFramebuffer(void)
{
#if QRETRO_HAVE_OPENGL
  /**
   * Create framebuffer if it is invalid or null.
   * The FBO must be at least max_width x max_height so cores that render
   * variable-size frames within a larger buffer can do so safely.
   */
  QSize fbo_size = QSize(static_cast<int>(m_Core.av_info.geometry.max_width),
                         static_cast<int>(m_Core.av_info.geometry.max_height));

  m_FboRequestedThisFrame = true;

  if ((!m_OpenGlFbo || m_OpenGlFbo->size() != fbo_size) && !fbo_size.isEmpty())
  {
    if (m_OpenGlFbo)
      delete m_OpenGlFbo;

    QOpenGLFramebufferObjectFormat format;

    if (m_Core.hw_render.stencil)
      format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    else if (m_Core.hw_render.depth)
      format.setAttachment(QOpenGLFramebufferObject::Depth);
    else
      format.setAttachment(QOpenGLFramebufferObject::NoAttachment);

    m_OpenGlFbo = new QOpenGLFramebufferObject(fbo_size, format);
  }

  /*
  if (m_OpenGlFbo && !m_ImageDrawing && m_OpenGlFbo->isValid() && !m_OpenGlFbo->isBound())
  {
    m_ImageRendering = true;
    m_Image = m_OpenGlFbo->toImage(m_Core.hw_render.bottom_left_origin);
    m_ImageRendering = false;
  }
  */

  return m_OpenGlFbo ? m_OpenGlFbo->handle() : 0;
#else
  return 0;
#endif
}

void* QRetro::glGetProcAddress(QThread *caller, const char *symbol)
{
#if QRETRO_HAVE_OPENGL
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
#else
  Q_UNUSED(caller)
  Q_UNUSED(symbol)
#endif

  return nullptr;
}

void QRetro::updateScaling()
{
  int bw = m_BaseRect.width();
  int bh = m_BaseRect.height();
  double ar;
  bool swap = (m_Rotation == 90 || m_Rotation == 270);

  if (bw <= 0 || bh <= 0)
    return;

  /* Setup preferred aspect ratio */
  if (m_UseAspectRatio && m_Core.av_info.geometry.aspect_ratio > 0.0f)
    ar = static_cast<double>(m_Core.av_info.geometry.aspect_ratio);
  else
    ar = static_cast<double>(bw) / bh;

  double fitW = swap ? static_cast<double>(size().width())  / bh
                     : static_cast<double>(size().width())  / (ar * bh);
  double fitH = swap ? static_cast<double>(size().height()) / (ar * bh)
                     : static_cast<double>(size().height()) / bh;
  double mult = qMin(fitW, fitH);

  /* Use integer scaling if requested */
  if (m_IntegerScaling && mult > 0)
    mult = floor(mult);

  int dispW = static_cast<int>(ar * bh * mult);
  int dispH = static_cast<int>(bh * mult);

  /* Center the rect in the available screenspace */
  m_Rect.setSize(QSize(dispW, dispH));
  m_Rect.moveLeft((size().width()  - dispW) / 2);
  m_Rect.moveTop((size().height() - dispH) / 2);

  m_ScalingFactor = mult;
}

void QRetro::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event)
  updateScaling();
  requestUpdate();
}

void QRetro::exposeEvent(QExposeEvent *event)
{
  Q_UNUSED(event)
}

void QRetro::setGeometry(const unsigned width, const unsigned height)
{
  if (m_BaseRect.width() != static_cast<int>(width) ||
      m_BaseRect.height() != static_cast<int>(height))
  {
    m_BaseRect = QRect(0, 0, static_cast<int>(width), static_cast<int>(height));
    m_Image = QImage(m_BaseRect.size(), m_PixelFormat);
    updateScaling();
  }
}

void QRetro::setAvInfo(const retro_system_av_info *info)
{
  m_Core.av_info = *info;
  setGeometry(info->geometry.base_width, info->geometry.base_height);

  if (m_Audio)
  {
    delete m_Audio;
    m_Audio = new QRetroAudio(info->timing.sample_rate, 60.0, m_TargetRefreshRate);
    m_Audio->start();
    m_Audio->setEnabled(m_AudioEnabled);
  }
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
  case QEvent::Close:
    if (m_Config)
      m_Config->close();
    unloadCore();
    deleteLater();
    return QWindow::event(ev);
  case QEvent::UpdateRequest:
    {
      QPainter painter;

      if (!isExposed())
        return false;

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

#if QRETRO_DEBUG
        painter.setPen(Qt::red);
        painter.drawText(16, size().height() - 16, "Software");
#endif
        painter.end();

        m_BackingStore->endPaint();
        m_BackingStore->flush(dirty ? QRect(0, 0, size().width(), size().height()) : m_Rect);
        m_ImageDrawing = false;
      }
#if QRETRO_HAVE_OPENGL
      else if (surfaceType() == QSurface::OpenGLSurface)
      {
        if (m_OpenGlContextCore)
        {
          /**
           * GL core: timing thread owns m_OpenGlContextCore and handles
           * rendering and buffer swapping. Nothing to do here.
           */
        }
        else if (m_OpenGlContext)
        {
          /* Software core on an OpenGL surface: draw m_Image via QPainter. */
          if (!m_OpenGlContext->makeCurrent(this))
            return false;

          if (!m_OpenGlDevice || m_OpenGlDevice->size() != size())
          {
            delete m_OpenGlDevice;
            m_OpenGlDevice = new QOpenGLPaintDevice(size());
          }

          painter.begin(m_OpenGlDevice);
          setupPainter(&painter);
          painter.fillRect(0, 0, size().width(), size().height(), Qt::black);
          painter.drawImage(m_Rect, m_Image);
          painter.end();
          m_OpenGlContext->swapBuffers(this);
          m_FramePresented.release();
        }
      }
#endif
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

  /**
   * If data is a valid pointer, treat it as image data.
   * If data is NULL, treat it as a duplicate frame.
   * If data is RETRO_HW_FRAME_BUFFER_VALID (-1), treat it as a signal a HW frame is ready.
   */

  if (data && data != reinterpret_cast<void*>(RETRO_HW_FRAME_BUFFER_VALID))
  {
    /* Data is a pointer to a new frame image, so use it */
    m_Image = QImage(
      reinterpret_cast<const uchar*>(data),
      static_cast<int>(width),
      static_cast<int>(height),
      static_cast<int>(pitch),
      m_PixelFormat
    );
    /* Update on the GUI thread */
    QMetaObject::invokeMethod(this, [this]() { requestUpdate(); },
                              Qt::QueuedConnection);

    return;
  }
#if QRETRO_HAVE_OPENGL
  else if (surfaceType() == QSurface::OpenGLSurface && m_OpenGlContextCore && m_Rect.isValid())
  {
    int h   = size().height();
    int sw  = (width  > 0) ? static_cast<int>(width)  : m_BaseRect.width();
    int sh  = (height > 0) ? static_cast<int>(height) : m_BaseRect.height();
    int dx0 = m_Rect.x();
    int dy0 = h - m_Rect.y() - m_Rect.height();
    int dx1 = m_Rect.x() + m_Rect.width();
    int dy1 = h - m_Rect.y();

    auto *ef = m_OpenGlContextCore->extraFunctions();
    GLenum filter = m_BilinearFilter ? GL_LINEAR : GL_NEAREST;
    if (ef && sw > 0 && sh > 0)
    {
      /* Some cores will scissor to their internal resolution, disable that */
      glDisable(GL_SCISSOR_TEST);

      if (m_FboRequestedThisFrame && m_OpenGlFbo && m_OpenGlFbo->isValid())
      {
        /* Core rendered into our FBO via get_current_framebuffer.
         * Blit it directly to the window at the scaled rect. */
        ef->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ef->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_OpenGlFbo->handle());
        if (m_Core.hw_render.bottom_left_origin)
          ef->glBlitFramebuffer(0, 0, sw, sh, dx0, dy0, dx1, dy1,
                                GL_COLOR_BUFFER_BIT, filter);
        else
          ef->glBlitFramebuffer(0, sh, sw, 0, dx0, dy0, dx1, dy1,
                                GL_COLOR_BUFFER_BIT, filter);
      }
      else
      {
        /* Core rendered directly to FBO 0 at native size (bottom-left).
         * Two-pass: copy native region to intermediate FBO, then blit scaled. */
        if (!m_OpenGlFboIntermediate ||
            m_OpenGlFboIntermediate->size() != m_BaseRect.size())
        {
          delete m_OpenGlFboIntermediate;
          m_OpenGlFboIntermediate = new QOpenGLFramebufferObject(m_BaseRect.size());
        }
        if (m_OpenGlFboIntermediate->isValid())
        {
          /* Pass 1: 1:1 copy from FBO 0 → intermediate FBO (always nearest) */
          ef->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
          ef->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_OpenGlFboIntermediate->handle());
          ef->glBlitFramebuffer(0, 0, sw, sh, 0, 0, sw, sh,
                                GL_COLOR_BUFFER_BIT, GL_NEAREST);

          /* Pass 2: blit intermediate FBO → FBO 0 at scaled rect */
          ef->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
          glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT);
          ef->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_OpenGlFboIntermediate->handle());
          ef->glBlitFramebuffer(0, 0, sw, sh, dx0, dy0, dx1, dy1,
                                GL_COLOR_BUFFER_BIT, filter);
        }
      }
    }

    /* Toggle vsync based on fast-forward state, then present. */
    if (m_pfnSwapInterval)
      m_pfnSwapInterval(m_FastForwarding ? 0 : 1);
    m_OpenGlContextCore->swapBuffers(this);
  }
#endif
  /* Pre-poll input immediately after vsync */
  m_Input.poll();
  m_InputPrePolled = true;

  QMetaObject::invokeMethod(this, [this]() { requestUpdate(); },
                            Qt::QueuedConnection);
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

  m_PointerPosition.setX(fabs(x) >= 0x8000 ? (x > 0 ? 0x7FFF : 0x8000) : static_cast<int>(x));
  m_PointerPosition.setY(fabs(y) >= 0x8000 ? (y > 0 ? 0x7FFF : 0x8000) : static_cast<int>(y));
  m_PointerValid = fabs(x) < 0x8000 && fabs(y) < 0x8000;
}

void QRetro::setRotation(const unsigned degrees)
{
  m_Rotation = degrees % 360;
}

void QRetro::execOnTimingThread(std::function<void()> action)
{
  QMutexLocker lock(&m_PendingMutex);
  m_PendingAction = std::move(action);
  m_PendingDone.wait(&m_PendingMutex);
}

size_t QRetro::serializeSize(void)
{
  size_t sz = 0;
  execOnTimingThread([&]() { sz = m_Core.retro_serialize_size(); });
  return sz;
}

bool QRetro::serialize(void *data, size_t size)
{
  if (!data || !size)
    return false;

  bool result = false;
  execOnTimingThread([&]() { result = m_Core.retro_serialize(data, size); });
  return result;
}

bool QRetro::unserialize(const void *data, size_t size)
{
  if (!data || !size)
    return false;

  bool result = false;
  execOnTimingThread([&]() { result = m_Core.retro_unserialize(data, size); });
  return result;
}

bool QRetro::serializeToFile(const QString &path, bool zipped)
{
  size_t sz = serializeSize();
  if (!sz)
    return false;

  std::vector<unsigned char> buf(sz);
  if (!serialize(buf.data(), sz))
    return false;

#if QRETRO_HAVE_ZIP
  if (zipped) {
    QZipWriter zip(path);
    QFileInfo info(path);
    zip.addFile(info.completeBaseName(), QByteArray(reinterpret_cast<const char*>(buf.data()), sz));
    return zip.status() == QZipWriter::NoError;
  }
#else
  (void)zipped;
#endif

  QFile f(path);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  return f.write(reinterpret_cast<const char*>(buf.data()), sz) == static_cast<qint64>(sz);
}

bool QRetro::unserializeFromFile(const QString &path)
{
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly))
    return false;

  QByteArray data = f.readAll();

#if QRETRO_HAVE_ZIP
  if (data.size() >= 4 &&
      (unsigned char)data[0] == 0x50 && (unsigned char)data[1] == 0x4B &&
      (unsigned char)data[2] == 0x03 && (unsigned char)data[3] == 0x04)
  {
    QZipReader zip(path);
    const auto entries = zip.fileInfoList();
    if (entries.isEmpty())
      return false;
    data = zip.fileData(entries.first().filePath);
    if (data.isEmpty())
      return false;
  }
#endif

  return unserialize(data.constData(), static_cast<size_t>(data.size()));
}

QString QRetro::stateFilePath(void)
{
  auto sanitize = [](const QString &s) {
    return s.toLower().replace(QRegularExpression("[^a-z0-9]"), "");
  };

  QString core    = sanitize(QString(m_Core.system_info.library_name));
  QString content = sanitize(QFileInfo(contentPath()).completeBaseName());
  QString dir     = QString(directories()->get(QRetroDirectories::State));

  return QStringLiteral("%1/%2_%3.state").arg(dir, core, content);
}

bool QRetro::stateSave(void)
{
  size_t sz = serializeSize();
  if (!sz)
    return false;

  if (m_QuickSave)
    free(m_QuickSave);
  m_QuickSave = (unsigned char*)calloc(1, sz);
  m_QuickSaveSize = sz;
  return serialize(m_QuickSave, m_QuickSaveSize);
}

bool QRetro::stateLoad(void)
{
  if (!m_QuickSave || !m_QuickSaveSize)
    return false;

  return unserialize(m_QuickSave, m_QuickSaveSize);
}

void QRetro::reset(void)
{
  execOnTimingThread([this]() { m_Core.retro_reset(); });
}

static bool _qr_load_function(void *func_ptr, QRETRO_LIBRARY_T library, const char *name)
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

#define _qr_load_function_name(a) success &= _qr_load_function(&core->a, library, #a)
static bool _qr_load_library(retro_core_t *core, QRETRO_LIBRARY_T library)
{
  bool success = true;

  memset(core, 0, sizeof(retro_core_t));
  _qr_load_function_name(retro_api_version);
  _qr_load_function_name(retro_cheat_reset);
  _qr_load_function_name(retro_cheat_set);
  _qr_load_function_name(retro_deinit);
  _qr_load_function_name(retro_get_memory_data);
  _qr_load_function_name(retro_get_memory_size);
  _qr_load_function_name(retro_get_region);
  _qr_load_function_name(retro_get_system_av_info);
  _qr_load_function_name(retro_get_system_info);
  _qr_load_function_name(retro_init);
  _qr_load_function_name(retro_load_game);
  _qr_load_function_name(retro_load_game_special);
  _qr_load_function_name(retro_reset);
  _qr_load_function_name(retro_run);
  _qr_load_function_name(retro_serialize);
  _qr_load_function_name(retro_serialize_size);
  _qr_load_function_name(retro_set_audio_sample);
  _qr_load_function_name(retro_set_audio_sample_batch);
  _qr_load_function_name(retro_set_controller_port_device);
  _qr_load_function_name(retro_set_environment);
  _qr_load_function_name(retro_set_input_poll);
  _qr_load_function_name(retro_set_input_state);
  _qr_load_function_name(retro_set_video_refresh);
  _qr_load_function_name(retro_unload_game);
  _qr_load_function_name(retro_unserialize);

  if (success)
  {
    core->symbols_inited = true;
    return true;
  }
  return false;
}

QRetroConfig* QRetro::config()
{
  if (!m_Config)
    m_Config = new QRetroConfig(this);
  return m_Config;
}

QRetro::QRetro(QWindow *parent, retro_hw_context_type format)
{
  setParent(parent);

  initVideo(format);

  m_Location = new QRetroLocation(this);
  m_Message  = new QRetroMessage(this);

#if QRETRO_HAVE_SDL3
  m_InputBackend = new QRetroInputBackendSDL3(this);
#elif QRETRO_HAVE_GAMEPAD
  m_InputBackend = new QRetroInputBackendQGamepad(this);
#endif
  if (m_InputBackend)
  {
    m_InputBackend->init(m_Input.joypads(), m_Input.maxUsers());
    m_Input.setBackend(m_InputBackend);
    /* Keyboard macros actively clear buttons each frame when their key isn't
     * held, which overwrites state set by the hardware backend. Disable them
     * so the backend has full control; re-enable with setUseMaps(true). */
    m_Input.setUseMaps(false);
  }

  setLanguage(qt2lr_language_system());
  setPreferredRenderer(format);

  /* Initialize member variables */
  memset(&m_Core, 0, sizeof(m_Core));
  memset(m_SupportedEnvCallbacks, true, sizeof(m_SupportedEnvCallbacks));

  /* Apply persisted config settings immediately. */
  config();
}

void QRetro::unloadCore(void)
{
  if (m_Active)
  {
    m_Active = false;

    /* Unblock the timing thread if it is waiting on the frame semaphore */
    m_FramePresented.release();

    if (m_ThreadTiming)
    {
      m_ThreadTiming->wait();
      delete m_ThreadTiming;
      m_ThreadTiming = nullptr;
    }

    if (m_ThreadSaving)
    {
      m_ThreadSaving->wait();
      delete m_ThreadSaving;
      m_ThreadSaving = nullptr;
    }
  }

#if QRETRO_HAVE_OPENGL
  if (surfaceType() == QSurface::OpenGLSurface && m_OpenGlContextCore)
  {
    /* Move core's OpenGL context here to close */
    m_OpenGlContextCore->moveToThread(QCoreApplication::instance()->thread());

    if (m_OpenGlContextCore->thread() == QThread::currentThread())
    {
      m_OpenGlContextCore->makeCurrent(this);
      if (m_Core.hw_render.context_destroy)
        m_Core.hw_render.context_destroy();
      if (m_Core.inited)
      {
        m_Core.retro_unload_game();
        m_Core.retro_deinit();
        m_Core.inited = false;
      }
      m_OpenGlContextCore->doneCurrent();
    }
    else
    {
      qDebug() << QRETRO_ERROR("Skipping GL context cleanup: context not on main thread.");
      if (m_Core.inited)
      {
        m_Core.retro_unload_game();
        m_Core.retro_deinit();
        m_Core.inited = false;
      }
    }

    delete m_OpenGlContextCore;
    m_OpenGlContextCore = nullptr;
  }
  else
#endif
  if (m_Core.inited)
  {
    m_Core.retro_unload_game();
    m_Core.retro_deinit();
    m_Core.inited = false;
  }

  if (m_Library)
  {
#ifdef _WIN32
    FreeLibrary(m_Library);
#else
    dlclose(m_Library);
#endif
    m_Library = nullptr;
  }

  if (!m_CoreTempPath.isEmpty())
  {
    QFile::remove(m_CoreTempPath);
    m_CoreTempPath.clear();
  }
}

QRetro::~QRetro(void)
{
  unloadCore();

  /* Delete member variables on heap */
  delete m_Audio;
  delete m_Location;

  /* Remove the object from our static thread map */
  _qrdelete(this);
}

void QRetro::timing()
{
  QElapsedTimer frameTimer;
  frameTimer.start();

#if QRETRO_HAVE_OPENGL
  m_pfnSwapInterval    = nullptr;
  m_SwapIntervalFetched = false;
#endif

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

  if (m_Core.content_loaded && !m_Core.retro_load_game(&m_Core.game_info))
  {
    emit onCoreLog(RETRO_LOG_ERROR,
                   QRETRO_ERROR(QString("Function retro_load_game failed for "
                                "an unknown reason:\n%1").arg(
                                m_Core.game_info.path)));
    return;
  }
  else if (!m_Core.content_loaded && !m_SupportsNoGame)
  {
    emit onCoreLog(RETRO_LOG_ERROR,
                   QRETRO_ERROR("The core attempted to start without content, "
                                "but there is no hint that it doing so is "
                                "supported."));
    return;
  }

  /* retro_load_game has completed — core memory functions are now safe to call.
   * Signal the saving thread to proceed before the first retro_run. */
  m_SramReady = true;

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
  m_Audio->setEnabled(m_AudioEnabled);

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
    frameTimer.restart();

    {
      QMutexLocker lock(&m_PendingMutex);
      if (m_PendingAction)
      {
        m_PendingAction();
        m_PendingAction = nullptr;
        m_PendingDone.wakeAll();
      }
    }

    if (inputReady() && // stall if waiting for input (netplay)
        isVisible() && // stall if window is not available in context
        !m_Paused && // stall if content is paused
        !m_Audio->excessFramesInBuffer()) // stall to play the audio queue
    {
#if QRETRO_HAVE_OPENGL
      /* Ensure the core context is current on this thread before retro_run.
       * This prevents the main thread's m_OpenGlContext from stealing the surface. */
      if (surfaceType() == QSurface::OpenGLSurface && m_OpenGlContextCore)
      {
        m_OpenGlContextCore->makeCurrent(this);
        m_FboRequestedThisFrame = false;

        if (!m_SwapIntervalFetched)
        {
          m_SwapIntervalFetched = true;
          auto p = m_OpenGlContextCore->getProcAddress("glXSwapIntervalMESA");
          if (!p) p = m_OpenGlContextCore->getProcAddress("wglSwapIntervalEXT");
          m_pfnSwapInterval = reinterpret_cast<SwapIntervalFn>(p);
        }
      }
#endif

      /** @todo Properly measure frametime */
      if (m_Core.frame_time_callback.callback)
        m_Core.frame_time_callback.callback(m_Core.frame_time_callback.reference);

      //m_Camera.update();

      emit frameBegin();
      m_Core.retro_run();
      emit frameEnd();

#if QRETRO_HAVE_OPENGL
      if (surfaceType() == QSurface::OpenGLSurface && m_OpenGlContextCore && !m_Active)
      {
        m_OpenGlContextCore->doneCurrent();
        /* Move the context back to the main thread while we are still on
         * the timing thread (its current owner), so the main thread can
         * safely make it current and delete it without timer warnings. */
        m_OpenGlContextCore->moveToThread(QCoreApplication::instance()->thread());
      }
#endif

      m_Frames++;
    }

    m_Core.retro_get_system_av_info(&m_Core.av_info);
    updateScaling();

    if (m_Core.audio_callback.callback)
      m_Core.audio_callback.callback();
    m_Audio->playFrame();

#if QRETRO_HAVE_OPENGL
    /* At normal speed, swapBuffers in setImagePtr already provided vsync pacing — no sleep needed.
     * When fast-forwarding, vsync is disabled so fall through to the sleep
     * logic below which respects m_FastForwardRatio. */
    if (surfaceType() == QSurface::OpenGLSurface && m_OpenGlContextCore && !m_FastForwarding)
      continue;
#endif

    if (m_FastForwarding && m_FastForwardRatio <= 1.0f)
      continue; /* Unlimited fast-forward: run as fast as possible */

    {
      int ms = static_cast<int>(1000.0 / m_TargetRefreshRate);
      if (m_FastForwarding && m_FastForwardRatio > 1.0f)
        ms = static_cast<int>(ms / m_FastForwardRatio);

      if (surfaceType() == QSurface::RasterSurface)
      {
        int remaining = ms - static_cast<int>(frameTimer.elapsed());
        if (remaining > 0)
          QThread::msleep(remaining);
      }
      else
        m_FramePresented.tryAcquire(1, ms + 4);

      /* Pre-poll input after run */
      m_Input.poll();
      m_InputPrePolled = true;
    }
  }

  /* QAudioOutput is a QObject created on this thread, so it must be deleted
   * here rather than from the main thread to avoid timer warnings. */
  delete m_Audio;
  m_Audio = nullptr;
}

/* TODO: Support flags, test */
bool QRetro::getCurrentSoftwareFramebuffer(retro_framebuffer *fb)
{
  auto width = static_cast<int>(fb->width);
  auto height = static_cast<int>(fb->height);
  auto format = lr2qt_pixel(fb->format);

  /* Free our buffer if it's being reconstructed to a new size */
  if (m_SoftwareFramebuffer && (m_Image.width() != width ||
                                m_Image.height() != height ||
                                m_Image.format() != format))
  {
    free(m_SoftwareFramebuffer);
    m_SoftwareFramebuffer = nullptr;
  }

  if (!m_SoftwareFramebuffer)
  {
    /* Allocate buffer as black canvas */
    m_SoftwareFramebuffer = reinterpret_cast<uchar*>(
      calloc(fb->width * fb->height * fb->pitch, sizeof(uchar)));

    /* Update our renderable */
    m_Image = QImage(m_SoftwareFramebuffer, width, height, format);
  }

  /* Finalize data to send back to core */
  fb->data = m_SoftwareFramebuffer;
  fb->memory_flags = RETRO_MEMORY_TYPE_CACHED;

  return m_Image.bits();
}

void QRetro::setFastForwardingOverride(retro_fastforwarding_override* ffo)
{
  m_Core.fastforwarding_override = *ffo;

  setFastForwarding(ffo->fastforward);
  setFastForwardingRatio(ffo->ratio);
}

void QRetro::keyReleaseEvent(QKeyEvent *event)
{
  auto key = qt2lr_keyboard(event->key());

  m_Input.setKey(key, false);

  if (m_Core.keyboard.callback)
    m_Core.keyboard.callback(false,
                             key,
                             event->nativeVirtualKey(),
                             qt2lr_keymod(event->modifiers()));
}

void QRetro::keyPressEvent(QKeyEvent *event)
{
  auto key = qt2lr_keyboard(event->key());

  m_Input.setKey(key, true);

  if (m_Core.keyboard.callback)
    m_Core.keyboard.callback(true,
                             key,
                             event->nativeVirtualKey(),
                             qt2lr_keymod(event->modifiers()));

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
      reset();
      break;
    case Qt::Key_S:
      initVideo(RETRO_HW_CONTEXT_NONE);
      break;
    case Qt::Key_W:
      setFastForwarding(!m_FastForwarding);
      break;
    case Qt::Key_Z:
      serializeToFile(stateFilePath());
      break;
    case Qt::Key_X:
      unserializeFromFile(stateFilePath());
      break;
    case Qt::Key_T:
      setRotation(m_Rotation + 90);
      break;
    case Qt::Key_F1:
      options()->update();
      options()->show();
      break;
    case Qt::Key_F2:
      config()->show();
      config()->raise();
      config()->activateWindow();
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
  QByteArray hash;
  QCryptographicHash hasher(QCryptographicHash::Md5);
  QFile save_file(QString("%1/%2.sav").arg(
    m_Directories.get(QRetroDirectories::Save),
    QFileInfo(contentPath()).completeBaseName())
  );

  QByteArray initialBuffer;

  while (!m_Active);

  /* Read save file into buffer once */
  if (save_file.exists() && save_file.open(QIODevice::ReadOnly))
  {
    initialBuffer = save_file.readAll();
    save_file.close();
  }

  /* Wait until retro_load_game has completed (core memory functions are safe),
   * but before the first retro_run (so frame 1 sees the restored SRAM). */
  while (m_Active && !m_SramReady);

  /* For first 15 frames, continuously copy buffer into SRAM */
  while (m_Active && m_Frames <= 15)
  {
    void *data = m_Core.retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    size_t size = m_Core.retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);

    if (data && size && !initialBuffer.isEmpty())
    {
      size_t copySize = std::min<size_t>(size, static_cast<size_t>(initialBuffer.size()));
      memcpy(data, initialBuffer.constData(), copySize);

      hasher.reset();
      hasher.addData(reinterpret_cast<const char*>(data), static_cast<int>(copySize));
      hash = hasher.result();
    }
  }

  /* Normal autosave loop */
  while (m_Active)
  {
    /* Sleep in small increments so stopCore() can interrupt promptly */
    for (unsigned i = 0; i < m_AutosaveInterval * 10 && m_Active; i++)
      QThread::msleep(100);

    auto data = m_Core.retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    auto size = m_Core.retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);

    if (!data || !size)
      continue;

    hasher.reset();
    hasher.addData(reinterpret_cast<const char*>(data), static_cast<int>(size));
    auto result = hasher.result();

    if (result == hash)
      continue;

    hash = result;

    if (save_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
      save_file.write(static_cast<const char*>(data), static_cast<qint64>(size));
      save_file.close();
      emit onSave();
    }
  }
}

bool QRetro::startCore(void)
{
  if (!m_Core.symbols_inited)
  {
    emit onCoreLog(RETRO_LOG_ERROR,
      "The core attempted to start without first being initialized.");
    return false;
  }
  else
  {
    m_Frames = 0;
    m_SramReady = false;
    m_Active = true;

    /* Paint black immediately so the window doesn't show garbage
       from whatever is underneath while the core initialises. */
    if (size().isValid())
    {
      m_Image = QImage(size(), m_PixelFormat);
      m_Image.fill(Qt::black);
    }
    QMetaObject::invokeMethod(this, [this]() { requestUpdate(); }, Qt::QueuedConnection);

    m_ThreadSaving = QThread::create([this]{saving();});
    m_ThreadSaving->start();

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

  /**
   * Copy the core to a unique temp path before opening it, so multiple
   * instances of the same core can be loaded
   */
  QString ext = fileinfo.suffix();
  m_CoreTempPath = QString("%1/qretro_%2%3")
    .arg(QDir::tempPath(),
         QUuid::createUuid().toString(QUuid::Id128),
         ext.isEmpty() ? "" : "." + ext);

  if (!QFile::copy(path, m_CoreTempPath))
  {
    emit onCoreLog(RETRO_LOG_ERROR, QRETRO_ERROR(QString(
      "Failed to copy core to temp path:\n%1").arg(m_CoreTempPath)));
    m_CoreTempPath.clear();
    return false;
  }

#ifdef _WIN32
  m_Library = LoadLibraryW(reinterpret_cast<LPCWSTR>(m_CoreTempPath.utf16()));
#else
  m_Library = dlopen(m_CoreTempPath.toUtf8().constData(), RTLD_LAZY);
#endif

  if (!m_Library)
  {
    QFile::remove(m_CoreTempPath);
    m_CoreTempPath.clear();
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

  if (!_qr_load_library(&m_Core, m_Library))
    return false;

  m_Input.setControllerPortDevice(m_Core.retro_set_controller_port_device);

  m_CorePath = fileinfo.absolutePath().toStdString();

  return true;
}

bool QRetro::initVideo(retro_hw_context_type format)
{
  /* Discard any semaphore tokens accumulated under the previous backend.
   * Without this, switching from a raster backend (where tokens are not
   * consumed) to an OpenGL one would cause a burst of unthrottled frames. */
  while (m_FramePresented.tryAcquire()) {}

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

#if QRETRO_HAVE_OPENGL
  case RETRO_HW_CONTEXT_OPENGL:
  case RETRO_HW_CONTEXT_OPENGL_CORE:
  case RETRO_HW_CONTEXT_OPENGLES2:
  case RETRO_HW_CONTEXT_OPENGLES3:
  case RETRO_HW_CONTEXT_OPENGLES_VERSION:
    setSurfaceType(QSurface::OpenGLSurface);
    create();

    if (!m_OpenGlContext)
    {
      m_OpenGlContext = new QOpenGLContext(this);
      m_OpenGlContext->setFormat(requestedFormat());
      m_OpenGlContext->create();
    }

    break;
#endif

  default:
    printf("INIT VIDEO ERROR!");
    return false;
  }

  /* DirectConnection: setImagePtr must run on the timing thread so that
   * m_OpenGlContextCore (current only on that thread) is valid for the blit.
   * UniqueConnection: initVideo can be called multiple times (e.g. switching
   * video backends), so guard against duplicate connections that would cause
   * setImagePtr — and thus swapBuffers — to fire more than once per frame. */
  connect(this, SIGNAL(onVideoRefresh(const void*, unsigned, unsigned, unsigned)),
          this, SLOT(setImagePtr(const void*, unsigned, unsigned, unsigned)),
          static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection));

  return true;
}

void QRetro::setPixelFormat(retro_pixel_format format)
{
  m_RetroPixelFormat = format;
  m_PixelFormatSet = true;
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
  m_Mousewheel[0] = event->angleDelta().y() / 8;
  m_Mousewheel[1] = event->angleDelta().x() / 8;
}
