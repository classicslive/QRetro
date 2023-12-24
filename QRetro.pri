message("QRetro: Start build")

QT += core gui multimedia

CONFIG += \
  QRETRO_CONFIG_GAMEPAD \
  QRETRO_CONFIG_LOCATION \
  QRETRO_CONFIG_MIDI \
  QRETRO_CONFIG_OPENGL \
  QRETRO_CONFIG_SENSORS

CONFIG(debug, debug|release) {
  DEFINES += QRETRO_DRAW_DEBUG=1
} else {
  DEFINES += QRETRO_DRAW_DEBUG=0
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

QRETRO_CONFIG_GAMEPAD {
  qtHaveModule(gamepad) {
    QT += gamepad
    DEFINES += QRETRO_HAVE_GAMEPAD=1
    message("Gamepad module added.")
  } else {
    warning("Gamepad module requested, but is not available.")
  }
}

QRETRO_CONFIG_LOCATION {
  qtHaveModule(positioning) {
    QT += positioning
    DEFINES += QRETRO_HAVE_LOCATION=1
    message("Location module added.")
  } else {
    warning("Location module requested, but is not available.")
  }
}

QRETRO_CONFIG_MIDI {
  include(external/QMidi/src/QMidi.pri)
  DEFINES += QRETRO_HAVE_MIDI=1
  message("MIDI module added.")
}

QRETRO_CONFIG_OPENGL {
  !isEmpty(QMAKE_LIBS_OPENGL) {
    LIBS += -lOpengl32
    DEFINES += QRETRO_HAVE_OPENGL=1
    message("OpenGL module added.")
  } else {
    message("OpenGL module requested, but is not available.")
  }
}

QRETRO_CONFIG_SENSORS {
  qtHaveModule(sensors) {
    QT += sensors
    DEFINES += QRETRO_HAVE_SENSORS=1
    message("Sensors module added.")
  } else {
    warning("Sensors module requested, but is not available.")
  }
}

SOURCES += \
  QRetro/QRetro.cpp \
  QRetro/QRetroAudio.cpp \
  QRetro/QRetroCommon.cpp \
  QRetro/QRetroDirectories.cpp \
  QRetro/QRetroEnvironment.cpp \
  QRetro/QRetroLocation.cpp \
  QRetro/QRetroMicrophone.cpp \
  QRetro/QRetroMidi.cpp \
  QRetro/QRetroOptions.cpp \
  QRetro/QRetroProcAddress.cpp \
  QRetro/QRetroSensors.cpp \
  QRetro/QRetroUsername.cpp

INCLUDEPATH += \
  $$PWD

HEADERS += \
  QRetro/QRetro.h \
  QRetro/QRetroAudio.h \
  QRetro/QRetroAudioVideoEnable.h \
  QRetro/QRetroCommon.h \
  QRetro/QRetroDirectories.h \
  QRetro/QRetroEnvironment.h \
  QRetro/QRetroLed.h \
  QRetro/QRetroLocation.h \
  QRetro/QRetroMicrophone.h \
  QRetro/QRetroMidi.h \
  QRetro/QRetroOptions.h \
  QRetro/QRetroProcAddress.h \
  QRetro/QRetroSensors.h \
  QRetro/QRetroUsername.h \
  QRetro/QRetro_global.h \
  QRetro/libretro.h \
  QRetro/libretro_core.h
