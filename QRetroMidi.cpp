#include <QDebug>
#include <QMidiIn.h>
#include <QMidiOut.h>

#include "QRetroMidi.h"

QRetroMidi::QRetroMidi()
{
  auto vals = QMidiIn::devices();

  if (vals.size() > 0)
  {
    m_In = new QMidiIn;
    m_InEnabled = m_In->connect(vals.firstKey());
    if (m_InEnabled)
    {
      qDebug() << "MIDI input connected to " << vals.first();
      m_In->start();
    }
  }

  vals = QMidiOut::devices();

  if (vals.size() > 0)
  {
    m_Out = new QMidiOut;
    m_OutEnabled = m_Out->connect(vals.firstKey());
    if (m_OutEnabled)
      qDebug() << "MIDI output connected to " << vals.first();
  }
}

bool QRetroMidi::inputEnabled(void) { return m_InEnabled; }

bool QRetroMidi::outputEnabled(void) { return m_OutEnabled; }

bool QRetroMidi::read(uint8_t *byte)
{
  if (m_OutEnabled)
  {
    m_Out->sendMsg(*byte);
    return true;
  }

  return false;
}

bool QRetroMidi::write(uint8_t byte, uint32_t delta_time)
{
  if (m_InEnabled)
  {
    emit m_In->midiEvent(byte, delta_time);
    return true;
  }

  return true;
}

bool QRetroMidi::flush(void)
{
  return true;
}
