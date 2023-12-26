# QRetro

QRetro is a libretro API frontend implemented within a Qt QWindow. It supports multi-instancing, allowing multiple instances of libretro cores simultaneously.

## Usage

### Static compilation

Clone the project and include the .pri file in your qmake project:

```qmake
include(QRetro/QRetro.pri)
```

Then instantiate a QRetro object, set it up, and display it:

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

| Feature                   | Configuration Flag           |
|---------------------------|------------------------------|
| Qt Gamepad Module         | `QRETRO_CONFIG_NO_GAMEPAD`   |
| Qt Positioning Module     | `QRETRO_CONFIG_NO_LOCATION`  |
| [QMidi](https://github.com/waddlesplash/QMidi) submodule | `QRETRO_CONFIG_NO_MIDI`      |
| OpenGL libraries          | `QRETRO_CONFIG_NO_OPENGL`    |
| Qt Sensors Module         | `QRETRO_CONFIG_NO_SENSORS`   |

```qmake
CONFIG += QRETRO_CONFIG_NO_LOCATION

include(QRetro/QRetro.pri)
```

## License

QRetro is licensed under the MIT License.
