#include "QRetroCommon.h"
#include "QRetroEnvironment.h"

#include <libretro.h>
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
    return _this->getCurrentFramebuffer();

  return 0;
}

static void* core_hw_get_proc_address(const char *sym)
{
  auto _this = _qrthis();

  if (_this)
    return _this->getProcAddress(QThread::currentThread(), sym);

  return nullptr;
}

/* TODO: Support distance interval */
static void core_location_set_interval(unsigned ms, unsigned dist)
{
  Q_UNUSED(dist)
  auto _this = _qrthis();

  if (_this && _this->location())
    _this->location()->setUpdateInterval(static_cast<int>(ms));
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

  emit _this->onCoreLog(level, final_string);
}

/* TODO */
static bool core_rumble(unsigned int port, retro_rumble_effect effect,
  unsigned short strength)
{
  Q_UNUSED(port) Q_UNUSED(effect) Q_UNUSED(strength)
  return true;
}

static bool core_sensor_set_state(unsigned port, retro_sensor_action action,
  unsigned rate)
{
  auto _this = _qrthis();

  if (!_this)
    return false;
  else
    return _this->sensors()->setState(port, action, rate);
}

static float core_sensor_get_input(unsigned port, unsigned id)
{
  auto _this = _qrthis();

  if (!_this)
    return false;
  else
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

  emit _this->onVideoRefresh(data, width, height, static_cast<unsigned>(pitch));
}

bool core_environment(unsigned cmd, void *data)
{
  auto _this = _qrthis();
  bool experimental = cmd & RETRO_ENVIRONMENT_EXPERIMENTAL;
  unsigned cmd_noflags = cmd & 0xFF;

  if (!_this)
  {
    qWarning("Environment callback %u called on a NULL QRetro instance.", cmd);
    return false;
  }

  /* The user can manually ignore certain environment callback IDs */
  if (!_this->environmentCallbackSupported(cmd_noflags))
    return false;

  switch (cmd)
  {
  /* 01 */
  case RETRO_ENVIRONMENT_SET_ROTATION:
    if (data)
      _this->setRotation(*(reinterpret_cast<const unsigned*>(data)) * 90);
    else
      return false;
    break;

  /* 02 / Deprecated */
  case RETRO_ENVIRONMENT_GET_OVERSCAN:
    core_log(RETRO_LOG_WARN, "RETRO_ENVIRONMENT_GET_OVERSCAN is deprecated!");
    if (data)
      *(reinterpret_cast<bool*>(data)) = _this->getOverscan();
    else
      return false;
    break;

  /* 03 */
  case RETRO_ENVIRONMENT_GET_CAN_DUPE:
    if (data)
      *(reinterpret_cast<bool*>(data)) = _this->supportsDuping();
    break;

  /* Callbacks 04 - 05 are deprecated, and their IDs are reserved. */
  case 4:
    core_log(RETRO_LOG_WARN, "Old-style GET_VARIABLE is deprecated!");
    return false;

  case 5:
    core_log(RETRO_LOG_WARN, "Old-style SET_VARIABLES is deprecated!");
    return false;

  /* 06 */
  case RETRO_ENVIRONMENT_SET_MESSAGE:
    emit _this->onCoreMessage(reinterpret_cast<const char*>(data));
    break;

  /* 07 / TODO: Test */
  case RETRO_ENVIRONMENT_SHUTDOWN:
    _this->close();
    break;

  /* 08 */
  case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
    _this->setPerformanceLevel(*reinterpret_cast<const unsigned*>(data));
    break;

  /* 09 */
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    *reinterpret_cast<const char**>(data) =
      _this->directories()->get(QRetroDirectories::System);
    break;

  /* 10 */
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    _this->setPixelFormat(
      *reinterpret_cast<enum retro_pixel_format*>(data));
    break;

  /* 11 / TODO */
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
  {
    auto desc = reinterpret_cast<const retro_input_descriptor*>(data);

    while (desc->description)
    {
      printf("%s: %u %u %u %u\n", desc->description, desc->port, desc->device, desc->index, desc->id);
      desc++;
    }
    break;
  }

  /* 12 */
  case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
    _this->core()->keyboard =
      *reinterpret_cast<retro_keyboard_callback*>(data);
    break;

  /* 13
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: */

  /* 14 / TODO: Vulkan, GLES testing */
  case RETRO_ENVIRONMENT_SET_HW_RENDER:
  {
    auto hw = reinterpret_cast<retro_hw_render_callback*>(data);

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

  /* 15 */
  case RETRO_ENVIRONMENT_GET_VARIABLE:
  {
    auto var = reinterpret_cast<retro_variable*>(data);

    if (!var || !var->key)
      return false;

    auto val = _this->options()->getOptionValue(var->key);

    if (!val)
      return false;

    var->value = val;

    break;
  }

  /* 16 */
  case RETRO_ENVIRONMENT_SET_VARIABLES:
    if (data)
      _this->options()->setOptions(reinterpret_cast<retro_variable*>(data));
    break;

  /* 17 / TODO: Test */
  case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
    if (data)
      *reinterpret_cast<bool*>(data) = _this->options()->variablesUpdated();
    break;

  /* 18 */
  case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
    if (data)
      _this->setSupportsNoGame(*(reinterpret_cast<bool*>(data)));
    break;

  /* 19 */
  case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
    if (data)
      *reinterpret_cast<const char**>(data) = _this->corePath();
    break;

  /* Callback 20 is deprecated/unused. */
  case 20:
    core_log(RETRO_LOG_WARN, "Old-style SET_AUDIO_CALLBACK is deprecated!");
    return false;

  /* 21 */
  case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
  {
    auto cb = reinterpret_cast<retro_frame_time_callback*>(data);

    if (cb && cb->callback)
      _this->core()->frame_time_callback = *cb;

    break;
  }

  /* 22 */
  case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
  {
    auto cb = reinterpret_cast<retro_audio_callback*>(data);

    if (cb && cb->callback && cb->set_state)
      _this->core()->audio_callback = *cb;

    break;
  }

  /* 23 */
  case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
  {
    auto cb = reinterpret_cast<retro_rumble_interface*>(data);

    cb->set_rumble_state = core_rumble;
    break;
  }

  /* 25 */
  case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE:
  {
    auto cb = reinterpret_cast<retro_sensor_interface*>(data);

    cb->set_sensor_state = core_sensor_set_state;
    cb->get_sensor_input = core_sensor_get_input;

    break;
  }

  /* 26 */
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

  /* 27 */
  case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
  {
    auto cb = reinterpret_cast<retro_log_callback*>(data);

    cb->log = core_log;
    break;
  }

  /* 28 / TODO
  case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
  {
    auto cb = reinterpret_cast<retro_perf_callback*>(data);
  }
  */

  /* 29 */
  case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE:
  {
    auto location = reinterpret_cast<retro_location_callback*>(data);

    location->set_interval = core_location_set_interval;
    location->start = core_location_start;
    location->stop = core_location_stop;
    location->get_position = core_location_get_position;
    location->initialized = core_location_initialized;
    location->deinitialized = core_location_deinitialized;

    break;
  }

  /* 30 */
  case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
    *reinterpret_cast<const char**>(data) =
      _this->directories()->get(QRetroDirectories::CoreAssets);
    break;

  /* 31 */
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    *reinterpret_cast<const char**>(data) =
      _this->directories()->get(QRetroDirectories::Save);
    break;

  /* 32 / TODO
  case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
    break;
  */

  /* 33 / TODO: Test */
  case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
    _this->core()->get_proc_address_interface =
      *reinterpret_cast<retro_get_proc_address_interface*>(data);
    break;

  /* 34 / TODO
  case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
    break;
  */

  /* 35 / TODO */
  case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
  {
    auto info = reinterpret_cast<const retro_controller_info*>(data);

    for (unsigned i = 0; i < info->num_types; i++)
      printf("%0X4: %s\n", info->types[i].id, info->types[i].desc);

    break;
  }

  /* 36 */
  case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
    _this->setMemoryMaps(reinterpret_cast<const retro_memory_map*>(data));
    break;

  /* 37 */
  case RETRO_ENVIRONMENT_SET_GEOMETRY:
  {
    auto geo = reinterpret_cast<const retro_game_geometry*>(data);

    if (geo)
      _this->setGeometry(geo->base_width, geo->base_height);
    break;
  }

  /* 38 */
  case RETRO_ENVIRONMENT_GET_USERNAME:
    *reinterpret_cast<const char**>(data) = _this->username()->get();
    break;

  /* 39 */
  case RETRO_ENVIRONMENT_GET_LANGUAGE:
    *reinterpret_cast<unsigned*>(data) = _this->getLanguage();
    break;

  /* 40 */
  case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
    return _this->getCurrentSoftwareFramebuffer(
      reinterpret_cast<retro_framebuffer*>(data));

  /* 41 / TODO */
  // case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:

  /* 42 */
  case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:
    _this->setSupportsAchievements(*reinterpret_cast<bool*>(data));
    break;

  /* 43 / TODO */
  //case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
  //{
  //  auto hw = reinterpret_cast<const struct retro_hw_render_context_negotiation_interface_vulkan*>(data);
  //  Q_UNUSED(hw)
    //if (hw->interface_type != RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN)
  //    return false;
  //}

  /* 44 / TODO / There is an API clash here. Special casing is needed until
   * it's fixed. */
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
  case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
    if (!experimental)
      _this->setSerializationQuirks(reinterpret_cast<uint64_t*>(data));
    else
      return false;
    break;

  /* 46 */
  case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
  {
    auto led = reinterpret_cast<retro_led_interface*>(data);

    if (led)
      led->set_led_state = core_led_set_state;

    break;
  }

  /* 47 */
  case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
    *(reinterpret_cast<int*>(data)) = _this->audioVideoEnable()->getFlags();
    break;

  /* 48 */
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

  /* 49 */
  case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
    *(reinterpret_cast<bool*>(data)) = _this->fastForwarding();
    break;

  /* 50 */
  case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
    *(reinterpret_cast<float*>(data)) =
      static_cast<float>(_this->targetRefreshRate());
    break;

  /* 51 */
  case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
    if (data)
      *(reinterpret_cast<bool*>(data)) = _this->input()->supportsBitmasks();
    return _this->input()->supportsBitmasks();

  /* 52 */
  case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
    *(reinterpret_cast<unsigned*>(data)) = _this->options()->maxVersion();
    break;

  /* 53 */
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
    _this->options()->setOptions(
      reinterpret_cast<retro_core_option_definition**>(&data));
    break;

  /* 54 */
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
    _this->options()->setOptions(
      reinterpret_cast<retro_core_options_intl*>(data));
    break;

  /* 55 */
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
  {
    auto var = reinterpret_cast<retro_core_option_display*>(data);

    _this->options()->setVisibility(var->key, var->visible);
    break;
  }

  /* 56 */
  case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
    *reinterpret_cast<unsigned*>(data) = _this->getPreferredRenderer();
    break;

  /* 61 */
  case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
    if (data)
      *reinterpret_cast<unsigned*>(data) = _this->input()->maxUsers();
    break;

  /* 64 */
  case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE:
    if (data)
      _this->setFastForwardingOverride(
        reinterpret_cast<retro_fastforwarding_override*>(data));
    break;

  /* 70 */
  case RETRO_ENVIRONMENT_SET_VARIABLE:
  {
    if (data)
    {
      auto var = reinterpret_cast<retro_variable*>(data);
      _this->options()->setOptionValue(var->key, var->value);
    }
    break;
  }

  /* 74 */
  case RETRO_ENVIRONMENT_GET_JIT_CAPABLE:
    if (!data)
      return false;
    else
      *reinterpret_cast<bool*>(data) = _this->jitCapable();
    break;

  /* 75 */
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

  /* Callback 76 is deprecated */
  case 76:
    core_log(RETRO_LOG_WARN, "Old-style SET_NETPACKET_INTERFACE is deprecated!");
    return false;

  /* 77 */
  case RETRO_ENVIRONMENT_GET_DEVICE_POWER:
    return _this->devicePower()->get(reinterpret_cast<retro_device_power*>(data));

  default:
    qWarning("Unimplemented environment callback %u%s%s.",
      cmd_noflags,
      cmd_noflags >= RETRO_ENVIRONMENT_SIZE ? " (above maximum)" : "",
      experimental ? " (experimental)" : "");
    return false;
  }

  return true;
}
