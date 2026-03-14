#ifndef LIBRETRO_RETROARCH_H
#define LIBRETRO_RETROARCH_H

/**
 * I hate this. RetroArch has special RetroArch-exclusive libretro callbacks
 * that some cores (Mupen64Plus-Next, Flycast) use. These are here so QRetro
 * can at least recognize them, though they will probably always be ignored.
 * https://github.com/libretro/RetroArch/blob/master/retroarch.h
 */

#define RETRO_ENVIRONMENT_RETROARCH_START_BLOCK 0x800000

#define RETRO_ENVIRONMENT_SET_SAVE_STATE_IN_BACKGROUND (2 | RETRO_ENVIRONMENT_RETROARCH_START_BLOCK)
                                            /* bool * --
                                            * Boolean value that tells the front end to save states in the
                                            * background or not.
                                            */

#define RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB (3 | RETRO_ENVIRONMENT_RETROARCH_START_BLOCK)
                                            /* retro_environment_t * --
                                            * Provides the callback to the frontend method which will cancel
                                            * all currently waiting threads.  Used when coordination is needed
                                            * between the core and the frontend to gracefully stop all threads.
                                            */

#define RETRO_ENVIRONMENT_POLL_TYPE_OVERRIDE (4 | RETRO_ENVIRONMENT_RETROARCH_START_BLOCK)
                                            /* unsigned * --
                                            * Tells the frontend to override the poll type behavior.
                                            * Allows the frontend to influence the polling behavior of the
                                            * frontend.
                                            *
                                            * Will be unset when retro_unload_game is called.
                                            *
                                            * 0 - Don't Care, no changes, frontend still determines polling type behavior.
                                            * 1 - Early
                                            * 2 - Normal
                                            * 3 - Late
                                            */

#define RETRO_ENVIRONMENT_SET_SAVE_STATE_DISABLE_UNDO (5 | RETRO_ENVIRONMENT_RETROARCH_START_BLOCK)
                                            /* bool * --
                                            * If true, disables the save state save/load undo feature to conserve memory.
                                            */

#endif
