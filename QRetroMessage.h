#ifndef QRETRO_MESSAGE_H
#define QRETRO_MESSAGE_H

#include <QString>
#include <QList>
#include <QObject>

#include "libretro.h"

struct QRetroMessageEntry
{
  QString message;
  unsigned duration = 0;
  unsigned priority = 0;
  retro_log_level level = RETRO_LOG_INFO;
  retro_message_target target = RETRO_MESSAGE_TARGET_ALL;
  retro_message_type type = RETRO_MESSAGE_TYPE_NOTIFICATION;
  int8_t progress = -1;
};

Q_DECLARE_METATYPE(QRetroMessageEntry)

class QRetroMessage : public QObject
{
  Q_OBJECT

public:
  enum InterfaceVersion : unsigned
  {
    SetMessageOnly = 0,
    SetMessageExt = 1,
  };

  explicit QRetroMessage(QObject *parent = nullptr, int maxEntries = 200);

  unsigned interfaceVersion(void) const { return static_cast<unsigned>(m_InterfaceVersion); }
  void setInterfaceVersion(unsigned v) { m_InterfaceVersion = static_cast<InterfaceVersion>(v); }

  void push(const QString &message);
  void push(const retro_message_ext *ext);

  const QList<QRetroMessageEntry> &entries() const { return m_Entries; }

  void clear() { m_Entries.clear(); }

  int maxEntries() const { return m_MaxEntries; }
  void setMaxEntries(int n) { m_MaxEntries = n; }

signals:
  void onMessage(QRetroMessageEntry entry);

private:
  InterfaceVersion m_InterfaceVersion = SetMessageExt;
  int m_MaxEntries;
  QList<QRetroMessageEntry> m_Entries;
};

#endif
