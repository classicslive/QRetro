#ifndef QRETRO_AUDIO_H
#define QRETRO_AUDIO_H

#include <QElapsedTimer>
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

class QRetroAudio : public QObject
{
  Q_OBJECT

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

  /* Whether emulation should wait for the audio queue to drain. A core may
   * queue as much as it likes; only a queue that stops draining is a fault. */
  bool shouldStallEmulation(void);

  /* Drops the queue without touching the device. */
  void flush(void);

  /* Drops the queue and restarts the device. */
  void reset(void);

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
  float volume(void) const { return m_Volume; }

  void setMute(bool mute);
  bool isMuted(void) const { return m_Muted; }

  bool start(void);

signals:
  /* Emitted from the timing thread, as a retro_log_level plus text. */
  void logMessage(int level, const QString &msg);

private:
  void applyVolume(void);

  bool m_Enabled = true;
  float m_Volume = 1.0f;
  bool m_Muted = false;
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
  QElapsedTimer m_StallTimer;
  qint64 m_PlayedUSecs = 0;
  bool m_Restarted = false;
  double m_ResampleRatio = 1.0;
  double m_ResamplePhase = 0.0;
  sample_t m_ResamplePrev[2] = { 0, 0 };
  QByteArray m_ResampleScratch;
  double m_SampleRateBase = 0.0;
  int m_SampleRateBytesPerFrame = 0;
  double m_SampleRateCurrent = 0.0;
  double m_SampleRateMultiplier = 1.0;
  unsigned m_TargetSampleRate = 44100;
};

#endif
