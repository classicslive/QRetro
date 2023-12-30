#ifndef QRETRO_INPUT_H
#define QRETRO_INPUT_H

#if QRETRO_HAVE_GAMEPAD
#include <QGamepad>
#endif

#include <vector>

#include <libretro.h>

#define QRETRO_INPUT_DEFAULT_MAX_JOYPADS 16
#define QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE 0

typedef struct
{
  int port;
  int keys[8];
  int buttons[8];
} qretro_input_kb_map_t;

class QRetroInputJoypad
{
public:
  enum InputMethod
  {
    InputMethodNone = 0,

    InputMethodGamepad,
    InputMethodKeyboard,

    InputMethodSize
  };

  int16_t analogButton(unsigned id);
  void setAnalogButton(unsigned id, int16_t value);

  int16_t analogStick(unsigned index, unsigned id);
  int16_t bitmask(void) { return m_Bitmask; }

  bool digitalButton(unsigned id);
  void setDigitalButton(unsigned id, bool value);

  unsigned inputMethods(void) { return m_InputMethods; }
  void setInputMethods(unsigned method);

  unsigned port(void) { return m_Port; }
  void setPort(unsigned port) { m_Port = port; }

  void poll(void);

#if QRETRO_HAVE_GAMEPAD
  QGamepad *gamepad() { return &m_Gamepad; }
  void setGamepadPort(unsigned port) { m_Gamepad.setDeviceId(port); }
#endif

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  int16_t m_Bitmask;
  int16_t m_Buttons[RETRO_DEVICE_ID_JOYPAD_R3 + 1];
  unsigned m_InputMethods = InputMethodGamepad | InputMethodKeyboard;
  unsigned m_Port = 0;
  int16_t m_Sticks[2][2];
  bool m_SupportsBitmasks = true;
#if QRETRO_HAVE_GAMEPAD
  QGamepad m_Gamepad;
#endif
};

class QRetroInput
{
public:
  QRetroInput(void);

  void poll(void);

  int16_t state(unsigned port, unsigned device, unsigned index, unsigned id);

  int16_t analogButtonDeadzone(void) { return m_AnalogButtonDeadzone; }

  void setAnalogButtonDeadzone(int16_t dz) { m_AnalogButtonDeadzone = dz; }

  bool key(retro_key key) { return m_Keys[key]; }

  void setKey(retro_key key, bool down) { m_Keys[key] = down; }

  /**
   * The number of joypads that will be polled and can have their state read.
   */
  unsigned maxUsers(void) { return m_MaxUsers; }

  /**
   * Sets the number of joypads that will be polled and can have their state read.
   */
  void setMaxUsers(unsigned max) { m_MaxUsers = max; }

  /**
   * Whether or not the frontend is reporting to support input bitmasks.
   */
  bool supportsBitmasks(void) { return m_SupportsBitmasks; }

  /**
   * Sets whether or not the frontend reports to support input bitmasks.
   * QRetro's default input implementation does, but if this is manually set to
   * false, it will pretend not to.
   */
  void setSupportsBitmasks(bool supports) { m_SupportsBitmasks = supports; }

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  QRetroInputJoypad m_Joypads[QRETRO_INPUT_DEFAULT_MAX_JOYPADS];
  std::vector<qretro_input_kb_map_t> m_KeyboardMaps;
  bool m_Keys[RETROK_LAST] = { false };
  unsigned m_MaxUsers = QRETRO_INPUT_DEFAULT_MAX_JOYPADS;
  bool m_SupportsBitmasks = true;
};

#endif
