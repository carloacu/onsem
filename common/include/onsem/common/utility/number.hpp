#ifndef ONSEM_COMMON_UTILITY_NUMBER_HPP
#define ONSEM_COMMON_UTILITY_NUMBER_HPP

#include <cmath>

namespace onsem
{

static inline bool hasNotMoreThanANumberOfDigits(int pNb,
                                                 int pNumberOfDigits)
{
  return pNb < pow(10, pNumberOfDigits);
}

} // End of namespace onsem

#endif // ONSEM_COMMON_UTILITY_NUMBER_HPP
