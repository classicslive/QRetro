#ifndef QRETRO_PROCADDRESS_H
#define QRETRO_PROCADDRESS_H

#include <libretro.h>
#include <map>
#include <string>

/**
 * A class managing pointers to functions that return nothing and take no
 * arguments, known as retro_proc_address_t, allowing the frontend to directly
 * call functions the core provides.
 * This functionality is only lightly tested, and it is unknown which (if any)
 * cores support it.
 */
class QRetroProcAddress
{
public:
  QRetroProcAddress();

  void add(const char *sym, retro_proc_address_t ptr);
  bool call(const char *sym);
  retro_proc_address_t get(const char *sym);
  bool init(const struct retro_get_proc_address_interface *iface);
  void remove(const char *sym);

private:
  std::map<std::string, retro_proc_address_t> m_Functions;
  struct retro_get_proc_address_interface m_Interface = { nullptr };
};

#endif
