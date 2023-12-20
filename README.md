# QRetro

QRetro is a libretro API frontend implemented within a Qt QWindow. It supports multi-instancing, allowing multiple instances of libretro cores simultaneously.

## Usage

   ```c++
   #include <QRetro.h>
   
   auto retro = new QRetro();
   
   retro->loadCore("press_f_libretro.dll");
   retro->loadContent("my_game.bin");
   retro->startCore();
   retro->show();
   ```

### OpenGL context

If you plan to use hardware-accelerated cores using OpenGL, include the following line before initializing your QApplication:

```c++
QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
```

## License

QRetro is licensed under the MIT License.
