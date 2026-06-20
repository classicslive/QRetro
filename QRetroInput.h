#ifndef QRETRO_INPUT_H
#define QRETRO_INPUT_H

#include <QObject>
#include <vector>
#include <string>
#include <libretro.h>

#include "QRetroInputBackend.h"

struct QRetroControllerType
{
  std::string desc;
  unsigned id;
};

struct QRetroInputDescriptor
{
  unsigned port;
  unsigned device;
  unsigned index;
  unsigned id;
  std::string description;
};

struct QRetroControllerPort
{
  std::vector<QRetroControllerType> types;
  unsigned selectedId = RETRO_DEVICE_JOYPAD;
};

#define QRETRO_INPUT_DEFAULT_MAX_JOYPADS 16
#define QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE 0
#define QRETRO_INPUT_DEFAULT_STICK_DEADZONE 4096
#define QRETRO_INPUT_DEFAULT_ANALOG_DIGITAL_THRESHOLD 0.5f

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
  enum
  {
    InputMethodNone = 0,

    InputMethodGamepad,
    InputMethodKeyboard,

    InputMethodSize
  } InputMethod;

  enum SensorState
  {
    SensorUninitialized = 0,
    SensorEnabled,
    SensorDisabled,

    SensorState_Size
  };

  int16_t analogButton(unsigned id);
  void setAnalogButton(unsigned id, int16_t value);

  int16_t analogStick(unsigned index, unsigned id);
  void setAnalogStick(unsigned index, unsigned id, int16_t value);

  bool analogStickToDigitalPad(void);
  void setAnalogStickToDigitalPad(bool on);

  float analogDigitalThreshold(void);
  void setAnalogDigitalThreshold(float t);

  int16_t analogStickDeadzone(void);
  void setAnalogStickDeadzone(int16_t dz);

  int16_t bitmask(void);

  bool digitalButton(unsigned id);
  void setDigitalButton(unsigned id, bool value);

  bool turbo(unsigned id) const;
  void setTurbo(unsigned id, bool value);

  bool forcedButton(unsigned id) const;
  void setForcedButton(unsigned id, bool value);

  bool digitalPadToAnalogStick(void);
  void setDigitalPadToAnalogStick(bool on);

  uint16_t rumbleStrong(void);
  uint16_t rumbleWeak(void);
  void setRumbleStrong(uint16_t v);
  void setRumbleWeak(uint16_t v);

  bool accelEnabled(void);
  unsigned accelRate(void);
  float accelX(void);
  float accelY(void);
  float accelZ(void);
  void setAccelRate(unsigned v);
  void setAccelState(QRetroInputJoypad::SensorState v);
  void setAccelX(float v);
  void setAccelY(float v);
  void setAccelZ(float v);

  bool gyroEnabled(void);
  unsigned gyroRate(void);
  float gyroX(void);
  float gyroY(void);
  float gyroZ(void);
  void setGyroRate(unsigned v);
  void setGyroState(QRetroInputJoypad::SensorState v);
  void setGyroX(float v);
  void setGyroY(float v);
  void setGyroZ(float v);

  unsigned inputMethods(void);
  void setInputMethods(unsigned method);

  unsigned port(void);
  void setPort(unsigned port);

  void poll(void);

private:
  bool analogToDigital(unsigned id) const;

  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  int16_t m_AnalogStickDeadzone = QRETRO_INPUT_DEFAULT_STICK_DEADZONE;
  float m_AnalogDigitalThreshold = QRETRO_INPUT_DEFAULT_ANALOG_DIGITAL_THRESHOLD;
  bool m_AnalogStickToDigitalPad = false;
  int16_t m_Bitmask = 0;
  int16_t m_Buttons[RETRO_DEVICE_ID_JOYPAD_R3 + 1] = {};
  uint16_t m_ForcedButtons = 0;
  uint16_t m_Turbo = 0;
  bool m_TurboPhase = false;
  bool m_DigitalPadToAnalogStick = false;
  unsigned m_InputMethods = InputMethodGamepad | InputMethodKeyboard;
  unsigned m_Port = 0;
  uint16_t m_RumbleStrong = 0;
  uint16_t m_RumbleWeak = 0;

  SensorState m_AccelState = SensorUninitialized;
  unsigned m_AccelRate = 0;
  float m_Accel[3] = {};

  SensorState m_GyroState = SensorUninitialized;
  unsigned m_GyroRate = 0;
  float m_Gyro[3] = {};

  int16_t m_Sticks[2][2] = {};
};

class QRetroInput : public QObject
{
  Q_OBJECT

public:
  QRetroInput(QObject *parent = nullptr);

  void setBackend(QRetroInputBackend *backend);
  QRetroInputBackend *backend();

  /**
   * Implements retro_input_poll. Polls the hardware backend, applies keyboard
   * macros, then finalizes each joypad's bitmask. The core must call this
   * once per retro_run() before querying state().
   */
  void poll(void);

  /**
   * Returns the current state of an input. Implements `retro_input_state`.
   * @param port Controller port (0-based player index)
   * @param device Device type (RETRO_DEVICE_JOYPAD, ANALOG, etc)
   * @param index Sub-index within the device (e.g. stick index for analog)
   * @param id Input ID within the device (e.g. RETRO_DEVICE_ID_JOYPAD_B)
   * @return The input value, or 0 if the arguments are unsupported
   */
  int16_t state(unsigned port, unsigned device, unsigned index, unsigned id);

  int16_t analogButtonDeadzone(void);
  void setAnalogButtonDeadzone(int16_t dz);

  bool key(retro_key key);
  void setKey(retro_key key, bool down);

  const std::vector<QRetroControllerPort> &controllerPorts(void) const;

  void setControllerPortDevice(void (*fn)(unsigned, unsigned));

  void setControllerInfo(const retro_controller_info *info);

  unsigned selectedControllerType(unsigned port) const;

  void setSelectedControllerType(unsigned port, unsigned id);

  unsigned maxUsers(void);
  void setMaxUsers(unsigned max);

  bool supportsBitmasks(void);
  void setSupportsBitmasks(bool supports);

  bool useMaps(void);
  void setUseMaps(bool use);

  bool setRumble(unsigned port, retro_rumble_effect effect, uint16_t strength);

  bool setSensorState(unsigned port, retro_sensor_action action, unsigned rate);

  float getSensorInput(unsigned port, unsigned id);

  bool backendHandlesSensor(unsigned port, unsigned id) const;

  const std::vector<QRetroInputDescriptor> &inputDescriptors(void) const;

  void setInputDescriptors(const retro_input_descriptor *desc);

  uint64_t deviceCapabilities(void) const;
  void setDeviceCapabilities(uint64_t caps);

  QRetroInputJoypad *joypads();

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  QRetroInputBackend *m_Backend = nullptr;
  void (*m_SetControllerPortDevice)(unsigned, unsigned) = nullptr;
  std::vector<QRetroControllerPort> m_ControllerPorts;
  uint64_t m_DeviceCapabilities = (1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_MOUSE) |
                                  (1 << RETRO_DEVICE_KEYBOARD) | (1 << RETRO_DEVICE_ANALOG) |
                                  (1 << RETRO_DEVICE_POINTER);
  std::vector<QRetroInputDescriptor> m_InputDescriptors;
  QRetroInputJoypad m_Joypads[QRETRO_INPUT_DEFAULT_MAX_JOYPADS];
  std::vector<qretro_input_kb_map_t> m_KeyboardMaps;
  bool m_BackendAccel[QRETRO_INPUT_DEFAULT_MAX_JOYPADS] = {};
  bool m_BackendGyro[QRETRO_INPUT_DEFAULT_MAX_JOYPADS] = {};
  bool m_Keys[RETROK_LAST] = { false };
  unsigned m_MaxUsers = QRETRO_INPUT_DEFAULT_MAX_JOYPADS;
  bool m_SupportsBitmasks = true;
  bool m_UseMaps = true;
};

#endif
