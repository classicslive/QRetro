#ifndef QRETRO_DISKCONTROL_H
#define QRETRO_DISKCONTROL_H

#include "libretro.h"

class QRetro;

class QRetroDiskControl
{
public:
  QRetroDiskControl(QRetro *owner)
    : m_owner(owner)
  {
  }

  enum Version
  {
    /// Format used by RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE.
    v0 = 0,

    /// Format used by RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE.
    v1 = 1,

    Invalid
  };

  QRetroDiskControl::Version version(void) { return m_Version; }

  bool setVersion(QRetroDiskControl::Version version)
  {
    if (version > m_MaxVersion)
      return false;
    m_Version = version;

    return true;
  }

  QRetroDiskControl::Version maxVersion(void) { return m_MaxVersion; }

  bool setMaxVersion(QRetroDiskControl::Version version)
  {
    if (version < m_Version)
      return false;
    m_MaxVersion = version;

    return true;
  }

  bool setInterface(const retro_disk_control_callback *callback);
  bool setExtInterface(const retro_disk_control_ext_callback *callback);

  /* v0 */

  bool setEjectState(bool ejected);
  bool getEjectState(void);
  unsigned getImageIndex(void);
  bool setImageIndex(unsigned index);
  unsigned getNumImages(void);
  bool replaceImageIndex(unsigned index, const retro_game_info *info);
  bool addImageIndex(void);

  /* v1 */

  bool setInitialImage(unsigned index, const char *path);
  bool getImagePath(unsigned index, char *s, size_t len);
  bool getImageLabel(unsigned index, char *s, size_t len);

private:
  QRetro *m_owner = nullptr;
  retro_disk_control_ext_callback m_Callback = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  /// The version of disk control interface currently in use. This is set by
  /// the type of interface sent by the core.
  QRetroDiskControl::Version m_Version = QRetroDiskControl::Invalid;

  /// The reported maximum version of disk control interface supported.
  /// The frontend can pretend to not support the extension interface by
  /// setting this with `setMaxVersion`.
  QRetroDiskControl::Version m_MaxVersion = QRetroDiskControl::v1;
};

#endif
