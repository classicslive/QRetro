message("QRetro: Start build")

CONFIG += c++17

QT += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
  DEFINES += QRETRO_DEBUG=1
} else {
  DEFINES += QRETRO_DEBUG=0
}

!QRETRO_CONFIG_NO_CAMERA {
  DEFINES += QRETRO_HAVE_CAMERA=1
  message("Camera module added.")
} else {
  DEFINES += QRETRO_HAVE_CAMERA=0
  message("Camera module disabled.")
}

!QRETRO_CONFIG_NO_GAMEPAD {
  qtHaveModule(gamepad) {
    QT += gamepad
    DEFINES += QRETRO_HAVE_GAMEPAD=1
    message("Gamepad module added.")
  } else {
    DEFINES += QRETRO_HAVE_GAMEPAD=0
    warning("Gamepad module requested, but is not available.")
  }
}

!QRETRO_CONFIG_NO_SDL3 {
  SDL3_PATH = $$(SDL3_PATH)
  !isEmpty(SDL3_PATH) {
    INCLUDEPATH += $$SDL3_PATH/include
    LIBS        += -L$$SDL3_PATH/lib -lSDL3
    DEFINES     += QRETRO_HAVE_SDL3=1
    message("SDL3 backend added (SDL3_PATH=$$SDL3_PATH).")
  } else {
    CONFIG += link_pkgconfig
    packagesExist(sdl3) {
      PKGCONFIG += sdl3
      DEFINES   += QRETRO_HAVE_SDL3=1
      message("SDL3 backend added (pkg-config).")
    } else {
      DEFINES += QRETRO_HAVE_SDL3=0
      warning("SDL3 backend requested, but SDL3 was not found via SDL3_PATH or pkg-config.")
    }
  }
} else {
  DEFINES += QRETRO_HAVE_SDL3=0
  message("SDL3 backend disabled.")
}

!QRETRO_CONFIG_NO_LOCATION {
  qtHaveModule(positioning) {
    QT += positioning
    DEFINES += QRETRO_HAVE_LOCATION=1
    message("Location module added.")
  } else {
    DEFINES += QRETRO_HAVE_LOCATION=0
    warning("Location module requested, but is not available.")
  }
}

!QRETRO_CONFIG_NO_MIDI {
  include(QMidi/src/QMidi.pri) {
    DEFINES += QRETRO_HAVE_MIDI=1
    message("MIDI module added.")
  } else {
    DEFINES += QRETRO_HAVE_MIDI=0
    warning("MIDI module requested, but is not available. Did you update submodules?")
  }
}

!QRETRO_CONFIG_NO_MULTIMEDIA {
  qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += QRETRO_HAVE_MULTIMEDIA=1
    message("Multimedia module added.")
  } else {
    DEFINES += QRETRO_HAVE_MULTIMEDIA=0
    warning("Multimedia module requested, but is not available.")
  }
}

!QRETRO_CONFIG_NO_OPENGL {
  win32 {
    LIBS += -lOpengl32
  }
  linux* {
    LIBS += -lGL
  }
  QT += opengl
  DEFINES += QRETRO_HAVE_OPENGL=1
  message("OpenGL module added.")
} else {
  DEFINES += QRETRO_HAVE_OPENGL=0
  message("OpenGL module disabled.")
}

!QRETRO_CONFIG_NO_SENSORS {
  qtHaveModule(sensors) {
    QT += sensors
    DEFINES += QRETRO_HAVE_SENSORS=1
    message("Sensors module added.")
  } else {
    DEFINES += QRETRO_HAVE_SENSORS=0
    warning("Sensors module requested, but is not available.")
  }
}

!QRETRO_CONFIG_NO_SYSTEMINFO {
  contains(MOBILITY_CONFIG, systeminfo) {
    CONFIG += mobility
    MOBILITY += systeminfo
    DEFINES += QRETRO_HAVE_SYSTEMINFO=1
    message("System info module added.")
  } else {
    DEFINES += QRETRO_HAVE_SYSTEMINFO=0
    warning("System info module requested, but is not available.")
  }
}

SOURCES += \
  $$PWD/QRetro.cpp \
  $$PWD/QRetroConfig.cpp \
  $$PWD/QRetroAudio.cpp \
  $$PWD/QRetroCamera.cpp \
  $$PWD/QRetroCommon.cpp \
  $$PWD/QRetroDevicePower.cpp \
  $$PWD/QRetroDirectories.cpp \
  $$PWD/QRetroEnvironment.cpp \
  $$PWD/QRetroInput.cpp \
  $$PWD/QRetroLocation.cpp \
  $$PWD/QRetroMicrophone.cpp \
  $$PWD/QRetroMessage.cpp \
  $$PWD/QRetroMidi.cpp \
  $$PWD/QRetroOptions.cpp \
  $$PWD/QRetroProcAddress.cpp \
  $$PWD/QRetroSensors.cpp \
  $$PWD/QRetroUsername.cpp

contains(DEFINES, QRETRO_HAVE_GAMEPAD=1) {
  SOURCES += $$PWD/QRetroInputBackendQGamepad.cpp
}

contains(DEFINES, QRETRO_HAVE_SDL3=1) {
  SOURCES += $$PWD/QRetroInputBackendSDL3.cpp
}

INCLUDEPATH += \
  $$PWD

HEADERS += \
  $$PWD/QRetro.h \
  $$PWD/QRetroConfig.h \
  $$PWD/QRetroAudio.h \
  $$PWD/QRetroAudioVideoEnable.h \
  $$PWD/QRetroCamera.h \
  $$PWD/QRetroCommon.h \
  $$PWD/QRetroDevicePower.h \
  $$PWD/QRetroDirectories.h \
  $$PWD/QRetroEnvironment.h \
  $$PWD/QRetroInput.h \
  $$PWD/QRetroInputBackend.h \
  $$PWD/QRetroLed.h \
  $$PWD/QRetroLog.h \
  $$PWD/QRetroMessage.h \
  $$PWD/QRetroLocation.h \
  $$PWD/QRetroMicrophone.h \
  $$PWD/QRetroMidi.h \
  $$PWD/QRetroOptions.h \
  $$PWD/QRetroProcAddress.h \
  $$PWD/QRetroSensors.h \
  $$PWD/QRetroUsername.h \
  $$PWD/QRetro_global.h \
  $$PWD/libretro.h \
  $$PWD/libretro_core.h \
  $$PWD/libretro_retroarch.h

contains(DEFINES, QRETRO_HAVE_GAMEPAD=1) {
  HEADERS += $$PWD/QRetroInputBackendQGamepad.h
}

contains(DEFINES, QRETRO_HAVE_SDL3=1) {
  HEADERS += $$PWD/QRetroInputBackendSDL3.h
}

DISTFILES += \
  $$PWD/resources/camera.png

RESOURCES += \
  $$PWD/resources/resources.qrc
