#ifndef QRETRO_LED_H
#define QRETRO_LED_H

#include <map>

#define QRETRO_LED_DEFAULT_STATE 0

class QRetroLed
{
public:
  /**
   * @brief Get the state of the LED with the specified index.
   * @param index The index of the LED.
   * @return State of the LED. If not found, returns QRETRO_LED_DEFAULT_STATE.
   */
  int state(int index)
  {
    auto led = m_Leds.find(index);
    return led == m_Leds.end() ? QRETRO_LED_DEFAULT_STATE : led->second;
  }

  /**
   * @brief Set the state of the LED with the specified index.
   * @param index The index of the LED.
   * @param state The new state to set for the LED.
   * @note If an LED with the index does not exist, it will be created.
   */
  void setState(int index, int state) { m_Leds[index] = state; }

private:
  std::map<int, int> m_Leds;
};

#endif
