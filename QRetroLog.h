#ifndef QRETRO_LOG_H
#define QRETRO_LOG_H

#include <QString>
#include <QList>

#include "libretro.h"

struct QRetroLogEntry
{
  retro_log_level level;
  QString         message;
};

class QRetroLog
{
public:
  explicit QRetroLog(int maxEntries = 200) : m_MaxEntries(maxEntries) {}

  void push(retro_log_level level, const QString &message)
  {
    if (m_Entries.size() >= m_MaxEntries)
      m_Entries.removeFirst();
    m_Entries.append({ level, message });
  }

  const QList<QRetroLogEntry>& entries() const { return m_Entries; }

  void clear() { m_Entries.clear(); }

  int maxEntries() const { return m_MaxEntries; }
  void setMaxEntries(int n) { m_MaxEntries = n; }

private:
  int                  m_MaxEntries;
  QList<QRetroLogEntry> m_Entries;
};

#endif
