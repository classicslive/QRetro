#include "QRetroDirectories.h"

#include <QDir>

QRetroDirectories::QRetroDirectories()
{
  QString dir;

  dir = QDir::currentPath() + "/save";
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  m_Directories[QRetroDirectories::Save] = dir.toUtf8();

  dir = QDir::currentPath() + "/system";
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  m_Directories[QRetroDirectories::System] = dir.toUtf8();

  dir = QDir::currentPath() + "/assets";
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  m_Directories[QRetroDirectories::CoreAssets] = dir.toUtf8();
}

const char* QRetroDirectories::get(QRetroDirectories::Type type)
{
  if (type >= QRetroDirectories::Type_Size)
    return nullptr;
  else
    return m_Directories[type].constData();
}

bool QRetroDirectories::set(QRetroDirectories::Type type, const QString &path, bool force)
{
  if (type >= QRetroDirectories::Type_Size)
    return false;
  else if (!QDir(path).exists() && !force)
    return false;
  m_Directories[type] = path.toUtf8();

  return true;
}

bool QRetroDirectories::set(QRetroDirectories::Type type, const std::string &path, bool force)
{
  return set(type, QString(path.c_str()), force);
}

bool QRetroDirectories::set(QRetroDirectories::Type type, const char *path, bool force)
{
  return set(type, QString(path), force);
}
