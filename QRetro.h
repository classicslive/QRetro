#ifndef QRETRO_H
#define QRETRO_H

#include <QBackingStore>
#if QRETRO_HAVE_OPENGL
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#endif
#include <QWindow>

#include "QRetro_global.h"
#include "libretro_core.h"
#include "QRetroAudio.h"
#include "QRetroAudioVideoEnable.h"
#include "QRetroCamera.h"
#include "QRetroDevicePower.h"
#include "QRetroDirectories.h"
#include "QRetroInput.h"
#include "QRetroLed.h"
#include "QRetroLocation.h"
#include "QRetroMicrophone.h"
#include "QRetroMidi.h"
#include "QRetroProcAddress.h"
#include "QRetroOptions.h"
#include "QRetroSensors.h"
#include "QRetroUsername.h"

/**
 * The size of the libretro enviroment callback enum.
 * Update this constant whenever a new libretro environment callback is added.
 */
#define RETRO_ENVIRONMENT_SIZE (RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE & 0xFF) + 1

#if QRETRO_HAVE_OPENGL
class QRETRO_EXPORT QRetro : public QWindow, protected QOpenGLFunctions
#else
class QRETRO_EXPORT QRetro : public QWindow
#endif
{
  Q_OBJECT

public:

  /* Initializers and deconstructors */

#if QRETRO_HAVE_OPENGL
  QRetro(QWindow *parent = nullptr, retro_hw_context_type format = RETRO_HW_CONTEXT_OPENGL);
#else
  QRetro(QWindow *parent = nullptr, retro_hw_context_type format = RETRO_HW_CONTEXT_NONE);
#endif
  ~QRetro(void) override;

  /* Functions that return pointers to subclasses */

  QRetroAudio* audio(void) { return m_Audio; }
  QRetroAudioVideoEnable* audioVideoEnable(void) { return &m_AudioVideoEnable; }
  QRetroCamera* camera(void) { return &m_Camera; }
  retro_core_t* core(void) { return &m_Core; }
  QRetroDevicePower* devicePower(void) { return &m_DevicePower; }
  QRetroInput* input(void) { return &m_Input; }
  QRetroLed* led(void) { return &m_Led; }
  QRetroLocation* location(void) { return m_Location; }
  QRetroMicrophone* microphone(void) { return &m_Microphone; }
  QRetroMidi* midi(void) { return &m_Midi; }
  QRetroOptions* options(void) { return &m_Options; }
  QRetroDirectories* directories(void) { return &m_Directories; }
  QRetroProcAddress* procAddress(void) { return &m_ProcAddress; }
  QRetroSensors* sensors(void) { return &m_Sensors; }
  QRetroUsername* username(void) { return &m_Username; }

/*
================================================================================
    Public functions (libretro)
================================================================================
*/
  bool loadCore(const char *path);
  bool loadContent(const char *path, const char *meta = nullptr);
  bool startCore(void);

  QPoint mouseDelta() { return m_MouseDelta; }
  QPoint mousePosition() { return m_MousePosition; }
  QPoint pointerPosition() { return m_PointerPosition; }
  bool pointerValid() { return m_PointerValid; }
  bool isActive() { return m_Active; }
  void updateMouse(void);
  bool initVideo(retro_hw_context_type format);

  const char* contentPath() { return m_ContentPath.c_str(); }
  const char* corePath() { return m_CorePath.c_str(); }

  void setGeometry(const unsigned width, const unsigned height);

  /**
   * Sets the pixel format of the framebuffer.
   **/
  void setPixelFormat(retro_pixel_format format);
  void setPixelFormat(QImage::Format format);

  void setRotation(const unsigned degrees);

  long long unsigned getCurrentFramebuffer();

  /**
   * Returns the function pointer for a given OpenGL symbol.
   **/
  void* getProcAddress(QThread *caller, const char *sym);

  bool supportsNoGame(void) { return m_SupportsNoGame; }
  void setSupportsNoGame(bool supports) { m_SupportsNoGame = supports; }

  /**
   * Gets how many seconds the saving thread waits before autosaving SRAM
   * contents. If this value is 0, SRAM contents will not be autosaved.
   */
  unsigned getAutosaveInterval() { return m_AutosaveInterval; }

  /**
   * Sets the amount of time (in seconds) the "saving" thread waits before
   * flushing SRAM contents to the disk.
   * If this is set to 0, SRAM contents will not be autosaved.
   */
  void setAutosaveInterval(unsigned secs) { m_AutosaveInterval = secs; }

  bool supportsDuping() { return m_CanDupe; }
  void setSupportsDuping(bool supports) { m_CanDupe = supports; }

  bool fastForwarding() { return m_FastForwarding; }
  void setFastForwarding(bool enabled);

  float fastForwardingRatio() { return m_FastForwardRatio; }
  void setFastForwardingRatio(float ratio) { m_FastForwardRatio = ratio; }

  unsigned frames(void) { return m_Frames; }

  bool getOverscan() { return m_Overscan; }
  void setOverscan(bool overscan) { m_Overscan = overscan; }

  retro_hw_context_type getPreferredRenderer(void) { return m_PreferredRenderer; }
  void setPreferredRenderer(retro_hw_context_type hw) { m_PreferredRenderer = hw; }

  bool supportsAchievements(void) { return m_SupportsAchievements; }
  void setSupportsAchievements(bool supports) { m_SupportsAchievements = supports; }

  uint64_t serializationQuirks(void) { return m_SerializationQuirks; }
  void setSerializationQuirks(uint64_t *sq)
  {
    m_SerializationQuirks = *sq;
    *sq = 0;
  }

  retro_memory_map* memoryMaps(void) { return &m_MemoryMaps; }
  void setMemoryMaps(const struct retro_memory_map* maps)
  {
    m_MemoryMaps.num_descriptors = maps->num_descriptors;
    auto descs = static_cast<retro_memory_descriptor*>(
      calloc(m_MemoryMaps.num_descriptors, sizeof(retro_memory_descriptor)));

    /* Do a deep copy */
    for (unsigned i = 0; i < m_MemoryMaps.num_descriptors; i++)
    {
      auto dst = &descs[i];
      auto src = &maps->descriptors[i];

      memcpy(dst, src, sizeof(retro_memory_descriptor));
      if (src->addrspace)
      {
        auto addr = static_cast<char*>(malloc(strlen(src->addrspace) + 1));

        memcpy(addr, src->addrspace, strlen(src->addrspace) + 1);
        addr[strlen(src->addrspace)] = '\0';
        dst->addrspace = addr;
      }
    }
    m_MemoryMaps.descriptors = descs;
  }

  double targetRefreshRate() { return m_TargetRefreshRate; }

  bool getCurrentSoftwareFramebuffer(retro_framebuffer*);

  /*
  ------------------------------------------------------------------------------
    Gets/sets a language hint that the core can read via
    RETRO_ENVIRONMENT_GET_LANGUAGE. The core can only request to read this,
    not set it.
  ------------------------------------------------------------------------------
  */
  retro_language getLanguage() { return m_Language; }
  bool setLanguage(retro_language lng)
  {
    if (lng >= RETRO_LANGUAGE_LAST)
      return false;
    else
    {
      m_Language = lng;
      return true;
    }
  }

  /**
   * Returns whether or not the frontend is reporting to support a given
   * environment callback ID.
   * Will always return true unless "setEnvironmentCallbackSupported" is used to
   * manually unset it.
   */
  bool environmentCallbackSupported(unsigned id)
  {
    if (id >= RETRO_ENVIRONMENT_SIZE)
      return false;
    else
      return m_SupportedEnvCallbacks[id];
  }

  /**
   * Sets whether or not the frontend will report to support a given environment
   * callback ID.
   * If set to false, the frontend will do nothing if this callback ID is
   * requested by the core.
   */
  void setEnvironmentCallbackSupported(unsigned id, bool supported)
  {
    if (id < RETRO_ENVIRONMENT_SIZE)
      m_SupportedEnvCallbacks[id] = supported;
  }

  /**
   * Gets the minimum log level required for a core log message to not be
   * ignored. Accepted messages will be emitted through "onCoreLog".
   * By default, this is RETRO_LOG_ERROR.
   */
  retro_log_level getLogLevel() { return m_LogLevel; }

  /**
   * Sets the minimum log level required for a core log message to not be
   * ignored. Accepted messages will be emitted through "onCoreLog".
   */
  void setLogLevel(retro_log_level lvl) { m_LogLevel = lvl; }

  /**
   * Gets a number representing the relative performance requirement for the
   * current core/content. Doesn't seem to be set by most cores.
   */
  unsigned getPerformanceLevel() { return m_PerformanceLevel; }

  /**
   * Sets a hint to the frontend about the relative performance requirement
   * for the current core/content.
   *
   * @warning This should only be called automatically by the core itself.
   */
  void setPerformanceLevel(const unsigned lvl) { m_PerformanceLevel = lvl; }

  /**
   * Returns whether or not a custom handler for polling inputs has been
   * installed.
   */
  bool hasInputPollHandler() { return m_InputPollHandler != nullptr; }

  /**
   * Installs a custom handler for polling inputs. The core will then call this
   * function during retro_run instead of using QRetro's built-in gamepad
   * handler.
   */
  bool installInputPollHandler(bool handler(void))
  {
    if (!handler)
      return false;
    m_InputPollHandler = handler;
    return true;
  }

  /**
   * Runs the installed custom handler for polling inputs and signals that the
   * core can proceed to the next frame if it succeeds.
   *
   * @return FALSE if a handler has not been installed. Otherwise, TRUE.
   * @todo This should be private.
   **/
  bool runInputPollHandler()
  {
    if (!hasInputPollHandler())
      return false;
    m_InputReady = m_InputPollHandler();
    return true;
  }

  /**
   * Removes the installed custom handler for polling inputs, allowing QRetro's
   * default handler to process inputs.
   *
   * @return FALSE if a handler has not been installed. Otherwise, TRUE.
   */
  bool uninstallInputPollHandler();

  bool hasInputStateHandler() { return m_InputStateHandler != nullptr; }
  bool installInputStateHandler(int16_t handler(unsigned, unsigned, unsigned, unsigned))
  {
    if (!handler)
      return false;
    m_InputStateHandler = handler;
    return true;
  }
  int16_t runInputStateHandler(unsigned port, unsigned device, unsigned index, unsigned id)
  {
    if (!hasInputStateHandler())
      return false;
    return m_InputStateHandler(port, device, index, id);
  }
  bool uninstallInputStateHandler();

  void setFastForwardingOverride(retro_fastforwarding_override*);

  void pause(void) { m_Paused = true; }
  void unpause(void) { m_Paused = false; }

  bool jitCapable(void) { return m_JitCapable; }
  void setJitCapable(bool capable) { m_JitCapable = capable; }

signals:
  void onCoreLog(int level, const QString msg);
  void onCoreMessage(const char *msg);
  void onCoreStart(void);
  void onFrame(void);
  void onSave(void);
  void onVideoRefresh(const void *ptr, unsigned width, unsigned height,
                      unsigned bytes_per_line);

public slots:
  bool coreSerialize(void *data, size_t size);
  bool coreUnserialize(void *data, size_t size);
  void setImagePtr(const void *ptr, unsigned width, unsigned height,
      unsigned bytes_per_line);

protected:
  bool event(QEvent *event) override;
  void exposeEvent(QExposeEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  QRetroAudio *m_Audio;
  QRetroAudioVideoEnable m_AudioVideoEnable;
  QRetroCamera m_Camera;
  QRetroDevicePower m_DevicePower;
  QRetroDirectories m_Directories;
  QRetroInput m_Input;
  QRetroLed m_Led;
  QRetroLocation *m_Location = nullptr;
  QRetroMicrophone m_Microphone;
  QRetroMidi m_Midi;
  QRetroOptions m_Options;
  QRetroProcAddress m_ProcAddress;
  QRetroSensors m_Sensors;
  QRetroUsername m_Username;

  retro_core_t m_Core;

  bool m_JitCapable = true;
  bool m_Overscan = true;

  /// Whether or not the core is running
  bool m_Active = false;

  /// Number of seconds to wait before flushing save RAM to disk
  unsigned m_AutosaveInterval = 5;

  /// Whether or not the core reports to duplicate frames
  bool m_CanDupe = true;

  /// Directory of the currently loaded content
  std::string m_ContentPath;

  /// Directory of the currently loaded core
  std::string m_CorePath;

  /// Whether or not the frontend is fast-forwarding
  bool m_FastForwarding = false;

  /// The maximum amount the frontend will fast-forward
  float m_FastForwardRatio = 0.0;

  /// Number of calls to retro_run completed
  unsigned long m_Frames = 0;

  /// Whether or not input has been received between calls to retro_run
  bool m_InputReady = false;

  /// The preferred language reported by the frontend
  retro_language m_Language = RETRO_LANGUAGE_ENGLISH;

  /// The platform-specific handle to the libretro dynamic library
  QRETRO_LIBRARY_T m_Library = nullptr;

  /// The minimum log level for messages to be emitted as signals
  retro_log_level m_LogLevel = RETRO_LOG_ERROR;

  /// The memory map reported by the core
  retro_memory_map m_MemoryMaps = { nullptr, 0 };

  /// The x and y values of how much mouse has moved since last input poll
  QPoint m_MouseDelta;

  /// The previous mouse position, used to calculate delta
  QPoint m_MousePosition;

  /// Whether the user has paused the frontend. Will halt calls to retro_run
  bool m_Paused = false;

  /// The performance level reported by the core
  /// @see RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL
  unsigned m_PerformanceLevel = 0;

  /// The position of the pointer in content screen space
  QPoint m_PointerPosition;

  /// Whether or not the pointer was in the content screenspace on last poll
  bool m_PointerValid;

  /// The preferred renderer reported by the frontend
  retro_hw_context_type m_PreferredRenderer;

  /// The reported serialization quirk flags of the core
  uint64_t m_SerializationQuirks = 0;

  /// A pointer to a framebuffer the frontend manages, rather than the core.
  /// @see RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER
  unsigned char *m_SoftwareFramebuffer = nullptr;

  /// Whether or not the frontend reports to support each environment callback
  bool m_SupportedEnvCallbacks[RETRO_ENVIRONMENT_SIZE] = { true };

  /// Whether or not the core reports to support achievements
  bool m_SupportsAchievements = false;

  /// Whether or not the core reports to support starting with no content
  bool m_SupportsNoGame = false;

  double m_TargetRefreshRate = 60.0;
  QThread *m_ThreadSaving;
  QThread *m_ThreadTiming;

  /* General video parameters */
  QImage m_Image;
  QRect m_BaseRect;
  QRect m_Rect;

  QImage::Format m_PixelFormat;

  bool m_BilinearFilter = true;
  bool m_IntegerScaling = false;
  unsigned m_Rotation = 0;
  double m_ScalingFactor = 1.0;
  bool m_UseAspectRatio = true;

  /* Software surface */
  QBackingStore *m_BackingStore = nullptr;

  /* OpenGL surface */
#if QRETRO_HAVE_OPENGL
  QOpenGLContext *m_OpenGlContext = nullptr;
  QOpenGLContext *m_OpenGlContextCore = nullptr;
  QOpenGLPaintDevice *m_OpenGlDevice = nullptr;
  QOpenGLFramebufferObject *m_OpenGlFbo = nullptr;
#endif
  bool m_ImageDrawing = false;
  bool m_ImageRendering = false;

  bool (*m_InputPollHandler)(void) = nullptr;
  int16_t (*m_InputStateHandler)(unsigned, unsigned, unsigned, unsigned) = nullptr;

  bool inputReady() { return hasInputPollHandler() ? m_InputReady : true; }
  void setupPainter(QPainter *painter);
  void updateScaling();

  /* The functions that get spun off into their own threads, defined above */
private slots:
  void saving();
  void timing();
};

#endif
