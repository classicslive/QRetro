#include <QRetroCommon.h>
#include <QRetroInput.h>

void QRetroInputJoypad::poll(void)
{
  uint16_t bitmask = 0;

  /* Reset all button values */
  //memset(m_Buttons, 0, sizeof(m_Buttons));

#if QRETRO_HAVE_GAMEPAD
  if (m_InputMethods & InputMethodGamepad && m_Gamepad.isConnected())
  {
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_A] |= m_Gamepad.buttonB();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_B] |= m_Gamepad.buttonA();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_X] |= m_Gamepad.buttonY();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_Y] |= m_Gamepad.buttonX();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_START] |= m_Gamepad.buttonStart();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_SELECT] |= m_Gamepad.buttonSelect();

    m_Buttons[RETRO_DEVICE_ID_JOYPAD_L] |= m_Gamepad.buttonL1();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_R] |= m_Gamepad.buttonR1();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_L2] =
      qt2lr_analog(m_Gamepad.buttonL2());
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_R2] =
      qt2lr_analog(m_Gamepad.buttonR2());
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_L3] |= m_Gamepad.buttonL3();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_R3] |= m_Gamepad.buttonR3();

    m_Buttons[RETRO_DEVICE_ID_JOYPAD_UP] |= m_Gamepad.buttonUp();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_DOWN] |= m_Gamepad.buttonDown();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_LEFT] |= m_Gamepad.buttonLeft();
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_RIGHT] |= m_Gamepad.buttonRight();

    m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X] =
      qt2lr_analog(m_Gamepad.axisLeftX());
    m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y] =
      qt2lr_analog(m_Gamepad.axisLeftY());
    m_Sticks[RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_X] =
      qt2lr_analog(m_Gamepad.axisRightX());
    m_Sticks[RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_Y] =
      qt2lr_analog(m_Gamepad.axisRightY());
  }
#endif

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

QRetroInput::QRetroInput(void)
{
  for (unsigned i = 0; i < m_MaxUsers; i++)
  {
    m_Joypads[i].setPort(i);
#if QRETRO_HAVE_GAMEPAD
    m_Joypads[i].setGamepadPort(i);
#endif

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
}

void QRetroInput::poll(void)
{
  for (unsigned i = 0; i < m_MaxUsers; i++)
    m_Joypads[i].poll();

  /* Check keyboard macros */
  for (const auto& map : m_KeyboardMaps)
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
