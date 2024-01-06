#include "QRetroUsername.h"

#include <QCoreApplication>
#include <QDir>

QRetroUsername::QRetroUsername(void)
{
  setFromApplication();
}

const char* QRetroUsername::get(void)
{
  auto name = m_Username.toUtf8();
  return name.constData();
}

void QRetroUsername::set(const char *username)
{
  m_Username = QString(username);
  m_Username.resize(QRETRO_USERNAME_LENGTH);
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
