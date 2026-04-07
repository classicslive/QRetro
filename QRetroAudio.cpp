#include <QtGlobal>
#if QRETRO_HAVE_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioSink>
#else
#include <QAudioOutput>
#endif
#endif
#include "QRetroAudio.h"

/**
 * The number of channels the implementation plays audio through by default.
 * As of now, libretro supports exactly two. If this is ever expanded, this
 * value should be made mutable.
 */
#define QRETRO_AUDIO_CHANNELS 2

QRetroAudio::QRetroAudio(void)
{
  m_SampleRateBase = m_TargetSampleRate;
  m_FramesPerSecond = 60.0;
  setTimingMultiplier(1.0);
}

QRetroAudio::QRetroAudio(double frequency, double core_fps)
{
  if (frequency < 1.0)
    frequency = m_TargetSampleRate;
  m_FramesPerSecond = core_fps;
  setTimingMultiplier(1.0);
}

QRetroAudio::QRetroAudio(double frequency, double core_fps, double emu_fps)
{
  double mult = emu_fps / core_fps;

  if (frequency < 1.0)
    frequency = m_TargetSampleRate;
  m_SampleRateBase = frequency;
  m_FramesPerSecond = emu_fps;

  /*
    TODO: We can't play samples faster than they arrive, but we need a better
      solution.
  */
  setTimingMultiplier(mult);

  qDebug("Sound variables\n"
         "  Frequency: %f\n"
         "  Core FPS: %f, Emulator FPS: %f\n"
         "  Speed Adjustment: %f",
    frequency, core_fps, emu_fps, mult);
}

QRetroAudio::~QRetroAudio(void)
{
#if QRETRO_HAVE_MULTIMEDIA
  if (m_AudioOutput)
  {
    m_AudioOutput->stop();
    delete m_AudioOutput;
  }
#endif
}

int QRetroAudio::framesInBuffer(void)
{
#if QRETRO_HAVE_MULTIMEDIA
  if (m_AudioOutput)
    return m_AudioBuffer.size() / m_SampleRateBytesPerFrame;
#endif
  return 0;
}

int QRetroAudio::excessFramesInBuffer(void)
{
  int result = framesInBuffer() - m_BufferFrames;
  return (result > 0) ? result : 0;
}

void QRetroAudio::playFrame(void)
{
#if QRETRO_HAVE_MULTIMEDIA
  if (m_AudioOutput && m_AudioDevice && m_AudioBuffer.size() >= m_SampleRateBytesPerFrame &&
      m_AudioOutput->bytesFree() >= m_SampleRateBytesPerFrame)
  {
    m_AudioDevice->write(m_AudioBuffer.data(), m_SampleRateBytesPerFrame);
    m_AudioBuffer.remove(0, m_SampleRateBytesPerFrame);
  }
#endif
}

void QRetroAudio::pushSamples(const sample_t *data, size_t frames)
{
  m_AudioBuffer.append(reinterpret_cast<const char *>(data),
    static_cast<int>(frames * QRETRO_AUDIO_CHANNELS * sizeof(sample_t)));
}

void QRetroAudio::setEnabled(bool v)
{
  m_Enabled = v;
  setVolume(v ? 1.0f : 0.0f);
}

void QRetroAudio::setVolume(float v)
{
#if QRETRO_HAVE_MULTIMEDIA
  if (m_AudioOutput)
    m_AudioOutput->setVolume(v);
#else
  Q_UNUSED(v)
#endif
}

void QRetroAudio::setTimingMultiplier(double mult)
{
  m_SampleRateCurrent = m_SampleRateBase * mult;
  m_SampleRateBytesPerFrame = static_cast<unsigned>(m_SampleRateBase / m_FramesPerSecond) *
                              sizeof(sample_t) * QRETRO_AUDIO_CHANNELS;
  /** @todo check - ensure amount is even (dolphin goes wacky without this) */
  m_SampleRateBytesPerFrame &= static_cast<unsigned>(~1);
  m_SampleRateMultiplier = mult;
}

bool QRetroAudio::start(void)
{
#if QRETRO_HAVE_MULTIMEDIA
  if (m_SampleRateCurrent > 0)
  {
    QAudioFormat format;

    format.setSampleRate(m_SampleRateCurrent);
    format.setChannelCount(QRETRO_AUDIO_CHANNELS);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    format.setSampleFormat(QAudioFormat::Int16);
#else
    format.setSampleType(QAudioFormat::SignedInt);
    format.setSampleSize(8 * sizeof(sample_t));
    format.setCodec("audio/pcm");
#endif

    /* Close and free audio handlers if they exist */
    if (m_AudioOutput)
    {
      m_AudioOutput->stop();
      delete m_AudioOutput;
      m_AudioOutput = nullptr;
    }
    if (m_AudioDevice)
    {
      m_AudioDevice->close();
      delete m_AudioDevice;
      m_AudioDevice = nullptr;
    }

    /* Fill the buffer with dummy data */
    m_AudioBuffer.clear();
    m_AudioBuffer.resize(m_BufferFrames * QRETRO_AUDIO_CHANNELS * sizeof(sample_t));
    m_AudioBuffer.fill(0);

    /* Start the new audio output */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_AudioOutput = new QAudioSink(format);
#else
    m_AudioOutput = new QAudioOutput(format);
#endif
    if (m_AudioOutput->error() == QAudio::NoError)
    {
      m_AudioDevice = m_AudioOutput->start();
      return true;
    }
  }
#endif

  return false;
}
