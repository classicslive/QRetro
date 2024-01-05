#include <QRetroCommon.h>
#include <QRetroInput.h>

void QRetroInputJoypad::poll(void)
{
  uint16_t bitmask = 0;

  /* Update the input bitmask, even if reporting not to support it. */
  for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
    bitmask |= m_Buttons[i] ? (1 << i) : 0;
  m_Bitmask = static_cast<int16_t>(bitmask);
}

int16_t QRetroInputJoypad::analogButton(unsigned id)
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return 0;
  else
    return m_Buttons[id];
}

void QRetroInputJoypad::setAnalogButton(unsigned id, int16_t value)
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return;
  else
    m_Buttons[id] = value;
}

bool QRetroInputJoypad::digitalButton(unsigned id)
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return 0;
  else if (m_AnalogStickToDigitalPad)
  {
    switch (id)
    {
    case RETRO_DEVICE_ID_JOYPAD_UP:
      return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y]
        > m_AnalogStickDeadzone;
    case RETRO_DEVICE_ID_JOYPAD_DOWN:
      return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y]
        < -m_AnalogStickDeadzone;
    case RETRO_DEVICE_ID_JOYPAD_LEFT:
      return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X]
        > m_AnalogStickDeadzone;
    case RETRO_DEVICE_ID_JOYPAD_RIGHT:
      return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X]
        < -m_AnalogStickDeadzone;
    }
  }

  return m_Buttons[id] > m_AnalogButtonDeadzone;
}

void QRetroInputJoypad::setDigitalButton(unsigned id, bool value)
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return;
  else
    m_Buttons[id] = value;
}

int16_t QRetroInputJoypad::analogStick(unsigned index, unsigned id)
{
  if (index > RETRO_DEVICE_INDEX_ANALOG_RIGHT || id > RETRO_DEVICE_ID_ANALOG_Y)
    return 0;
  else
    return m_Sticks[index][id];
}

void QRetroInputJoypad::setAnalogStick(unsigned index, unsigned id,
                                       int16_t value)
{
  if (index > RETRO_DEVICE_INDEX_ANALOG_RIGHT || id > RETRO_DEVICE_ID_ANALOG_Y)
    return;
  else
    m_Sticks[index][id] = value;
}

#define QRETRO_DEFAULT_MAP_JOYPAD(a, b) \
  m_KeyboardMaps.push_back( \
  { \
    { a, -1 }, \
    { \
      { 0, RETRO_DEVICE_JOYPAD, 0, b, true }, \
      { -1, 0, 0, 0, 0 } \
    } \
  })

#define QRETRO_DEFAULT_MAP_ANALOG(a, b, c, d) \
  m_KeyboardMaps.push_back( \
  { \
    { a, -1 }, \
    { \
      { 0, RETRO_DEVICE_ANALOG, b, c, d }, \
      { -1, 0, 0, 0, 0 } \
    } \
  })

#define QRETRO_CONNECT_DIGITAL(a, b) \
  connect(m_Joypads[i].gamepad(), \
          &a, \
          this, \
          [this, i](bool pressed) \
          { m_Joypads[i].setDigitalButton(b, pressed); })

#define QRETRO_CONNECT_STICK(a, b, c) \
  connect(m_Joypads[i].gamepad(), \
          &a, \
          this, \
          [this, i](double value) \
          { m_Joypads[i].setAnalogStick(b, c, qt2lr_analog(value)); })

#define QRETRO_CONNECT_TRIGGER(a, b) \
  connect(m_Joypads[i].gamepad(), \
          &a, \
          this, \
          [this, i](double value) \
          { m_Joypads[i].setAnalogButton(b, qt2lr_analog(value)); })

QRetroInput::QRetroInput(QObject *parent)
{
  setParent(parent);

#if QRETRO_HAVE_GAMEPAD
  auto gamepads = QGamepadManager::instance()->connectedGamepads();
#endif

  for (unsigned i = 0; i < m_MaxUsers; i++)
  {
    m_Joypads[i].setPort(i);

#if QRETRO_HAVE_GAMEPAD
#if _WIN32
    /* On Windows, just allow xinput controllers to populate at any time */
    m_Joypads[i].setGamepadPort(i);
#else
    /* On Linux, use the actual IDs of connected gamepads */
    if (gamepads.size() <= static_cast<int>(i))
      break;
    m_Joypads[i].setGamepadPort(gamepads.at(static_cast<int>(i)));
#endif
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonUpChanged, RETRO_DEVICE_ID_JOYPAD_UP);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonDownChanged, RETRO_DEVICE_ID_JOYPAD_DOWN);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonLeftChanged, RETRO_DEVICE_ID_JOYPAD_LEFT);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonRightChanged, RETRO_DEVICE_ID_JOYPAD_RIGHT);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonAChanged, RETRO_DEVICE_ID_JOYPAD_B);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonBChanged, RETRO_DEVICE_ID_JOYPAD_A);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonXChanged, RETRO_DEVICE_ID_JOYPAD_Y);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonYChanged, RETRO_DEVICE_ID_JOYPAD_X);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonL1Changed, RETRO_DEVICE_ID_JOYPAD_L);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonL3Changed, RETRO_DEVICE_ID_JOYPAD_L3);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonR1Changed, RETRO_DEVICE_ID_JOYPAD_R);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonR3Changed, RETRO_DEVICE_ID_JOYPAD_R3);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonStartChanged, RETRO_DEVICE_ID_JOYPAD_START);
    QRETRO_CONNECT_DIGITAL(QGamepad::buttonSelectChanged, RETRO_DEVICE_ID_JOYPAD_SELECT);

    QRETRO_CONNECT_STICK(QGamepad::axisLeftXChanged, 0, 0);
    QRETRO_CONNECT_STICK(QGamepad::axisLeftYChanged, 0, 1);
    QRETRO_CONNECT_STICK(QGamepad::axisRightXChanged, 1, 0);
    QRETRO_CONNECT_STICK(QGamepad::axisRightYChanged, 1, 1);

    QRETRO_CONNECT_TRIGGER(QGamepad::buttonL2Changed, RETRO_DEVICE_ID_JOYPAD_L2);
    QRETRO_CONNECT_TRIGGER(QGamepad::buttonR2Changed, RETRO_DEVICE_ID_JOYPAD_R2);
#endif
  }

  /* Setup default P1 keyboard controls */
  /* Digital buttons */
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_UP, RETRO_DEVICE_ID_JOYPAD_UP);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_z, RETRO_DEVICE_ID_JOYPAD_B);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_x, RETRO_DEVICE_ID_JOYPAD_A);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_a, RETRO_DEVICE_ID_JOYPAD_Y);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_s, RETRO_DEVICE_ID_JOYPAD_X);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_SPACE, RETRO_DEVICE_ID_JOYPAD_SELECT);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_RETURN, RETRO_DEVICE_ID_JOYPAD_START);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_q, RETRO_DEVICE_ID_JOYPAD_L);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_w, RETRO_DEVICE_ID_JOYPAD_R);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_t, RETRO_DEVICE_ID_JOYPAD_L3);
  QRETRO_DEFAULT_MAP_JOYPAD(RETROK_y, RETRO_DEVICE_ID_JOYPAD_R3);

  /* Analog sticks */
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_UP, 0, RETRO_DEVICE_ID_ANALOG_Y, -32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_DOWN, 0, RETRO_DEVICE_ID_ANALOG_Y, 32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_LEFT, 0, RETRO_DEVICE_ID_ANALOG_X, -32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_RIGHT, 0, RETRO_DEVICE_ID_ANALOG_X, 32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_i, 1, RETRO_DEVICE_ID_ANALOG_Y, -32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_k, 1, RETRO_DEVICE_ID_ANALOG_Y, 32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_j, 1, RETRO_DEVICE_ID_ANALOG_X, -32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_l, 1, RETRO_DEVICE_ID_ANALOG_X, 32767);

  /* Analog triggers; full and half press */
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_e, 2, RETRO_DEVICE_ID_JOYPAD_L2, 32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_r, 2, RETRO_DEVICE_ID_JOYPAD_R2, 32767);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_e, 2, RETRO_DEVICE_ID_JOYPAD_L2, 32767/2);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_r, 2, RETRO_DEVICE_ID_JOYPAD_R2, 32767/2);
}

void QRetroInput::poll(void)
{
  for (unsigned i = 0; i < m_MaxUsers; i++)
    m_Joypads[i].poll();

  /* Check keyboard macros */
  if (!m_UseMaps)
    return;
  else for (const auto& map : m_KeyboardMaps)
  {
    auto button = &map.buttons[0];
    auto key = &map.keys[0];
    bool pressed = true;

    for (unsigned i = 0; i < QRETRO_INPUT_MAPPING_MAX; i++)
    {
      if (*key <= RETROK_UNKNOWN)
        break;
      pressed &= m_Keys[*key];
      key++;
    }

    for (unsigned i = 0; i < QRETRO_INPUT_MAPPING_MAX; i++)
    {
      if (button->port < 0)
        break;
      switch (button->device)
      {
      case RETRO_DEVICE_JOYPAD:
        m_Joypads[button->port].setDigitalButton(button->id,
                                                 pressed ? button->value : !button->value);
        break;
      case RETRO_DEVICE_ANALOG:
        if (button->index != RETRO_DEVICE_INDEX_ANALOG_BUTTON)
        {
          if (pressed)
            m_Joypads[button->port].setAnalogStick(button->index, button->id, static_cast<int16_t>(button->value));
          else if (m_Joypads[button->port].analogStick(button->index, button->id) == button->value)
            m_Joypads[button->port].setAnalogStick(button->index, button->id, 0);
        }
        else
          m_Joypads[button->port].setAnalogButton(button->id,
                                                  pressed ? button->value : 0);
        break;
      }
      button++;
    }
  }
}

int16_t QRetroInput::state(unsigned port, unsigned device, unsigned index,
                           unsigned id)
{
  if (port < m_MaxUsers)
  {
    switch (device)
    {
    case RETRO_DEVICE_JOYPAD:
      if (id == RETRO_DEVICE_ID_JOYPAD_MASK)
        return m_SupportsBitmasks ? m_Joypads[port].bitmask() : 0;
      else
        return m_Joypads[port].digitalButton(id);
    case RETRO_DEVICE_ANALOG:
      if (index == RETRO_DEVICE_INDEX_ANALOG_BUTTON)
        return m_Joypads[port].analogButton(id);
      else
        return m_Joypads[port].analogStick(index, id);
    }
  }

  return 0;
}
