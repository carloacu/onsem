#include <onsem/texttosemantic/dbtype/semanticquantity.hpp>
#include <iomanip>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>

namespace onsem
{

SemanticFloat::SemanticFloat(int pValue,
                             std::size_t pValueAfterTheDecimalPoint,
                             unsigned char pNbOfSignificantDigit)
  : value(pValue),
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
  return value == pNb &&
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
  if (value != pOther.value)
    return value < pOther.value;

  auto minNbOfDigits = std::min(nbOfSignificantDigit, pOther.nbOfSignificantDigit);
  auto nb1 = valueAfterTheDecimalPoint * (nbOfSignificantDigit - minNbOfDigits) * 10;
  auto nb2 = pOther.valueAfterTheDecimalPoint * (pOther.nbOfSignificantDigit - minNbOfDigits) * 10;
  if (value >= 0)
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
  if (value != pOther.value)
    return value >= pOther.value;

  auto minNbOfDigits = std::min(nbOfSignificantDigit, pOther.nbOfSignificantDigit);
  auto nb1 = valueAfterTheDecimalPoint * (nbOfSignificantDigit - minNbOfDigits) * 10;
  auto nb2 = pOther.valueAfterTheDecimalPoint * (pOther.nbOfSignificantDigit - minNbOfDigits) * 10;
  if (value >= 0)
    return nb1 >= nb2;
  return nb1 <= nb2;
}

bool SemanticFloat::operator>=(int pNb) const
{
  if (value != pNb)
    return value >= pNb;

  if (nbOfSignificantDigit == 0)
    return true;

  if (value >= 0)
    return valueAfterTheDecimalPoint >= 0;
  return valueAfterTheDecimalPoint <= 0;
}

SemanticFloat SemanticFloat::operator+(const SemanticFloat& pOther) const
{
  SemanticFloat res;
  res.value = value + pOther.value;
  res.nbOfSignificantDigit = std::max(nbOfSignificantDigit, pOther.nbOfSignificantDigit);

  long nb1 = valueAfterTheDecimalPoint * (res.nbOfSignificantDigit - nbOfSignificantDigit) * 10;
  if (nb1 == 0)
    nb1 = valueAfterTheDecimalPoint;
  if (value < 0)
    nb1 *= -1;

  long nb2 = pOther.valueAfterTheDecimalPoint * (res.nbOfSignificantDigit - pOther.nbOfSignificantDigit) * 10;
  if (nb2 == 0)
    nb2 = pOther.valueAfterTheDecimalPoint;
  if (pOther.value < 0)
    nb2 *= -1;

  res.valueAfterTheDecimalPoint = std::abs(nb1 + nb2);
  return res;
}

void SemanticFloat::add(const SemanticFloat& pOther)
{
  value += pOther.value;
  auto newNbOfSignificantDigit = std::max(nbOfSignificantDigit, pOther.nbOfSignificantDigit);

  long nb1 = valueAfterTheDecimalPoint * (newNbOfSignificantDigit - nbOfSignificantDigit) * 10;
  if (nb1 == 0)
    nb1 = valueAfterTheDecimalPoint;
  if (value < 0)
    nb1 *= -1;

  long nb2 = pOther.valueAfterTheDecimalPoint * (newNbOfSignificantDigit - pOther.nbOfSignificantDigit) * 10;
  if (nb2 == 0)
    nb2 = pOther.valueAfterTheDecimalPoint;
  if (pOther.value < 0)
    nb2 *= -1;

  valueAfterTheDecimalPoint = std::abs(nb1 + nb2);
  nbOfSignificantDigit = newNbOfSignificantDigit;
}

void SemanticFloat::add(int pValue)
{
  value += pValue;
}

void SemanticFloat::set(int pValue)
{
  value = pValue;
  valueAfterTheDecimalPoint = 0;
  nbOfSignificantDigit = 0;
}

bool SemanticFloat::isAnInteger() const
{
  return valueAfterTheDecimalPoint == 0 &&
      nbOfSignificantDigit == 0;
}

std::string SemanticFloat::toStr(SemanticLanguageEnum pLanguage) const
{
  std::stringstream ss;
  ss << value;
  if (nbOfSignificantDigit > 0)
  {
    if (pLanguage == SemanticLanguageEnum::FRENCH)
      ss << ",";
    else
      ss << ".";
    ss << std::setfill('0') << std::setw(nbOfSignificantDigit) << valueAfterTheDecimalPoint;
  }
  return ss.str();
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

void SemanticQuantity::increaseNumber(int pIncreaseValue)
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


