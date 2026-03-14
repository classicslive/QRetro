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
  unsigned    id;
};

struct QRetroInputDescriptor
{
  unsigned    port;
  unsigned    device;
  unsigned    index;
  unsigned    id;
  std::string description;
};

struct QRetroControllerPort
{
  std::vector<QRetroControllerType> types;
  unsigned                          selectedId = RETRO_DEVICE_JOYPAD;
};

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

  bool analogStickToDigitalPad(void) { return m_AnalogStickToDigitalPad; }
  void setAnalogStickToDigitalPad(bool on) { m_AnalogStickToDigitalPad = on; }

  int16_t bitmask(void) { return m_Bitmask; }

  bool digitalButton(unsigned id);
  void setDigitalButton(unsigned id, bool value);

  bool digitalPadToAnalogStick(void) { return m_DigitalPadToAnalogStick; }
  void setDigitalPadToAnalogStick(bool on) { m_DigitalPadToAnalogStick = on; }

  uint16_t rumbleStrong(void) { return m_RumbleStrong; }
  uint16_t rumbleWeak(void) { return m_RumbleWeak; }
  void setRumbleStrong(uint16_t v) { m_RumbleStrong = v; }
  void setRumbleWeak(uint16_t v) { m_RumbleWeak = v; }

  bool accelEnabled(void) { return m_AccelState == SensorEnabled; }
  unsigned accelRate(void) { return m_AccelRate; }
  float accelX(void) { return m_Accel[0]; }
  float accelY(void) { return m_Accel[1]; }
  float accelZ(void) { return m_Accel[2]; }
  void setAccelRate(unsigned v) { m_AccelRate = v; }
  void setAccelState(QRetroInputJoypad::SensorState v) { m_AccelState = v; }
  void setAccelX(float v) { m_Accel[0] = v; }
  void setAccelY(float v) { m_Accel[1] = v; }
  void setAccelZ(float v) { m_Accel[2] = v; }

  bool gyroEnabled(void) { return m_GyroState == SensorEnabled; }
  unsigned gyroRate(void) { return m_GyroRate; }
  float gyroX(void) { return m_Gyro[0]; }
  float gyroY(void) { return m_Gyro[1]; }
  float gyroZ(void) { return m_Gyro[2]; }
  void setGyroRate(unsigned v) { m_GyroRate = v; }
  void setGyroState(QRetroInputJoypad::SensorState v) { m_GyroState = v; }
  void setGyroX(float v) { m_Gyro[0] = v; }
  void setGyroY(float v) { m_Gyro[1] = v; }
  void setGyroZ(float v) { m_Gyro[2] = v; }

  unsigned inputMethods(void) { return m_InputMethods; }
  void setInputMethods(unsigned method);

  unsigned port(void) { return m_Port; }
  void setPort(unsigned port) { m_Port = port; }

  void poll(void);

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  int16_t m_AnalogStickDeadzone = 0;
  bool m_AnalogStickToDigitalPad = false;
  int16_t m_Bitmask;
  int16_t m_Buttons[RETRO_DEVICE_ID_JOYPAD_R3 + 1];
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

  int16_t m_Sticks[2][2];
};

class QRetroInput : public QObject
{
  Q_OBJECT

public:
  QRetroInput(QObject *parent = nullptr);

  /**
   * Sets the active gamepad input backend. The backend must be initialized
   * before calling setBackend(); QRetroInput does not take ownership.
   * Call backend()->init(joypads(), maxUsers()) after constructing the backend.
   */
  void setBackend(QRetroInputBackend *backend) { m_Backend = backend; }
  QRetroInputBackend *backend() { return m_Backend; }

  void poll(void);

  int16_t state(unsigned port, unsigned device, unsigned index, unsigned id);

  int16_t analogButtonDeadzone(void) { return m_AnalogButtonDeadzone; }

  void setAnalogButtonDeadzone(int16_t dz) { m_AnalogButtonDeadzone = dz; }

  bool key(retro_key key) { return m_Keys[key]; }

  void setKey(retro_key key, bool down) { m_Keys[key] = down; }

  const std::vector<QRetroControllerPort>& controllerPorts(void) const { return m_ControllerPorts; }

  /**
   * Sets the core's retro_set_controller_port_device function pointer.
   * Called automatically by setSelectedControllerType() to notify the core
   * when the active device type for a port changes.
   */
  void setControllerPortDevice(void (*fn)(unsigned, unsigned)) { m_SetControllerPortDevice = fn; }

  void setControllerInfo(const retro_controller_info *info)
  {
    m_ControllerPorts.clear();
    if (!info) return;
    for (const retro_controller_info *ci = info; ci->types; ci++)
    {
      QRetroControllerPort port;
      for (unsigned i = 0; i < ci->num_types; i++)
        port.types.push_back({ ci->types[i].desc ? ci->types[i].desc : "", ci->types[i].id });
      m_ControllerPorts.push_back(std::move(port));
    }
  }

  unsigned selectedControllerType(unsigned port) const
  {
    return port < m_ControllerPorts.size() ? m_ControllerPorts[port].selectedId : RETRO_DEVICE_JOYPAD;
  }

  void setSelectedControllerType(unsigned port, unsigned id)
  {
    if (port < m_ControllerPorts.size())
    {
      m_ControllerPorts[port].selectedId = id;
      if (m_SetControllerPortDevice)
        m_SetControllerPortDevice(port, id);
    }
  }

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

  /**
   * Delegates a rumble request to the active backend.
   * Returns false if no backend is active or the backend does not support rumble.
   */
  bool setRumble(unsigned port, retro_rumble_effect effect, uint16_t strength)
  {
    if (port < m_MaxUsers)
    {
      switch (effect)
      {
      case RETRO_RUMBLE_STRONG:
        m_Joypads[port].setRumbleStrong(strength);
        break;
      case RETRO_RUMBLE_WEAK:
        m_Joypads[port].setRumbleWeak(strength);
        break;
      /* No default to encourage a warning if more rumble types are added */
      case RETRO_RUMBLE_DUMMY:
        return false;
      }
    }

    return m_Backend ? m_Backend->setRumble(port, effect, strength) : false;
  }

  /**
   * Enables or disables a sensor on the active backend for a given port.
   * Returns true if the backend accepted the request; false if unhandled
   * (caller should fall back to QRetroSensors).
   */
  bool setSensorState(unsigned port, retro_sensor_action action, unsigned rate)
  {
    if (port < m_MaxUsers)
    {
      switch (action)
      {
      case RETRO_SENSOR_ACCELEROMETER_ENABLE:
        m_Joypads[port].setAccelState(QRetroInputJoypad::SensorEnabled);
        m_Joypads[port].setAccelRate(rate);
        break;
      case RETRO_SENSOR_ACCELEROMETER_DISABLE:
        m_Joypads[port].setAccelState(QRetroInputJoypad::SensorDisabled);
        break;
      case RETRO_SENSOR_GYROSCOPE_ENABLE:
        m_Joypads[port].setGyroState(QRetroInputJoypad::SensorEnabled);
        m_Joypads[port].setGyroRate(rate);
        break;
      case RETRO_SENSOR_GYROSCOPE_DISABLE:
        m_Joypads[port].setGyroState(QRetroInputJoypad::SensorDisabled);
        break;
      /* No default to encourage a warning if more sensor actions are added */
      case RETRO_SENSOR_ILLUMINANCE_ENABLE:
      case RETRO_SENSOR_ILLUMINANCE_DISABLE:
      case RETRO_SENSOR_DUMMY:
        return false;
      }
    }

    return m_Backend ? m_Backend->setSensorState(port, action, rate) : false;
  }

  /**
   * Returns a sensor reading from the active backend for a given port and axis.
   * Only valid when the backend accepted the corresponding setSensorState() call.
   */
  float getSensorInput(unsigned port, unsigned id)
  {
    if (port >= m_MaxUsers)
      return 0.0f;
    QRetroInputJoypad &jp = m_Joypads[port];
    switch (id)
    {
    case RETRO_SENSOR_ACCELEROMETER_X: return jp.accelX();
    case RETRO_SENSOR_ACCELEROMETER_Y: return jp.accelY();
    case RETRO_SENSOR_ACCELEROMETER_Z: return jp.accelZ();
    case RETRO_SENSOR_GYROSCOPE_X:     return jp.gyroX();
    case RETRO_SENSOR_GYROSCOPE_Y:     return jp.gyroY();
    case RETRO_SENSOR_GYROSCOPE_Z:     return jp.gyroZ();
    default:                           return 0.0f;
    }
  }

  /**
   * Returns true if the backend currently has a live sensor feed for the
   * given port and axis. Delegates to the backend's sensorActive() so the
   * answer reflects real-time state rather than stale cached flags.
   */
  bool backendHandlesSensor(unsigned port, unsigned id) const
  {
    if (port >= m_MaxUsers || !m_Backend) return false;
    return m_Backend->sensorActive(port, id);
  }

  /**
   * Returns the input descriptors provided by the core via SET_INPUT_DESCRIPTORS.
   * Each descriptor describes the function of a specific button/axis for a given port.
   */
  const std::vector<QRetroInputDescriptor>& inputDescriptors(void) const { return m_InputDescriptors; }

  void setInputDescriptors(const retro_input_descriptor *desc)
  {
    m_InputDescriptors.clear();
    if (!desc) return;
    for (; desc->description; desc++)
      m_InputDescriptors.push_back({ desc->port, desc->device, desc->index, desc->id,
                                     desc->description });
  }

  /**
   * Returns the bitmask of supported input device types reported to the core.
   * Each bit N corresponds to device type N (i.e. (1 << RETRO_DEVICE_x)).
   */
  uint64_t deviceCapabilities(void) const { return m_DeviceCapabilities; }

  /**
   * Sets the bitmask of supported input device types to report to the core.
   */
  void setDeviceCapabilities(uint64_t caps) { m_DeviceCapabilities = caps; }

  /**
   * Returns the raw joypad array. Primarily used to pass to a backend's init().
   */
  QRetroInputJoypad *joypads() { return m_Joypads; }

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
  QRetroInputBackend *m_Backend = nullptr;
  void (*m_SetControllerPortDevice)(unsigned, unsigned) = nullptr;
  std::vector<QRetroControllerPort> m_ControllerPorts;
  uint64_t m_DeviceCapabilities =
    (1 << RETRO_DEVICE_JOYPAD)   |
    (1 << RETRO_DEVICE_MOUSE)    |
    (1 << RETRO_DEVICE_KEYBOARD) |
    (1 << RETRO_DEVICE_ANALOG)   |
    (1 << RETRO_DEVICE_POINTER);
  std::vector<QRetroInputDescriptor> m_InputDescriptors;
  QRetroInputJoypad m_Joypads[QRETRO_INPUT_DEFAULT_MAX_JOYPADS];
  std::vector<qretro_input_kb_map_t> m_KeyboardMaps;
  bool m_BackendAccel[QRETRO_INPUT_DEFAULT_MAX_JOYPADS] = {};
  bool m_BackendGyro[QRETRO_INPUT_DEFAULT_MAX_JOYPADS]  = {};
  bool m_Keys[RETROK_LAST] = { false };
  unsigned m_MaxUsers = QRETRO_INPUT_DEFAULT_MAX_JOYPADS;
  bool m_SupportsBitmasks = true;
  bool m_UseMaps = true;
};

#endif
