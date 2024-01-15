#ifndef QRETRO_USERNAME_H
#define QRETRO_USERNAME_H

#include <QString>

#define QRETRO_USERNAME_LENGTH 32

class QRetroUsername
{
public:
  QRetroUsername(void);

  const char* get(void);
  void set(const char *username);
  void set(const std::string &username);
  void set(const QString &username);

  /**
   * Sets the username to that of the current Qt application name.
   */
  void setFromApplication(void);

  /**
   * Sets the username to that of the operating system's user account.
   */
  void setFromSystem(void);

private:
  QByteArray m_Username;
};

#endif
