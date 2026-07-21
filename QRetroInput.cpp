#include <QRetroCommon.h>
#include <QRetroInput.h>

void QRetroInputJoypad::poll(void)
{
  uint16_t bitmask = 0;

  /* Handle analog-to-digital */
  if (m_AnalogStickToDigitalPad)
  {
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_UP] |= analogToDigital(RETRO_DEVICE_ID_JOYPAD_UP);
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_DOWN] |= analogToDigital(RETRO_DEVICE_ID_JOYPAD_DOWN);
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_LEFT] |= analogToDigital(RETRO_DEVICE_ID_JOYPAD_LEFT);
    m_Buttons[RETRO_DEVICE_ID_JOYPAD_RIGHT] |= analogToDigital(RETRO_DEVICE_ID_JOYPAD_RIGHT);
  }

  /* Handle turbo */
  m_TurboPhase = !m_TurboPhase;
  if (m_Turbo && !m_TurboPhase)
  {
    for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
      if (m_Turbo & (1 << i))
        m_Buttons[i] = 0;
  }

  /* Handle programmatically forced buttons */
  if (m_ForcedButtons)
    for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
      m_Buttons[i] |= static_cast<int16_t>((m_ForcedButtons >> i) & 1);

  /* Handle remappings */
  if (m_HasRemap)
  {
    int16_t remapped[RETRO_DEVICE_ID_JOYPAD_R3 + 1] = {};
    for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
    {
      const unsigned dst = m_Remap[i];
      if (dst <= RETRO_DEVICE_ID_JOYPAD_R3 && m_Buttons[i] > remapped[dst])
        remapped[dst] = m_Buttons[i];
    }
    for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
      m_Buttons[i] = remapped[i];
  }

  /* Update the input bitmask, even if reporting not to support it. */
  for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
    bitmask |= m_Buttons[i] ? (1 << i) : 0;

  m_Bitmask = static_cast<int16_t>(bitmask);
}

bool QRetroInputJoypad::analogToDigital(unsigned id) const
{
  int16_t threshold = static_cast<int16_t>(m_AnalogDigitalThreshold * 32767);

  switch (id)
  {
  case RETRO_DEVICE_ID_JOYPAD_UP:
    return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y] < -threshold;
  case RETRO_DEVICE_ID_JOYPAD_DOWN:
    return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y] > threshold;
  case RETRO_DEVICE_ID_JOYPAD_LEFT:
    return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X] < -threshold;
  case RETRO_DEVICE_ID_JOYPAD_RIGHT:
    return m_Sticks[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X] > threshold;
  default:
    return false;
  }
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

  return m_Buttons[id] > m_AnalogButtonDeadzone;
}

void QRetroInputJoypad::setDigitalButton(unsigned id, bool value)
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return;
  else
    m_Buttons[id] = value;
}

void QRetroInputJoypad::setButtonRemap(unsigned from, unsigned to)
{
  if (from > RETRO_DEVICE_ID_JOYPAD_R3 || to > RETRO_DEVICE_ID_JOYPAD_R3)
    return;
  if (!m_HasRemap)
  {
    for (unsigned i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
      m_Remap[i] = i;
    m_HasRemap = true;
  }
  m_Remap[from] = to;
}

void QRetroInputJoypad::clearButtonRemaps(void)
{
  m_HasRemap = false;
}

void QRetroInputJoypad::setAnalogStickRemap(unsigned dest, unsigned src)
{
  if (dest > RETRO_DEVICE_INDEX_ANALOG_RIGHT || src > RETRO_DEVICE_INDEX_ANALOG_RIGHT)
    return;
  if (!m_HasStickRemap)
  {
    m_StickRemap[RETRO_DEVICE_INDEX_ANALOG_LEFT] = RETRO_DEVICE_INDEX_ANALOG_LEFT;
    m_StickRemap[RETRO_DEVICE_INDEX_ANALOG_RIGHT] = RETRO_DEVICE_INDEX_ANALOG_RIGHT;
    m_HasStickRemap = true;
  }
  m_StickRemap[dest] = src;
}

void QRetroInputJoypad::clearAnalogStickRemaps(void)
{
  m_HasStickRemap = false;
}

void QRetroInputJoypad::setAnalogAxisRemap(
  unsigned destIndex, unsigned destId, unsigned srcIndex, unsigned srcId, int sign)
{
  if (destIndex > RETRO_DEVICE_INDEX_ANALOG_RIGHT || destId > RETRO_DEVICE_ID_ANALOG_Y ||
      srcIndex > RETRO_DEVICE_INDEX_ANALOG_RIGHT || srcId > RETRO_DEVICE_ID_ANALOG_Y)
    return;
  if (!m_HasAxisRemap)
  {
    /* Seed the table with identity so untouched axes still read straight through. */
    for (unsigned i = 0; i <= RETRO_DEVICE_INDEX_ANALOG_RIGHT; i++)
      for (unsigned j = 0; j <= RETRO_DEVICE_ID_ANALOG_Y; j++)
        m_AxisRemap[i][j] = { i, j, 1 };
    m_HasAxisRemap = true;
  }
  m_AxisRemap[destIndex][destId] = { srcIndex, srcId, sign < 0 ? -1 : 1 };
}

void QRetroInputJoypad::clearAnalogAxisRemaps(void)
{
  m_HasAxisRemap = false;
}

bool QRetroInputJoypad::forcedButton(unsigned id) const
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return false;
  return m_ForcedButtons & (1u << id);
}

void QRetroInputJoypad::setForcedButton(unsigned id, bool value)
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return;
  if (value)
    m_ForcedButtons |= static_cast<uint16_t>(1u << id);
  else
    m_ForcedButtons &= static_cast<uint16_t>(~(1u << id));
}

bool QRetroInputJoypad::turbo(unsigned id) const
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return false;
  return m_Turbo & (1 << id);
}

void QRetroInputJoypad::setTurbo(unsigned id, bool value)
{
  if (id > RETRO_DEVICE_ID_JOYPAD_R3)
    return;
  if (value)
    m_Turbo |= static_cast<uint16_t>(1 << id);
  else
    m_Turbo &= static_cast<uint16_t>(~(1 << id));
}

int16_t QRetroInputJoypad::analogStick(unsigned index, unsigned id)
{
  if (index > RETRO_DEVICE_INDEX_ANALOG_RIGHT || id > RETRO_DEVICE_ID_ANALOG_Y)
    return 0;
  if (m_HasAxisRemap)
  {
    const AxisRemap &r = m_AxisRemap[index][id];
    int32_t v = static_cast<int32_t>(m_Sticks[r.index][r.id]) * r.sign;
    /* Clamp: negating -32768 would overflow the int16_t range. */
    if (v > 32767)
      v = 32767;
    else if (v < -32768)
      v = -32768;
    return static_cast<int16_t>(v);
  }
  if (m_HasStickRemap)
    index = m_StickRemap[index];
  return m_Sticks[index][id];
}

void QRetroInputJoypad::setAnalogStick(unsigned index, unsigned id, int16_t value)
{
  if (index > RETRO_DEVICE_INDEX_ANALOG_RIGHT || id > RETRO_DEVICE_ID_ANALOG_Y)
    return;
  else
    m_Sticks[index][id] = value;
}

void QRetroInputJoypad::setInputMethods(unsigned method)
{
  m_InputMethods = method;
}

#define QRETRO_DEFAULT_MAP_JOYPAD(a, b)                                                            \
  m_KeyboardMaps.push_back(                                                                        \
    { { a, -1 }, { { 0, RETRO_DEVICE_JOYPAD, 0, b, true }, { -1, 0, 0, 0, 0 } } })

#define QRETRO_DEFAULT_MAP_ANALOG(a, b, c, d)                                                      \
  m_KeyboardMaps.push_back(                                                                        \
    { { a, -1 }, { { 0, RETRO_DEVICE_ANALOG, b, c, d }, { -1, 0, 0, 0, 0 } } })

QRetroInput::QRetroInput(QObject *parent)
{
  setParent(parent);

  for (unsigned i = 0; i < m_MaxUsers; i++)
    m_Joypads[i].setPort(i);

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
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_e, 2, RETRO_DEVICE_ID_JOYPAD_L2, 32767 / 2);
  QRETRO_DEFAULT_MAP_ANALOG(RETROK_r, 2, RETRO_DEVICE_ID_JOYPAD_R2, 32767 / 2);
}

void QRetroInput::poll(void)
{
  /* Update backend state first. */
  if (m_Backend)
    m_Backend->poll();

  /* Check keyboard macros, which may also update m_Buttons. Macros always
   * target whichever port the keyboard is currently assigned to, not the
   * port baked into the default mapping (which is always 0). */
  if (m_UseMaps && m_KeyboardPort >= 0)
    for (const auto &map : m_KeyboardMaps)
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

      QRetroInputJoypad &joypad = m_Joypads[m_KeyboardPort];
      for (unsigned i = 0; i < QRETRO_INPUT_MAPPING_MAX; i++)
      {
        if (button->port < 0)
          break;
        switch (button->device)
        {
        case RETRO_DEVICE_JOYPAD:
          joypad.setDigitalButton(button->id, pressed ? button->value : !button->value);
          break;
        case RETRO_DEVICE_ANALOG:
          if (button->index != RETRO_DEVICE_INDEX_ANALOG_BUTTON)
          {
            if (pressed)
              joypad.setAnalogStick(button->index, button->id, static_cast<int16_t>(button->value));
            else if (joypad.analogStick(button->index, button->id) == button->value)
              joypad.setAnalogStick(button->index, button->id, 0);
          }
          else
            joypad.setAnalogButton(button->id, pressed ? button->value : 0);
          break;
        }
        button++;
      }
    }

  /* Snapshot all button state into the bitmask now that every input source
   * (hardware backend + keyboard macros) has had a chance to update m_Buttons. */
  for (unsigned i = 0; i < m_MaxUsers; i++)
    m_Joypads[i].poll();
}

int16_t QRetroInput::state(unsigned port, unsigned device, unsigned index, unsigned id)
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

bool QRetroInputJoypad::analogStickToDigitalPad(void)
{
  return m_AnalogStickToDigitalPad;
}
void QRetroInputJoypad::setAnalogStickToDigitalPad(bool on)
{
  m_AnalogStickToDigitalPad = on;
}

float QRetroInputJoypad::analogDigitalThreshold(void)
{
  return m_AnalogDigitalThreshold;
}
void QRetroInputJoypad::setAnalogDigitalThreshold(float t)
{
  m_AnalogDigitalThreshold = t;
}

int16_t QRetroInputJoypad::analogStickDeadzone(void)
{
  return m_AnalogStickDeadzone;
}
void QRetroInputJoypad::setAnalogStickDeadzone(int16_t dz)
{
  m_AnalogStickDeadzone = dz;
}

int16_t QRetroInputJoypad::bitmask(void)
{
  return m_Bitmask;
}

bool QRetroInputJoypad::digitalPadToAnalogStick(void)
{
  return m_DigitalPadToAnalogStick;
}
void QRetroInputJoypad::setDigitalPadToAnalogStick(bool on)
{
  m_DigitalPadToAnalogStick = on;
}

uint16_t QRetroInputJoypad::rumbleStrong(void)
{
  return m_RumbleStrong;
}
uint16_t QRetroInputJoypad::rumbleWeak(void)
{
  return m_RumbleWeak;
}
void QRetroInputJoypad::setRumbleStrong(uint16_t v)
{
  m_RumbleStrong = v;
}
void QRetroInputJoypad::setRumbleWeak(uint16_t v)
{
  m_RumbleWeak = v;
}

bool QRetroInputJoypad::accelEnabled(void)
{
  return m_AccelState == SensorEnabled;
}
unsigned QRetroInputJoypad::accelRate(void)
{
  return m_AccelRate;
}
float QRetroInputJoypad::accelX(void)
{
  return m_Accel[0];
}
float QRetroInputJoypad::accelY(void)
{
  return m_Accel[1];
}
float QRetroInputJoypad::accelZ(void)
{
  return m_Accel[2];
}
void QRetroInputJoypad::setAccelRate(unsigned v)
{
  m_AccelRate = v;
}
void QRetroInputJoypad::setAccelState(QRetroInputJoypad::SensorState v)
{
  m_AccelState = v;
}
void QRetroInputJoypad::setAccelX(float v)
{
  m_Accel[0] = v;
}
void QRetroInputJoypad::setAccelY(float v)
{
  m_Accel[1] = v;
}
void QRetroInputJoypad::setAccelZ(float v)
{
  m_Accel[2] = v;
}

bool QRetroInputJoypad::gyroEnabled(void)
{
  return m_GyroState == SensorEnabled;
}
unsigned QRetroInputJoypad::gyroRate(void)
{
  return m_GyroRate;
}
float QRetroInputJoypad::gyroX(void)
{
  return m_Gyro[0];
}
float QRetroInputJoypad::gyroY(void)
{
  return m_Gyro[1];
}
float QRetroInputJoypad::gyroZ(void)
{
  return m_Gyro[2];
}
void QRetroInputJoypad::setGyroRate(unsigned v)
{
  m_GyroRate = v;
}
void QRetroInputJoypad::setGyroState(QRetroInputJoypad::SensorState v)
{
  m_GyroState = v;
}
void QRetroInputJoypad::setGyroX(float v)
{
  m_Gyro[0] = v;
}
void QRetroInputJoypad::setGyroY(float v)
{
  m_Gyro[1] = v;
}
void QRetroInputJoypad::setGyroZ(float v)
{
  m_Gyro[2] = v;
}

unsigned QRetroInputJoypad::inputMethods(void)
{
  return m_InputMethods;
}

unsigned QRetroInputJoypad::port(void)
{
  return m_Port;
}
void QRetroInputJoypad::setPort(unsigned port)
{
  m_Port = port;
}

void QRetroInput::setBackend(QRetroInputBackend *backend)
{
  m_Backend = backend;
}
QRetroInputBackend *QRetroInput::backend()
{
  return m_Backend;
}

QVector<QRetroGamepadInfo> QRetroInput::connectedGamepads() const
{
  return m_Backend ? m_Backend->connectedGamepads() : QVector<QRetroGamepadInfo>();
}

bool QRetroInput::assignGamepad(unsigned port, unsigned deviceId)
{
  return m_Backend && m_Backend->assignGamepad(port, deviceId);
}

unsigned QRetroInput::assignedDeviceId(unsigned port) const
{
  return m_Backend ? m_Backend->assignedDeviceId(port) : QRETRO_NO_DEVICE;
}

int16_t QRetroInput::analogButtonDeadzone(void)
{
  return m_AnalogButtonDeadzone;
}
void QRetroInput::setAnalogButtonDeadzone(int16_t dz)
{
  m_AnalogButtonDeadzone = dz;
}

void QRetroInput::remapButton(unsigned port, unsigned from, unsigned to)
{
  if (port < QRETRO_INPUT_DEFAULT_MAX_JOYPADS)
    m_Joypads[port].setButtonRemap(from, to);
}

void QRetroInput::clearButtonRemaps(unsigned port)
{
  if (port < QRETRO_INPUT_DEFAULT_MAX_JOYPADS)
    m_Joypads[port].clearButtonRemaps();
}

bool QRetroInput::key(retro_key key)
{
  return m_Keys[key];
}
void QRetroInput::setKey(retro_key key, bool down)
{
  m_Keys[key] = down;
}

const std::vector<QRetroControllerPort> &QRetroInput::controllerPorts(void) const
{
  return m_ControllerPorts;
}

void QRetroInput::setControllerPortDevice(void (*fn)(unsigned, unsigned))
{
  m_SetControllerPortDevice = fn;
}

void QRetroInput::setControllerInfo(const retro_controller_info *info)
{
  m_ControllerPorts.clear();
  if (!info)
    return;
  for (const retro_controller_info *ci = info; ci->types; ci++)
  {
    QRetroControllerPort port;
    for (unsigned i = 0; i < ci->num_types; i++)
      port.types.push_back({ ci->types[i].desc ? ci->types[i].desc : "", ci->types[i].id });
    if (!port.types.empty())
      port.selectedId = port.types[0].id;
    m_ControllerPorts.push_back(std::move(port));
  }
  /* Only push defaults to the core once retro_init has run; otherwise a core that
   * sends SET_CONTROLLER_INFO from retro_set_environment (e.g. fceumm) would have
   * retro_set_controller_port_device called before it is initialized. The pending
   * ports are applied by setPortDeviceReady(). */
  if (m_SetControllerPortDevice && m_PortDeviceReady)
    for (unsigned p = 0; p < m_ControllerPorts.size(); p++)
      m_SetControllerPortDevice(p, m_ControllerPorts[p].selectedId);
}

void QRetroInput::setPortDeviceReady(bool ready)
{
  m_PortDeviceReady = ready;
  if (ready && m_SetControllerPortDevice)
    for (unsigned p = 0; p < m_ControllerPorts.size(); p++)
      m_SetControllerPortDevice(p, m_ControllerPorts[p].selectedId);
}

unsigned QRetroInput::selectedControllerType(unsigned port) const
{
  return port < m_ControllerPorts.size() ? m_ControllerPorts[port].selectedId : RETRO_DEVICE_JOYPAD;
}

void QRetroInput::setSelectedControllerType(unsigned port, unsigned id)
{
  if (port < m_ControllerPorts.size())
  {
    m_ControllerPorts[port].selectedId = id;
    if (m_SetControllerPortDevice)
      m_SetControllerPortDevice(port, id);
  }
}

unsigned QRetroInput::maxUsers(void)
{
  return m_MaxUsers;
}
void QRetroInput::setMaxUsers(unsigned max)
{
  m_MaxUsers = max;
}

int QRetroInput::keyboardPort(void) const
{
  return m_KeyboardPort;
}
void QRetroInput::setKeyboardPort(int port)
{
  m_KeyboardPort =
    port >= 0 && port < static_cast<int>(QRETRO_INPUT_DEFAULT_MAX_JOYPADS) ? port : -1;
}

bool QRetroInput::supportsBitmasks(void)
{
  return m_SupportsBitmasks;
}
void QRetroInput::setSupportsBitmasks(bool supports)
{
  m_SupportsBitmasks = supports;
}

bool QRetroInput::useMaps(void)
{
  return m_UseMaps;
}
void QRetroInput::setUseMaps(bool use)
{
  m_UseMaps = use;
}

bool QRetroInput::setRumble(unsigned port, retro_rumble_effect effect, uint16_t strength)
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

bool QRetroInput::setSensorState(unsigned port, retro_sensor_action action, unsigned rate)
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

float QRetroInput::getSensorInput(unsigned port, unsigned id)
{
  if (port >= m_MaxUsers)
    return 0.0f;
  QRetroInputJoypad &jp = m_Joypads[port];
  switch (id)
  {
  case RETRO_SENSOR_ACCELEROMETER_X:
    return jp.accelX();
  case RETRO_SENSOR_ACCELEROMETER_Y:
    return jp.accelY();
  case RETRO_SENSOR_ACCELEROMETER_Z:
    return jp.accelZ();
  case RETRO_SENSOR_GYROSCOPE_X:
    return jp.gyroX();
  case RETRO_SENSOR_GYROSCOPE_Y:
    return jp.gyroY();
  case RETRO_SENSOR_GYROSCOPE_Z:
    return jp.gyroZ();
  default:
    return 0.0f;
  }
}

bool QRetroInput::backendHandlesSensor(unsigned port, unsigned id) const
{
  if (port >= m_MaxUsers || !m_Backend)
    return false;
  return m_Backend->sensorActive(port, id);
}

const std::vector<QRetroInputDescriptor> &QRetroInput::inputDescriptors(void) const
{
  return m_InputDescriptors;
}

void QRetroInput::setInputDescriptors(const retro_input_descriptor *desc)
{
  m_InputDescriptors.clear();
  if (!desc)
    return;
  for (; desc->description; desc++)
    m_InputDescriptors.push_back(
      { desc->port, desc->device, desc->index, desc->id, desc->description });
}

uint64_t QRetroInput::deviceCapabilities(void) const
{
  return m_DeviceCapabilities;
}
void QRetroInput::setDeviceCapabilities(uint64_t caps)
{
  m_DeviceCapabilities = caps;
}

QRetroInputJoypad *QRetroInput::joypads()
{
  return m_Joypads;
}
