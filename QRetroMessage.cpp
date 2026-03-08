#include "QRetroMessage.h"

QRetroMessage::QRetroMessage(QObject *parent, int maxEntries)
  : QObject(parent)
  , m_MaxEntries(maxEntries)
{
  qRegisterMetaType<QRetroMessageEntry>();
}

void QRetroMessage::push(const QString &message)
{
  if (m_Entries.size() >= m_MaxEntries)
    m_Entries.removeFirst();
  m_Entries.append({ message });
  emit onMessage(m_Entries.last());
}

void QRetroMessage::push(const retro_message_ext *ext)
{
  if (m_Entries.size() >= m_MaxEntries)
    m_Entries.removeFirst();
  m_Entries.append({
    QString::fromUtf8(ext->msg),
    ext->duration,
    ext->priority,
    ext->level,
    ext->target,
    ext->type,
    ext->progress
  });
  emit onMessage(m_Entries.last());
}
