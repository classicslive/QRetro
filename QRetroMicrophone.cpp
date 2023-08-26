#include <QAudioInput>

#include "QRetroMicrophone.h"

struct qretro_microphone_t
{
  QAudioInput *input;
  QIODevice *device;
};

retro_microphone_t* QRetroMicrophone::open(const retro_microphone_params_t *params)
{
  auto mic = new qretro_microphone_t;
  QAudioFormat format;

  format.setSampleRate(params->rate);
  format.setChannelCount(1);
  format.setSampleSize(sizeof(int16_t));
  format.setCodec("audio/pcm");
  //mic->format.setByteOrder(QAudioFormat::LittleEndian);
  format.setSampleType(QAudioFormat::SignedInt);

  QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
  if (!info.isFormatSupported(format))
    format = info.nearestFormat(format);

  mic->input = new QAudioInput(format);
  mic->device = mic->input->start();

  if (!mic->device)
  {
    close(reinterpret_cast<retro_microphone_t*>(mic));
    return NULL;
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
    params->rate = mic->input->format().sampleRate();
    return true;
  }
}

bool QRetroMicrophone::setState(retro_microphone_t *microphone, bool state)
{
  auto mic = reinterpret_cast<qretro_microphone_t*>(microphone);

  if (!mic || !mic->input)
    return false;
  else if (state)
    mic->input->start(mic->device);
  else
    mic->input->stop();

  return true;
}

bool QRetroMicrophone::getState(const retro_microphone_t *microphone)
{
  auto mic = reinterpret_cast<const qretro_microphone_t*>(microphone);

  return (mic &&
          mic->input &&
          mic->input->state() == QAudio::ActiveState);
}

int QRetroMicrophone::read(retro_microphone_t *microphone, int16_t *samples,
                           size_t num_samples)
{
  auto mic = reinterpret_cast<qretro_microphone_t*>(microphone);

  return mic->device->read(reinterpret_cast<char*>(samples),
                           num_samples * sizeof(int16_t));
}
