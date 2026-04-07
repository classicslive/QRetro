libretro environment callbacks are API extensions that the core requests from the frontend, which it can optionally support. QRetro intends to support as many of these as possible.

# Key
- ✔ Supported and tested
- ❔ Supported in code, but untested
- ⚠ Partially supported
- ❌ Unsupported

# QRetro support status
| ID | Symbol | Status | Module | Comments / Examples |
| - | - | - | - | - |
| 1 | RETRO_ENVIRONMENT_SET_ROTATION | ✔ | QRetro | MAME sets this during load_game and, for example, mario.zip displays correctly. |
| 2 | RETRO_ENVIRONMENT_GET_OVERSCAN | ❔ | QRetro | Deprecated. No functionality. |
| 3 | RETRO_ENVIRONMENT_GET_CAN_DUPE | ❔ | QRetro | No functionality. |
| 4 | Deprecated | - | - | - |
| 5 | Deprecated | - | - | - |
| 6 | RETRO_ENVIRONMENT_SET_MESSAGE | ✔ | QRetro | Emits message through the onCoreMessage signal. |
| 7 | RETRO_ENVIRONMENT_SHUTDOWN | ✔ | QRetro | Closes the window which should call all deconstructors. MelonDSDS supports this. |
| 8 | RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL | ✔ | QRetro | Supported but seemingly useless. |
| 9 | RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY | ✔ | QRetroDirectories | PCSX2 loads BIOS from this location correctly. |
| 10 | RETRO_ENVIRONMENT_SET_PIXEL_FORMAT | ✔ | QRetro | |
| 11 | RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS | ❔ | QRetroInput | Reads information but does nothing with it right now. |
| 12 | RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK | ⚠ | QRetro | Certain keys that Qt only reads through events are not yet supported. DOSBox should use this? |
| 13 | RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE | ✔ | QRetroDiskControl | Tested with Dolphin and SwanStation |
| 14 | RETRO_ENVIRONMENT_SET_HW_RENDER | ⚠ | QRetro | Only OpenGL is supported, and quite buggy. |
| 15 | RETRO_ENVIRONMENT_GET_VARIABLE | ✔ | QRetroOptions | Older cores using core options v0 use this. |
| 16 | RETRO_ENVIRONMENT_SET_VARIABLES | ✔ | QRetroOptions | Older cores using core options v0 use this. |
| 17 | RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE | ✔ | QRetroOptions | Gambette uses this and immediately updates palette when changed via options UI. |
| 18 | RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME | ✔ | QRetro | Press F, QUASI88 support this. Need testing. |
| 19 | RETRO_ENVIRONMENT_GET_LIBRETRO_PATH | ✔ | QRetroDirectories | - |
| 20 | Deprecated | - | - | - |
| 21 | RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK | ⚠ | QRetro | Supported, but just returns a static value. Tested with EasyRPG. [nogg-libretro](https://github.com/kivutar/nogg-libretro) appears to support this. |
| 22 | RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK | ✔ | QRetro | Tested with EasyRPG. |
| 23 | RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE | ✔ | QRetroInput | Supported in code and with physical controllers when using the SDL3 input backend |
| 24 | RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES | ❌ | QRetro | Unknown which cores support this. |
| 25 | RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE | ✔ | QRetroSensors, QRetroInput | Gyroscope and accelerometer are supported via SDL3 input backend, these and luminosity are supported through device itself in Qt. | 
| 26 | RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE | ⚠ | QRetroCamera | Only spoofing is available by passing an image; camera code is available but disabled. Unknown which cores support this. |
| 27 | RETRO_ENVIRONMENT_GET_LOG_INTERFACE | ✔ | QRetroEnvironment | - |
| 28 | RETRO_ENVIRONMENT_GET_PERF_INTERFACE | ❌ | - | Unknown which cores support this. |
| 29 | RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE | ❔ | QRetroLocation | Spoofing works using a custom core. Actual location untested. |
| 30 | RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY | ✔ | QRetroDirectories | - |
| 31 | RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY | ✔ | QRetroDirectories | MelonDS, Dolphin, PCSX2 tested. |
| 32 | RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO | ✔ | QRetro | Video scales automatically. Dolphin, PCSX2, SwanStation tested. |
| 33 | RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK | ✔ | QRetroProcAddress | Supported, but unknown which cores use it. |
| 34 | RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO | ❌ | - | - |
| 35 | RETRO_ENVIRONMENT_SET_CONTROLLER_INFO | ❔ | QRetroInput | - |
| 36 | RETRO_ENVIRONMENT_SET_MEMORY_MAPS | ✔ | QRetroMemory | Can be used to perform reads/writes to core-supported virtual addresses |
| 37 | RETRO_ENVIRONMENT_SET_GEOMETRY | ✔ | QRetro | Dolphin, Swanstation tested on games that change resolution (Star Fox Adventures, Mr. Driller G). |
| 38 | RETRO_ENVIRONMENT_GET_USERNAME | ✔ | QRetroUsername | Desmume uses this. |
| 39 | RETRO_ENVIRONMENT_GET_LANGUAGE | ✔ | QRetro | Used for options intl, QUASI88 supports this. |
| 40 | RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER | ✔ | QRetro | Confirmed usage in a test core. Swanstation previously used this? |
| 41 | RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE | ❌ | - | - | 
| 42 | RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS | ✔ | QRetro | - |
| 43 | RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE | ❌ | - | - | 
| 44 | RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS | ❌ | QRetro | Recorded but not used for anything | 
| 44 | RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT | ❌ | - | - | 
| 45 | RETRO_ENVIRONMENT_GET_VFS_INTERFACE | ❌ | - | - | 
| 46 | RETRO_ENVIRONMENT_GET_LED_INTERFACE | ✔ | QRetroLed | Press F uses this for Scach | 
| 47 | RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE | ❔ | QRetroAudioVideoEnable | Supported in code but untested. |
| 48 | RETRO_ENVIRONMENT_GET_MIDI_INTERFACE | ⚠ | QRetro | [QMidi](https://github.com/waddlesplash/QMidi) is used for this. Needs further testing. | 
| 49 | RETRO_ENVIRONMENT_GET_FASTFORWARDING | ❔ | QRetro | Unknown which cores support this. |
| 50 | RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE | ❔ | QRetro | Unknown which cores support this. |
| 51 | RETRO_ENVIRONMENT_GET_INPUT_BITMASKS | ✔ | QRetro | FCEUmm, PCSX2, Beetle VB, Beetle Saturn, and more support this. |
| 52 | RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION | ❔ | QRetroOptions | Cores that support options version > 0 should use this. |
| 53 | RETRO_ENVIRONMENT_SET_CORE_OPTIONS | ✔ | QRetroOptions | Cores with options version > 0 support this. |
| 54 | RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL | ✔ | QRetroOptions | QAUSI88 supports this. |
| 55 | RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY | ❔ | QRetroOptions | - |
| 56 | RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER | ✔ | QRetro | PCSX2, PPSSPP support this. |
| 57 | RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION | ❔ | QRetroDiskControl | - |
| 58 | RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE | ❔ | QRetroDiskControl | - |
| 59 | RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION | ❔ | QRetroMessage | Unknown which cores support this. |
| 60 | RETRO_ENVIRONMENT_SET_MESSAGE_EXT | ❔ | QRetroMessage | Unknown which cores support this. |
| 61 | RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS | ❔ | QRetroInput | Unknown which cores support this. |
| 62 | RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK | ❌ | QRetroAudio | Unknown which cores support this. |
| 63 | RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY | ❌ | QRetroAudio | Unknown which cores support this. |
| 64 | RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE | ❔ | QRetro | Unknown which cores support this. |
| 65 | RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE | ❌ | - | Unknown which cores support this. |
| 66 | RETRO_ENVIRONMENT_GET_GAME_INFO_EXT | ❌ | - | Unknown which cores support this. |
| 67 | RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 | ❔ | QRetroOptions | - |
| 68 | RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL | ❔ | QRetroOptions | - |
| 69 | RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK | ❔ | QRetroOptions | - |
| 70 | RETRO_ENVIRONMENT_SET_VARIABLE | ❔ | QRetroOptions | - |
| 71 | RETRO_ENVIRONMENT_GET_THROTTLE_STATE | ❌ | - | - |
| 72 | RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT | ❌ | - | - |
| 73 | RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT | ❌ | - | - |
| 74 | RETRO_ENVIRONMENT_GET_JIT_CAPABLE | ❔ | QRetro | Unknown which cores support this. |
| 75 | RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE | ✔ | QRetroMicrophone | MelonDS DS uses this. |
| 76 | Deprecated | - | -| - |
| 77 | RETRO_ENVIRONMENT_GET_DEVICE_POWER | ❔ | QRetroDevicePower | Supported but untested, as it relies on Qt Mobility. |
| 78 | RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE | ❌ | - | - |
| 79 | RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY | ❔ | QRetroDirectories | - |
| 80 | RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY | ❔ | QRetroDirectories | - |
| 81 | RETRO_ENVIRONMENT_GET_TARGET_SAMPLE_RATE | ❔ | QRetroAudio | - |
| 82 | RETRO_ENVIRONMENT_GET_NETPLAY_CLIENT_INDEX | ❌ | - | - |
