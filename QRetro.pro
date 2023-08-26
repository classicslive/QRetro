QT += core gui gamepad multimedia positioning sensors

LIBS += -lOpengl32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib

DEFINES += -DQRETRO_LIBRARY=1

CONFIG += c++11 staticlib

SOURCES += \
  QRetro.cpp \
  QRetroAudio.cpp \
  QRetroCommon.cpp \
  QRetroDirectories.cpp \
  QRetroEnvironment.cpp \
  QRetroLocation.cpp \
  QRetroMicrophone.cpp \
  QRetroOptions.cpp \
  QRetroProcAddress.cpp \
  QRetroSensors.cpp \
  QRetroUsername.cpp

HEADERS += \
  QRetro.h \
  QRetroAudio.h \
  QRetroAudioVideoEnable.h \
  QRetroCommon.h \
  QRetroDirectories.h \
  QRetroEnvironment.h \
  QRetroLocation.h \
  QRetroMicrophone.h \
  QRetroMicrophone.h \
  QRetroOptions.h \
  QRetroProcAddress.h \
  QRetroSensors.h \
  QRetroUsername.h \
  QRetro_global.h \
  libretro.h \
  libretro_core.h

TARGET = QRetro

# Default rules for deployment.
unix {
  target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
