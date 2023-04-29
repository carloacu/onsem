#ifndef ONSEM_COMMON_UTILITY_RADOM_HPP
#define ONSEM_COMMON_UTILITY_RADOM_HPP

#include <list>
#include <string>
#include <vector>
#include <atomic>
#include "../api.hpp"

namespace onsem
{

class ONSEM_COMMON_API Random
{
public:
  static int getCurrentNumber()
  {
    return _currentNumber.load();
  }

  static void advanceFromNb(int nb)
  {
    _currentNumber = nb;
    _getNextNumberBetweenZeroAndOneHundred();
  }

  template<typename T>
  static typename std::list<T>::const_iterator advanceConstIterator(const std::list<T>& pList)
  {
    auto itRes = pList.begin();
    if (pList.size() > 1)
      std::advance(itRes, Random::_getNextNumberBetweenZeroAndOneHundred() % pList.size());
    return itRes;
  }

  template<typename T>
  static typename std::list<T>::iterator advanceIterator(std::list<T>& pList)
  {
    auto itRes = pList.begin();
    if (pList.size() > 1)
      std::advance(itRes, Random::_getNextNumberBetweenZeroAndOneHundred() % pList.size());
    return itRes;
  }

  template<typename T>
  static typename std::size_t getRandId(const std::vector<T>& pList)
  {
    if (pList.size() > 1)
    {
      return Random::_getNextNumberBetweenZeroAndOneHundred() % pList.size();
    }
    return 0;
  }

  static std::string generateUuid();
  static std::string generateUuidWithoutHyphen();
  static std::size_t getRandomNumber(std::size_t pFrom, std::size_t pTo);

private:
  static std::atomic<int> _currentNumber;

  static int _getNextNumberBetweenZeroAndOneHundred()
  {
    if (_currentNumber.load() >= 100)
    {
      _currentNumber = 0;
    }
    ++_currentNumber;
    return _currentNumber.load();
  }

};


} // End of namespace onsem


#endif // ONSEM_COMMON_UTILITY_RADOM_HPP
