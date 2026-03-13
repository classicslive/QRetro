#ifndef QRETRO_INPUT_BACKEND_H
#define QRETRO_INPUT_BACKEND_H

#include <QObject>
#include <libretro.h>

class QRetroInputJoypad;

class QRetroInputBackend : public QObject
{
  Q_OBJECT

public:
  explicit QRetroInputBackend(QObject *parent = nullptr)
    : QObject(parent) {}

  virtual ~QRetroInputBackend() = default;

  /**
   * Called once after construction. The backend receives a pointer to the
   * shared joypad array and its capacity. The backend does NOT own the array.
   */
  virtual void init(QRetroInputJoypad *joypads, unsigned maxUsers) = 0;

  /**
   * Called by QRetroInput::poll() before keyboard-macro processing.
   * Signal-driven backends (e.g. QGamepad) may leave this as a no-op.
   */
  virtual void poll() = 0;

  /**
   * Sets the rumble motor strength for a given port.
   * effect is RETRO_RUMBLE_STRONG (low-frequency) or RETRO_RUMBLE_WEAK
   * (high-frequency). strength ranges from 0 (off) to 0xFFFF (full).
   * Returns true if rumble was applied, false if unsupported.
   */
  virtual bool setRumble(unsigned port, retro_rumble_effect effect,
                         uint16_t strength) { Q_UNUSED(port) Q_UNUSED(effect) Q_UNUSED(strength) return false; }

  /**
   * Enables or disables a sensor (accelerometer, gyroscope, illuminance).
   * Returns true if the sensor was handled, false if unsupported.
   */
  virtual bool setSensorState(unsigned port, retro_sensor_action action,
                              unsigned rate) { Q_UNUSED(port) Q_UNUSED(action) Q_UNUSED(rate) return false; }

  /**
   * Returns the current value of a sensor axis for a given port.
   * id is one of the RETRO_SENSOR_* constants.
   */
  virtual float getSensorInput(unsigned port, unsigned id) { Q_UNUSED(port) Q_UNUSED(id) return 0.0f; }

  /**
   * Returns true if the backend currently has a live sensor feed for the
   * given port and axis id. Unlike cached flags, this reflects real-time
   * state — e.g. returns false if the gamepad is not connected, even if
   * the core previously called setSensorState(ENABLE).
   */
  virtual bool sensorActive(unsigned port, unsigned id) { Q_UNUSED(port) Q_UNUSED(id) return false; }

signals:
  void gamepadConnected(unsigned port);
  void gamepadDisconnected(unsigned port);
};

#endif
