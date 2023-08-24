#include "QRetroSensors.h"

QRetroSensors::QRetroSensors()
{
  /* Ignore force of gravity */
#if QRETRO_HAVE_SENSORS
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
      break;
    case RETRO_SENSOR_ACCELEROMETER_DISABLE:
      m_AccelerometerEnabled = false;
      break;
    case RETRO_SENSOR_GYROSCOPE_ENABLE:
      m_GyroscopeEnabled = true;
      m_GyroscopeRate = rate;
      break;
    case RETRO_SENSOR_GYROSCOPE_DISABLE:
      m_GyroscopeEnabled = false;
      break;
    case RETRO_SENSOR_ILLUMINANCE_ENABLE:
      m_IlluminanceEnabled = true;
      m_IlluminanceRate = rate;
      break;
    case RETRO_SENSOR_ILLUMINANCE_DISABLE:
      m_IlluminanceEnabled = false;
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
  else
  {
#if QRETRO_HAVE_SENSORS
    switch (id)
    {
    case RETRO_SENSOR_ACCELEROMETER_X:
      return m_AccelerometerEnabled ?
        static_cast<float>(m_AccelerometerSensor.reading()->x()) : 0;
    case RETRO_SENSOR_ACCELEROMETER_Y:
      return m_AccelerometerEnabled ?
        static_cast<float>(m_AccelerometerSensor.reading()->y()) : 0;
    case RETRO_SENSOR_ACCELEROMETER_Z:
      return m_AccelerometerEnabled ?
        static_cast<float>(m_AccelerometerSensor.reading()->z()) : 0;
    case RETRO_SENSOR_GYROSCOPE_X:
      return m_GyroscopeEnabled ?
        static_cast<float>(m_GyroscopeSensor.reading()->x()) : 0;
    case RETRO_SENSOR_GYROSCOPE_Y:
      return m_GyroscopeEnabled ?
        static_cast<float>(m_GyroscopeSensor.reading()->y()) : 0;
    case RETRO_SENSOR_GYROSCOPE_Z:
      return m_GyroscopeEnabled ?
        static_cast<float>(m_GyroscopeSensor.reading()->z()) : 0;
    case RETRO_SENSOR_ILLUMINANCE:
      return m_IlluminanceEnabled ? m_IlluminanceValue : 0;
    default:
      return 0;
    }
#else
    return 0;
#endif
  }
}
