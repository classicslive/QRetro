#include <QAudioInput>

#include "QRetroMicrophone.h"

struct qretro_microphone_t
{
  QAudioInput *input;
  QIODevice *device;
};

typedef int16_t qretro_sample_t;

retro_microphone_t* QRetroMicrophone::open(const retro_microphone_params_t *params)
{
  auto mic = new qretro_microphone_t;
  QAudioFormat format;

  format.setSampleRate(static_cast<int>(params->rate));
  format.setChannelCount(1);
  format.setSampleSize(sizeof(qretro_sample_t) * 8);
  format.setCodec("audio/pcm");
  format.setSampleType(QAudioFormat::SignedInt);

  QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
  if (!info.isFormatSupported(format))
    format = info.nearestFormat(format);

  mic->input = new QAudioInput(format);
  mic->device = mic->input->start();

  if (!mic->device)
  {
    close(reinterpret_cast<retro_microphone_t*>(mic));
    return nullptr;
  }
  else
    return reinterpret_cast<retro_microphone_t*>(mic);
}

void QRetroMicrophone::close(retro_microphone_t *microphone)
{
  auto mic = reinterpret_cast<qretro_microphone_t*>(microphone);

  if (mic)
  {
    mic->input->stop();
    delete mic->input;
    delete mic->device;
    delete mic;
  }
}

bool QRetroMicrophone::getParams(const retro_microphone_t *microphone,
                                 retro_microphone_params_t *params)
{
  auto mic = reinterpret_cast<const qretro_microphone_t*>(microphone);

  if (!mic || !mic->input)
    return false;
  else
  {
    params->rate = static_cast<unsigned>(mic->input->format().sampleRate());
    return true;
  }
}

bool QRetroMicrophone::setState(retro_microphone_t *microphone, bool state)
{
  auto mic = reinterpret_cast<qretro_microphone_t*>(microphone);

  if (!mic || !mic->input)
    return false;
  else if (state)
    mic->device = mic->input->start();
  else
    mic->input->stop();

  return true;
}

bool QRetroMicrophone::getState(const retro_microphone_t *microphone)
{
  auto mic = reinterpret_cast<const qretro_microphone_t*>(microphone);

  return mic &&
         mic->input &&
         (mic->input->state() == QAudio::ActiveState ||
          mic->input->state() == QAudio::IdleState);
}

int QRetroMicrophone::read(retro_microphone_t *microphone, int16_t *samples,
                           size_t num_samples)
{
  auto mic = reinterpret_cast<qretro_microphone_t*>(microphone);

  if (!(mic->input->state() == QAudio::ActiveState ||
        mic->input->state() == QAudio::IdleState))
    return -1;

  /* Clean the buffer with empty samples */
  memset(samples, 0, num_samples * sizeof(qretro_sample_t));

  auto read = static_cast<unsigned>(mic->device->read(reinterpret_cast<char*>(samples),
                                    static_cast<qint64>(num_samples * sizeof(qretro_sample_t))));

  /* Return number of samples, not number of bytes */
  return read / sizeof(qretro_sample_t);
}
