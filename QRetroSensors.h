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

  /* Fake accel (m/s²) */
  void setFakeAccelEnabled(bool v)             { m_FakeAccelEnabled = v; }
  void setFakeAccel(float x, float y, float z) { m_FakeAccel[0] = x; m_FakeAccel[1] = y; m_FakeAccel[2] = z; }

  /* Fake gyro (rad/s) */
  void setFakeGyroEnabled(bool v)              { m_FakeGyroEnabled = v; }
  void setFakeGyro(float x, float y, float z)  { m_FakeGyro[0] = x; m_FakeGyro[1] = y; m_FakeGyro[2] = z; }

  /* Fake illuminance (0–1) */
  void setFakeIllumEnabled(bool v)             { m_FakeIllumEnabled = v; }
  void setFakeIllum(float v)                   { m_FakeIllum = v; }

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

  /* Fake sensor state */
  bool  m_FakeAccelEnabled = false;
  float m_FakeAccel[3]     = {0, 0, 0};

  bool  m_FakeGyroEnabled  = false;
  float m_FakeGyro[3]      = {0, 0, 0};

  bool  m_FakeIllumEnabled = false;
  float m_FakeIllum        = 0;

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
