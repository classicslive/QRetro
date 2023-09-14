#ifndef QRETRO_MICROPHONE_H
#define QRETRO_MICROPHONE_H

#include "libretro.h"

static_assert(RETRO_MICROPHONE_INTERFACE_VERSION == 1,
              "Microphone API version updated!");

/**
 * A factory for retro_microphone_t structs which the core is responsible for
 * keeping track of and eventually freeing.
 */
class QRetroMicrophone
{
public:
  retro_microphone_t* open(const retro_microphone_params_t *params);

  void close(retro_microphone_t *microphone);

  bool getParams(const retro_microphone_t *microphone,
                 retro_microphone_params_t *params);

  bool setState(retro_microphone_t *microphone, bool state);

  bool getState(const retro_microphone_t *microphone);

  int read(retro_microphone_t *microphone, int16_t *samples,
           size_t num_samples);

  unsigned interfaceVersion(void) { return m_Version; }

  void setInterfaceVersion(unsigned version) { m_Version = version; }

private:
  unsigned m_Version = RETRO_MICROPHONE_INTERFACE_VERSION;
};

#endif
