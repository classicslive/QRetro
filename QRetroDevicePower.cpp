#if QRETRO_HAVE_SYSTEMINFO
#include <QSystemBatteryInfo>

QTM_USE_NAMESPACE
#endif

#include "QRetroDevicePower.h"

bool QRetroDevicePower::get(retro_device_power *power)
{
  /* On null pointer, just report support state */
  if (!power)
    return true;
  else if (m_Spoofing)
  {
    *power = m_SpoofPower;
    return true;
  }

#if QRETRO_HAVE_SYSTEMINFO
  QSystemBatteryInfo battery;

  /** Set device power state */
  auto state = battery.chargingState();
  auto type = battery.chargingType();

  if (state == QSystemBatteryInfo::ChargingError)
    return false;
  else if (state == QSystemBatteryInfo::BatteryFull)
    power->state = RETRO_POWERSTATE_CHARGED;
  else if (state == QSystemBatteryInfo::NotCharging)
  {
    if (type == QSystemBatteryInfo::BatteryUnknown)
      power->state = RETRO_POWERSTATE_PLUGGED_IN;
    else
      power->state = RETRO_POWERSTATE_DISCHARGING;
  }
  else if (state == QSystemBatteryInfo::Charging)
    power->state = RETRO_POWERSTATE_CHARGING;

  /**
   * Set device power seconds.
   * Will use default value if not running off battery.
   */
  if (power->state != RETRO_POWERSTATE_DISCHARGING)
    power->seconds = RETRO_POWERSTATE_NO_ESTIMATE;
  else
  {
    auto remain = battery.remainingCapacity();
    auto flow = battery.currentFlow();

    power->seconds = static_cast<int>(remain / flow * 3600);
  }

  /** Set device power percent */
  power->percent = static_cast<int8_t>(battery.remainingCapacityPercent());

  return true;
#else
  return false;
#endif
}
