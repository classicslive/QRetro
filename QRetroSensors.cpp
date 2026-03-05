#include "QRetroSensors.h"

QRetroSensors::QRetroSensors()
{
#if QRETRO_HAVE_SENSORS
  /**
   * Probe backend availability for each sensor type by attempting a start.
   * On platforms without real hardware the backend may be absent or broken;
   * this lets us skip sensor API calls later rather than crashing.
   */
  m_AccelerometerAvailable = m_AccelerometerSensor.start();
  m_AccelerometerSensor.stop();

  m_GyroscopeAvailable = m_GyroscopeSensor.start();
  m_GyroscopeSensor.stop();

  m_IlluminanceAvailable = m_IlluminanceSensor.start();
  m_IlluminanceSensor.stop();

  /* Ignore the force of gravity for accelerometer */
  m_AccelerometerSensor.setAccelerationMode(QAccelerometer::User);
#endif
}

bool QRetroSensors::setState(unsigned port, retro_sensor_action action,
  unsigned rate)
{
  if (port)
    return false;
  else
  {
    switch (action)
    {
    case RETRO_SENSOR_ACCELEROMETER_ENABLE:
      m_AccelerometerEnabled = true;
      m_AccelerometerRate = rate;
#if QRETRO_HAVE_SENSORS
      if (m_AccelerometerAvailable)
      {
        m_AccelerometerSensor.stop();
        m_AccelerometerSensor.setDataRate(static_cast<int>(rate));
        m_AccelerometerSensor.start();
      }
#endif
      break;
    case RETRO_SENSOR_ACCELEROMETER_DISABLE:
      m_AccelerometerEnabled = false;
#if QRETRO_HAVE_SENSORS
      if (m_AccelerometerAvailable)
        m_AccelerometerSensor.stop();
#endif
      break;
    case RETRO_SENSOR_GYROSCOPE_ENABLE:
      m_GyroscopeEnabled = true;
      m_GyroscopeRate = rate;
#if QRETRO_HAVE_SENSORS
      if (m_GyroscopeAvailable)
      {
        m_GyroscopeSensor.stop();
        m_GyroscopeSensor.setDataRate(static_cast<int>(rate));
        m_GyroscopeSensor.start();
      }
#endif
      break;
    case RETRO_SENSOR_GYROSCOPE_DISABLE:
      m_GyroscopeEnabled = false;
#if QRETRO_HAVE_SENSORS
      if (m_GyroscopeAvailable)
        m_GyroscopeSensor.stop();
#endif
      break;
    case RETRO_SENSOR_ILLUMINANCE_ENABLE:
      m_IlluminanceEnabled = true;
      m_IlluminanceRate = rate;
#if QRETRO_HAVE_SENSORS
      if (m_IlluminanceAvailable)
      {
        m_IlluminanceSensor.stop();
        m_IlluminanceSensor.setDataRate(static_cast<int>(rate));
        m_IlluminanceSensor.start();
      }
#endif
      break;
    case RETRO_SENSOR_ILLUMINANCE_DISABLE:
      m_IlluminanceEnabled = false;
#if QRETRO_HAVE_SENSORS
      if (m_IlluminanceAvailable)
        m_IlluminanceSensor.stop();
#endif
      break;
    default:
      return false;
    }

    return true;
  }
}

float QRetroSensors::getInput(unsigned port, unsigned id)
{
  if (port)
    return 0;

  switch (id)
  {
  case RETRO_SENSOR_ACCELEROMETER_X:
    m_AccelXHasBeenRead = true;
    if (m_SpoofAccelEnabled) return m_SpoofAccel[0];
#if QRETRO_HAVE_SENSORS
    if (m_AccelerometerEnabled && m_AccelerometerSensor.reading())
      return static_cast<float>(m_AccelerometerSensor.reading()->x());
#endif
    return 0;

  case RETRO_SENSOR_ACCELEROMETER_Y:
    m_AccelYHasBeenRead = true;
    if (m_SpoofAccelEnabled) return m_SpoofAccel[1];
#if QRETRO_HAVE_SENSORS
    if (m_AccelerometerEnabled && m_AccelerometerSensor.reading())
      return static_cast<float>(m_AccelerometerSensor.reading()->y());
#endif
    return 0;

  case RETRO_SENSOR_ACCELEROMETER_Z:
    m_AccelZHasBeenRead = true;
    if (m_SpoofAccelEnabled) return m_SpoofAccel[2];
#if QRETRO_HAVE_SENSORS
    if (m_AccelerometerEnabled && m_AccelerometerSensor.reading())
      return static_cast<float>(m_AccelerometerSensor.reading()->z());
#endif
    return 0;

  case RETRO_SENSOR_GYROSCOPE_X:
    m_GyroXHasBeenRead = true;
    if (m_SpoofGyroEnabled) return m_SpoofGyro[0];
#if QRETRO_HAVE_SENSORS
    if (m_GyroscopeEnabled && m_GyroscopeSensor.reading())
      return static_cast<float>(m_GyroscopeSensor.reading()->x());
#endif
    return 0;

  case RETRO_SENSOR_GYROSCOPE_Y:
    m_GyroYHasBeenRead = true;
    if (m_SpoofGyroEnabled) return m_SpoofGyro[1];
#if QRETRO_HAVE_SENSORS
    if (m_GyroscopeEnabled && m_GyroscopeSensor.reading())
      return static_cast<float>(m_GyroscopeSensor.reading()->y());
#endif
    return 0;

  case RETRO_SENSOR_GYROSCOPE_Z:
    m_GyroZHasBeenRead = true;
    if (m_SpoofGyroEnabled) return m_SpoofGyro[2];
#if QRETRO_HAVE_SENSORS
    if (m_GyroscopeEnabled && m_GyroscopeSensor.reading())
      return static_cast<float>(m_GyroscopeSensor.reading()->z());
#endif
    return 0;

  case RETRO_SENSOR_ILLUMINANCE:
    m_IllumHasBeenRead = true;
    if (m_SpoofIllumEnabled)
      return m_SpoofIllum;
#if QRETRO_HAVE_SENSORS
    if (m_IlluminanceEnabled && m_IlluminanceSensor.reading())
      return static_cast<float>(m_IlluminanceSensor.reading()->lux());
#endif
    return 0;

  default:
    return 0;
  }
}
