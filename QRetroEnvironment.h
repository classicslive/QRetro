#ifndef QRETRO_ENVIRONMENT_H
#define QRETRO_ENVIRONMENT_H

#include <stdint.h>

bool core_environment(unsigned cmd, void *data);
void core_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch);
void core_audio_sample(int16_t left, int16_t right);
size_t core_audio_sample_batch(const int16_t *data, size_t frames);

#endif
