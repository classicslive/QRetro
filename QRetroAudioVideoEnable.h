#ifndef QRETRO_AUDIOVIDEOENABLE_H
#define QRETRO_AUDIOVIDEOENABLE_H

/**
 * A class used for callback RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE that
 * reports to the core whether or not the frontend wants certain data.
 */
class QRetroAudioVideoEnable
{
public:
  QRetroAudioVideoEnable(bool audio = true, bool video = true, bool ufss = false, bool hda = false)
  {
    m_Flags.fields.enableAudio = audio;
    m_Flags.fields.enableVideo = video;
    m_Flags.fields.useFastSavestates = ufss;
    m_Flags.fields.hardDisableAudio = hda;
  }

  void setEnableAudio(bool enabled) { m_Flags.fields.enableAudio = enabled; }
  bool enableAudio(void) { return m_Flags.fields.enableAudio; }

  void setEnableVideo(bool enabled) { m_Flags.fields.enableVideo = enabled; }
  bool enableVideo(void) { return m_Flags.fields.enableVideo; }

  void setUseFastSavestates(bool enabled) { m_Flags.fields.useFastSavestates = enabled; }
  bool useFastSavestates(void) { return m_Flags.fields.useFastSavestates; }

  void setHardDisableAudio(bool enabled) { m_Flags.fields.hardDisableAudio = enabled; }
  bool hardDisableAudio(void) { return m_Flags.fields.hardDisableAudio; }

  int getFlags(void) { return m_Flags.bits; }
  void setFlags(int flags) { m_Flags.bits = flags; }

private:
  union
  {
    struct
    {
      int enableAudio : 1;
      int enableVideo : 1;
      int useFastSavestates : 1;
      int hardDisableAudio : 1;
      int unused : 28;
    } fields;
    int bits;
  } m_Flags;
  static_assert(sizeof(m_Flags) == sizeof(int), "QRetroAudioVideoEnable size does not match API!");
};

#endif
