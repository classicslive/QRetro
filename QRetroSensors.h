#ifndef QRETRO_SENSORS_H
#define QRETRO_SENSORS_H

#if QRETRO_HAVE_SENSORS
#include <QAccelerometer>
#include <QAmbientLightSensor>
#include <QRotationSensor>
#endif

#include "libretro.h"

/**
 * A class managing the values of various external sensors the libretro API
 * defines.
 * @todo Rate limiting, multiport, illuminance, spoofing, and testing
 */
class QRetroSensors
{
public:
  QRetroSensors();

  float getInput(unsigned port, unsigned id);
  bool setState(unsigned port, retro_sensor_action action, unsigned rate);

private:
#if QRETRO_HAVE_SENSORS
  QAccelerometer m_AccelerometerSensor;
#endif
  bool m_AccelerometerEnabled = false;
  unsigned m_AccelerometerRate = 0;

#if QRETRO_HAVE_SENSORS
  QRotationSensor m_GyroscopeSensor;
#endif
  bool m_GyroscopeEnabled = false;
  unsigned m_GyroscopeRate = 0;

#if QRETRO_HAVE_SENSORS
  QAmbientLightSensor m_IlluminanceSensor;
#endif
  bool m_IlluminanceEnabled = false;
  unsigned m_IlluminanceRate = 0;
  float m_IlluminanceValue = 0;
};

#endif
