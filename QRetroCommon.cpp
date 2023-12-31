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

int lr2qt_kayboard(retro_key key)
{
  switch (key)
  {
  case RETROK_a:
    return Qt::Key_A;
  case RETROK_b:
    return Qt::Key_B;
  case RETROK_c:
    return Qt::Key_C;
  case RETROK_d:
    return Qt::Key_D;
  case RETROK_e:
    return Qt::Key_E;
  case RETROK_f:
    return Qt::Key_F;
  case RETROK_g:
    return Qt::Key_G;
  case RETROK_h:
    return Qt::Key_H;
  case RETROK_i:
    return Qt::Key_I;
  case RETROK_j:
    return Qt::Key_J;
  case RETROK_k:
    return Qt::Key_K;
  case RETROK_l:
    return Qt::Key_L;
  case RETROK_m:
    return Qt::Key_M;
  case RETROK_n:
    return Qt::Key_N;
  case RETROK_o:
    return Qt::Key_O;
  case RETROK_p:
    return Qt::Key_P;
  case RETROK_q:
    return Qt::Key_Q;
  case RETROK_r:
    return Qt::Key_R;
  case RETROK_s:
    return Qt::Key_S;
  case RETROK_t:
    return Qt::Key_T;
  case RETROK_u:
    return Qt::Key_U;
  case RETROK_v:
    return Qt::Key_V;
  case RETROK_w:
    return Qt::Key_W;
  case RETROK_x:
    return Qt::Key_X;
  case RETROK_y:
    return Qt::Key_Y;
  case RETROK_z:
    return Qt::Key_Z;
  case RETROK_0:
    return Qt::Key_0;
  case RETROK_1:
    return Qt::Key_1;
  case RETROK_2:
    return Qt::Key_2;
  case RETROK_3:
    return Qt::Key_3;
  case RETROK_4:
    return Qt::Key_4;
  case RETROK_5:
    return Qt::Key_5;
  case RETROK_6:
    return Qt::Key_6;
  case RETROK_7:
    return Qt::Key_7;
  case RETROK_8:
    return Qt::Key_8;
  case RETROK_9:
    return Qt::Key_9;
  case RETROK_UP:
    return Qt::Key_Up;
  case RETROK_DOWN:
    return Qt::Key_Down;
  case RETROK_LEFT:
    return Qt::Key_Left;
  case RETROK_RIGHT:
    return Qt::Key_Right;
  case RETROK_SPACE:
    return Qt::Key_Space;
  case RETROK_RETURN:
    return Qt::Key_Return;
  case RETROK_ESCAPE:
    return Qt::Key_Escape;
  case RETROK_TAB:
    return Qt::Key_Tab;
  case RETROK_BACKSPACE:
    return Qt::Key_Backspace;
  case RETROK_DELETE:
    return Qt::Key_Delete;
  case RETROK_INSERT:
    return Qt::Key_Insert;
  case RETROK_HOME:
    return Qt::Key_Home;
  case RETROK_END:
    return Qt::Key_End;
  case RETROK_PAGEUP:
    return Qt::Key_PageUp;
  case RETROK_PAGEDOWN:
    return Qt::Key_PageDown;
  case RETROK_LSHIFT:
    return Qt::Key_Shift;
  case RETROK_LCTRL:
    return Qt::Key_Control;
  case RETROK_LALT:
    return Qt::Key_Alt;
  case RETROK_F1:
    return Qt::Key_F1;
  case RETROK_F2:
    return Qt::Key_F2;
  case RETROK_F3:
    return Qt::Key_F3;
  case RETROK_F4:
    return Qt::Key_F4;
  case RETROK_F5:
    return Qt::Key_F5;
  case RETROK_F6:
    return Qt::Key_F6;
  case RETROK_F7:
    return Qt::Key_F7;
  case RETROK_F8:
    return Qt::Key_F8;
  case RETROK_F9:
    return Qt::Key_F9;
  case RETROK_F10:
    return Qt::Key_F10;
  case RETROK_F11:
    return Qt::Key_F11;
  case RETROK_F12:
    return Qt::Key_F12;
  case RETROK_F13:
    return Qt::Key_F13;
  case RETROK_F14:
    return Qt::Key_F14;
  case RETROK_F15:
    return Qt::Key_F15;
  case RETROK_EQUALS:
    return Qt::Key_Equal;
  case RETROK_MINUS:
    return Qt::Key_Minus;
  case RETROK_LEFTBRACKET:
    return Qt::Key_BracketLeft;
  case RETROK_RIGHTBRACKET:
    return Qt::Key_BracketRight;
  case RETROK_SEMICOLON:
    return Qt::Key_Semicolon;
  case RETROK_QUOTE:
    return Qt::Key_Apostrophe;
  case RETROK_QUOTEDBL:
    return Qt::Key_QuoteDbl;
  case RETROK_COMMA:
    return Qt::Key_Comma;
  case RETROK_PERIOD:
    return Qt::Key_Period;
  case RETROK_SLASH:
    return Qt::Key_Slash;
  case RETROK_BACKSLASH:
    return Qt::Key_Backslash;
  case RETROK_PLUS:
    return Qt::Key_Plus;
  case RETROK_COLON:
    return Qt::Key_Colon;
  case RETROK_LESS:
    return Qt::Key_Less;
  case RETROK_GREATER:
    return Qt::Key_Greater;
  case RETROK_QUESTION:
    return Qt::Key_Question;
  case RETROK_AT:
    return Qt::Key_At;
  case RETROK_CARET:
    return Qt::Key_AsciiCircum;
  case RETROK_UNDERSCORE:
    return Qt::Key_Underscore;
  case RETROK_BACKQUOTE:
    return Qt::Key_QuoteLeft;
  case RETROK_LEFTBRACE:
    return Qt::Key_BraceLeft;
  case RETROK_BAR:
    return Qt::Key_Bar;
  case RETROK_RIGHTBRACE:
    return Qt::Key_BraceRight;
  case RETROK_TILDE:
    return Qt::Key_AsciiTilde;
  case RETROK_CLEAR:
    return Qt::Key_Clear;
  case RETROK_PAUSE:
    return Qt::Key_Pause;
  case RETROK_EXCLAIM:
    return Qt::Key_Exclam;
  case RETROK_HASH:
    return Qt::Key_NumberSign;
  case RETROK_DOLLAR:
    return Qt::Key_Dollar;
  case RETROK_AMPERSAND:
    return Qt::Key_Ampersand;
  case RETROK_LEFTPAREN:
    return Qt::Key_ParenLeft;
  case RETROK_RIGHTPAREN:
    return Qt::Key_ParenRight;
  case RETROK_ASTERISK:
    return Qt::Key_Asterisk;
  case RETROK_HELP:
    return Qt::Key_Help;
  case RETROK_PRINT:
    return Qt::Key_Print;
  case RETROK_SYSREQ:
    return Qt::Key_SysReq;
  case RETROK_MENU:
    return Qt::Key_Menu;
  case RETROK_POWER:
    return Qt::Key_PowerOff;
  case RETROK_UNDO:
    return Qt::Key_Undo;

  /** @todo These keys are currently unsupported in both functions. */
  case RETROK_KP0:
  case RETROK_KP1:
  case RETROK_KP2:
  case RETROK_KP3:
  case RETROK_KP4:
  case RETROK_KP5:
  case RETROK_KP6:
  case RETROK_KP7:
  case RETROK_KP8:
  case RETROK_KP9:
  case RETROK_KP_PERIOD:
  case RETROK_KP_DIVIDE:
  case RETROK_KP_ENTER:
  case RETROK_KP_EQUALS:
  case RETROK_KP_MINUS:
  case RETROK_KP_MULTIPLY:
  case RETROK_KP_PLUS:
  case RETROK_NUMLOCK:
  case RETROK_CAPSLOCK:
  case RETROK_SCROLLOCK:
  case RETROK_RSHIFT:
  case RETROK_BREAK:
  case RETROK_RCTRL:
  case RETROK_RALT:
  case RETROK_RMETA:
  case RETROK_LMETA:
  case RETROK_LSUPER:
  case RETROK_RSUPER:
  case RETROK_MODE:
  case RETROK_COMPOSE:
  case RETROK_EURO:
  case RETROK_OEM_102:

  case RETROK_UNKNOWN:
  case RETROK_LAST:
  case RETROK_DUMMY:
    return Qt::Key_unknown;
  }

  return Qt::Key_unknown;
}

retro_key qt2lr_keyboard(int key)
{
  switch (key)
  {
  case Qt::Key_A:
    return RETROK_a;
  case Qt::Key_B:
    return RETROK_b;
  case Qt::Key_C:
    return RETROK_c;
  case Qt::Key_D:
    return RETROK_d;
  case Qt::Key_E:
    return RETROK_e;
  case Qt::Key_F:
    return RETROK_f;
  case Qt::Key_G:
    return RETROK_g;
  case Qt::Key_H:
    return RETROK_h;
  case Qt::Key_I:
    return RETROK_i;
  case Qt::Key_J:
    return RETROK_j;
  case Qt::Key_K:
    return RETROK_k;
  case Qt::Key_L:
    return RETROK_l;
  case Qt::Key_M:
    return RETROK_m;
  case Qt::Key_N:
    return RETROK_n;
  case Qt::Key_O:
    return RETROK_o;
  case Qt::Key_P:
    return RETROK_p;
  case Qt::Key_Q:
    return RETROK_q;
  case Qt::Key_R:
    return RETROK_r;
  case Qt::Key_S:
    return RETROK_s;
  case Qt::Key_T:
    return RETROK_t;
  case Qt::Key_U:
    return RETROK_u;
  case Qt::Key_V:
    return RETROK_v;
  case Qt::Key_W:
    return RETROK_w;
  case Qt::Key_X:
    return RETROK_x;
  case Qt::Key_Y:
    return RETROK_y;
  case Qt::Key_Z:
    return RETROK_z;
  case Qt::Key_0:
    return RETROK_0;
  case Qt::Key_1:
    return RETROK_1;
  case Qt::Key_2:
    return RETROK_2;
  case Qt::Key_3:
    return RETROK_3;
  case Qt::Key_4:
    return RETROK_4;
  case Qt::Key_5:
    return RETROK_5;
  case Qt::Key_6:
    return RETROK_6;
  case Qt::Key_7:
    return RETROK_7;
  case Qt::Key_8:
    return RETROK_8;
  case Qt::Key_9:
    return RETROK_9;
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
  case Qt::Key_Escape:
    return RETROK_ESCAPE;
  case Qt::Key_Tab:
    return RETROK_TAB;
  case Qt::Key_Backspace:
    return RETROK_BACKSPACE;
  case Qt::Key_Delete:
    return RETROK_DELETE;
  case Qt::Key_Insert:
    return RETROK_INSERT;
  case Qt::Key_Home:
    return RETROK_HOME;
  case Qt::Key_End:
    return RETROK_END;
  case Qt::Key_PageUp:
    return RETROK_PAGEUP;
  case Qt::Key_PageDown:
    return RETROK_PAGEDOWN;
  case Qt::Key_Shift:
    return RETROK_LSHIFT;
  case Qt::Key_Control:
    return RETROK_LCTRL;
  case Qt::Key_Alt:
    return RETROK_LALT;
  case Qt::Key_F1:
    return RETROK_F1;
  case Qt::Key_F2:
    return RETROK_F2;
  case Qt::Key_F3:
    return RETROK_F3;
  case Qt::Key_F4:
    return RETROK_F4;
  case Qt::Key_F5:
    return RETROK_F5;
  case Qt::Key_F6:
    return RETROK_F6;
  case Qt::Key_F7:
    return RETROK_F7;
  case Qt::Key_F8:
    return RETROK_F8;
  case Qt::Key_F9:
    return RETROK_F9;
  case Qt::Key_F10:
    return RETROK_F10;
  case Qt::Key_F11:
    return RETROK_F11;
  case Qt::Key_F12:
    return RETROK_F12;
  case Qt::Key_F13:
    return RETROK_F13;
  case Qt::Key_F14:
    return RETROK_F14;
  case Qt::Key_F15:
    return RETROK_F15;
  case Qt::Key_Equal:
    return RETROK_EQUALS;
  case Qt::Key_Minus:
    return RETROK_MINUS;
  case Qt::Key_BracketLeft:
    return RETROK_LEFTBRACKET;
  case Qt::Key_BracketRight:
    return RETROK_RIGHTBRACKET;
  case Qt::Key_Semicolon:
    return RETROK_SEMICOLON;
  case Qt::Key_Apostrophe:
    return RETROK_QUOTE;
  case Qt::Key_QuoteDbl:
    return RETROK_QUOTEDBL;
  case Qt::Key_Comma:
    return RETROK_COMMA;
  case Qt::Key_Period:
    return RETROK_PERIOD;
  case Qt::Key_Slash:
    return RETROK_SLASH;
  case Qt::Key_Backslash:
    return RETROK_BACKSLASH;
  case Qt::Key_Plus:
    return RETROK_PLUS;
  case Qt::Key_Colon:
    return RETROK_COLON;
  case Qt::Key_Less:
    return RETROK_LESS;
  case Qt::Key_Greater:
    return RETROK_GREATER;
  case Qt::Key_Question:
    return RETROK_QUESTION;
  case Qt::Key_At:
    return RETROK_AT;
  case Qt::Key_AsciiCircum:
    return RETROK_CARET;
  case Qt::Key_Underscore:
    return RETROK_UNDERSCORE;
  case Qt::Key_QuoteLeft:
    return RETROK_BACKQUOTE;
  case Qt::Key_BraceLeft:
    return RETROK_LEFTBRACE;
  case Qt::Key_Bar:
    return RETROK_BAR;
  case Qt::Key_BraceRight:
    return RETROK_RIGHTBRACE;
  case Qt::Key_AsciiTilde:
    return RETROK_TILDE;
  case Qt::Key_Clear:
    return RETROK_CLEAR;
  case Qt::Key_Pause:
    return RETROK_PAUSE;
  case Qt::Key_Exclam:
    return RETROK_EXCLAIM;
  case Qt::Key_NumberSign:
    return RETROK_HASH;
  case Qt::Key_Dollar:
    return RETROK_DOLLAR;
  case Qt::Key_Ampersand:
    return RETROK_AMPERSAND;
  case Qt::Key_ParenLeft:
    return RETROK_LEFTPAREN;
  case Qt::Key_ParenRight:
    return RETROK_RIGHTPAREN;
  case Qt::Key_Asterisk:
    return RETROK_ASTERISK;
  case Qt::Key_Help:
    return RETROK_HELP;
  case Qt::Key_Print:
    return RETROK_PRINT;
  case Qt::Key_SysReq:
    return RETROK_SYSREQ;
  case Qt::Key_Menu:
    return RETROK_MENU;
  case Qt::Key_PowerDown:
  case Qt::Key_PowerOff:
    return RETROK_POWER;
  case Qt::Key_Undo:
    return RETROK_UNDO;
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

static_assert(RETRO_LANGUAGE_LAST == RETRO_LANGUAGE_BELARUSIAN + 1,
              "Update libretro language values!");
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
