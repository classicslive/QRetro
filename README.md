# QRetro

QRetro is a libretro API frontend implemented within a Qt QWindow. It supports multi-instancing, allowing multiple instances of libretro cores simultaneously.

## Usage

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
auto RetroWidget = QWidget::createWindowContainer(retro);
RetroWidget->show();
```

### OpenGL context

If you plan to use hardware-accelerated cores using OpenGL, include the following line before initializing your QApplication:

```c++
QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
```

## Building

QRetro requires the following Qt modules:
- `core`
- `gui`
- `gamepad`
- `multimedia`

The following modules are optional, and are only used if the associated compile-time option is set:
- `positioning` : `QRETRO_HAVE_LOCATION`
- `sensors` : `QRETRO_HAVE_SENSORS`

## License

QRetro is licensed under the MIT License.
