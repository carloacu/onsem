#ifndef ONSEM_COMMON_UTILITY_ARE_EQUALS_HPP
#define ONSEM_COMMON_UTILITY_ARE_EQUALS_HPP

#include <memory>
#include <assert.h>


namespace onsem
{

template <typename TYPE>
bool areEquals(const std::unique_ptr<TYPE>& pUniqueExp1,
               const std::unique_ptr<TYPE>& pUniqueExp2)
{
  if (!pUniqueExp1 && !pUniqueExp2)
    return true;
  if (!pUniqueExp1 || !pUniqueExp2)
    return false;
  return *pUniqueExp1 == *pUniqueExp2;
}


} // end namespace onsem



#endif // ONSEM_COMMON_UTILITY_ARE_EQUALS_HPP
