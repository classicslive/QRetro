#ifndef QRETRO_INPUT_H
#define QRETRO_INPUT_H

#if QRETRO_HAVE_GAMEPAD
#include <QGamepad>
#else
#include <QObject>
#endif

#include <vector>

#include <libretro.h>

#define QRETRO_INPUT_DEFAULT_MAX_JOYPADS 16
#define QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE 0

/// The maximum number of keys that can be combined in one mapping, and the
/// number of buttons one macro can press
#define QRETRO_INPUT_MAPPING_MAX 8

typedef struct
{
  int port;
  unsigned device, index, id;
  int value;
} qretro_input_t;

typedef struct
{
  int keys[QRETRO_INPUT_MAPPING_MAX];
  qretro_input_t buttons[QRETRO_INPUT_MAPPING_MAX];
} qretro_input_kb_map_t;

class QRetroInputJoypad : public QObject
{
  Q_OBJECT

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
  void setAnalogStick(unsigned index, unsigned id, int16_t value);

  bool analogStickToDigitalPad(void) { return m_AnalogStickToDigitalPad; }
  void setAnalogStickToDigitalPad(bool on) { m_AnalogStickToDigitalPad = on; }

  int16_t bitmask(void) { return m_Bitmask; }

  bool digitalButton(unsigned id);
  void setDigitalButton(unsigned id, bool value);

  bool digitalPadToAnalogStick(void) { return m_DigitalPadToAnalogStick; }
  void setDigitalPadToAnalogStick(bool on) { m_DigitalPadToAnalogStick = on; }

  unsigned inputMethods(void) { return m_InputMethods; }
  void setInputMethods(unsigned method);

  unsigned port(void) { return m_Port; }
  void setPort(unsigned port) { m_Port = port; }

  void poll(void);

#if QRETRO_HAVE_GAMEPAD
  QGamepad *gamepad() { return &m_Gamepad; }
  void setGamepadPort(int port) { m_Gamepad.setDeviceId(port); }
#endif

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  int16_t m_AnalogStickDeadzone = 0;
  bool m_AnalogStickToDigitalPad = false;
  int16_t m_Bitmask;
  int16_t m_Buttons[RETRO_DEVICE_ID_JOYPAD_R3 + 1];
  bool m_DigitalPadToAnalogStick = false;
  unsigned m_InputMethods = InputMethodGamepad | InputMethodKeyboard;
  unsigned m_Port = 0;
  int16_t m_Sticks[2][2];
  bool m_SupportsBitmasks = true;
#if QRETRO_HAVE_GAMEPAD
  QGamepad m_Gamepad;
#endif
};

class QRetroInput : public QObject
{
  Q_OBJECT

public:
  QRetroInput(QObject *parent = nullptr);

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

  bool useMaps(void) { return m_UseMaps; }
  void setUseMaps(bool use) { m_UseMaps = use; }

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  QRetroInputJoypad m_Joypads[QRETRO_INPUT_DEFAULT_MAX_JOYPADS];
  std::vector<qretro_input_kb_map_t> m_KeyboardMaps;
  bool m_Keys[RETROK_LAST] = { false };
  unsigned m_MaxUsers = QRETRO_INPUT_DEFAULT_MAX_JOYPADS;
  bool m_SupportsBitmasks = true;
#if QRETRO_HAVE_GAMEPAD
  bool m_UseMaps = false;
#else
  bool m_UseMaps = true;
#endif
};

#endif
