#include "QRetroProcAddress.h"

QRetroProcAddress::QRetroProcAddress()
{
}

void QRetroProcAddress::add(const char *sym, retro_proc_address_t ptr)
{
  m_Functions.emplace(sym, ptr);
}

bool QRetroProcAddress::call(const char *sym)
{
  auto ptr = get(sym);

  if (ptr)
  {
    ptr();
    return true;
  }

  return false;
}

retro_proc_address_t QRetroProcAddress::get(const char *sym)
{
  auto entry = m_Functions.find(sym);

  return entry == m_Functions.end() ? nullptr : entry->second;
}

void QRetroProcAddress::remove(const char *sym)
{
  m_Functions.erase(sym);
}
