# QRetro

QRetro is a libretro API frontend implemented within a Qt QWindow. It supports multi-instancing, allowing multiple instances of libretro cores to run simultaneously.

## Getting started

To use QRetro in your project, use one of the following options:

### Static compilation

If you wish to statically compile QRetro into your project, clone this repo and include the .pri file in your qmake project:

```qmake
include(QRetro/QRetro.pri)
```

### Linking the QRetro binary

If you wish to instead link against a QRetro binary, build QRetro using the .pro file instead. Pre-built binaries are not yet available.

## Usage

To instantiate a QRetro object, set it up, and display it:

```c++
#include <QRetro.h>
   
auto retro = new QRetro();

retro->loadCore("C:/core_libretro.dll");
retro->loadContent("C:/content.bin");
retro->startCore();
retro->show();
```

### QWidget

QRetro can be wrapped in a QWidget for display in a Qt UI layout like so:

```c++
auto retrowidget = QWidget::createWindowContainer(retro);
retrowidget->show();
```

### OpenGL context

If you plan to use hardware-accelerated cores using OpenGL, include the following line before initializing your QApplication:

```c++
QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
```

## Compatibility

See the following wiki pages for information on QRetro's compatibility with specific libretro cores and features:

- https://github.com/classicslive/QRetro/wiki/Cores
- https://github.com/classicslive/QRetro/wiki/Environment-callbacks

## Building

QRetro requires the following Qt modules:
- `core`
- `gui`
- `multimedia`

The following modules and libraries are optional, and can be disabled by including the associated CONFIG define in your qmake project:

| Dependency | Configuration Flag |
|---------------------------|------------------------------|
| QCamera / Qt multimedia module | `QRETRO_CONFIG_NO_CAMERA` |
| Qt gamepad module | `QRETRO_CONFIG_NO_GAMEPAD` |
| Qt positioning module | `QRETRO_CONFIG_NO_LOCATION` |
| [QMidi](https://github.com/waddlesplash/QMidi) submodule | `QRETRO_CONFIG_NO_MIDI` |
| Qt multimedia module | `QRETRO_CONFIG_NO_MULTIMEDIA` |
| Qt opengl module / OpenGL libraries | `QRETRO_CONFIG_NO_OPENGL` |
| Qt sensors module | `QRETRO_CONFIG_NO_SENSORS` |
| Qt Mobility systeminfo module | `QRETRO_CONFIG_NO_SYSTEMINFO` |

```qmake
CONFIG += QRETRO_CONFIG_NO_LOCATION

include(QRetro/QRetro.pri)
```

## License

QRetro is licensed under the MIT License.
