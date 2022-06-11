#include "QRetroUsername.h"

#include <QCoreApplication>
#include <QDir>

QRetroUsername::QRetroUsername()
{
  setFromApplication();
}

void QRetroUsername::set(const char *username)
{
  strncpy_s(m_Username, username, QRETRO_USERNAME_LENGTH);
}

void QRetroUsername::set(const std::string &username)
{
  set(username.c_str());
}

void QRetroUsername::set(const QString &username)
{
  set(username.toStdString());
}

void QRetroUsername::setFromApplication(void)
{
  set(QCoreApplication::applicationName());
}

void QRetroUsername::setFromSystem(void)
{
  set(QDir::home().dirName());
}
