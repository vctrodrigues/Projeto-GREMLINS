#include <stddef.h>
#include "StoragePool.hpp"
#include "mempool_common.h"

#ifndef SLPOOL_H
#define SLPOOL_H

namespace mp
{
template <size_t BLK_SIZE = 16>
class SLPool : public StoragePool
{
public:
  struct Header
  {
    size_t m_length;
    Header() : m_length(0u){/* Empty */};
  };

  struct Block : public Header
  {
    union {
      Block *m_next;                         // Pointer to next block OR...
      char m_raw[BLK_SIZE - sizeof(Header)]; // Client's raw area
    };

    Block() : Header(), m_next(nullptr){/* Empty */};
  };

private:
  unsigned int m_n_blocks; //!< Number of blocks in the list.
  Block *m_pool;           //!< Head of list.
  Block &m_sentinel;       //!< End of the list.

public:
  static constexpr size_t BLK_SZ = sizeof(mp::SLPool<BLK_SIZE>::Block);     //!< The block size in bytes.
  static constexpr size_t TAG_SZ = sizeof(mp::Tag);                         //!< The Tag size in bytes (each reserved area has a tag).
  static constexpr size_t HEADER_SZ = sizeof(mp::SLPool<BLK_SIZE>::Header); //!< The header size in bytes.

  /// Constructor of SLPool, set the number of blocks, the sentinel and the memory pool.
  explicit SLPool(size_t bytes) : m_n_blocks{(unsigned int)std::ceil((bytes + sizeof(Header)) / BLK_SIZE) + 1u},
                                  m_pool{new Block[m_n_blocks]},
                                  m_sentinel{m_pool[m_n_blocks - 1]}
  {
    this->m_pool[0].m_length = (m_n_blocks - 1);
    this->m_pool[0].m_next = nullptr;

    this->m_sentinel.m_next = this->m_pool;
    this->m_sentinel.m_length = 0;
  }

  /// Destructs the SLPool.
  ~SLPool()
  {
    delete[] m_pool;
  }

  void *Allocate(size_t bytes)
  {
    Block *fast = this->m_sentinel.m_next;
    Block *slow = &this->m_sentinel;
    size_t blocks = std::ceil((bytes + HEADER_SZ) / BLK_SZ);

    while (fast != nullptr)
    {
      if (fast->m_length == blocks)
      {
        slow->m_next = fast->m_next;
        fast->m_length = blocks;
        return reinterpret_cast<void *>(reinterpret_cast<Header *>(fast) + (1U));
      }
      else if (fast->m_length > blocks)
      {
        slow->m_next = fast + blocks;
        slow->m_next->m_next = fast->m_next;
        slow->m_next->m_length = fast->m_length - blocks;
        fast->m_length = blocks;

        return reinterpret_cast<void *>(reinterpret_cast<Header *>(fast) + (1U));
      }
      else
      {
        slow = fast;
        fast = fast->m_next;
      }
    }

    throw std::bad_alloc();
  }

  void Free(void *ptr)
  {
    ptr = reinterpret_cast<Block *>(reinterpret_cast<Header *>(ptr) - (1U));

    Block *current = (Block *)ptr;
    Block *fast = this->m_sentinel.m_next;
    Block *slow = &this->m_sentinel;

    if(fast == nullptr) 
    {
      slow->m_next = current;
      fast = current;
    }

    while (fast != nullptr)
    {
      if ((current - fast) < (current - slow) and (current - fast) > 0)
      {
        slow = fast;
        fast = fast->m_next;
      }
      else
          break;
    }
    
    Block *pre = fast;
    Block *pos = fast->m_next;

    if ((current - pre) == (long int)pre->m_length and (pos - current) == (long int)current->m_length)
    {
      pre->m_next = pos->m_next;
      pre->m_length = pre->m_length + current->m_length + pos->m_length;
    }
    else if ((current - pre) == (long int)pre->m_length)
    {
      pre->m_next = pos;
      pre->m_length = pre->m_length + current->m_length;
    }
    else if ((pos - current) == (long int)current->m_length)
    {
      pre->m_next = current;
      current->m_next = pos->m_next;
      current->m_length = current->m_length + pos->m_length;
    }
    else
    {
      current->m_next = pre->m_next;
      pre->m_next = current;
    }
  }

  friend std::ostream &operator<<(std::ostream &stream, const SLPool &obj)
  {
    stream << " SLPool { blocks: " << obj.m_n_blocks << " } " << std::endl;

    return stream;
  }
};
} // namespace mp

#endif