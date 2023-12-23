#include <QMidiIn.h>
#include <QMidiOut.h>

#include "QRetroMidi.h"

QRetroMidi::QRetroMidi()
{
  auto vals = QMidiIn::devices();

  if (vals.size() > 0)
  {
    m_In = new QMidiIn;
    m_InEnabled = m_In->connect(vals.first());
    m_In->start();
  }

  vals = QMidiOut::devices();

  if (vals.size() > 0)
  {
    m_Out = new QMidiOut;
    m_OutEnabled = m_Out->connect(vals.first());
  }
}

bool QRetroMidi::inputEnabled(void) { return m_InEnabled; }

bool QRetroMidi::outputEnabled(void) { return m_OutEnabled; }

bool QRetroMidi::read(uint8_t *byte)
{
  Q_UNUSED(byte)
  return false;
}

bool QRetroMidi::write(uint8_t byte, uint32_t delta_time)
{
  Q_UNUSED(byte)
  Q_UNUSED(delta_time)
  emit m_In->midiEvent(byte, delta_time);

  return true;
}

bool QRetroMidi::flush(void)
{
  return true;
}
