#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticdurationgrounding.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/utility/number.hpp>
#include <onsem/texttosemantic/printer/semlinetoprint.hpp>

namespace onsem
{

bool SemanticDuration::operator<
(const SemanticDuration& pOther) const
{
  std::map<SemanticTimeUnity, int>::const_iterator itCurr = timeInfos.begin();
  std::map<SemanticTimeUnity, int>::const_iterator itOther = pOther.timeInfos.begin();

  if (sign != pOther.sign)
  {
    if (sign == Sign::NEGATIVE)
      return true;
    if (pOther.sign == Sign::NEGATIVE)
      return false;
  }

  auto invertResultIfNecessary = [&](bool pRes)
  {
    if (sign == Sign::NEGATIVE)
      return !pRes;
    return pRes;
  };

  while (itCurr != timeInfos.end())
  {
    if (itCurr->second == 0)
    {
      ++itCurr;
      continue;
    }

    if (itOther == pOther.timeInfos.end())
      return invertResultIfNecessary(false);
    if (itOther->second == 0)
    {
      ++itOther;
      continue;
    }

    if (itCurr->first == itOther->first)
    {
      if (itCurr->second != itOther->second)
        return invertResultIfNecessary(itCurr->second < itOther->second);
    }
    else
      return invertResultIfNecessary(itCurr->first > itOther->first);
    ++itCurr;
    ++itOther;
  }
  if (itOther == pOther.timeInfos.end())
    return false;
  return invertResultIfNecessary(true);
}


bool SemanticDuration::operator==
(const SemanticDuration& pOther) const
{
  return sign == pOther.sign &&
      timeInfos == pOther.timeInfos;
}

SemanticDuration SemanticDuration::operator+(const SemanticDuration& pOther) const
{
  return _addition(pOther, 1);
}

SemanticDuration SemanticDuration::operator-(const SemanticDuration& pOther) const
{
  return _addition(pOther, -1);
}


SemanticDuration SemanticDuration::fromRadixMapStr(const std::string& pRadixMapStr)
{
  SemanticDuration res;
  std::size_t i = 0;
  std::size_t pRadixMapStr_size = pRadixMapStr.size();
  if (i >= pRadixMapStr_size)
    return res;
  bool isDirPositive = true;
  if (pRadixMapStr[i] == 'z')
  {
    res.sign = Sign::NEGATIVE;
    isDirPositive = false;
    ++i;
  }
  while (i < pRadixMapStr_size)
  {
    SemanticTimeUnity timeUnity = isDirPositive ?
      semanticTimeUnity_fromUnorderredChar(pRadixMapStr[i++]) :
      semanticTimeUnity_fromChar(pRadixMapStr[i++]);
    assert(i < pRadixMapStr_size);
    int nbOfDigits = semanticTimeUnity_toNbOfDigits(timeUnity);
    try
    {
      res.timeInfos.emplace(timeUnity, mystd::lexical_cast<int>(pRadixMapStr.substr(i, nbOfDigits)));
      i += nbOfDigits;
    }
    catch (...)
    {
      assert(false);
    }
    assert(i <= pRadixMapStr_size);
  }
  return res;
}


std::string SemanticDuration::toRadixMapStr() const
{
  std::stringstream ss;
  bool isDirPositive = true;
  if (sign == Sign::NEGATIVE)
  {
    ss << "z";
    isDirPositive = false;
  }
  for (const auto& currTimeInfos : timeInfos)
  {
    if (isDirPositive)
      ss << semanticTimeUnity_toUnorderredChar(currTimeInfos.first);
    else
      ss << semanticTimeUnity_toChar(currTimeInfos.first);
    int nbOfDigits = semanticTimeUnity_toNbOfDigits(currTimeInfos.first);
    assert(hasNotMoreThanANumberOfDigits(currTimeInfos.second, nbOfDigits));
    ss << std::setfill('0') << std::setw(nbOfDigits) << currTimeInfos.second;
  }
  return ss.str();
}


bool SemanticDuration::isNearlyEqual(const SemanticDuration& pOther) const
{
  return _isNearlyEqualOneSideCheck(pOther) &&
      pOther._isNearlyEqualOneSideCheck(*this);
}

bool SemanticDuration::_isNearlyEqualOneSideCheck(const SemanticDuration& pOther) const
{
  auto itOtherTimeInfo = pOther.timeInfos.begin();
  for (const auto& currTimeInfo : timeInfos)
  {
    if (currTimeInfo.first == SemanticTimeUnity::LESS_THAN_A_MILLISECOND)
      break;
    if (itOtherTimeInfo == pOther.timeInfos.end() ||
        currTimeInfo.second != itOtherTimeInfo->second)
      return false;
    ++itOtherTimeInfo;
  }
  return true;
}


void SemanticDuration::add(SemanticTimeUnity pTimeUnity,
                           int pNbToAdd)
{
  timeInfos[pTimeUnity] += pNbToAdd;
  _normalize();
}

SemanticDuration SemanticDuration::_addition(const SemanticDuration& pOther,
                                             int pCoefOfOther) const
{
  SemanticDuration res = abs(*this);
  std::map<SemanticTimeUnity, int>::const_iterator itOther = pOther.timeInfos.begin();

  if (sign != pOther.sign)
  {
    SemanticDuration otherAbs = abs(pOther);
    if (res < otherAbs)
    {
      res.sign = pOther.sign;
    }
    else if (res == otherAbs)
    {
      res.clear();
      res.sign = Sign::POSITIVE;
      return res;
    }
    else
    {
      res.sign = sign;
    }
  }

  auto invertResultIfNecessary = [&](int pRes)
  {
    if (sign != pOther.sign)
      return -pRes;
    return pRes;
  };

  while (itOther != pOther.timeInfos.end())
  {
    res.timeInfos[itOther->first] += pCoefOfOther * invertResultIfNecessary(itOther->second);
    ++itOther;
  }
  res._normalize();
  return res;
}



SemanticDuration SemanticDuration::abs(const SemanticDuration& pDuration)
{
  SemanticDuration res;
  res.sign = Sign::POSITIVE;
  res.timeInfos = pDuration.timeInfos;
  return res;
}


void SemanticDuration::printDuration
(std::list<std::string>& pListElts,
 const std::string& pDurationLabelName) const
{
  if (timeInfos.empty())
    return;
  std::stringstream ss;
  ss << pDurationLabelName << "(";
  ss << sign_toStr(sign);
  for (const auto& currElt : timeInfos)
    ss << currElt.second << semanticTimeUnity_toAbreviation(currElt.first);
  ss << ")";
  pListElts.emplace_back(ss.str());
}



SemanticTimeUnity SemanticDuration::precision() const
{
  if (timeInfos.empty())
    return SemanticTimeUnity::LESS_THAN_A_MILLISECOND;
  return (--timeInfos.end())->first;
}


bool SemanticDuration::_invertDirectionIfNecessary()
{
  bool needToInvertOrder = false;
  auto itTimeInfo = timeInfos.begin();
  if (itTimeInfo != timeInfos.end() &&
      itTimeInfo->second < 0)
    needToInvertOrder = true;

  if (needToInvertOrder)
  {
    if (sign == Sign::POSITIVE)
      sign = Sign::NEGATIVE;
    else if (sign == Sign::NEGATIVE)
      sign = Sign::POSITIVE;
    for (; itTimeInfo != timeInfos.end(); ++itTimeInfo)
      itTimeInfo->second = -itTimeInfo->second;
  }
  return needToInvertOrder;
}


void SemanticDuration::clear()
{
  sign = Sign::POSITIVE;
  timeInfos.clear();
}


bool SemanticDuration::isEmpty() const
{
  return timeInfos.empty();
}


void SemanticDuration::_normalize()
{
  int decNecessaryAtUpperLevel = 0;

  auto adjustUnity = [&](int& pValue, int pMax)
  {
    while (pValue < 0)
    {
      --decNecessaryAtUpperLevel;
      pValue += pMax;
    }

    while (pValue >= pMax)
    {
      ++decNecessaryAtUpperLevel;
      pValue -= pMax;
    }
  };

  adjustUnity(timeInfos[SemanticTimeUnity::SECOND], 60);
  timeInfos[SemanticTimeUnity::MINUTE] += decNecessaryAtUpperLevel;
  decNecessaryAtUpperLevel = 0;
  adjustUnity(timeInfos[SemanticTimeUnity::MINUTE], 60);
  timeInfos[SemanticTimeUnity::HOUR] += decNecessaryAtUpperLevel;
  decNecessaryAtUpperLevel = 0;
  adjustUnity(timeInfos[SemanticTimeUnity::HOUR], 24);
  timeInfos[SemanticTimeUnity::DAY] += decNecessaryAtUpperLevel;
  decNecessaryAtUpperLevel = 0;
  adjustUnity(timeInfos[SemanticTimeUnity::DAY], 30);
  timeInfos[SemanticTimeUnity::MONTH] += decNecessaryAtUpperLevel;
  decNecessaryAtUpperLevel = 0;
  adjustUnity(timeInfos[SemanticTimeUnity::MONTH], 12);
  timeInfos[SemanticTimeUnity::YEAR] += decNecessaryAtUpperLevel;

  removeEmptyValues();
  if (_invertDirectionIfNecessary())
    _normalize();
}



void SemanticDuration::removeEmptyValues()
{
  // remove empty values
  auto itTimeInfo = timeInfos.begin();
  while (itTimeInfo != timeInfos.end())
  {
    if (itTimeInfo->second == 0)
    {
      itTimeInfo = timeInfos.erase(itTimeInfo);
      continue;
    }
    ++itTimeInfo;
  }
}


bool SemanticDuration::isEqualWithMarginOfError(const SemanticDuration& pOther,
                                                const SemanticDuration& pMarginOfError) const
{
  return abs(*this - pOther) < pMarginOfError;
}


int64_t SemanticDuration::nbMilliseconds() const
{
  int64_t res = 0;
  for (const auto& currTimeInfo : timeInfos)
    res += semanticTimeUnity_toNbOfMilliseconds(currTimeInfo.first) * currTimeInfo.second;
  if (sign != Sign::NEGATIVE)
    return res;
  return -res;
}



} // End of namespace onsem
