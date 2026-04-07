#include "QRetroLed.h"

int QRetroLed::state(int index)
{
  auto led = m_Leds.find(index);
  return led == m_Leds.end() ? QRETRO_LED_DEFAULT_STATE : led->second;
}

void QRetroLed::setState(int index, int state)
{
  if (m_Leds[index] != state)
  {
    m_Leds[index] = state;
    emit changed(index, state);
  }
}
