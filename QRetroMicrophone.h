#ifndef QRETRO_MICROPHONE_H
#define QRETRO_MICROPHONE_H

#include <QObject>

#include "libretro.h"

/**
 * A factory for retro_microphone_t structs which the core is responsible for
 * keeping track of and eventually freeing.
 */
class QRetroMicrophone
{
  Q_OBJECT

public:
  QRetroMicrophone();

  retro_microphone_t* open(const retro_microphone_params_t *params);

  void close(retro_microphone_t *microphone);

  bool getParams(const retro_microphone_t *microphone,
                 retro_microphone_params_t *params);

  bool setState(retro_microphone_t *microphone, bool state);

  bool getState(retro_microphone_t *microphone);

  int read(retro_microphone_t *microphone, int16_t *samples,
           size_t num_samples);
};

#endif
