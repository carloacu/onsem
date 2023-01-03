#ifndef ONSEM_COMMON_UTILITY_NUMBER_HPP
#define ONSEM_COMMON_UTILITY_NUMBER_HPP

#include <cmath>

namespace onsem
{

static inline double tenPow(char pNb) {
  switch (pNb) {
  case -10:
    return 0.0000000001;
  case -9:
    return 0.000000001;
  case -8:
    return 0.00000001;
  case -7:
    return 0.0000001;
  case -6:
    return 0.000001;
  case -5:
    return 0.00001;
  case -4:
    return 0.0001;
  case -3:
    return 0.001;
  case -2:
    return 0.01;
  case -1:
    return 0.1;
  case 0:
    return 1;
  case 1:
    return 10;
  case 2:
    return 100;
  case 3:
    return 1000;
  case 4:
    return 10000;
  case 5:
    return 100000;
  case 6:
    return 1000000;
  case 7:
    return 10000000;
  case 8:
    return 100000000;
  case 9:
    return 1000000000;
  case 10:
    return 10000000000;
  default:
    return pow(10, pNb);
  }
}


static inline bool hasNotMoreThanANumberOfDigits(int pNb,
                                                 int pNumberOfDigits)
{
  return pNb < tenPow(pNumberOfDigits);
}


} // End of namespace onsem

#endif // ONSEM_COMMON_UTILITY_NUMBER_HPP
