#ifndef QRETRO_AUDIO_H
#define QRETRO_AUDIO_H

#include <QWidget>

typedef int16_t sample_t;

QT_FORWARD_DECLARE_CLASS(QAudioOutput);

class QRetroAudio
{
public:
  QRetroAudio(void);
  QRetroAudio(double frequency, double core_fps);
  QRetroAudio(double frequency, double core_fps, double emu_fps);
  ~QRetroAudio(void);

  /**
   * Returns the number of unplayed audio frames stored in the audio buffer.
   */
  int framesInBuffer(void);

  /**
   * Returns the number of unplayed audio frames stored in the audio buffer,
   * minus the number of required buffer frames. All excess frames should be
   * played before emulation continues.
   */
  int excessFramesInBuffer(void);

  void playFrame(void);

  void pushSamples(const sample_t *data, size_t frames);

  /*
    The number of pre-rendered audio frames that must be stored in the audio
    buffer before popping a frame. Raising this number can reduce crackling at
    the expense of increased audio latency.
  */
  void setBufferFrames(unsigned frames) { m_BufferFrames = frames; }

  void setTimingMultiplier(double mult);

  bool start(void);

private:
  QAudioOutput *m_AudioOutput = nullptr;
  QByteArray    m_AudioBuffer;
  QIODevice    *m_AudioDevice = nullptr;

  double m_FramesPerSecond;
  unsigned m_BufferFrames = 1;
  double m_SampleRateBase;
  int m_SampleRateBytesPerFrame;
  double m_SampleRateCurrent;
  double m_SampleRateMultiplier;
};

#endif
