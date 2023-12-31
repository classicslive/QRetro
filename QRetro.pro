TEMPLATE = lib

DEFINES += QRETRO_LIBRARY

CONFIG += c++11 staticlib

include(QRetro.pri)

TARGET = QRetro

# Default rules for deployment.
unix {
  target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
