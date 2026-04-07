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

# Snes9X

<img src="https://github.com/classicslive/QRetro/assets/33245078/b3c18e8c-ead4-4b3d-88b2-020a802000d1" height="256">

Perfect.

# PCSX2
<img src="https://github.com/classicslive/QRetro/assets/33245078/caa0aa89-fe57-481b-936f-334a17c9ab29" height="256">
<img src="https://github.com/classicslive/QRetro/assets/33245078/8c543893-a6e1-4fcf-b162-cf581ea9b236" height="256">
<img src="https://github.com/classicslive/QRetro/assets/33245078/12a8fd24-7014-4d84-aef3-f07899a39db6" height="256">

Bad. Variable-resolution games do not display correctly in all contexts

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

Crashes. Needs FBO to be setup in the correct context.

```
(../GPU/GPU.cpp:GPU_Init:67): [draw || gpuCore == GPUCORE_SOFTWARE] (menu, 0.1s) Assert!
```

# SwanStation
<img src="https://github.com/classicslive/QRetro/assets/33245078/adba58bb-e612-4244-83f1-2c834e495c95" height="256">
<img src="https://github.com/classicslive/QRetro/assets/33245078/f1f09953-ef9f-4fba-9649-a4834508e881" height="256">

Very good on older binaries, bad on current ones. Scaling was broken at some point.
