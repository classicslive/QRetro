#ifndef QRETRO_DIRECTORIES_H
#define QRETRO_DIRECTORIES_H

#include <string>
#include <QString>

/**
 * Sets the path used for a RETRO_ENVIRONMENT_GET_X_DIRECTORY environment
 * callback.
 * It is recommended to handle this before calling "loadCore", as many
 * cores call this once in "retro_init" and store a copy.

 * Returns FALSE if "type" is invalid, or if "path" does not exist, otherwise
 * TRUE.
 **/
class QRetroDirectories
{
public:
  enum Type
  {
    /// Returned when the core uses environment callback
    /// RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY (31).
    Save = 0,

    /// Returned when the core uses environment callback
    /// RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY (9).
    System,

    /// Returned when the core uses environment callback
    /// RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY (30).
    CoreAssets,

    /// Alias of Directory_CoreAssets.
    Content = CoreAssets,

    /* The following are internal, unrelated to libretro callbacks.
    State,
    CoreInfo,
    ContentMeta,
    ...but they're also completely unused right now. */

    /// Size of Directory enum.
    Type_Size
  };

  QRetroDirectories();

  const char* get(QRetroDirectories::Type);

  bool set(QRetroDirectories::Type type, const char *path, bool force = false);
  bool set(QRetroDirectories::Type type, const std::string &path, bool force = false);
  bool set(QRetroDirectories::Type type, const QString &path, bool force = false);

private:
  QString m_Directories[QRetroDirectories::Type_Size];
};

#endif
