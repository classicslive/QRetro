#include <QAudioOutput>
#include "QRetroAudio.h"

/*
  The number of channels the implementation plays audio through by default.
  As of now, libretro supports exactly two. If this is ever expanded, this
  value should be made mutable.
*/
#define QRETRO_AUDIO_CHANNELS 2

QRetroAudio::QRetroAudio()
{
  m_AudioDevice = nullptr;
  m_AudioOutput = nullptr;
  m_SampleRateBase = 44100;
  m_FramesPerSecond = 60.0;
  setTimingMultiplier(1.0);
}

QRetroAudio::QRetroAudio(double frequency, double core_fps)
{
  m_AudioDevice = nullptr;
  m_AudioOutput = nullptr;
  m_SampleRateBase = frequency;
  m_FramesPerSecond = core_fps;
  setTimingMultiplier(1.0);
}

QRetroAudio::QRetroAudio(double frequency, double core_fps, double emu_fps)
{
  double mult = emu_fps / core_fps;

  m_AudioDevice = nullptr;
  m_AudioOutput = nullptr;
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
         frequency,
         core_fps, emu_fps,
         mult);
}

QRetroAudio::~QRetroAudio()
{
  m_AudioOutput->stop();
  delete m_AudioOutput;
}

int QRetroAudio::framesInBuffer()
{
  return m_AudioBuffer.size() / m_SampleRateBytesPerFrame;
}

int QRetroAudio::excessFramesInBuffer()
{
  return framesInBuffer() - m_BufferFrames;
}

void QRetroAudio::playFrame()
{
  if (m_AudioBuffer.size() >= m_SampleRateBytesPerFrame * m_BufferFrames)
  {
    m_AudioDevice->write(m_AudioBuffer.data(), m_SampleRateBytesPerFrame);
    m_AudioBuffer.remove(0, m_SampleRateBytesPerFrame);
  }
}

void QRetroAudio::pushSamples(const sample_t *data, size_t frames)
{
  m_AudioBuffer.append(
    reinterpret_cast<const char*>(data),
    static_cast<int>(frames * QRETRO_AUDIO_CHANNELS * sizeof(sample_t)));
}

void QRetroAudio::setTimingMultiplier(double mult)
{
  m_SampleRateCurrent = m_SampleRateBase * mult;
  m_SampleRateBytesPerFrame = static_cast<unsigned>(m_SampleRateBase / m_FramesPerSecond) * sizeof(sample_t) * QRETRO_AUDIO_CHANNELS;
  m_SampleRateBytesPerFrame &= static_cast<unsigned>(~1); // ensure amount is even (dolphin goes wacky without this)
  m_SampleRateMultiplier = mult;
}

void QRetroAudio::start()
{
  QAudioFormat format;
  format.setSampleRate(static_cast<int>(m_SampleRateCurrent));
  format.setChannelCount(QRETRO_AUDIO_CHANNELS);
  format.setSampleType(QAudioFormat::SignedInt);
  format.setSampleSize(16);
  format.setCodec("audio/pcm");

  m_AudioBuffer.clear();

  if (m_AudioOutput)
  {
    m_AudioOutput->stop();
    delete m_AudioOutput;
  }
  if (m_AudioDevice)
    delete m_AudioDevice;

  m_AudioOutput = new QAudioOutput(format);
  m_AudioDevice = m_AudioOutput->start();
}
