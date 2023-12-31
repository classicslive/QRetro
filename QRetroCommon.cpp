#include <QLocale>

#include "QRetroCommon.h"

std::map<std::thread::id, QRetro*> _qr_thread_map;

/**
 * @todo Actually removing it triggers a crash, probably due to race conditions
 * with other instances. Fix so we don't keep a map of dead pointers. 
 */
bool _qrdelete(QRetro *retro)
{
  if (!retro)
    return false;
  for (auto it = _qr_thread_map.begin(); it != _qr_thread_map.end(); it++)
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
  return QString("Error in %1, line %2:\n\n%3").arg(QString(file),
                                                    QString::number(line),
                                                    msg);
}

bool _qrnew(std::thread::id id, QRetro* retro)
{
  if (!retro)
    return false;
  else
    _qr_thread_map.insert(std::pair<std::thread::id, QRetro*>(id, retro));

  return true;
}

/**
 * To get around needing to send static function pointers to the core while
 * still accessing member variables, we create a map of emulation timing
 * thread IDs to the QRetro objects they're instantiated from.
 *
 * Using "_qrthis" will return what is effectively "this" in member
 * contexts, or NULL if the thread ID doesn't reference a managed
 * QRetro object.
 *
 * @internal This function is meant for internal use and not for user code.
 */
QRetro* _qrthis()
{
  auto it = _qr_thread_map.find(std::this_thread::get_id());

  if (it == _qr_thread_map.end())
  {
    auto last_entry = _qr_thread_map.rbegin()->second;

    _qr_thread_map.insert(std::pair<std::thread::id, QRetro*>(
                          std::this_thread::get_id(), last_entry));

    return last_entry;
  }
  else
    return it->second;
}

int16_t qt2lr_analog(double qt)
{
  return static_cast<int16_t>(qt * 0x7FFF);
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
  case Qt::Key_Up:
    return RETROK_UP;
  case Qt::Key_Down:
    return RETROK_DOWN;
  case Qt::Key_Left:
    return RETROK_LEFT;
  case Qt::Key_Right:
    return RETROK_RIGHT;
  case Qt::Key_Space:
    return RETROK_SPACE;
  case Qt::Key_Return:
    return RETROK_RETURN;
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

retro_language qt2lr_language(const QLocale &qt)
{
  switch (qt.language())
  {
  case QLocale::English:
    return qt.country() == QLocale::UnitedKingdom ?
      RETRO_LANGUAGE_BRITISH_ENGLISH : RETRO_LANGUAGE_ENGLISH;
  case QLocale::Japanese:
    return RETRO_LANGUAGE_JAPANESE;
  case QLocale::French:
    return RETRO_LANGUAGE_FRENCH;
  case QLocale::German:
    return RETRO_LANGUAGE_GERMAN;
  case QLocale::Spanish:
    return RETRO_LANGUAGE_SPANISH;
  case QLocale::Italian:
    return RETRO_LANGUAGE_ITALIAN;
  case QLocale::Dutch:
    return RETRO_LANGUAGE_DUTCH;
  case QLocale::Portuguese:
    return qt.country() == QLocale::Brazil ?
      RETRO_LANGUAGE_PORTUGUESE_BRAZIL : RETRO_LANGUAGE_PORTUGUESE_PORTUGAL;
  case QLocale::Russian:
    return RETRO_LANGUAGE_RUSSIAN;
  case QLocale::Korean:
    return RETRO_LANGUAGE_KOREAN;
  case QLocale::Chinese:
    return qt.script() == QLocale::TraditionalChineseScript ?
      RETRO_LANGUAGE_CHINESE_TRADITIONAL : RETRO_LANGUAGE_CHINESE_SIMPLIFIED;
  case QLocale::Esperanto:
    return RETRO_LANGUAGE_ESPERANTO;
  case QLocale::Polish:
    return RETRO_LANGUAGE_POLISH;
  case QLocale::Vietnamese:
    return RETRO_LANGUAGE_VIETNAMESE;
  case QLocale::Arabic:
    return RETRO_LANGUAGE_ARABIC;
  case QLocale::Greek:
    return RETRO_LANGUAGE_GREEK;
  case QLocale::Turkish:
    return RETRO_LANGUAGE_TURKISH;
  case QLocale::Slovak:
    return RETRO_LANGUAGE_SLOVAK;
  case QLocale::Persian:
    return RETRO_LANGUAGE_PERSIAN;
  case QLocale::Hebrew:
    return RETRO_LANGUAGE_HEBREW;
  case QLocale::Asturian:
    return RETRO_LANGUAGE_ASTURIAN;
  case QLocale::Finnish:
    return RETRO_LANGUAGE_FINNISH;
  case QLocale::Indonesian:
    return RETRO_LANGUAGE_INDONESIAN;
  case QLocale::Swedish:
    return RETRO_LANGUAGE_SWEDISH;
  case QLocale::Ukrainian:
    return RETRO_LANGUAGE_UKRAINIAN;
  case QLocale::Czech:
    return RETRO_LANGUAGE_CZECH;
  case QLocale::Catalan:
    return RETRO_LANGUAGE_CATALAN; /// @todo Catalan (Valencian)?
  case QLocale::Hungarian:
    return RETRO_LANGUAGE_HUNGARIAN;
  case QLocale::Belarusian:
    return RETRO_LANGUAGE_BELARUSIAN;
  default:
    return RETRO_LANGUAGE_ENGLISH;
  }
}

retro_language qt2lr_language_system(void)
{
  return qt2lr_language(QLocale::system());
}
