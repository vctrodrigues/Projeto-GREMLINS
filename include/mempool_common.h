#include <cstdlib>
#include "StoragePool.hpp"

using namespace mp;

#ifndef MEMPOOL_COMMON
#define MEMPOOL_COMMON

namespace mp
{
struct Tag
{
  StoragePool *pool;
};
} //namespace mp

void *operator new[](size_t bytes, StoragePool &p)
{
  Tag *const tag = reinterpret_cast<Tag *>(p.Allocate(bytes + sizeof(Tag)));
  tag->pool = &p;

  // skip sizeof tag to get the raw data-block.
  return (reinterpret_cast<void *>(tag + 1U));
}

void *operator new(size_t bytes, StoragePool &p)
{
  Tag *const tag = reinterpret_cast<Tag *>(p.Allocate(bytes + sizeof(Tag)));
  tag->pool = &p;

  return (reinterpret_cast<void *>(tag + 1U));
}

void *operator new(size_t bytes)
{
  Tag *const tag = reinterpret_cast<Tag *>(std::malloc(bytes + sizeof(Tag)));
  tag->pool = nullptr;

  return (reinterpret_cast<void *>(tag + 1U));
}

void *operator new[](size_t bytes)
{
  Tag *const tag = reinterpret_cast<Tag *>(std::malloc(bytes + sizeof(Tag)));
  tag->pool = nullptr;

  return (reinterpret_cast<void *>(tag + 1U));
}

void operator delete(void *arg) noexcept
{
  // We need subtract 1U (in fact, pointer arithmetics) because arg
  // points to the raw data (second block of information).
  // The pool id (tag) is located 'sizeof(Tag)' bytes before.
  Tag *const tag = reinterpret_cast<Tag *>(arg) - 1U;
  if (nullptr != tag->pool) // Memory block belongs to a particular GM.
    tag->pool->Free(tag);
  else
    std::free(tag); // Memory block belongs to the operational system.
}

void operator delete[](void *arg) noexcept
{
  Tag *const tag = reinterpret_cast<Tag *>(arg) - 1U;
  if (nullptr != tag->pool)
    tag->pool->Free(tag);
  else
    std::free(tag);
}

#endif