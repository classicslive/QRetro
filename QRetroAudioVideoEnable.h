#ifndef QRETRO_AUDIOVIDEOENABLE_H
#define QRETRO_AUDIOVIDEOENABLE_H

#include "libretro.h"

/**
 * A class used for callback RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE that
 * reports to the core whether or not the frontend wants certain data.
 */
class QRetroAudioVideoEnable
{
public:
  QRetroAudioVideoEnable(bool audio = true, bool video = true, bool ufss = false, bool hda = false)
  {
    setEnableAudio(audio);
    setEnableVideo(video);
    setUseFastSavestates(ufss);
    setHardDisableAudio(hda);
  }

  void setEnableAudio(bool e)
  {
    e ? m_flags |= RETRO_AV_ENABLE_AUDIO : m_flags &= ~RETRO_AV_ENABLE_AUDIO;
  }
  bool enableAudio(void) { return m_flags & RETRO_AV_ENABLE_AUDIO; }

  void setEnableVideo(bool e)
  {
    e ? m_flags |= RETRO_AV_ENABLE_VIDEO : m_flags &= ~RETRO_AV_ENABLE_VIDEO;
  }
  bool enableVideo(void) { return m_flags & RETRO_AV_ENABLE_VIDEO; }

  void setUseFastSavestates(bool e)
  {
    e ? m_flags |= RETRO_AV_ENABLE_FAST_SAVESTATES : m_flags &= ~RETRO_AV_ENABLE_FAST_SAVESTATES;
  }
  bool useFastSavestates(void) { return m_flags & RETRO_AV_ENABLE_FAST_SAVESTATES; }

  void setHardDisableAudio(bool e)
  {
    e ? m_flags |= RETRO_AV_ENABLE_HARD_DISABLE_AUDIO
      : m_flags &= ~RETRO_AV_ENABLE_HARD_DISABLE_AUDIO;
  }
  bool hardDisableAudio(void) { return m_flags & RETRO_AV_ENABLE_HARD_DISABLE_AUDIO; }

  int getFlags(void) { return m_flags; }
  void setFlags(int flags) { m_flags = flags; }

private:
  int m_flags = 0;
};

#endif
