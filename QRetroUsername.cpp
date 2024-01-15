#include "QRetroUsername.h"

#include <QCoreApplication>
#include <QDir>

QRetroUsername::QRetroUsername(void)
{
  setFromApplication();
}

const char* QRetroUsername::get(void)
{
  return m_Username.constData();
}

void QRetroUsername::set(const char *username)
{
  auto qusername = QString(username);

  qusername.resize(QRETRO_USERNAME_LENGTH);
  m_Username = qusername.toUtf8();
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
