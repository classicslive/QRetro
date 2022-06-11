#include "QRetroDirectories.h"

#include <QDir>

QRetroDirectories::QRetroDirectories()
{
  QString dir;

  /* TODO: Ugly */

  dir = QDir::currentPath() + "/save";
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  strncpy(m_Directories[QRetroDirectories::Save], dir.toStdString().c_str(), PATHLEN_TEMP);

  dir = QDir::currentPath() + "/system";
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  strncpy(m_Directories[QRetroDirectories::System], dir.toStdString().c_str(), PATHLEN_TEMP);

  dir = QDir::currentPath() + "/assets";
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  strncpy(m_Directories[QRetroDirectories::CoreAssets], dir.toStdString().c_str(), PATHLEN_TEMP);
}

const char* QRetroDirectories::get(QRetroDirectories::Type type)
{
  if (type >= QRetroDirectories::Type_Size)
    return nullptr;

  return m_Directories[type];
}

bool QRetroDirectories::set(QRetroDirectories::Type type, const char *path, bool force)
{
  if (type >= QRetroDirectories::Type_Size)
    return false;
  else if (!QDir(path).exists() && !force)
    return false;

  strncpy(m_Directories[type], path, PATHLEN_TEMP);
  return true;
}

bool QRetroDirectories::set(QRetroDirectories::Type type, const std::string &path, bool force)
{
  return set(type, path.c_str(), force);
}

bool QRetroDirectories::set(QRetroDirectories::Type type, const QString &path, bool force)
{
  return set(type, path.toStdString(), force);
}
