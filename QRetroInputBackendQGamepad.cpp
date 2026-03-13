#include "QRetroInputBackendQGamepad.h"

#if QRETRO_HAVE_GAMEPAD

#include "QRetroCommon.h"

#define CONNECT_DIGITAL(gp, jp, sig, id) \
  connect((gp), &QGamepad::sig, this, \
    [jp](bool pressed) { (jp)->setDigitalButton(id, pressed); })

#define CONNECT_STICK(gp, jp, sig, index, id) \
  connect((gp), &QGamepad::sig, this, \
    [jp](double v) { (jp)->setAnalogStick(index, id, qt2lr_analog(v)); })

#define CONNECT_TRIGGER(gp, jp, sig, id) \
  connect((gp), &QGamepad::sig, this, \
    [jp](double v) { (jp)->setAnalogButton(id, qt2lr_analog(v)); })

QRetroInputBackendQGamepad::QRetroInputBackendQGamepad(QObject *parent)
  : QRetroInputBackend(parent)
{
  auto *mgr = QGamepadManager::instance();

  connect(mgr, &QGamepadManager::gamepadConnected,
          this, &QRetroInputBackendQGamepad::onGamepadConnected);
  connect(mgr, &QGamepadManager::gamepadDisconnected,
          this, &QRetroInputBackendQGamepad::onGamepadDisconnected);
}

void QRetroInputBackendQGamepad::init(QRetroInputJoypad *joypads, unsigned maxUsers)
{
  auto connectedIds = QGamepadManager::instance()->connectedGamepads();

  m_Joypads  = joypads;
  m_MaxUsers = maxUsers;

  for (unsigned i = 0; i < m_MaxUsers; i++)
  {
#if _WIN32
    /* On Windows, XInput controllers enumerate by slot index. */
    connectPort(i, static_cast<int>(i));
#else
    /* On Linux, use the actual IDs of connected gamepads. */
    if (connectedIds.size() <= static_cast<int>(i))
      break;
    connectPort(i, connectedIds.at(static_cast<int>(i)));
#endif
  }
}

void QRetroInputBackendQGamepad::connectPort(unsigned port, int deviceId)
{
  if (port >= m_MaxUsers)
    return;

  if (m_Gamepads[port])
  {
    disconnect(m_Gamepads[port], nullptr, this, nullptr);
    delete m_Gamepads[port];
  }

  auto *gp = new QGamepad(deviceId, this);
  m_Gamepads[port] = gp;
  QRetroInputJoypad *jp = &m_Joypads[port];

  CONNECT_DIGITAL(gp, jp, buttonUpChanged,     RETRO_DEVICE_ID_JOYPAD_UP);
  CONNECT_DIGITAL(gp, jp, buttonDownChanged,   RETRO_DEVICE_ID_JOYPAD_DOWN);
  CONNECT_DIGITAL(gp, jp, buttonLeftChanged,   RETRO_DEVICE_ID_JOYPAD_LEFT);
  CONNECT_DIGITAL(gp, jp, buttonRightChanged,  RETRO_DEVICE_ID_JOYPAD_RIGHT);
  CONNECT_DIGITAL(gp, jp, buttonAChanged,      RETRO_DEVICE_ID_JOYPAD_B);
  CONNECT_DIGITAL(gp, jp, buttonBChanged,      RETRO_DEVICE_ID_JOYPAD_A);
  CONNECT_DIGITAL(gp, jp, buttonXChanged,      RETRO_DEVICE_ID_JOYPAD_Y);
  CONNECT_DIGITAL(gp, jp, buttonYChanged,      RETRO_DEVICE_ID_JOYPAD_X);
  CONNECT_DIGITAL(gp, jp, buttonL1Changed,     RETRO_DEVICE_ID_JOYPAD_L);
  CONNECT_DIGITAL(gp, jp, buttonL3Changed,     RETRO_DEVICE_ID_JOYPAD_L3);
  CONNECT_DIGITAL(gp, jp, buttonR1Changed,     RETRO_DEVICE_ID_JOYPAD_R);
  CONNECT_DIGITAL(gp, jp, buttonR3Changed,     RETRO_DEVICE_ID_JOYPAD_R3);
  CONNECT_DIGITAL(gp, jp, buttonStartChanged,  RETRO_DEVICE_ID_JOYPAD_START);
  CONNECT_DIGITAL(gp, jp, buttonSelectChanged, RETRO_DEVICE_ID_JOYPAD_SELECT);

  CONNECT_STICK(gp, jp, axisLeftXChanged,  RETRO_DEVICE_INDEX_ANALOG_LEFT,  RETRO_DEVICE_ID_ANALOG_X);
  CONNECT_STICK(gp, jp, axisLeftYChanged,  RETRO_DEVICE_INDEX_ANALOG_LEFT,  RETRO_DEVICE_ID_ANALOG_Y);
  CONNECT_STICK(gp, jp, axisRightXChanged, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X);
  CONNECT_STICK(gp, jp, axisRightYChanged, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y);

  CONNECT_TRIGGER(gp, jp, buttonL2Changed, RETRO_DEVICE_ID_JOYPAD_L2);
  CONNECT_TRIGGER(gp, jp, buttonR2Changed, RETRO_DEVICE_ID_JOYPAD_R2);
}

void QRetroInputBackendQGamepad::onGamepadConnected(int deviceId)
{
  for (unsigned i = 0; i < m_MaxUsers; i++)
  {
    if (!m_Gamepads[i] || !m_Gamepads[i]->isConnected())
    {
      connectPort(i, deviceId);
      emit gamepadConnected(i);
      return;
    }
  }
}

void QRetroInputBackendQGamepad::onGamepadDisconnected(int deviceId)
{
  for (unsigned i = 0; i < m_MaxUsers; i++)
  {
    if (m_Gamepads[i] && m_Gamepads[i]->deviceId() == deviceId)
    {
      delete m_Gamepads[i];
      m_Gamepads[i] = nullptr;
      emit gamepadDisconnected(i);
      return;
    }
  }
}

#endif
