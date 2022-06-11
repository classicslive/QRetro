#ifndef QRETRO_SENSORS_H
#define QRETRO_SENSORS_H

#include <QAccelerometer>
#include <QRotationSensor>

#include "libretro.h"

/**
 * A class managing the values of various external sensors the libretro API
 * defines.
 *
 * TODO: Rate limiting, multiport, illuminance (or spoofing), and testing
 */
class QRetroSensors
{
public:
  QRetroSensors();

  float getInput(unsigned port, unsigned id);
  bool setState(unsigned port, retro_sensor_action action, unsigned rate);

private:
  QAccelerometer m_AccelerometerSensor;
  bool m_AccelerometerEnabled = false;
  unsigned m_AccelerometerRate = 0;

  QRotationSensor m_GyroscopeSensor;
  bool m_GyroscopeEnabled = false;
  unsigned m_GyroscopeRate = 0;

  bool m_IlluminanceEnabled = false;
  unsigned m_IlluminanceRate = 0;
  float m_IlluminanceValue = 0;
};

#endif
