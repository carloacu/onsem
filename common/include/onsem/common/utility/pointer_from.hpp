#ifndef ONSEM_COMMON_UTILITY_POINTERFROM_HPP
#define ONSEM_COMMON_UTILITY_POINTERFROM_HPP

#include <vector>

namespace onsem
{
namespace mystd
{

template<typename T>
static inline T* pointer_from(typename std::vector<T>::iterator it, typename std::vector<T>::iterator itEnd)
{
  if (it != itEnd)
    return &*it;
  return nullptr;
}

template<typename T>
static inline const T* pointer_from(typename std::vector<T>::const_iterator it, typename std::vector<T>::const_iterator itEnd)
{
  if (it != itEnd)
    return &*it;
  return nullptr;
}


} // End of namespace mystd
} // End of namespace onsem


#endif // ONSEM_COMMON_UTILITY_POINTERFROM_HPP
