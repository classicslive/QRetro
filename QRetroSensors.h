#ifndef QRETRO_SENSORS_H
#define QRETRO_SENSORS_H

#if QRETRO_HAVE_SENSORS
#include <QAccelerometer>
#include <QLightSensor>
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

  /* Spoof accel (m/s²) */
  void setSpoofAccelEnabled(bool v)             { m_SpoofAccelEnabled = v; }
  void setSpoofAccel(float x, float y, float z) { m_SpoofAccel[0] = x; m_SpoofAccel[1] = y; m_SpoofAccel[2] = z; }

  /* Spoof gyro (rad/s) */
  void setSpoofGyroEnabled(bool v)              { m_SpoofGyroEnabled = v; }
  void setSpoofGyro(float x, float y, float z)  { m_SpoofGyro[0] = x; m_SpoofGyro[1] = y; m_SpoofGyro[2] = z; }

  /* Spoof illuminance (lux) */
  void setSpoofIllumEnabled(bool v)             { m_SpoofIllumEnabled = v; }
  void setSpoofIllum(float v)                   { m_SpoofIllum = v; }

  /* Core-requested state (set via setState()) */
  bool     accelEnabled() const { return m_AccelerometerEnabled; }
  unsigned accelRate()    const { return m_AccelerometerRate; }
  bool     gyroEnabled()  const { return m_GyroscopeEnabled; }
  unsigned gyroRate()     const { return m_GyroscopeRate; }
  bool     illumEnabled() const { return m_IlluminanceEnabled; }
  unsigned illumRate()    const { return m_IlluminanceRate; }

  /* Whether the core has called getInput() for each individual axis */
  bool accelXHasBeenRead() const { return m_AccelXHasBeenRead; }
  bool accelYHasBeenRead() const { return m_AccelYHasBeenRead; }
  bool accelZHasBeenRead() const { return m_AccelZHasBeenRead; }
  bool gyroXHasBeenRead()  const { return m_GyroXHasBeenRead;  }
  bool gyroYHasBeenRead()  const { return m_GyroYHasBeenRead;  }
  bool gyroZHasBeenRead()  const { return m_GyroZHasBeenRead;  }
  bool illumHasBeenRead()  const { return m_IllumHasBeenRead;  }

private:
#if QRETRO_HAVE_SENSORS
  QAccelerometer m_AccelerometerSensor;
  bool m_AccelerometerAvailable = false;
#endif
  bool m_AccelerometerEnabled = false;
  unsigned m_AccelerometerRate = 0;

#if QRETRO_HAVE_SENSORS
  QRotationSensor m_GyroscopeSensor;
  bool m_GyroscopeAvailable = false;
#endif
  bool m_GyroscopeEnabled = false;
  unsigned m_GyroscopeRate = 0;

#if QRETRO_HAVE_SENSORS
  QLightSensor m_IlluminanceSensor;
  bool m_IlluminanceAvailable = false;
#endif
  bool m_IlluminanceEnabled = false;
  unsigned m_IlluminanceRate = 0;
  float m_IlluminanceValue = 0;

  /* Spoof sensor state */
  bool  m_SpoofAccelEnabled = false;
  float m_SpoofAccel[3]     = {0, 0, 0};

  bool  m_SpoofGyroEnabled  = false;
  float m_SpoofGyro[3]      = {0, 0, 0};

  bool  m_SpoofIllumEnabled = false;
  float m_SpoofIllum        = 0;

  /* Read-tracking flags (set on first getInput() call per axis) */
  bool m_AccelXHasBeenRead = false;
  bool m_AccelYHasBeenRead = false;
  bool m_AccelZHasBeenRead = false;
  bool m_GyroXHasBeenRead  = false;
  bool m_GyroYHasBeenRead  = false;
  bool m_GyroZHasBeenRead  = false;
  bool m_IllumHasBeenRead  = false;
};

#endif
