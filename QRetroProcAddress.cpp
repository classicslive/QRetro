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

  if (entry != m_Functions.end())
    return entry->second;
  else if (sym && m_Interface.get_proc_address)
  {
    auto func = m_Interface.get_proc_address(sym);

    if (func)
    {
      add(sym, func);
      return func;
    }
  }

  return nullptr;
}

bool QRetroProcAddress::init(const retro_get_proc_address_interface *interface)
{
  if (interface)
  {
    m_Interface = *interface;
    return true;
  }
  else
    return false;
}

void QRetroProcAddress::remove(const char *sym)
{
  m_Functions.erase(sym);
}
