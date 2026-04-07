#include <QtGlobal>
#if QRETRO_HAVE_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDevice>
#include <QAudioSource>
#include <QMediaDevices>
#else
#include <QAudioDeviceInfo>
#include <QAudioInput>
#endif
#endif

#include "QRetroMicrophone.h"

#if QRETRO_HAVE_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
struct qretro_microphone_t
{
  QAudioSource *input = nullptr;
  QIODevice *device = nullptr;
};
#else
struct qretro_microphone_t
{
  QAudioInput *input = nullptr;
  QIODevice *device = nullptr;
};
#endif
#endif

typedef int16_t qretro_mic_sample_t;

#define QRETRO_MIC_BUFFER_FRAMES 10

retro_microphone_t *QRetroMicrophone::open(const retro_microphone_params_t *params)
{
#if QRETRO_HAVE_MULTIMEDIA
  auto mic = new qretro_microphone_t;
  QAudioFormat format;

  format.setSampleRate(static_cast<int>(params->rate));
  format.setChannelCount(1);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  format.setSampleFormat(QAudioFormat::Int16);
  QAudioDevice info = QMediaDevices::defaultAudioInput();
  mic->input = new QAudioSource(info, format);
#else
  format.setSampleSize(sizeof(qretro_mic_sample_t) * 8);
  format.setCodec("audio/pcm");
  format.setSampleType(QAudioFormat::SignedInt);
  QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
  if (!info.isFormatSupported(format))
    format = info.nearestFormat(format);
  mic->input = new QAudioInput(format);
#endif

  if (!mic->input || mic->input->error())
  {
    delete mic;
    return nullptr;
  }

  auto *handle = reinterpret_cast<retro_microphone_t *>(mic);
  m_OpenMics.append(handle);
  return handle;
#else
  Q_UNUSED(params)
  return nullptr;
#endif
}

QString QRetroMicrophone::deviceName() const
{
#if QRETRO_HAVE_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  return QMediaDevices::defaultAudioInput().description();
#else
  return QAudioDeviceInfo::defaultInputDevice().deviceName();
#endif
#else
  return {};
#endif
}

void QRetroMicrophone::close(retro_microphone_t *microphone)
{
#if QRETRO_HAVE_MULTIMEDIA
  m_OpenMics.removeOne(microphone);
  auto mic = reinterpret_cast<qretro_microphone_t *>(microphone);

  if (mic)
  {
    mic->input->stop();
    delete mic->input;
    delete mic->device;
    delete mic;
  }
#else
  Q_UNUSED(microphone)
#endif
}

bool QRetroMicrophone::getParams(
  const retro_microphone_t *microphone, retro_microphone_params_t *params)
{
#if QRETRO_HAVE_MULTIMEDIA
  auto mic = reinterpret_cast<const qretro_microphone_t *>(microphone);

  if (!mic || !mic->input)
    return false;
  else
  {
    params->rate = static_cast<unsigned>(mic->input->format().sampleRate());
    return true;
  }
#else
  Q_UNUSED(microphone)
  Q_UNUSED(params)
  return false;
#endif
}

bool QRetroMicrophone::setState(retro_microphone_t *microphone, bool state)
{
#if QRETRO_HAVE_MULTIMEDIA
  auto mic = reinterpret_cast<qretro_microphone_t *>(microphone);

  if (!mic || !mic->input)
    return false;
  else if (state)
  {
    mic->input->setBufferSize(mic->input->format().sampleRate() / 60 * QRETRO_MIC_BUFFER_FRAMES);
    mic->device = mic->input->start();
  }
  else
    mic->input->stop();

  return true;
#else
  Q_UNUSED(microphone)
  Q_UNUSED(state)
  return false;
#endif
}

bool QRetroMicrophone::getState(const retro_microphone_t *microphone)
{
#if QRETRO_HAVE_MULTIMEDIA
  auto mic = reinterpret_cast<const qretro_microphone_t *>(microphone);

  return mic && mic->input &&
         (mic->input->state() == QAudio::ActiveState || mic->input->state() == QAudio::IdleState);
#else
  Q_UNUSED(microphone)
  return false;
#endif
}

int QRetroMicrophone::read(retro_microphone_t *microphone, int16_t *samples, size_t num_samples)
{
#if QRETRO_HAVE_MULTIMEDIA
  auto mic = reinterpret_cast<qretro_microphone_t *>(microphone);

  if (!(mic->input->state() == QAudio::ActiveState || mic->input->state() == QAudio::IdleState))
    return -1;

  /* Clean the buffer with empty samples */
  memset(samples, 0, num_samples * sizeof(qretro_mic_sample_t));

  auto bytes_read = static_cast<unsigned>(mic->device->read(reinterpret_cast<char *>(samples),
    static_cast<qint64>(num_samples * sizeof(qretro_mic_sample_t))));

  /* Return number of samples, not number of bytes */
  return bytes_read / sizeof(qretro_mic_sample_t);
#else
  Q_UNUSED(microphone)
  Q_UNUSED(samples)
  Q_UNUSED(num_samples)
  return -1;
#endif
}
