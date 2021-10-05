#ifndef ONSEM_COMMON_BINARY_SIMPLEINTMAPPING_HPP
#define ONSEM_COMMON_BINARY_SIMPLEINTMAPPING_HPP

#include <functional>
#include <onsem/common/binary/binaryloader.hpp>

namespace onsem
{
enum class ComparisonEnum
{
  EQUAL,
  MORE,
  LESS
};

inline static const uint32_t* readSimpleIntMapping
(const uint32_t* pPtr,
 uint32_t pNbOfElts,
 const std::function<ComparisonEnum(uint32_t)>& pComparator)
{
  auto size = pNbOfElts;
  while (true)
  {
    if (size < 5)
    {
      for (std::size_t i = 0; i < size; ++i)
      {
        auto comp = pComparator(*pPtr);
        if (comp == ComparisonEnum::EQUAL)
          return pPtr;
        if (comp == ComparisonEnum::MORE)
          return nullptr;
        ++pPtr;
      }
      return nullptr;
    }

    std::size_t newSize = size >> 1; // -> size / 2;
    auto* newPtr = pPtr + newSize;
    auto comp = pComparator(*newPtr);
    if (comp == ComparisonEnum::EQUAL)
      return newPtr;

    if (comp == ComparisonEnum::MORE)
    {
      size = newSize;
      continue;
    }

    pPtr = newPtr + 1;
    size = size - newSize - 1;
  }
  return nullptr;
}


} // End of namespace onsem


#endif // ONSEM_COMMON_BINARY_SIMPLEINTMAPPING_HPP
