#include "QRetroCommon.h"
#include "QRetroEnvironment.h"

#include "libretro_retroarch.h"

#include <libretro.h>

#include <QApplication>
#include <QThread>

void core_audio_sample(int16_t left, int16_t right)
{
  auto _this = _qrthis();

  if (_this && _this->audio())
  {
    sample_t samples[2] = {left, right};
    _this->audio()->pushSamples(samples, 1);
  }
}

size_t core_audio_sample_batch(const int16_t *data, size_t frames)
{
  auto _this = _qrthis();

  if (_this && _this->audio())
    _this->audio()->pushSamples(data, frames);

  return frames;
}

static bool core_camera_start(void)
{
  auto _this = _qrthis();

  if (_this && _this->camera())
    return _this->camera()->start();
  else
    return false;
}

static void core_camera_stop(void)
{
  auto _this = _qrthis();

  if (_this && _this->camera())
    _this->camera()->stop();
}

static long long unsigned core_hw_get_current_framebuffer()
{
  auto _this = _qrthis();

  if (_this && _this->core()->hw_render.context_type)
    return _this->glGetCurrentFramebuffer();

  return 0;
}

static void* core_hw_get_proc_address(const char *sym)
{
  auto _this = _qrthis();

  if (_this)
    return _this->glGetProcAddress(QThread::currentThread(), sym);

  return nullptr;
}

void core_input_poll(void)
{
  auto _this = _qrthis();

  if (!_this)
    return;

  /* If a custom input poll handler has been installed, use that instead */
  if (_this->hasInputPollHandler())
  {
    _this->runInputPollHandler();
    return;
  }
  else if (_this->inputPrePolled())
    _this->clearInputPrePolled();
  else
    _this->input()->poll();

  _this->updateMouse();
}

int16_t core_input_state(unsigned port, unsigned device, unsigned index,
  unsigned id)
{
  auto _this = _qrthis();

  if (!_this)
    return 0;

  if (_this->hasInputStateHandler())
    return _this->runInputStateHandler(port, device, index, id);

  if (device == RETRO_DEVICE_JOYPAD || device == RETRO_DEVICE_ANALOG)
    return _this->input()->state(port, device, index, id);
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
      return _this->mousewheelV() > 0;
    case RETRO_DEVICE_ID_MOUSE_WHEELDOWN:
      return _this->mousewheelV() < 0;
    case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP:
      return _this->mousewheelH() > 0;
    case RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:
      return _this->mousewheelH() < 0;
    default:
      return 0;
    }
  }

  return 0;
}

/* TODO: Support distance interval */
static void core_location_set_interval(unsigned ms, unsigned dist)
{
  Q_UNUSED(dist)
  auto _this = _qrthis();

  if (_this && _this->location())
    _this->location()->setInterval(ms, dist);
}

static bool core_location_start(void)
{
  auto _this = _qrthis();

  if (_this && _this->location())
    return _this->location()->startUpdates();

  return false;
}

static void core_location_stop(void)
{
  auto _this = _qrthis();

  if (_this && _this->location())
    _this->location()->stopUpdates();
}

static bool core_location_get_position(double *lat, double *lon,
  double *horiz_accuracy, double *vert_accuracy)
{
  auto _this = _qrthis();

  if (_this && _this->location())
    return _this->location()->getPosition(lat, lon, horiz_accuracy, vert_accuracy);

  return false;
}

/* TODO: Stubbed. Do we need these? */
static void core_location_initialized(void)
{
}

static void core_location_deinitialized(void)
{
}

static void core_log(enum retro_log_level level, const char *fmt, ...)
{
  auto _this = _qrthis();
  char msg[1024];
  va_list args;
  QString final_string;

  if (!_this || level < _this->getLogLevel())
    return;

  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  final_string = QString(msg);

  _this->log()->push(level, final_string);
}

static bool core_rumble(unsigned int port, retro_rumble_effect effect,
  unsigned short strength)
{
  auto _this = _qrthis();

  if (!_this)
    return false;

  return _this->input()->setRumble(port, effect, strength);
}

static bool core_sensor_set_state(unsigned port, retro_sensor_action action,
  unsigned rate)
{
  bool input_ok, system_ok;
  auto _this = _qrthis();

  if (!_this)
    return false;

  /* Try initializing sensors on both controller backend and system level */
  input_ok = _this->input()->setSensorState(port, action, rate);
  system_ok = _this->sensors()->setState(port, action, rate);

  return input_ok || system_ok;
}

static float core_sensor_get_input(unsigned port, unsigned id)
{
  auto _this = _qrthis();

  if (!_this)
    return 0.0f;

  if (_this->input()->backendHandlesSensor(port, id))
    return _this->input()->getSensorInput(port, id);

  return _this->sensors()->getInput(port, id);
}

static void core_led_set_state(int led, int state)
{
  auto _this = _qrthis();

  if (_this)
    _this->led()->setState(led, state);
}

static void core_microphone_close_mic(retro_microphone_t *microphone)
{
  auto _this = _qrthis();

  if (_this)
    return _this->microphone()->close(microphone);
}

static bool core_microphone_get_mic_state(const retro_microphone_t *microphone)
{
  auto _this = _qrthis();

  if (!_this)
    return false;
  else
    return _this->microphone()->getState(microphone);
}

static bool core_microphone_set_mic_state(retro_microphone_t *microphone, bool state)
{
  auto _this = _qrthis();

  if (!_this)
    return false;
  else
    return _this->microphone()->setState(microphone, state);
}

static bool core_microphone_get_params(const retro_microphone_t *microphone,
                                       retro_microphone_params_t *params)
{
  auto _this = _qrthis();

  if (!_this)
    return false;
  else
    return _this->microphone()->getParams(microphone, params);
}

static retro_microphone_t* core_microphone_open_mic(const retro_microphone_params_t *params)
{
  auto _this = _qrthis();

  if (!_this)
    return nullptr;
  else
    return _this->microphone()->open(params);
}

static int core_microphone_read_mic(retro_microphone_t *microphone,
                                    int16_t *samples, size_t num_samples)
{
  auto _this = _qrthis();

  if (!_this)
    return 0;
  else
    return _this->microphone()->read(microphone, samples, num_samples);
}

static bool core_midi_input_enabled(void)
{
  auto _this = _qrthis();

  return _this && _this->midi()->inputEnabled();
}

static bool core_midi_output_enabled(void)
{
  auto _this = _qrthis();

  return _this && _this->midi()->outputEnabled();
}

static bool core_midi_read(uint8_t *byte)
{
  auto _this = _qrthis();

  return _this && _this->midi()->read(byte);
}

static bool core_midi_write(uint8_t byte, uint32_t delta_time)
{
  auto _this = _qrthis();

  return _this && _this->midi()->write(byte, delta_time);
}

static bool core_midi_flush()
{
  auto _this = _qrthis();

  return _this && _this->midi()->flush();
}

void core_video_refresh(const void *data, unsigned width,
  unsigned height, size_t pitch)
{
  auto _this = _qrthis();

  if (!_this)
    return;

  _this->setVideoSize(width, height);
  emit _this->onVideoRefresh(data, width, height, static_cast<unsigned>(pitch));
}

bool core_environment(unsigned cmd, void *data)
{
  auto _this = _qrthis();
  bool experimental = (cmd & RETRO_ENVIRONMENT_EXPERIMENTAL) ? true : false;
  bool privated = (cmd & RETRO_ENVIRONMENT_PRIVATE) ? true : false;
  bool retroarch = (cmd & RETRO_ENVIRONMENT_RETROARCH_START_BLOCK) ? true : false;
  unsigned cmd_noflags = cmd & 0xFF;
  unsigned cmd_dispatch = cmd & ~RETRO_ENVIRONMENT_PRIVATE;

  if (!_this)
  {
    qWarning("Environment callback %u called on a NULL QRetro instance.", cmd);
    return false;
  }

  /* The user can manually ignore certain environment callback IDs */
  if (!_this->environmentCallbackSupported(cmd_noflags))
    return false;

  switch (cmd_dispatch)
  {
  /// 01
  /// const unsigned*
  /// The core sets the desired screen rotation (0=0, 1=90, 2=180, 3=270)
  /// Returns true if the screen rotation was set successfully
  case RETRO_ENVIRONMENT_SET_ROTATION:
    if (data)
      _this->setRotation(*(reinterpret_cast<const unsigned*>(data)) * 90);
    else
      return false;
    break;

  /// 02 (deprecated)
  /// bool*
  /// The frontend indicates whether the core should use overscan
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_OVERSCAN:
    core_log(RETRO_LOG_WARN, "RETRO_ENVIRONMENT_GET_OVERSCAN is deprecated!");
    if (data)
      *(reinterpret_cast<bool*>(data)) = _this->getOverscan();
    break;

  /// 03
  /// bool*
  /// The frontend indicates whether it supports frame duplication (passing NULL to the video callback)
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    if (data)
      *(reinterpret_cast<bool*>(data)) = _this->supportsDuping();
    break;

  /// 04 (deprecated/reserved)
  /// nullptr
  /// Obsolete GET_VARIABLE; no longer used by any supported core
  /// Returns false (not supported)
  case 4:
    core_log(RETRO_LOG_WARN, "Old-style GET_VARIABLE is deprecated!");
    return false;

  /// 05 (deprecated/reserved)
  /// nullptr
  /// Obsolete SET_VARIABLES; no longer used by any supported core
  /// Returns false (not supported)
  case 5:
    core_log(RETRO_LOG_WARN, "Old-style SET_VARIABLES is deprecated!");
    return false;

  /// 06
  /// const struct retro_message*
  /// The core requests a user-facing message be displayed for a set number of frames
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_SET_MESSAGE:
  {
    auto *msg = reinterpret_cast<const struct retro_message*>(data);
    if (msg)
      _this->message()->push(QString::fromUtf8(msg->msg));
    break;
  }

  /// 07
  /// nullptr
  /// The core requests the frontend to shut it down
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_SHUTDOWN:
    _this->close();
    break;

  /// 08
  /// const unsigned*
  /// The core hints its relative performance demand level (higher = more demanding)
  /// Return value is undefined
  case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
    if (data)
      _this->setPerformanceLevel(*reinterpret_cast<const unsigned*>(data));
    break;

  /// 09
  /// const char**
  /// The frontend provides the path to its system/BIOS directory
  /// Returns true if available, even if the directory path is NULL
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    if (data)
      *reinterpret_cast<const char**>(data) =
        _this->directories()->get(QRetroDirectories::System);
    break;

  /// 10
  /// const enum retro_pixel_format*
  /// The core sets the pixel format used for video output frames
  /// Returns false if the requested format is not supported by the frontend
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    if (data)
      _this->setPixelFormat(*reinterpret_cast<enum retro_pixel_format*>(data));
    break;

  /// 11
  /// const struct retro_input_descriptor*
  /// The core provides human-readable labels for its input bindings to the frontend
  /// Returns true if the environment call is recognized
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    _this->input()->setInputDescriptors(
      reinterpret_cast<const retro_input_descriptor*>(data));
    break;

  /// 12
  /// const struct retro_keyboard_callback*
  /// The core registers a callback to receive raw keyboard events from the frontend
  /// Returns true if keyboard callback is supported
  case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
    if (data)
      _this->core()->keyboard =
        *reinterpret_cast<retro_keyboard_callback*>(data);
    break;

  /// 13 @todo
  /// const struct retro_disk_control_callback*
  /// The core provides an interface for the frontend to swap disk images at runtime
  /// Returns true if disk control is supported
  /// case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:

  /// 14
  /// struct retro_hw_render_callback*
  /// The core requests a hardware rendering context; the frontend fills in get_proc_address and get_current_framebuffer
  /// Returns false if data is NULL or the requested rendering API is not supported
  case RETRO_ENVIRONMENT_SET_HW_RENDER:
  {
    auto hw = reinterpret_cast<retro_hw_render_callback*>(data);

    if (!hw)
      return false;

    switch (hw->context_type)
    {
    case RETRO_HW_CONTEXT_NONE:
      _this->initVideo(hw->context_type);
      break;
    case RETRO_HW_CONTEXT_OPENGL:
    case RETRO_HW_CONTEXT_OPENGL_CORE:
    case RETRO_HW_CONTEXT_OPENGLES2:
    case RETRO_HW_CONTEXT_OPENGLES3:
    case RETRO_HW_CONTEXT_OPENGLES_VERSION:
      _this->initVideo(hw->context_type);

      hw->get_proc_address =
        reinterpret_cast<retro_hw_get_proc_address_t>(
        core_hw_get_proc_address);

      hw->get_current_framebuffer =
        reinterpret_cast<retro_hw_get_current_framebuffer_t>(
        core_hw_get_current_framebuffer);

      _this->core()->hw_render = *hw;
      break;
    default:
      core_log(RETRO_LOG_ERROR, "Requested unsupported renderer %u",
        hw->context_type);
      return false;
    }

    break;
  }

  /// 15
  /// struct retro_variable*
  /// The core retrieves the current value of a named core option; frontend fills in value field
  /// Returns true if the environment call is available, even if the data is invalid
  case RETRO_ENVIRONMENT_GET_VARIABLE:
  {
    auto var = reinterpret_cast<retro_variable*>(data);

    if (!var || !var->key)
      break;

    auto val = _this->options()->getOptionValue(var->key);

    if (!val)
      break;

    var->value = val;

    break;
  }

  /// 16
  /// const struct retro_variable*
  /// The core declares its available options and their possible values (deprecated v0 API)
  /// Returns true if the environment call is available, even if data is NULL
  case RETRO_ENVIRONMENT_SET_VARIABLES:
    if (data)
      _this->options()->setOptions(reinterpret_cast<retro_variable*>(data));
    break;

  /// 17
  /// bool*
  /// The frontend indicates whether any core option value has changed since the last GET_VARIABLE
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
    if (data)
      *reinterpret_cast<bool*>(data) = _this->options()->variablesUpdated();
    break;

  /// 18
  /// const bool*
  /// The core notifies the frontend that it supports being started without loading any content
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
    if (data)
      _this->setSupportsNoGame(*(reinterpret_cast<bool*>(data)));
    break;

  /// 19
  /// const char**
  /// The frontend provides the absolute path from which this core was loaded
  /// Returns true if the environment call is available; value may still be NULL
  case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
    if (data)
      *reinterpret_cast<const char**>(data) = _this->corePath();
    break;

  /// 20 (deprecated/unused)
  /// nullptr
  /// Obsolete SET_AUDIO_CALLBACK; no longer used by any supported core
  /// Returns false (not supported)
  case 20:
    core_log(RETRO_LOG_WARN, "Old-style SET_AUDIO_CALLBACK is deprecated!");
    return false;

  /// 21
  /// const struct retro_frame_time_callback*
  /// The core registers a callback notifying it of elapsed time since last retro_run iteration
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
  {
    auto cb = reinterpret_cast<retro_frame_time_callback*>(data);

    if (cb && cb->callback)
      _this->core()->frame_time_callback = *cb;

    break;
  }

  /// 22
  /// const struct retro_audio_callback*
  /// The core registers callbacks for when the frontend is ready for audio output
  /// Returns true if this environment call is available, even if data is NULL
  case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
  {
    auto cb = reinterpret_cast<retro_audio_callback*>(data);

    if (cb && cb->callback && cb->set_state)
      _this->core()->audio_callback = *cb;

    break;
  }

  /// 23
  /// struct retro_rumble_interface*
  /// The frontend provides an interface to access controller rumble motors
  /// Returns true if the environment call is available, even if the device doesn't support vibration
  case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
    if (data)
      reinterpret_cast<retro_rumble_interface*>(data)->set_rumble_state = core_rumble;
    break;

  /// 24
  /// uint64_t*
  /// The frontend provides a bitmask of supported input device types
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
    if (data)
      *reinterpret_cast<uint64_t*>(data) = _this->input()->deviceCapabilities();
    break;

  /// 25
  /// struct retro_sensor_interface*
  /// The frontend provides an interface to access and configure available sensors (accelerometer, gyroscope)
  /// Returns true if the environment call is available, even if the device has no supported sensors
  case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE:
  {
    auto cb = reinterpret_cast<retro_sensor_interface*>(data);

    if (!cb)
      return false;

    cb->set_sensor_state = core_sensor_set_state;
    cb->get_sensor_input = core_sensor_get_input;

    break;
  }

  /// 26
  /// struct retro_camera_callback*
  /// The frontend provides an interface to the device's video camera
  /// Returns true if this environment call is available, even if an actual camera isn't
  case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE:
  {
    auto cb = reinterpret_cast<retro_camera_callback*>(data);

    if (cb)
    {
      cb->start = core_camera_start;
      cb->stop = core_camera_stop;
      _this->camera()->init(cb);
    }

    break;
  }

  /// 27
  /// struct retro_log_callback*
  /// The frontend provides an interface for cross-platform logging
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
  {
    auto cb = reinterpret_cast<retro_log_callback*>(data);

    if (!cb)
      return false;

    cb->log = core_log;
    break;
  }

  /// 28 @todo
  /// struct retro_perf_callback*
  /// The frontend provides an interface for profiling code and accessing CPU performance counters
  /// Returns true if the environment call is available
  /// case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:

  /// 29
  /// struct retro_location_callback*
  /// The frontend provides an interface to retrieve the device's geographic location
  /// Returns true if the environment call is available, even if there's no location information available
  case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE:
  {
    auto location = reinterpret_cast<retro_location_callback*>(data);

    if (!location)
      return false;

    location->set_interval = core_location_set_interval;
    location->start = core_location_start;
    location->stop = core_location_stop;
    location->get_position = core_location_get_position;
    location->initialized = core_location_initialized;
    location->deinitialized = core_location_deinitialized;

    break;
  }

  /// 30
  /// const char**
  /// The frontend provides the path to its "core assets" directory for art assets or level data
  /// Returns true if the environment call is available, even if the value returned is NULL
  case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
    if (data)
      *reinterpret_cast<const char**>(data) =
        _this->directories()->get(QRetroDirectories::CoreAssets);
    break;

  /// 31
  /// const char**
  /// The frontend provides the path to its save data directory for game-specific save data
  /// Returns true if the environment call is available, even if the value returned is NULL
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    if (data)
      *reinterpret_cast<const char**>(data) =
        _this->directories()->get(QRetroDirectories::Save);
    break;

  /// 32
  /// const struct retro_system_av_info*
  /// The core provides new video and audio parameters, allowing the frontend to reinitialize A/V without unloading the core
  /// Returns true if the environment call is available and the new av_info struct was accepted
  case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
  {
    auto info = reinterpret_cast<const retro_system_av_info*>(data);

    if (info)
      _this->setAvInfo(info);
    break;
  }

  /// 33
  /// const struct retro_get_proc_address_interface*
  /// The core provides an interface for the frontend to obtain function pointers by name
  /// Returns true if the environment call is available and the interface was accepted
  case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
    if (data)
      _this->procAddress()->init(
        reinterpret_cast<const struct retro_get_proc_address_interface*>(data));
    break;

  /// 34 @todo
  /// const struct retro_subsystem_info*
  /// The core declares support for subsystems (secondary platforms, e.g. Super Game Boy)
  /// Returns true if this environment call is available
  // case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
  //   break;

  /// 35
  /// const struct retro_controller_info*
  /// The core declares which types of controllers it supports per port
  /// Returns true if this environment call is available
  case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
    if (data)
      _this->input()->setControllerInfo(
        reinterpret_cast<const retro_controller_info*>(data));
    break;

  /// 36
  /// const struct retro_memory_map*
  /// The core provides a description of the address spaces used by the emulated hardware
  /// Returns true if this environment call is available
  case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
    _this->setMemoryMaps(reinterpret_cast<const retro_memory_map*>(data));
    break;

  /// 37
  /// const struct retro_game_geometry*
  /// The core requests a viewport resize without reinitializing the video driver
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_SET_GEOMETRY:
  {
    auto geo = reinterpret_cast<const retro_game_geometry*>(data);

    if (geo)
      _this->setGeometry(geo->base_width, geo->base_height);
    break;
  }

  /// 38
  /// const char**
  /// The frontend provides the username of the user running the frontend
  /// Returns true if the environment call is available, even if the frontend couldn't provide a name
  case RETRO_ENVIRONMENT_GET_USERNAME:
    if (data)
      *reinterpret_cast<const char**>(data) = _this->username()->get();
    break;

  /// 39
  /// retro_language*
  /// The frontend provides its configured display language
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_LANGUAGE:
    if (data)
      *reinterpret_cast<unsigned*>(data) = _this->getLanguage();
    break;

  /// 40
  /// struct retro_framebuffer*
  /// The frontend provides a frontend-managed framebuffer for the core to render into directly
  /// Returns true if the environment call was recognized and the framebuffer was successfully returned
  case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
    return _this->getCurrentSoftwareFramebuffer(
      reinterpret_cast<retro_framebuffer*>(data));

  /// 41 @todo
  /// const struct retro_hw_render_interface*
  /// The frontend provides a hardware rendering API interface specific to the context type in use
  /// Returns true if the environment call is available and the interface is supported
  // case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:

  /// 42
  /// const bool*
  /// The core declares whether it supports achievements
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
    if (data)
      _this->setSupportsAchievements(*reinterpret_cast<bool*>(data));
    break;

  /// 43 @todo
  /// const struct retro_hw_render_context_negotiation_interface*
  /// The core provides a context negotiation interface for the hardware renderer (e.g. Vulkan)
  /// Returns true if the interface type is supported
  //case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
  //{
  //  auto hw = reinterpret_cast<const struct retro_hw_render_context_negotiation_interface_vulkan*>(data);
  //  Q_UNUSED(hw)
    //if (hw->interface_type != RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN)
  //    return false;
  //}

  /// 44 @todo / There is an API clash here. Special casing is needed until it's fixed.
  /// uint64_t*
  /// The core declares quirks associated with its serialization (save state) behavior
  /// Returns true if this environment call is supported
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
  case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
    if (!experimental)
      _this->setSerializationQuirks(reinterpret_cast<uint64_t*>(data));
    else
      return false;
    break;

  /// 45 @todo
  /// const struct retro_vfs_interface_info*
  /// The frontend provides a virtual filesystem interface for file I/O abstraction
  /// Returns true if the environment call is available and the requested VFS version is supported
  /// case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:

  /// 46
  /// struct retro_led_interface*
  /// The frontend provides an interface to set the state of accessible device LEDs
  /// Returns true if the environment call is available, even if data is NULL or no LEDs are accessible
  case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
    if (data)
      reinterpret_cast<retro_led_interface*>(data)->set_led_state = core_led_set_state;
    break;

  /// 47
  /// retro_av_enable_flags*
  /// The frontend provides hints about which A/V processing steps the core may skip for this frame
  /// Returns true if the environment call is available, regardless of the value output to data
  case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
    if (data)
      *(reinterpret_cast<int*>(data)) = _this->audioVideoEnable()->getFlags();
    break;

  /// 48
  /// struct retro_midi_interface*
  /// The frontend provides an interface for raw MIDI I/O
  /// Returns true if the environment call is available, even if data is NULL
  case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE:
  {
    auto midi = reinterpret_cast<retro_midi_interface*>(data);

    if (midi)
    {
      midi->input_enabled = core_midi_input_enabled;
      midi->output_enabled = core_midi_output_enabled;
      midi->read = core_midi_read;
      midi->write = core_midi_write;
      midi->flush = core_midi_flush;
    }

    break;
  }

  /// 49
  /// bool*
  /// The frontend provides whether it is currently running in fast-forward mode
  /// Returns true if this environment call is available, regardless of the value returned in data
  case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
    if (data)
      *(reinterpret_cast<bool*>(data)) = _this->fastForwarding();
    break;

  /// 50
  /// float*
  /// The frontend provides the refresh rate it is targeting, in Hz
  /// Returns true if this environment call is available, regardless of the value returned in data
  case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
    if (data)
      *(reinterpret_cast<float*>(data)) =
        static_cast<float>(_this->targetRefreshRate());
    break;

  /// 51
  /// bool* (ignored)
  /// The core queries whether the frontend can return all digital joypad button states as a bitmask
  /// Returns true if the frontend can report the complete digital joypad state as a bitmask
  case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
    if (data)
      *(reinterpret_cast<bool*>(data)) = _this->input()->supportsBitmasks();
    return _this->input()->supportsBitmasks();

  /// 52
  /// unsigned*
  /// The frontend provides the version of the core options API it supports
  /// Returns true if the environment call is available; false otherwise
  case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
    if (data)
      *(reinterpret_cast<unsigned*>(data)) = _this->options()->maxVersion();
    break;

  /// 53
  /// const struct retro_core_option_definition*
  /// The core provides its option definitions using the v1 interface
  /// Returns true if this environment call is available
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
    _this->options()->setOptions(
      reinterpret_cast<retro_core_option_definition**>(&data));
    break;

  /// 54
  /// const struct retro_core_options_intl*
  /// The core provides its option definitions with internationalization support (v1 intl interface)
  /// Return value is undefined
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
    _this->options()->setOptions(
      reinterpret_cast<retro_core_options_intl*>(data));
    break;

  /// 55
  /// const struct retro_core_option_display*
  /// The core requests a named core option be shown or hidden in the frontend's UI
  /// Returns true if this environment call is available, even if data is NULL or the option doesn't exist
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
  {
    auto var = reinterpret_cast<retro_core_option_display*>(data);

    if (var)
      _this->options()->setVisibility(var->key, var->visible);
    break;
  }

  /// 56
  /// retro_hw_context_type*
  /// The frontend provides its preferred hardware rendering API
  /// Returns true if the environment call is available and the frontend can use a hardware rendering API
  case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
    if (data)
      *reinterpret_cast<unsigned*>(data) = _this->getPreferredRenderer();
    break;

  /// 57 @todo
  /// unsigned*
  /// The frontend provides the version of the disk control interface it supports
  /// Returns true if this environment call is available
  // case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:

  /// 58 @todo
  /// const struct retro_disk_control_ext_callback*
  /// The core provides an extended interface for swapping disk images at runtime
  /// Returns true if disk control is supported
  // case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:

  /// 59
  /// unsigned*
  /// The frontend provides the version of the message interface it supports
  /// Returns true if this environment call is available
  case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
    if (data)
      *reinterpret_cast<unsigned*>(data) = _this->message()->interfaceVersion();
    break;

  /// 60
  /// const struct retro_message_ext*
  /// The core requests the frontend display a user-facing message with extended display options
  /// Returns true if this environment call is available
  case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
    if (data)
      _this->message()->push(reinterpret_cast<const struct retro_message_ext*>(data));
    break;

  /// 61
  /// unsigned*
  /// The frontend provides the number of active input devices it currently supports
  /// Returns true if this environment call is available
  case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
    if (data)
      *reinterpret_cast<unsigned*>(data) = _this->input()->maxUsers();
    break;

  /// 62 @todo
  /// const struct retro_audio_buffer_status_callback*
  /// The core registers a callback to receive audio buffer occupancy information from the frontend
  /// Returns true if this environment call is available
  // case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:

  /// 63 @todo
  /// const unsigned*
  /// The core requests a minimum audio latency in milliseconds
  /// Returns true if this environment call is available
  // case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:

  /// 64
  /// const struct retro_fastforwarding_override*
  /// The core provides parameters controlling when and how fast-forward mode may be activated
  /// Returns true if this environment call is available, even if data is NULL
  case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE:
    if (data)
      _this->setFastForwardingOverride(
        reinterpret_cast<retro_fastforwarding_override*>(data));
    break;

  /// 65 @todo
  /// const struct retro_system_content_info_override*
  /// The core overrides how the frontend loads its content (e.g. persistent data, full path required)
  /// Returns true if this environment call is available
  // case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE:

  /// 66 @todo
  /// const struct retro_game_info_ext**
  /// The frontend provides extended information about the loaded content
  /// Returns true if this environment call is available
  // case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:

  /// 67
  /// const struct retro_core_options_v2*
  /// The core provides its option definitions with category support using the v2 interface
  /// Returns true if this environment call is available and the frontend supports categories
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
    if (data)
      _this->options()->setOptions(
        reinterpret_cast<retro_core_options_v2*>(data));
    break;

  /// 68
  /// const struct retro_core_options_v2_intl*
  /// The core provides its option definitions with category support and internationalization (v2 intl interface)
  /// Return value is undefined
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
    if (data)
      _this->options()->setOptions(
        reinterpret_cast<retro_core_options_v2_intl*>(data));
    break;

  /// 69
  /// const struct retro_core_options_update_display_callback*
  /// The core registers a callback to be called when option visibility should be updated
  /// Returns true if this environment call is available, even if data is NULL
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK:
  {
    auto cb = reinterpret_cast<retro_core_options_update_display_callback*>(data);
    if (cb)
      _this->options()->setUpdateDisplayCallback(cb->callback);
    break;
  }

  /// 70
  /// const struct retro_variable*
  /// The core forcibly sets a core option's value, overriding any user-configured setting
  /// Returns true if this environment call is available and the option was successfully set
  case RETRO_ENVIRONMENT_SET_VARIABLE:
  {
    if (data)
    {
      auto var = reinterpret_cast<retro_variable*>(data);
      _this->options()->setOptionValue(var->key, var->value);
    }
    break;
  }

  /// 71 @todo
  /// struct retro_throttle_state*
  /// The frontend provides a struct for reading throttle state
  /// Returns whether or not the environment call is available
  /// case RETRO_ENVIRONMENT_GET_THROTTLE_STATE:

  /// 72
  /// case RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT:

  /// 73 @todo
  /// case RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT:

  /// 74
  /// bool*
  /// The frontend provides whether JIT is available
  /// Returns whether or not the environment call is available
  case RETRO_ENVIRONMENT_GET_JIT_CAPABLE:
    if (data)
      *reinterpret_cast<bool*>(data) = _this->jitCapable();
    break;

  /// 75
  /// retro_microphone_interface*
  /// The frontend provides an interface for the core to receive microphone input
  /// Returns true if microphone support is available, even if no microphones are plugged in; false if disabled or data is NULL
  case RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE:
  {
    if (!data)
      return false;
    else
    {
      auto cb = reinterpret_cast<retro_microphone_interface*>(data);

      cb->interface_version = _this->microphone()->interfaceVersion();
      cb->close_mic = core_microphone_close_mic;
      cb->get_mic_state = core_microphone_get_mic_state;
      cb->set_mic_state = core_microphone_set_mic_state;
      cb->get_params = core_microphone_get_params;
      cb->open_mic = core_microphone_open_mic;
      cb->read_mic = core_microphone_read_mic;
    }
    break;
  }

  /// 76 (deprecated)
  /// nullptr
  /// Deprecated SET_NETPACKET_INTERFACE; cores using this must migrate to callback 78
  /// Returns false (not supported)
  case 76:
    core_log(RETRO_LOG_WARN, "Old-style SET_NETPACKET_INTERFACE is deprecated!");
    return false;

  /// 77
  /// struct retro_device_power*
  /// The frontend provides the device's current power state (e.g. battery level and charging status)
  /// Returns true if the environment call is available, even if data is NULL
  case RETRO_ENVIRONMENT_GET_DEVICE_POWER:
    if (data)
      return _this->devicePower()->get(
        reinterpret_cast<retro_device_power*>(data));
    break;

  /// 78 @todo
  /// const struct retro_netpacket_callback*
  /// The core provides callbacks for sending and receiving raw network packets (netplay)
  /// Returns true if this environment call is available
  /// case RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE:

  /// 79
  /// const char**
  /// The frontend provides the path to its "playlist" directory for core-generated playlists
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY:
    if (data)
      *reinterpret_cast<const char**>(data) =
        _this->directories()->get(QRetroDirectories::Playlist);
    break;
  
  /// 80
  /// const char**
  /// The frontend provides the starting directory path for its file browser
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY:
    if (data)
      *reinterpret_cast<const char**>(data) =
        _this->directories()->get(QRetroDirectories::FileBrowserStart);
    break;

  /// 81
  /// unsigned*
  /// The frontend provides the target audio sample rate it prefers the core to output
  /// Returns true if the environment call is available
  case RETRO_ENVIRONMENT_GET_TARGET_SAMPLE_RATE:
    if (data)
      *reinterpret_cast<unsigned*>(data) =
        _this->audio()->targetSampleRate();
    break;

  /// 82
  /// unsigned*
  /// The frontend provides the client index for the current netplay session
  /// Returns true if the environment call is available
  /// case RETRO_ENVIRONMENT_GET_NETPLAY_CLIENT_INDEX:

  /// RA 2
  /// case RETRO_ENVIRONMENT_SET_SAVE_STATE_IN_BACKGROUND:

  /// RA 3
  /// case RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB:

  /// RA 4
  /// case RETRO_ENVIRONMENT_POLL_TYPE_OVERRIDE:

  /// RA 5
  /// case RETRO_ENVIRONMENT_SET_SAVE_STATE_DISABLE_UNDO:

  default:
    qWarning("Unimplemented environment callback %u (%08X)%s%s%s%s.",
      cmd_noflags, cmd,
      cmd_noflags >= RETRO_ENVIRONMENT_SIZE ? " (above maximum)" : "",
      experimental ? " (experimental)" : "",
      privated ? " (private)" : "",
      retroarch ? " (RetroArch)" : "");
    return false;
  }

  return true;
}
