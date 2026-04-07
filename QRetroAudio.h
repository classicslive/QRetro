#ifndef QRETRO_AUDIO_H
#define QRETRO_AUDIO_H

#include <QWidget>
#include <QtGlobal>

typedef int16_t sample_t;

#if QRETRO_HAVE_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QT_FORWARD_DECLARE_CLASS(QAudioSink);
#else
QT_FORWARD_DECLARE_CLASS(QAudioOutput);
#endif
#endif

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

  double baseSampleRate(void) { return m_SampleRateBase; }
  double sampleRate(void) { return m_SampleRateCurrent; }

  unsigned targetSampleRate(void) { return m_TargetSampleRate; }
  void setTargetSampleRate(unsigned rate) { m_TargetSampleRate = rate; }

  /*
    The number of pre-rendered audio frames that must be stored in the audio
    buffer before popping a frame. Raising this number can reduce crackling at
    the expense of increased audio latency.
  */
  void setBufferFrames(unsigned frames) { m_BufferFrames = frames; }

  void setEnabled(bool v);
  bool isEnabled(void) const { return m_Enabled; }

  void setTimingMultiplier(double mult);
  void setVolume(float v);

  bool start(void);

private:
  bool m_Enabled = true;
#if QRETRO_HAVE_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  QAudioSink *m_AudioOutput = nullptr;
#else
  QAudioOutput *m_AudioOutput = nullptr;
#endif
  QByteArray m_AudioBuffer;
  QIODevice *m_AudioDevice = nullptr;
#endif

  double m_FramesPerSecond = 60.0;
  unsigned m_BufferFrames = 1;
  double m_SampleRateBase = 0.0;
  int m_SampleRateBytesPerFrame = 0;
  double m_SampleRateCurrent = 0.0;
  double m_SampleRateMultiplier = 1.0;
  unsigned m_TargetSampleRate = 44100;
};

#endif
