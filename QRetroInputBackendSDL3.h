#ifndef QRETRO_INPUT_BACKEND_SDL3_H
#define QRETRO_INPUT_BACKEND_SDL3_H

#if QRETRO_HAVE_SDL3

#include <SDL3/SDL.h>

#include "QRetroInputBackend.h"
#include "QRetroInput.h"

class QRetroInputBackendSDL3 : public QRetroInputBackend
{
  Q_OBJECT

public:
  explicit QRetroInputBackendSDL3(QObject *parent = nullptr);
  ~QRetroInputBackendSDL3() override;

  void init(QRetroInputJoypad *joypads, unsigned maxUsers) override;
  void poll() override;

  bool  setRumble(unsigned port, retro_rumble_effect effect, uint16_t strength) override;
  bool  setSensorState(unsigned port, retro_sensor_action action, unsigned rate) override;
  bool  sensorActive(unsigned port, unsigned id) override;

private:
  QRetroInputJoypad *m_Joypads  = nullptr;
  unsigned           m_MaxUsers = 0;

  // SDL3 gamepad handles; index == port. nullptr means slot is empty.
  SDL_Gamepad    *m_Gamepads[QRETRO_INPUT_DEFAULT_MAX_JOYPADS]   = {};

  // Instance IDs for mapping SDL events back to port numbers.
  SDL_JoystickID  m_InstanceIds[QRETRO_INPUT_DEFAULT_MAX_JOYPADS] = {};

  void openGamepad(unsigned port, SDL_JoystickID instanceId);
  void closeGamepad(SDL_JoystickID instanceId);
  void updatePort(unsigned port);

  // SDL3 axes return Sint16 [-32768, 32767]; libretro uses [-32767, 32767].
  // Clamp the minimum so the ranges are symmetric.
  static int16_t sdl2lr_axis(Sint16 v)
  {
    return (v == -32768) ? -32767 : static_cast<int16_t>(v);
  }

  // SDL3 triggers return Sint16 [0, 32767], matching libretro's analog button range.
  static int16_t sdl2lr_trigger(Sint16 v)
  {
    return static_cast<int16_t>(v);
  }
};

#endif
#endif
