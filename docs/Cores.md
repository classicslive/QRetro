# 2048
<img src="https://github.com/classicslive/QRetro/assets/33245078/f1e62d2b-035e-4797-bf4e-f6647ae331e9" height="256">

Very good. Requires support for cores providing 0 as audio frequency ([de69522](https://github.com/classicslive/QRetro/commit/de695225cfe43a64cf58f4c1760b6a6286dccdc6))

# Beetle Saturn
<img src="https://github.com/classicslive/QRetro/assets/33245078/9bd652d7-b685-454a-b2ff-3b8957cc5857" height="256">

Very good.

# bsnes
<img src="https://github.com/classicslive/QRetro/assets/33245078/f7d53bbb-da87-4865-b521-047106faf9bc" height="256">

Very good. Needs retro_load_game_special support.

# Citra
<img src="https://github.com/classicslive/QRetro/assets/33245078/b0c24cae-bca0-4bee-aecd-290cf673624c" height="256">
<img src="https://github.com/classicslive/QRetro/assets/33245078/c5c676cf-10ce-47d7-9c04-3ad74c8fe436" height="256">

Very good.

# Dolphin
<img src="https://github.com/classicslive/QRetro/assets/33245078/1092365d-6fca-4f0c-99a8-37415826a81f" height="256">
<img src="https://github.com/classicslive/QRetro/assets/33245078/119c4f5d-2f3c-43a5-a4dc-a4c86399731c" height="256">

Good. Relies on GPU hardware access, using OpenGL, DirectX, etc.

Some features like savestates crash unless `"dolphin_main_cpu_thread"` is set to `"disabled"` because of libretro callbacks being buggy with the dual-core mode.

Multi-instancing will break very quickly without `"dolphin_fastmem"` set to `"disabled"`, as this makes Dolphin allocate memory to a fixed location.

# DOSBox Pure
<img src="https://github.com/classicslive/QRetro/assets/33245078/1f81afec-9819-450e-9daf-51955f8f7c2e" height="256">

Ok. Emulation timing seems slightly incorrect. Needs further MIDI testing.

# EasyRPG
<img src="https://github.com/classicslive/QRetro/assets/33245078/151d5111-202e-4a02-8171-b98433bf5130" height="256">

Very good. Relies on RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK and RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK. Needs further MIDI testing.

# FCEUmm
<img src="https://github.com/classicslive/QRetro/assets/33245078/cb5230d4-bf5c-4879-bbcf-18ce8715dcbf" height="256">

Perfect.

# Flycast
<img src="https://github.com/classicslive/QRetro/assets/33245078/d8c38bac-4cdb-444f-bf59-2793fd79122a" height="256">

Very good. Changing internal resolution currently does not work.

# Genesis Plus GX
<img src="https://github.com/classicslive/QRetro/assets/33245078/29544781-1342-41f2-9f44-64bcf40f6278" height="256">

Perfect.

# MelonDS DS
https://github.com/classicslive/QRetro/assets/33245078/da4c16e2-3e04-4b7d-afaf-7268ad72a414

Very good. Uses microphone interface.

Crashes when switching to the OpenGL renderer at runtime, but if it's already enabled it will boot into it fine.

# Mupen64Plus-Next

<img height="256" alt="image" src="https://github.com/user-attachments/assets/f6316487-efe8-40ee-9bfc-9262a755a9e2" />

<img height="256" alt="image" src="https://github.com/user-attachments/assets/3242f686-9755-4c4b-8439-e2786ca712af" />

Good.

Vulkan support is needed to support the paraLLEl renderer options.

Mario Kart 64, and probably other games, have some unrendered graphics for reasons unknown to me.

# Snes9X

<img src="https://github.com/classicslive/QRetro/assets/33245078/b3c18e8c-ead4-4b3d-88b2-020a802000d1" height="256">

Perfect.

# PCSX2

<img height="256" alt="image" src="https://github.com/user-attachments/assets/ec655a80-10f5-4367-91b9-25b226712a18" />
<img height="256" alt="image" src="https://github.com/user-attachments/assets/424899cc-2010-4592-8844-f5a5e6ee0203" />

Good.

<img height="256" alt="image" src="https://github.com/user-attachments/assets/0ec2c54d-2a6d-4e4b-91fb-925b95f994c0" />

Some games need `"pcsx2_renderer"` set to `"Software"` to display correctly. Not sure if this is a core bug or QRetro.

# Play!

Crashes.

```
CGSH_OpenGL_Libretro::InitializeImpl
Error: Missing GL version
```

# Powder Toy, The
<img src="https://github.com/classicslive/QRetro/assets/33245078/0cc1ffc1-a42b-42c0-b78b-cc769ee7d5a1" height="256">

Very good.

# PPSSPP

<img width="613" height="429" alt="image" src="https://github.com/user-attachments/assets/6c94eac1-636e-41ae-83bc-8a5f83f1d1bb" />

Seems good.

Spams a context reset when changing internal resolution at runtime.

# SwanStation
<img height="256" alt="image" src="https://github.com/user-attachments/assets/28083402-e0b2-4746-ad79-a9ae7c8f55e9" />

Seems good.
