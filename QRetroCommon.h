#ifndef QRETRO_COMMON_H
#define QRETRO_COMMON_H

#include <map>
#include <thread>

#include "QRetro.h"

extern std::map<std::thread::id, QRetro*> _qr_thread_map;

#define QRETRO_ERROR(a) _qrerror(__FILE__, __LINE__, a)
QString _qrerror(const char *file, int line, QString msg);

/**
 * Removes a QRetro instance from the internal static thread map.
 * NOT MEANT FOR USE IN USER CODE, as it is called automatically during a
 * QRetro object's deconstruction.
 * @param retro A pointer to a QRetro object.
 * @return true if the instance is successfully removed, false otherwise.
 */
bool _qrdelete(QRetro *retro);

/**
 * Adds a QRetro instance to the internal static thread map.
 * NOT MEANT FOR USE IN USER CODE, as it is called automatically during a
 * QRetro object's lifetime.
 * @return true if the instance is successfully added, false otherwise.
 */
bool _qrnew(std::thread::id id, QRetro* retro);

QRetro* _qrthis();

retro_pixel_format qt2lr_pixel(const QImage::Format qt);
retro_key qt2lr_keyboard(int key);
uint16_t qt2lr_keymod(Qt::KeyboardModifiers qt);

QImage::Format lr2qt_pixel(const retro_pixel_format lr);

#endif
