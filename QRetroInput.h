#ifndef QRETRO_INPUT_H
#define QRETRO_INPUT_H

#if QRETRO_HAVE_GAMEPAD
#include <QGamepad>
#else
#include <QObject>
#endif

#include <vector>

#include <string>
#include <libretro.h>

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

  const std::vector<QRetroControllerPort>& controllerPorts(void) const { return m_ControllerPorts; }

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
      m_ControllerPorts[port].selectedId = id;
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

private:
  int16_t m_AnalogButtonDeadzone = QRETRO_INPUT_DEFAULT_BUTTON_DEADZONE;
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
