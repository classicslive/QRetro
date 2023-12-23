#ifndef QRETRO_MIDI_H
#define QRETRO_MIDI_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QMidiIn);
QT_FORWARD_DECLARE_CLASS(QMidiOut);

class QRetroMidi
{
public:
  QRetroMidi();

  bool inputEnabled(void);

  bool outputEnabled(void);

  bool read(uint8_t *byte);

  bool write(uint8_t byte, uint32_t delta_time);

  bool flush(void);

private:
  QMidiIn *m_In;
  bool m_InEnabled = false;
  QByteArray m_InStream;

  QMidiOut *m_Out;
  bool m_OutEnabled = false;
  QByteArray m_OutStream;
};

#endif
