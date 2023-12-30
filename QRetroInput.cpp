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
  else
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

QRetroInput::QRetroInput(void)
{
  for (unsigned i = 0; i < m_MaxUsers; i++)
  {
    m_Joypads[i].setPort(i);
#if QRETRO_HAVE_GAMEPAD
    m_Joypads[i].setGamepadPort(i);
#endif

    /* Setup default P1 keyboard controls */
    m_KeyboardMaps.push_back({ 0, { RETROK_UP, -1 }, { RETRO_DEVICE_ID_JOYPAD_UP, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_DOWN, -1 }, { RETRO_DEVICE_ID_JOYPAD_DOWN, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_LEFT, -1 }, { RETRO_DEVICE_ID_JOYPAD_LEFT, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_RIGHT, -1 }, { RETRO_DEVICE_ID_JOYPAD_RIGHT, -1 } });

    m_KeyboardMaps.push_back({ 0, { RETROK_q, -1 }, { RETRO_DEVICE_ID_JOYPAD_L, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_w, -1 }, { RETRO_DEVICE_ID_JOYPAD_R, -1 } });

    m_KeyboardMaps.push_back({ 0, { RETROK_z, -1 }, { RETRO_DEVICE_ID_JOYPAD_B, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_x, -1 }, { RETRO_DEVICE_ID_JOYPAD_A, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_a, -1 }, { RETRO_DEVICE_ID_JOYPAD_Y, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_s, -1 }, { RETRO_DEVICE_ID_JOYPAD_X, -1 } });

    m_KeyboardMaps.push_back({ 0, { RETROK_RSHIFT, -1 }, { RETRO_DEVICE_ID_JOYPAD_SELECT, -1 } });
    m_KeyboardMaps.push_back({ 0, { RETROK_RETURN, -1 }, { RETRO_DEVICE_ID_JOYPAD_START, -1 } });
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

    for (unsigned i = 0; i < sizeof(map.keys) / sizeof(retro_key); i++)
    {
      if (*key <= RETROK_UNKNOWN)
        break;
      pressed &= m_Keys[*key];
      key++;
    }

    for (unsigned i = 0; i < sizeof(map.buttons) / sizeof(int); i++)
    {
      if (*button < 0)
        break;
      m_Joypads[map.port].setDigitalButton(static_cast<unsigned>(*button),
                                           pressed);
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
