#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICQUANTITY_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICQUANTITY_HPP

#include "../api.hpp"
#include <onsem/common/enum/semanticsubjectivequantity.hpp>
#include <onsem/common/enum/semanticquantitytype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/misc/sign.hpp>

namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API SemanticFloat
{
  SemanticFloat(int pValue = 0,
                std::size_t pValueAfterTheDecimalPoint = 0u,
                unsigned char pNbOfSignificantDigit = 0u);
  bool operator==(const SemanticFloat& pOther) const;
  bool operator==(int pNb) const;
  bool operator!=(const SemanticFloat& pOther) const;
  bool operator!=(int pNb) const;
  bool operator<(const SemanticFloat& pOther) const;
  bool operator>(const SemanticFloat& pOther) const;
  bool operator>=(const SemanticFloat& pOther) const;
  bool operator>=(int pNb) const;
  bool operator<=(const SemanticFloat& pOther) const;
  bool operator<=(int pNb) const;
  SemanticFloat operator+=(const SemanticFloat& rhs);
  SemanticFloat operator+(const SemanticFloat& pOther) const;
  SemanticFloat operator-=(const SemanticFloat& pOther);
  SemanticFloat operator-(const SemanticFloat& pOther) const;
  void add(const SemanticFloat& pOther);
  void substract(const SemanticFloat& pOther);
  SemanticFloat operator*(const SemanticFloat& pOther) const;
  void set(int pValue);
  bool isPositive() const;
  bool isAnInteger() const;
  int signedValueWithoutDecimal() const;

  double toDouble() const;

  std::string toStr(SemanticLanguageEnum pLanguage = SemanticLanguageEnum::ENGLISH) const;
  std::string toStrWithSeparator(char pSeparator) const;

  bool fromStr(const std::string& pStr, SemanticLanguageEnum pLanguage = SemanticLanguageEnum::ENGLISH);
  bool fromStrWithSeparator(const std::string& pStr, char pSeparator);

  Sign sign;
  std::size_t value;
  std::size_t valueAfterTheDecimalPoint;
  unsigned char nbOfSignificantDigit;
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticQuantity
{
  bool operator==(const SemanticQuantity& pOther) const;

  void setNumber(int pNumber);
  void setNumber(const SemanticFloat& pNumber);

  void increaseNumber(const SemanticFloat& pIncreaseValue);

  void setNumberToFill(int pParamId,
                       std::string pAttributeName);

  bool getNumberToFill(int& pParamId,
                       std::string& pAttributeName) const;

  void setPlural();

  bool isPlural() const;

  bool isUnknown() const;

  bool isEqualToInit() const;

  bool isEqualTo(int pNb) const;

  bool isEqualToZero() const;

  bool isEqualToOne() const;

  void clear();

  SemanticQuantityType type = SemanticQuantityType::UNKNOWN;
  SemanticFloat nb;
  std::string paramSpec = std::string{""}; // if type is SemanticQuantityType::NUMBERTOFILL
  SemanticSubjectiveQuantity subjectiveValue = SemanticSubjectiveQuantity::UNKNOWN;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICQUANTITY_HPP
