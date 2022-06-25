#include "QRetroCommon.h"

static std::map<std::thread::id, QRetro*> thread_map;

/*
 * TODO: Actually removing it triggers a crash, probably due to race conditions
 * with other instances. Fix so we don't keep a map of dead pointers.
 */
bool _qrdelete(QRetro *retro)
{
  if (!retro)
    return false;
  for (auto it = thread_map.begin(); it != thread_map.end(); it++)
  {
    if (it->second == retro)
    {
      it->second = nullptr;
      return true;
    }
  }

  return false;
}

QString _qrerror(const char *file, int line, QString msg)
{
  return QString("Error in %1, line %2:\n\n%3").arg(
    QString(file),
    QString::number(line),
    msg);
}

bool _qrnew(std::thread::id id, QRetro* retro)
{
  if (!retro)
    return false;
  else
    thread_map.insert(std::pair<std::thread::id, QRetro*>(id, retro));

  return true;
}

/*
  To get around needing to send static function pointers to the core while
  still accessing member variables, we create a map of emulation timing
  thread IDs to the QRetro objects they're instantiated from.

  Using "_qrthis" will return what is effectively "this" in member
  contexts, or NULL if the thread ID doesn't reference a managed
  QRetro object.
*/
QRetro* _qrthis()
{
  auto it = thread_map.find(std::this_thread::get_id());

  if (it == thread_map.end())
  {
    auto last_entry = thread_map.rbegin()->second;

    thread_map.insert(std::pair<std::thread::id, QRetro*>(
                      std::this_thread::get_id(), last_entry));

    return last_entry;
  }
  else
    return it->second;
}

/* Converts a Qt pixel format enum to its libretro equivalent */
retro_pixel_format qt2lr_pixel(const QImage::Format qt)
{
  switch (qt)
  {
  case QImage::Format_RGB555:
    return RETRO_PIXEL_FORMAT_0RGB1555;
  case QImage::Format_RGB16:
    return RETRO_PIXEL_FORMAT_RGB565;
  case QImage::Format_RGBA8888:
  case QImage::Format_RGBX8888:
  case QImage::Format_RGB32:
  case QImage::Format_ARGB32:
    return RETRO_PIXEL_FORMAT_XRGB8888;
  default:
    return RETRO_PIXEL_FORMAT_UNKNOWN;
  }
}

/* Converts a libretro pixel format enum to its Qt equivalent */
QImage::Format lr2qt_pixel(const retro_pixel_format lr)
{
  switch (lr)
  {
  case RETRO_PIXEL_FORMAT_0RGB1555:
    return QImage::Format_RGB555;
  case RETRO_PIXEL_FORMAT_RGB565:
    return QImage::Format_RGB16;
  case RETRO_PIXEL_FORMAT_XRGB8888:
    return QImage::Format_RGB32;
  default:
    return QImage::Format_Invalid;
  }
}

#define QT2LRK_RANGE(a, b, c, d) \
  if (a >= b && a <= c) \
    return static_cast<retro_key>(static_cast<Qt::Key>(d) + a - b)
retro_key qt2lr_keyboard(int key)
{
  QT2LRK_RANGE(key, Qt::Key_A, Qt::Key_Z, RETROK_a);
  QT2LRK_RANGE(key, Qt::Key_0, Qt::Key_9, RETROK_0);
  QT2LRK_RANGE(key, Qt::Key_F1, Qt::Key_F15, RETROK_F15);
  //QT2LRK_RANGE(Qt::Key_, Qt::Key_, RETROK_KP0) TODO: Tenkey, with Qt::KeypadModifier

  // TODO: Add more keys here
  switch (key)
  {
  case Qt::Key_Space:
    return RETROK_SPACE;
  default:
    return RETROK_UNKNOWN;
  }
}

/* TODO: Caps lock, numlock, etc */
uint16_t qt2lr_keymod(Qt::KeyboardModifiers qt)
{
  uint16_t lr = 0;

  lr |= qt.testFlag(Qt::ShiftModifier)   ? RETROKMOD_SHIFT : 0;
  lr |= qt.testFlag(Qt::ControlModifier) ? RETROKMOD_CTRL  : 0;
  lr |= qt.testFlag(Qt::AltModifier)     ? RETROKMOD_ALT   : 0;
  lr |= qt.testFlag(Qt::MetaModifier)    ? RETROKMOD_META  : 0;

  return lr;
}
