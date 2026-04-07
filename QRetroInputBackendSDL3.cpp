#include "QRetroInputBackendSDL3.h"

#if QRETRO_HAVE_SDL3

QRetroInputBackendSDL3::QRetroInputBackendSDL3(QObject *parent)
  : QRetroInputBackend(parent)
{
  if (!SDL_WasInit(SDL_INIT_GAMEPAD))
    SDL_InitSubSystem(SDL_INIT_GAMEPAD);
}

QRetroInputBackendSDL3::~QRetroInputBackendSDL3()
{
  unsigned i;

  for (i = 0; i < m_MaxUsers; i++)
  {
    if (m_Gamepads[i])
    {
      SDL_CloseGamepad(m_Gamepads[i]);
      m_Gamepads[i] = nullptr;
    }
  }
  SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

void QRetroInputBackendSDL3::init(QRetroInputJoypad *joypads, unsigned maxUsers)
{
  m_Joypads = joypads;
  m_MaxUsers = maxUsers;

  int count = 0;
  SDL_JoystickID *ids = SDL_GetGamepads(&count);
  if (ids)
  {
    for (int i = 0; i < count && static_cast<unsigned>(i) < m_MaxUsers; i++)
      openGamepad(static_cast<unsigned>(i), ids[i]);
    SDL_free(ids);
  }
}

void QRetroInputBackendSDL3::poll()
{
  // Pump the event queue so SDL's internal state is up-to-date.
  SDL_PumpEvents();

  // Drain gamepad add/remove events without consuming window/keyboard events.
  // SDL_EVENT_GAMEPAD_ADDED and SDL_EVENT_GAMEPAD_REMOVED are not adjacent in
  // SDL3, so drain each type separately.
  SDL_Event event;

  while (
    SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_GAMEPAD_ADDED, SDL_EVENT_GAMEPAD_ADDED) > 0)
  {
    SDL_JoystickID id = event.gdevice.which;

    // Ignore stale add-events for gamepads already opened during init().
    bool already_open = false;
    for (unsigned port = 0; port < m_MaxUsers; port++)
    {
      if (m_InstanceIds[port] == id)
      {
        already_open = true;
        break;
      }
    }
    if (already_open)
      continue;

    for (unsigned port = 0; port < m_MaxUsers; port++)
    {
      if (!m_Gamepads[port])
      {
        openGamepad(port, id);
        emit gamepadConnected(port);
        break;
      }
    }
  }

  while (SDL_PeepEvents(
           &event, 1, SDL_GETEVENT, SDL_EVENT_GAMEPAD_REMOVED, SDL_EVENT_GAMEPAD_REMOVED) > 0)
    closeGamepad(event.gdevice.which);

  // Read direct state for each open gamepad.
  for (unsigned port = 0; port < m_MaxUsers; port++)
  {
    if (m_Gamepads[port])
      updatePort(port);
  }
}

void QRetroInputBackendSDL3::updatePort(unsigned port)
{
  SDL_Gamepad *gp = m_Gamepads[port];
  QRetroInputJoypad *jp = &m_Joypads[port];

  if (jp->accelEnabled())
    SDL_SetGamepadSensorEnabled(gp, SDL_SENSOR_ACCEL, true);
  if (jp->gyroEnabled())
    SDL_SetGamepadSensorEnabled(gp, SDL_SENSOR_GYRO, true);

#define SDL_BTN(lr_id, sdl_btn)                                                                    \
  jp->setAnalogButton((lr_id), SDL_GetGamepadButton(gp, (sdl_btn)) ? 32767 : 0)

  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_B, SDL_GAMEPAD_BUTTON_SOUTH);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_A, SDL_GAMEPAD_BUTTON_EAST);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_Y, SDL_GAMEPAD_BUTTON_WEST);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_X, SDL_GAMEPAD_BUTTON_NORTH);

  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_UP);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_DOWN, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_LEFT, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_RIGHT, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_SELECT, SDL_GAMEPAD_BUTTON_BACK);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_START, SDL_GAMEPAD_BUTTON_START);

  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_L, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_R, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);

  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_L3, SDL_GAMEPAD_BUTTON_LEFT_STICK);
  SDL_BTN(RETRO_DEVICE_ID_JOYPAD_R3, SDL_GAMEPAD_BUTTON_RIGHT_STICK);

#undef SDL_BTN

  /* Sticks */
  jp->setAnalogStick(RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X,
    sdl2lr_axis(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_LEFTX)));
  jp->setAnalogStick(RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y,
    sdl2lr_axis(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_LEFTY)));
  jp->setAnalogStick(RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X,
    sdl2lr_axis(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_RIGHTX)));
  jp->setAnalogStick(RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y,
    sdl2lr_axis(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_RIGHTY)));

  /* Triggers */
  jp->setAnalogButton(RETRO_DEVICE_ID_JOYPAD_L2,
    sdl2lr_trigger(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_LEFT_TRIGGER)));
  jp->setAnalogButton(RETRO_DEVICE_ID_JOYPAD_R2,
    sdl2lr_trigger(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)));

  /* Sensors */
  float accel[3] = {}, gyro[3] = {};
  SDL_GetGamepadSensorData(gp, SDL_SENSOR_ACCEL, accel, 3);
  SDL_GetGamepadSensorData(gp, SDL_SENSOR_GYRO, gyro, 3);
  jp->setAccelX(accel[0]);
  jp->setAccelY(accel[1]);
  jp->setAccelZ(accel[2]);
  jp->setGyroX(gyro[0]);
  jp->setGyroY(gyro[1]);
  jp->setGyroZ(gyro[2]);
}

bool QRetroInputBackendSDL3::setRumble(unsigned port, retro_rumble_effect effect, uint16_t strength)
{
  Q_UNUSED(effect)
  Q_UNUSED(strength)

  if (port >= m_MaxUsers || !m_Gamepads[port])
    return false;
  else
  {
    QRetroInputJoypad *jp = &m_Joypads[port];
    /* Use a duration of 0ms to turn off, or a short window */
    Uint32 duration = (jp->rumbleStrong() || jp->rumbleWeak()) ? 5000 : 0;

    return SDL_RumbleGamepad(m_Gamepads[port], jp->rumbleStrong(), jp->rumbleWeak(), duration);
  }
}

bool QRetroInputBackendSDL3::setSensorState(
  unsigned port, retro_sensor_action action, unsigned rate)
{
  Q_UNUSED(rate)
  if (port >= m_MaxUsers || !m_Gamepads[port])
    return false;
  else
  {
    SDL_Gamepad *gp = m_Gamepads[port];

    switch (action)
    {
    case RETRO_SENSOR_ACCELEROMETER_ENABLE:
    case RETRO_SENSOR_ACCELEROMETER_DISABLE:
      if (!SDL_GamepadHasSensor(gp, SDL_SENSOR_ACCEL))
        return false;
      else
        return SDL_SetGamepadSensorEnabled(
          gp, SDL_SENSOR_ACCEL, action == RETRO_SENSOR_ACCELEROMETER_ENABLE);
    case RETRO_SENSOR_GYROSCOPE_ENABLE:
    case RETRO_SENSOR_GYROSCOPE_DISABLE:
      if (!SDL_GamepadHasSensor(gp, SDL_SENSOR_GYRO))
        return false;
      else
        return SDL_SetGamepadSensorEnabled(
          gp, SDL_SENSOR_GYRO, action == RETRO_SENSOR_GYROSCOPE_ENABLE);
    case RETRO_SENSOR_ILLUMINANCE_ENABLE:
    case RETRO_SENSOR_ILLUMINANCE_DISABLE:
    case RETRO_SENSOR_DUMMY:
      return false;
    }
  }

  return false;
}

bool QRetroInputBackendSDL3::sensorActive(unsigned port, unsigned id)
{
  if (port >= m_MaxUsers || !m_Gamepads[port])
    return false;

  if (id <= RETRO_SENSOR_ACCELEROMETER_Z)
    return m_Joypads[port].accelEnabled();

  if (id >= RETRO_SENSOR_GYROSCOPE_X && id <= RETRO_SENSOR_GYROSCOPE_Z)
    return m_Joypads[port].gyroEnabled();

  return false;
}

void QRetroInputBackendSDL3::openGamepad(unsigned port, SDL_JoystickID instanceId)
{
  if (port >= m_MaxUsers)
    return;

  if (m_Gamepads[port])
  {
    SDL_CloseGamepad(m_Gamepads[port]);
    m_Gamepads[port] = nullptr;
    m_InstanceIds[port] = 0;
  }

  SDL_Gamepad *gp = SDL_OpenGamepad(instanceId);
  if (!gp)
    return;

  m_Gamepads[port] = gp;
  m_InstanceIds[port] = instanceId;
}

void QRetroInputBackendSDL3::closeGamepad(SDL_JoystickID instanceId)
{
  for (unsigned port = 0; port < m_MaxUsers; port++)
  {
    if (m_InstanceIds[port] == instanceId)
    {
      if (m_Gamepads[port])
      {
        SDL_CloseGamepad(m_Gamepads[port]);
        m_Gamepads[port] = nullptr;
      }
      m_InstanceIds[port] = 0;
      m_Joypads[port].setRumbleStrong(0);
      m_Joypads[port].setRumbleWeak(0);
      emit gamepadDisconnected(port);
      return;
    }
  }
}

#endif
