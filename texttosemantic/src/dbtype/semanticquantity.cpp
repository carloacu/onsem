#include <onsem/texttosemantic/dbtype/semanticquantity.hpp>
#include <iomanip>
#include <math.h>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>

namespace onsem
{
namespace
{

double _tenPow(unsigned char pNb) {
  switch (pNb) {
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

void _add(SemanticFloat& pRes, const SemanticFloat& pNb1, const SemanticFloat& pNb2, Sign pNb2Sign)
{
  auto newNbOfSignificantDigit = std::max(pNb1.nbOfSignificantDigit, pNb2.nbOfSignificantDigit);
  long nb1 = pNb1.sign == Sign::POSITIVE ? pNb1.valueAfterTheDecimalPoint : -pNb1.valueAfterTheDecimalPoint;
  if (newNbOfSignificantDigit > pNb1.nbOfSignificantDigit)
    nb1 *= _tenPow(newNbOfSignificantDigit - pNb1.nbOfSignificantDigit);
  long nb2 = pNb2Sign == Sign::POSITIVE ? pNb2.valueAfterTheDecimalPoint : -pNb2.valueAfterTheDecimalPoint;
  if (newNbOfSignificantDigit > pNb2.nbOfSignificantDigit)
    nb2 *= _tenPow(newNbOfSignificantDigit - pNb2.nbOfSignificantDigit);

  int signedValueWithoutDecimal = pNb1.signedValueWithoutDecimal() + (pNb2Sign == Sign::POSITIVE ? pNb2.value : -pNb2.value);
  pRes.sign = signedValueWithoutDecimal >= 0 ? Sign::POSITIVE : Sign::NEGATIVE;
  pRes.value = std::abs(signedValueWithoutDecimal);
  auto nbSum = nb1 + nb2;
  if (nbSum >= 0)
  {
    pRes.valueAfterTheDecimalPoint = nbSum;
  }
  else
  {
    pRes.valueAfterTheDecimalPoint = _tenPow(newNbOfSignificantDigit) + nbSum;
    --pRes.value;
  }
  pRes.nbOfSignificantDigit = newNbOfSignificantDigit;
}

}

SemanticFloat::SemanticFloat(int pValue,
                             std::size_t pValueAfterTheDecimalPoint,
                             unsigned char pNbOfSignificantDigit)
  : sign(pValue >= 0 ? Sign::POSITIVE : Sign::NEGATIVE),
    value(std::abs(pValue)),
    valueAfterTheDecimalPoint(pValueAfterTheDecimalPoint),
    nbOfSignificantDigit(pNbOfSignificantDigit)
{
}

bool SemanticFloat::operator==(const SemanticFloat& pOther) const
{
  return value == pOther.value &&
      valueAfterTheDecimalPoint == pOther.valueAfterTheDecimalPoint &&
      nbOfSignificantDigit == pOther.nbOfSignificantDigit;
}

bool SemanticFloat::operator==(int pNb) const
{
  return sign == (pNb >= 0 ? Sign::POSITIVE : Sign::NEGATIVE) &&
      value == static_cast<std::size_t>(std::abs(pNb)) &&
      valueAfterTheDecimalPoint == 0 &&
      nbOfSignificantDigit == 0;
}

bool SemanticFloat::operator!=(const SemanticFloat& pOther) const
{
  return !operator==(pOther);
}

bool SemanticFloat::operator!=(int pNb) const
{
  return !operator==(pNb);
}

bool SemanticFloat::operator<(const SemanticFloat& pOther) const
{
  if (sign != pOther.sign || value != pOther.value)
    return signedValueWithoutDecimal() < pOther.signedValueWithoutDecimal();

  auto minNbOfDigits = std::min(nbOfSignificantDigit, pOther.nbOfSignificantDigit);
  auto nb1 = valueAfterTheDecimalPoint * _tenPow(nbOfSignificantDigit - minNbOfDigits);
  auto nb2 = pOther.valueAfterTheDecimalPoint * _tenPow(pOther.nbOfSignificantDigit - minNbOfDigits);
  if (sign == Sign::POSITIVE)
    return nb1 < nb2;
  return nb1 > nb2;
}

bool SemanticFloat::operator>(const SemanticFloat& pOther) const
{
  if (operator==(pOther))
    return false;
  return !operator<(pOther);
}

bool SemanticFloat::operator>=(const SemanticFloat& pOther) const
{
  if (sign != pOther.sign || value != pOther.value)
    return signedValueWithoutDecimal() >= pOther.signedValueWithoutDecimal();

  auto minNbOfDigits = std::min(nbOfSignificantDigit, pOther.nbOfSignificantDigit);
  auto nb1 = valueAfterTheDecimalPoint * _tenPow(nbOfSignificantDigit - minNbOfDigits);
  auto nb2 = pOther.valueAfterTheDecimalPoint * _tenPow(pOther.nbOfSignificantDigit - minNbOfDigits);
  if (sign == Sign::POSITIVE)
    return nb1 >= nb2;
  return nb1 <= nb2;
}

bool SemanticFloat::operator>=(int pNb) const
{
  if ((sign == Sign::POSITIVE) != (pNb > 0) || value != static_cast<std::size_t>(std::abs(pNb)))
    return signedValueWithoutDecimal() >= pNb;

  if (nbOfSignificantDigit == 0)
    return true;

  if (sign == Sign::POSITIVE)
    return valueAfterTheDecimalPoint >= 0;
  return valueAfterTheDecimalPoint <= 0;
}


bool SemanticFloat::operator<=(const SemanticFloat& pOther) const
{
  if (sign != pOther.sign || value != pOther.value)
    return signedValueWithoutDecimal() <= pOther.signedValueWithoutDecimal();

  auto minNbOfDigits = std::min(nbOfSignificantDigit, pOther.nbOfSignificantDigit);
  auto nb1 = valueAfterTheDecimalPoint * _tenPow(nbOfSignificantDigit - minNbOfDigits);
  auto nb2 = pOther.valueAfterTheDecimalPoint * _tenPow(pOther.nbOfSignificantDigit - minNbOfDigits);
  if (sign == Sign::POSITIVE)
    return nb1 <= nb2;
  return nb1 >= nb2;
}

bool SemanticFloat::operator<=(int pNb) const
{
  if ((sign == Sign::POSITIVE) != (pNb > 0) || value != static_cast<std::size_t>(std::abs(pNb)))
    return signedValueWithoutDecimal() <= pNb;

  if (nbOfSignificantDigit == 0)
    return true;

  if (sign == Sign::POSITIVE)
    return valueAfterTheDecimalPoint <= 0;
  return valueAfterTheDecimalPoint >= 0;
}


SemanticFloat SemanticFloat::operator+=(const SemanticFloat& pOther)
{
  *this = *this + pOther;
  return *this;
}

SemanticFloat SemanticFloat::operator+(const SemanticFloat& pOther) const
{
  SemanticFloat res;
  _add(res, *this, pOther, pOther.sign);
  return res;
}

SemanticFloat SemanticFloat::operator-=(const SemanticFloat& pOther)
{
  *this = *this - pOther;
  return *this;
}

SemanticFloat SemanticFloat::operator-(const SemanticFloat& pOther) const
{
  SemanticFloat res;
  _add(res, *this, pOther, invert(pOther.sign));
  return res;
}

void SemanticFloat::add(const SemanticFloat& pOther)
{
  _add(*this, *this, pOther, pOther.sign);
}

void SemanticFloat::substract(const SemanticFloat& pOther)
{
  _add(*this, *this, pOther, invert(pOther.sign));
}

void SemanticFloat::multiply(const SemanticFloat& pOther)
{
  fromDouble(toDouble() * pOther.toDouble());
}

SemanticFloat SemanticFloat::operator*(const SemanticFloat& pOther) const
{
  SemanticFloat res;
  res.fromDouble(toDouble() * pOther.toDouble());
  return res;
}

SemanticFloat SemanticFloat::operator*(double pNb) const
{
  SemanticFloat res;
  res.fromDouble(toDouble() * pNb);
  return res;
}

void SemanticFloat::set(int pValue)
{
  sign = pValue >= 0 ? Sign::POSITIVE : Sign::NEGATIVE;
  value = std::abs(pValue);
  valueAfterTheDecimalPoint = 0u;
  nbOfSignificantDigit = 0u;
}

bool SemanticFloat::isPositive() const
{
  return sign == Sign::POSITIVE;
}

bool SemanticFloat::isAnInteger() const
{
  return valueAfterTheDecimalPoint == 0 &&
      nbOfSignificantDigit == 0;
}

int SemanticFloat::signedValueWithoutDecimal() const
{
  if (sign == Sign::POSITIVE)
    return value;
  return -value;
}

double SemanticFloat::toDouble() const
{
  double res = signedValueWithoutDecimal();
  res += valueAfterTheDecimalPoint / _tenPow(nbOfSignificantDigit);
  return res;
}

void SemanticFloat::fromDouble(double pNb)
{
  sign = pNb >= 0 ? Sign::POSITIVE : Sign::NEGATIVE;
  auto absNb = std::abs(pNb);
  value = static_cast<std::size_t>(absNb);

  nbOfSignificantDigit = 7;
  int coef = 10000000;
  long long longNb = (absNb - value) * coef;
  valueAfterTheDecimalPoint = longNb;

  bool firstIteration = true;
  while (nbOfSignificantDigit > 0)
  {
    if (firstIteration)
      firstIteration = false;
    else if (valueAfterTheDecimalPoint % 10 != 0)
      break;
    valueAfterTheDecimalPoint /= 10;
    --nbOfSignificantDigit;
  }
}

std::string SemanticFloat::toStr(SemanticLanguageEnum pLanguage) const
{
  if (pLanguage == SemanticLanguageEnum::FRENCH)
    return toStrWithSeparator(',');
  return toStrWithSeparator('.');
}

std::string SemanticFloat::toStrWithSeparator(char pSeparator) const
{
  std::stringstream ss;
  ss << sign_toStr(sign) << value;
  if (nbOfSignificantDigit > 0)
  {
    ss << pSeparator;
    ss << std::setfill('0') << std::setw(nbOfSignificantDigit) << valueAfterTheDecimalPoint;
  }
  return ss.str();
}

bool SemanticFloat::fromStr(const std::string& pStr, SemanticLanguageEnum pLanguage)
{
  if (pLanguage == SemanticLanguageEnum::FRENCH)
    return fromStrWithSeparator(pStr, ',');
  return fromStrWithSeparator(pStr, '.');
}

bool SemanticFloat::fromStrWithSeparator(const std::string& pStr, char pSeparator)
{
  if (pStr.empty())
    return false;
  auto newSign = pStr[0] == '-' ? Sign::NEGATIVE : Sign::POSITIVE;
  std::size_t sepPos = pStr.find(pSeparator);
  if (sepPos != std::string::npos)
  {
    try {
      std::size_t newValue = 0u;
      if (newSign == Sign::POSITIVE)
        newValue = mystd::lexical_cast_unigned<std::size_t>(pStr.substr(0, sepPos));
      else
        newValue = mystd::lexical_cast_unigned<std::size_t>(pStr.substr(1, sepPos - 1));
      auto newNbOfSignificantDigit = pStr.size() - sepPos - 1;

      valueAfterTheDecimalPoint = mystd::lexical_cast_unigned<int>(pStr.substr(sepPos + 1, newNbOfSignificantDigit));
      sign = newSign;
      value = newValue;
      nbOfSignificantDigit = newNbOfSignificantDigit;
      return true;
    } catch (...) {}
  }
  else
  {
    try {
      if (newSign == Sign::POSITIVE)
        value = mystd::lexical_cast_unigned<int>(pStr);
      else
        value = mystd::lexical_cast_unigned<int>(pStr.substr(1, pStr.size() - 1));
      sign = newSign;
      valueAfterTheDecimalPoint = 0u;
      nbOfSignificantDigit = 0u;
      return true;
    } catch (...) {}
  }
  return false;
}


bool SemanticQuantity::operator==(const SemanticQuantity& pOther) const
{
  return type == pOther.type &&
      nb == pOther.nb &&
      paramSpec == pOther.paramSpec &&
      subjectiveValue == pOther.subjectiveValue;
}

void SemanticQuantity::setNumber(int pNumber)
{
  type = SemanticQuantityType::NUMBER;
  nb.set(pNumber);
}

void SemanticQuantity::setNumber(const SemanticFloat& pNumber)
{
  type = SemanticQuantityType::NUMBER;
  nb = pNumber;
}

void SemanticQuantity::increaseNumber(const SemanticFloat& pIncreaseValue)
{
  if (type == SemanticQuantityType::NUMBER)
    nb.add(pIncreaseValue);
  else
    setNumber(pIncreaseValue);
}

void SemanticQuantity::setNumberToFill(int pParamId,
                     std::string pAttributeName)
{
  type = SemanticQuantityType::NUMBERTOFILL;
  std::stringstream ss;
  ss << pParamId;
  if (!pAttributeName.empty())
    ss << "_" << pAttributeName;
  paramSpec = ss.str();
}

bool SemanticQuantity::getNumberToFill(int& pParamId,
                                       std::string& pAttributeName) const
{
  if (type == SemanticQuantityType::NUMBERTOFILL)
  {
    std::size_t sepPos = paramSpec.find('_');
    std::string paramIdStr;
    if (sepPos != std::string::npos)
    {
      paramIdStr = paramSpec.substr(0, sepPos);
      std::size_t begOfAttributeName = sepPos + 1;
      if (paramSpec.size() > begOfAttributeName)
        pAttributeName = paramSpec.substr(begOfAttributeName, paramSpec.size() - begOfAttributeName);
    }
    else
    {
      paramIdStr = paramSpec;
    }
    try
    {
      pParamId = mystd::lexical_cast<int>(paramIdStr);
      return true;
    }
    catch (...) {}
  }
  return false;
}

void SemanticQuantity::setPlural()
{
  type = SemanticQuantityType::MOREOREQUALTHANNUMBER;
  nb.set(2);
}

bool SemanticQuantity::isPlural() const
{
  return (type == SemanticQuantityType::MOREOREQUALTHANNUMBER && nb >= 2) ||
      (type == SemanticQuantityType::NUMBER && nb >= 2) ||
      type == SemanticQuantityType::MAXNUMBER;
}

bool SemanticQuantity::isUnknown() const
{
  return type == SemanticQuantityType::UNKNOWN && nb == 0 && subjectiveValue == SemanticSubjectiveQuantity::UNKNOWN;
}

bool SemanticQuantity::isEqualToInit() const
{
  return type == SemanticQuantityType::UNKNOWN && nb == 0 && paramSpec.empty() && subjectiveValue == SemanticSubjectiveQuantity::UNKNOWN;
}

bool SemanticQuantity::isEqualTo(int pNb) const
{
  return type == SemanticQuantityType::NUMBER && nb == pNb;
}

bool SemanticQuantity::isEqualToZero() const
{
  return isEqualTo(0);
}

bool SemanticQuantity::isEqualToOne() const
{
  return isEqualTo(1);
}

void SemanticQuantity::clear()
{
  type = SemanticQuantityType::UNKNOWN;
  nb.set(0);
}

} // End of namespace onsem


