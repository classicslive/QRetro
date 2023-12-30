message("QRetro: Start build")

QT += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
  DEFINES += QRETRO_DRAW_DEBUG=1
} else {
  DEFINES += QRETRO_DRAW_DEBUG=0
}

CONFIG += QRETRO_CONFIG_NO_GAMEPAD

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
  !isEmpty(QMAKE_LIBS_OPENGL) {
    win32 {
      LIBS += -lOpengl32
    }
    linux* {
      LIBS += -lGL
    }
    DEFINES += QRETRO_HAVE_OPENGL=1
    message("OpenGL module added.")
  } else {
    DEFINES += QRETRO_HAVE_OPENGL=0
    message("OpenGL module requested, but is not available.")
  }
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
  $$PWD/QRetroAudio.cpp \
  $$PWD/QRetroCamera.cpp \
  $$PWD/QRetroCommon.cpp \
  $$PWD/QRetroDevicePower.cpp \
  $$PWD/QRetroDirectories.cpp \
  $$PWD/QRetroEnvironment.cpp \
  $$PWD/QRetroInput.cpp \
  $$PWD/QRetroLocation.cpp \
  $$PWD/QRetroMicrophone.cpp \
  $$PWD/QRetroMidi.cpp \
  $$PWD/QRetroOptions.cpp \
  $$PWD/QRetroProcAddress.cpp \
  $$PWD/QRetroSensors.cpp \
  $$PWD/QRetroUsername.cpp

INCLUDEPATH += \
  $$PWD

HEADERS += \
  $$PWD/QRetro.h \
  $$PWD/QRetroAudio.h \
  $$PWD/QRetroAudioVideoEnable.h \
  $$PWD/QRetroCamera.h \
  $$PWD/QRetroCommon.h \
  $$PWD/QRetroDevicePower.h \
  $$PWD/QRetroDirectories.h \
  $$PWD/QRetroEnvironment.h \
  $$PWD/QRetroInput.h \
  $$PWD/QRetroLed.h \
  $$PWD/QRetroLocation.h \
  $$PWD/QRetroMicrophone.h \
  $$PWD/QRetroMidi.h \
  $$PWD/QRetroOptions.h \
  $$PWD/QRetroProcAddress.h \
  $$PWD/QRetroSensors.h \
  $$PWD/QRetroUsername.h \
  $$PWD/QRetro_global.h \
  $$PWD/libretro.h \
  $$PWD/libretro_core.h

DISTFILES += \
  $$PWD/resources/camera.png

RESOURCES += \
  $$PWD/resources/resources.qrc
