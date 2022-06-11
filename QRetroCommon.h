#ifndef QRETRO_COMMON_H
#define QRETRO_COMMON_H

#include <map>
#include <thread>

#include "QRetro.h"

#define QRETRO_ERROR(a) _qrerror(__FILE__, __LINE__, a)
QString _qrerror(const char *file, int line, QString msg);

QRetro* _qrthis();

retro_pixel_format qt2lr_pixel(const QImage::Format qt);
retro_key qt2lr_keyboard(int key);
uint16_t qt2lr_keymod(Qt::KeyboardModifiers qt);

QImage::Format lr2qt_pixel(const retro_pixel_format lr);

extern std::map<std::thread::id, QRetro*> thread_map;

#endif
