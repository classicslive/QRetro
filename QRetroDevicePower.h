#ifndef QRETRO_DEVICEPOWER_H
#define QRETRO_DEVICEPOWER_H

#include "libretro.h"

class QRetroDevicePower
{
public:
  bool get(retro_device_power *power);

  bool spoofing(void) { return m_Spoofing; }
  retro_device_power spoofData(void) { return m_SpoofPower; }

  void setSpoofing(bool spoofing) { m_Spoofing = spoofing; }
  void setSpoofState(const retro_power_state state) { m_SpoofPower.state = state; }
  void setSpoofSeconds(const int seconds) { m_SpoofPower.seconds = seconds; }
  void setSpoofPercent(const int8_t percent) { m_SpoofPower.percent = percent; }
  void setSpoofData(const retro_device_power &power) { m_SpoofPower = power; }

private:
  bool m_Spoofing = false;
  retro_device_power m_SpoofPower = { RETRO_POWERSTATE_UNKNOWN,
                                      RETRO_POWERSTATE_NO_ESTIMATE,
                                      0 };
};

#endif
