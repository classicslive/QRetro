#ifndef QRETRO_MEMORY_H
#define QRETRO_MEMORY_H

#include <algorithm>
#include <cstring>

#include "libretro.h"

class QRetroMemory
{
public:
  retro_memory_map *memoryMaps(void) { return &m_MemoryMaps; }
  void setMemoryMaps(const struct retro_memory_map *maps);

  /**
   * Reads data from the core's address space into a buffer.
   * @param ptr Pointer to the buffer to read into
   * @param address Address in the core's address space to read from
   * @param size Number of bytes to read
   * @return Number of bytes successfully read
   */
  size_t readBuffer(void *ptr, size_t address, size_t size);

  /**
   * Writes data from a buffer into the core's address space.
   * @param ptr Pointer to the buffer to write from
   * @param address Address in the core's address space to write to
   * @param size Number of bytes to write
   * @return Number of bytes successfully written
   */
  size_t writeBuffer(const void *ptr, size_t address, size_t size);

  /**
   * Reads one value from the core's address space, respecting libretro
   * memory descriptor flags such as endianness and word size.
   * @param value Pointer to the variable to read the value into
   * @param address Address in the core's address space to read from
   * @return Whether the value was successfully read
   */
  template <typename T> bool readValue(T *value, size_t address)
  {
    const retro_memory_descriptor *desc = containing(address);

    if (!desc || !desc->ptr)
      return false;

    size_t offset = address - desc->start + desc->offset;

    if (offset + sizeof(T) > desc->len)
      return false;

    memcpy(value, static_cast<char *>(desc->ptr) + offset, sizeof(T));

    if (desc->flags & RETRO_MEMDESC_BIGENDIAN)
    {
      /* Swap endianness */
      char *bytes = reinterpret_cast<char *>(value);
      for (size_t i = 0; i < sizeof(T) / 2; i++)
        std::swap(bytes[i], bytes[sizeof(T) - 1 - i]);
    }

    return true;
  }

  /**
   * Writes one value into the core's address space, respecting libretro
   * memory descriptor flags such as endianness and word size.
   * @param value The variable to write into the core's address space
   * @param address Address in the core's address space to write to
   * @return Whether the value was successfully written
   */
  template <typename T> bool writeValue(const T &value, size_t address)
  {
    const retro_memory_descriptor *desc = containing(address);

    if (!desc || !desc->ptr)
      return false;

    size_t offset = address - desc->start + desc->offset;

    if (offset + sizeof(T) > desc->len)
      return false;

    if (desc->flags & RETRO_MEMDESC_BIGENDIAN)
    {
      /* Swap endianness */
      char bytes[sizeof(T)];
      memcpy(bytes, &value, sizeof(T));
      for (size_t i = 0; i < sizeof(T) / 2; i++)
        std::swap(bytes[i], bytes[sizeof(T) - 1 - i]);
      memcpy(static_cast<char *>(desc->ptr) + offset, bytes, sizeof(T));
    }
    else
      memcpy(static_cast<char *>(desc->ptr) + offset, &value, sizeof(T));

    return true;
  }

  /**
   * Finds the memory descriptor containing the given address, or nullptr if
   * no such descriptor exists.
   */
  const retro_memory_descriptor *containing(size_t address);

private:
  retro_memory_map m_MemoryMaps = { nullptr, 0 };
};

#endif
