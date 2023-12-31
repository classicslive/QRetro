#ifndef LIBRETRO_CORE_H
#define LIBRETRO_CORE_H

#include <libretro.h>

#ifdef WIN32
#include <windows.h>
#define QRETRO_LIBRARY_T HINSTANCE
#else
#define QRETRO_LIBRARY_T void*
#endif

typedef void (*QRETRO_FUNCTION_T)(void);

typedef struct retro_core_t
{
  /* libretro function pointers */
  void (*retro_init)(void);
  void (*retro_deinit)(void);
  unsigned (*retro_api_version)(void);
  void (*retro_get_system_info)(retro_system_info*);
  void (*retro_get_system_av_info)(retro_system_av_info*);
  void (*retro_set_environment)(retro_environment_t);
  void (*retro_set_video_refresh)(retro_video_refresh_t);
  void (*retro_set_audio_sample)(retro_audio_sample_t);
  void (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
  void (*retro_set_input_poll)(retro_input_poll_t);
  void (*retro_set_input_state)(retro_input_state_t);
  void (*retro_set_controller_port_device)(unsigned, unsigned);
  void (*retro_reset)(void);
  void (*retro_run)(void);
  size_t (*retro_serialize_size)(void);
  bool (*retro_serialize)(void*, size_t);
  bool (*retro_unserialize)(const void*, size_t);
  void (*retro_cheat_reset)(void);
  void (*retro_cheat_set)(unsigned, bool, const char*);
  bool (*retro_load_game)(const retro_game_info*);
  bool (*retro_load_game_special)(unsigned, const retro_game_info*, size_t);
  void (*retro_unload_game)(void);
  unsigned (*retro_get_region)(void);
  void *(*retro_get_memory_data)(unsigned);
  size_t (*retro_get_memory_size)(unsigned);

  /* libretro structs */
  retro_audio_callback audio_callback;
  retro_fastforwarding_override fastforwarding_override;
  retro_frame_time_callback frame_time_callback;
  retro_game_info game_info;
  retro_get_proc_address_interface get_proc_address_interface;
  retro_hw_render_callback hw_render;
  retro_keyboard_callback keyboard;
  retro_pixel_format pixel_format;
  retro_system_av_info av_info;
  retro_system_info system_info;

  /* Additional information */
  unsigned poll_type;
  bool inited;
  bool symbols_inited;
  bool content_loaded;
  bool input_polled;
  bool has_set_subsystems;
  bool has_set_input_descriptors;
  uint64_t serialization_quirks_v;
} retro_core_t;

bool load_function(void *func_ptr, QRETRO_LIBRARY_T library, const char *name);
bool load_library(retro_core_t *core, QRETRO_LIBRARY_T library);

#endif
