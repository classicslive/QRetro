#include "QRetroMemory.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

const retro_memory_descriptor *QRetroMemory::containing(size_t address)
{
  if (!m_MemoryMaps.descriptors)
    return nullptr;

  for (unsigned i = 0; i < m_MemoryMaps.num_descriptors; i++)
  {
    auto &desc = m_MemoryMaps.descriptors[i];
    if (address >= desc.start && address < desc.start + desc.len)
      return &desc;
  }
  return nullptr;
}

void QRetroMemory::setMemoryMaps(const struct retro_memory_map *maps)
{
  m_MemoryMaps.num_descriptors = maps->num_descriptors;
  auto descs = static_cast<retro_memory_descriptor *>(
    calloc(m_MemoryMaps.num_descriptors, sizeof(retro_memory_descriptor)));

  /* Do a deep copy */
  for (unsigned i = 0; i < m_MemoryMaps.num_descriptors; i++)
  {
    auto dst = &descs[i];
    auto src = &maps->descriptors[i];

    memcpy(dst, src, sizeof(retro_memory_descriptor));
    if (src->addrspace)
    {
      auto addr = static_cast<char *>(malloc(strlen(src->addrspace) + 1));

      memcpy(addr, src->addrspace, strlen(src->addrspace) + 1);
      addr[strlen(src->addrspace)] = '\0';
      dst->addrspace = addr;
    }
  }
  m_MemoryMaps.descriptors = descs;
}

size_t QRetroMemory::readBuffer(void *ptr, size_t address, size_t size)
{
  const retro_memory_descriptor *desc = containing(address);

  if (!desc || !desc->ptr)
    return 0;
  else
  {
    size_t offset = address - desc->start + desc->offset;
    size_t read = desc->len - offset;

    read = read < size ? read : size;
    memcpy(ptr, static_cast<char *>(desc->ptr) + offset, read);

    return read;
  }
}

size_t QRetroMemory::writeBuffer(const void *ptr, size_t address, size_t size)
{
  const retro_memory_descriptor *desc = containing(address);

  if (!desc || !desc->ptr)
    return 0;
  else
  {
    size_t offset = address - desc->start + desc->offset;
    size_t written = desc->len - offset;

    written = written < size ? written : size;
    memcpy(static_cast<char *>(desc->ptr) + offset, ptr, written);

    return written;
  }
}
