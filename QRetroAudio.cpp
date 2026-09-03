#include <QtGlobal>
#if QRETRO_HAVE_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDevice>
#include <QAudioSink>
#include <QMediaDevices>
#else
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#endif
#endif
#include "QRetroAudio.h"
#include "libretro.h"

#include <cmath>

/**
 * The number of channels the implementation plays audio through by default.
 * As of now, libretro supports exactly two. If this is ever expanded, this
 * value should be made mutable.
 */
#define QRETRO_AUDIO_CHANNELS 2

/* How long the queue may fail to drain before the device is presumed stuck. */
#define QRETRO_AUDIO_NO_DRAIN_MS 250

/**
 * Linearly resamples interleaved frames, reading `ratio` input frames per output
 * frame. `phase` carries the fractional read position between calls so blocks
 * join without a seam. Returns the number of frames written.
 */
static size_t qretro_resample(const sample_t *in, size_t in_frames, sample_t *out, size_t out_max,
  double ratio, double *phase, sample_t *prev)
{
  size_t written = 0;
  double pos = *phase;
  unsigned c;

  while (written < out_max && pos < static_cast<double>(in_frames))
  {
    const size_t i = static_cast<size_t>(pos);
    const double frac = pos - static_cast<double>(i);

    for (c = 0; c < QRETRO_AUDIO_CHANNELS; c++)
    {
      /* Position 0 sits on the frame carried over from the last call, so blocks
       * join without a seam. */
      const double a = i ? in[(i - 1) * QRETRO_AUDIO_CHANNELS + c] : prev[c];
      const double b = in[i * QRETRO_AUDIO_CHANNELS + c];

      out[written * QRETRO_AUDIO_CHANNELS + c] = static_cast<sample_t>(a + (b - a) * frac);
    }
    written++;
    pos += ratio;
  }

  if (in_frames)
  {
    for (c = 0; c < QRETRO_AUDIO_CHANNELS; c++)
      prev[c] = in[(in_frames - 1) * QRETRO_AUDIO_CHANNELS + c];
    pos -= static_cast<double>(in_frames);
  }
  *phase = pos < 0.0 ? 0.0 : pos;

  return written;
}

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
  /* Without a device nothing can drain the queue, so report it empty rather than
   * let emulation be held on audio that will never play. */
  if (m_AudioOutput && m_AudioDevice)
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

bool QRetroAudio::shouldStallEmulation(void)
{
#if QRETRO_HAVE_MULTIMEDIA
  int frames = excessFramesInBuffer();
  qint64 played;

  if (!frames)
  {
    m_StallTimer.invalidate();
    m_Restarted = false;
    return false;
  }

  /* The device is the only thing that empties the queue, so if it has not played
   * anything for a while it has stopped, however full the queue looks. */
  played = m_AudioOutput ? m_AudioOutput->processedUSecs() : 0;

  if (!m_StallTimer.isValid() || played != m_PlayedUSecs)
  {
    m_PlayedUSecs = played;
    m_StallTimer.start();
  }
  else if (m_StallTimer.hasExpired(QRETRO_AUDIO_NO_DRAIN_MS))
  {
    emit logMessage(
      RETRO_LOG_WARN, QString("Audio device stopped after playing %1 ms, %2 frames queued.")
                        .arg(played / 1000)
                        .arg(frames));
    m_StallTimer.invalidate();

    /* Never hold emulation on a device that is not playing. Try once to get it
     * back; after that just keep the queue clear. */
    flush();
    if (!m_Restarted)
    {
      m_Restarted = true;
      start();
    }
    return false;
  }

  return true;
#else
  return false;
#endif
}

void QRetroAudio::flush(void)
{
  m_AudioBuffer.clear();
  m_AudioBuffer.resize(m_BufferFrames * QRETRO_AUDIO_CHANNELS * sizeof(sample_t));
  m_AudioBuffer.fill(0);
}

void QRetroAudio::reset(void)
{
#if QRETRO_HAVE_MULTIMEDIA
  if (!m_AudioOutput)
    return;

  m_AudioOutput->reset();
  flush();
  m_AudioDevice = m_AudioOutput->start();
  applyVolume();
#endif
}

void QRetroAudio::pushSamples(const sample_t *data, size_t frames)
{
  if (m_ResampleRatio != 1.0)
  {
    const size_t max = static_cast<size_t>(frames / m_ResampleRatio) + 2;
    size_t written;

    m_ResampleScratch.resize(static_cast<int>(max * QRETRO_AUDIO_CHANNELS * sizeof(sample_t)));
    written = qretro_resample(data, frames, reinterpret_cast<sample_t *>(m_ResampleScratch.data()),
      max, m_ResampleRatio, &m_ResamplePhase, m_ResamplePrev);
    m_AudioBuffer.append(m_ResampleScratch.constData(),
      static_cast<int>(written * QRETRO_AUDIO_CHANNELS * sizeof(sample_t)));
    return;
  }

  m_AudioBuffer.append(reinterpret_cast<const char *>(data),
    static_cast<int>(frames * QRETRO_AUDIO_CHANNELS * sizeof(sample_t)));
}

void QRetroAudio::setEnabled(bool v)
{
  if (v == m_Enabled)
    return;
  m_Enabled = v;
  applyVolume();
}

void QRetroAudio::setVolume(float v)
{
  if (v == m_Volume)
    return;
  m_Volume = v;
  applyVolume();
}

void QRetroAudio::setMute(bool mute)
{
  if (mute == m_Muted)
    return;
  m_Muted = mute;
  applyVolume();
}

void QRetroAudio::applyVolume(void)
{
#if QRETRO_HAVE_MULTIMEDIA
  if (m_AudioOutput)
    m_AudioOutput->setVolume((m_Enabled && !m_Muted) ? m_Volume : 0.0f);
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

    emit logMessage(RETRO_LOG_INFO, QString("Audio: core %1 Hz at %2 fps, timing x%3")
                                      .arg(m_SampleRateBase)
                                      .arg(m_FramesPerSecond)
                                      .arg(m_SampleRateMultiplier));

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
    /* The device belongs to the sink, which just took it with it. */
    m_AudioDevice = nullptr;

    /* Fill the buffer with dummy data */
    m_AudioBuffer.clear();
    m_AudioBuffer.resize(m_BufferFrames * QRETRO_AUDIO_CHANNELS * sizeof(sample_t));
    m_AudioBuffer.fill(0);

    /* Cores ask for rates the device may not take, and an
     * unsupported format opens without an error but never plays. */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QAudioDevice info = QMediaDevices::defaultAudioOutput();
    const QString name = info.description();
#else
    const QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    const QString name = info.deviceName();
#endif
    const bool supported = info.isFormatSupported(format);

    emit logMessage(supported ? RETRO_LOG_INFO : RETRO_LOG_WARN,
      QString("Audio: requesting %1 Hz, %2 ch on \"%3\" (supported: %4, prefers %5 Hz)")
        .arg(format.sampleRate())
        .arg(format.channelCount())
        .arg(name)
        .arg(supported ? "yes" : "NO")
        .arg(info.preferredFormat().sampleRate()));

    m_ResampleRatio = 1.0;
    m_ResamplePhase = 0.0;

    if (!supported)
    {
      const int rate = info.preferredFormat().sampleRate();

      if (rate > 0)
      {
        emit logMessage(RETRO_LOG_INFO,
          QString("Audio: converting %1 Hz to %2 Hz").arg(format.sampleRate()).arg(rate));
        m_ResampleRatio = m_SampleRateCurrent / rate;
        format.setSampleRate(rate);
      }
    }

    /* The queue holds device-rate samples now, so pace the drain off that. */
    m_SampleRateBytesPerFrame = static_cast<int>(format.sampleRate() / m_FramesPerSecond) *
                                sizeof(sample_t) * QRETRO_AUDIO_CHANNELS;
    m_SampleRateBytesPerFrame &= ~1;

    /* Start the new audio output */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_AudioOutput = new QAudioSink(info, format);
#else
    m_AudioOutput = new QAudioOutput(info, format);
#endif
    m_AudioDevice = m_AudioOutput->start();
    applyVolume();

    if (m_AudioDevice)
    {
      emit logMessage(RETRO_LOG_INFO, QString("Audio: started, state %1, %2 bytes free")
                                        .arg(static_cast<int>(m_AudioOutput->state()))
                                        .arg(static_cast<qint64>(m_AudioOutput->bytesFree())));
      return true;
    }

    /* The rate the core wants is not one this device takes */
    emit logMessage(RETRO_LOG_ERROR,
      QString("Audio: device would not open at %1 Hz, error %2. Continuing without sound.")
        .arg(format.sampleRate())
        .arg(static_cast<int>(m_AudioOutput->error())));
  }
#endif

  return false;
}
