# QRetro

<img width="640" height="480" alt="image" src="https://github.com/user-attachments/assets/752211c2-6b24-46b7-bbcf-d4238a8a837c" />

**QRetro** is a [libretro API](https://www.libretro.com/) frontend implemented entirely within a Qt [QWindow](https://doc.qt.io/qt-6/qwindow.html). It supports multi-instancing, allowing multiple instances of libretro cores to run simultaneously and self-contained.

<img width="822" height="429" alt="image" src="https://github.com/user-attachments/assets/2dd856e7-1573-4f4d-856e-ddf4cdfb2c1a" />

An extensive configuration menu is available by pressing **Shift+F2**, providing access to nearly every libretro feature, useful for both end users and core creators/debuggers.

---

## Getting Started

To use QRetro in your Qt-based project, choose one of the following approaches:

### Static Compilation

Clone this repository and include the `.pri` file in your qmake project:

```qmake
include(QRetro/QRetro.pri)
```

### Linking the QRetro Binary

Alternatively (not recommended), you can build QRetro using the `.pro` file and link against the resulting binary.

## Usage

### QWindow

To instantiate and display a QRetro instance:

```cpp
#include <QRetro.h>
   
auto retro = new QRetro();

retro->loadCore("C:/core_libretro.dll");
retro->loadContent("C:/content.bin");
retro->startCore();
retro->show();
```

This will display the core in its own window:

<img height="480" alt="image" src="https://github.com/user-attachments/assets/252a635f-2cb9-43c1-9479-ab299d4b5bc9" />

### QWidget

You can wrap QRetro inside a QWidget for integration into a Qt UI layout like so:

```cpp
auto retrowidget = QWidget::createWindowContainer(retro);
retrowidget->show();
```

<img height="480" alt="image" src="https://github.com/user-attachments/assets/1b4f0007-ef3e-4e5c-a383-44055fa683c3" />

## Demo

An example program that initializes QRetro instances is available in `examples/minimal`. Simply open the `minimal.pro` file in Qt Creator or compile it manually.

## Compatibility

QRetro aims to support any libretro core and as many libretro extensions as possible.

For detailed compatibility information:

* [https://github.com/classicslive/QRetro/blob/master/docs/Cores.md](https://github.com/classicslive/QRetro/blob/master/docs/Cores.md)
* [https://github.com/classicslive/QRetro/blob/master/docs/Environment.md](https://github.com/classicslive/QRetro/blob/master/docs/Environment.md)

## Building

### Required Qt Modules

* `core`
* `gui`

### Optional Modules & Configuration Flags

These dependencies can be disabled via `CONFIG` flags in your qmake project:

| Dependency                                               | Configuration Flag            |
| -------------------------------------------------------- | ----------------------------- |
| QCamera / Qt multimedia module                           | `QRETRO_CONFIG_NO_CAMERA`     |
| Qt gamepad module                                        | `QRETRO_CONFIG_NO_GAMEPAD`    |
| Qt positioning module                                    | `QRETRO_CONFIG_NO_LOCATION`   |
| [QMidi](https://github.com/waddlesplash/QMidi) submodule | `QRETRO_CONFIG_NO_MIDI`       |
| Qt multimedia module                                     | `QRETRO_CONFIG_NO_MULTIMEDIA` |
| Qt opengl module / OpenGL libraries                      | `QRETRO_CONFIG_NO_OPENGL`     |
| SDL3 input module                                        | `QRETRO_CONFIG_NO_SDL3`       |
| Qt sensors module                                        | `QRETRO_CONFIG_NO_SENSORS`    |
| Qt Mobility systeminfo module                            | `QRETRO_CONFIG_NO_SYSTEMINFO` |

Example:

```qmake
CONFIG += QRETRO_CONFIG_NO_LOCATION

include(QRetro/QRetro.pri)
```

## License

QRetro is licensed under the MIT License.
