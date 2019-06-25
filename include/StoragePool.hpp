#include <stddef.h>
#ifndef STORAGE_POOL_H
#define STORAGE_POOL_H

namespace mp
{
class StoragePool
{
public:
  //virtual ~StoragePool() = 0;
  virtual void *Allocate(size_t) = 0;
  virtual void Free(void *) = 0;
};
} // namespace mp

#endif