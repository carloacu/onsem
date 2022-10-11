#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICGENERICGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICGENERICGROUNDING_HPP

#include <list>
#include <set>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/utility/optional.hpp>
#include <onsem/common/enum/semanticentitytype.hpp>
#include <onsem/common/enum/semanticquantitytype.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticsubjectivequantity.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/misc/coreference.hpp>
#include "semanticgrounding.hpp"
#include "../../api.hpp"

namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticQuantity
{
  bool operator==(const SemanticQuantity& pOther) const
  {
    return type == pOther.type &&
        nb == pOther.nb &&
        paramSpec == pOther.paramSpec &&
        subjectiveValue == pOther.subjectiveValue;
  }

  void setNumber(int pNumber)
  {
    type = SemanticQuantityType::NUMBER;
    nb = pNumber;
  }

  void increaseNumber(int pIncreaseValue)
  {
    if (type == SemanticQuantityType::NUMBER)
      nb = nb + pIncreaseValue;
    else
      setNumber(pIncreaseValue);
  }

  void setNumberToFill(int pParamId,
                       std::string pAttributeName)
  {
    type = SemanticQuantityType::NUMBERTOFILL;
    std::stringstream ss;
    ss << pParamId;
    if (!pAttributeName.empty())
      ss << "_" << pAttributeName;
    paramSpec = ss.str();
  }

  bool getNumberToFill(int& pParamId,
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

  void setPlural()
  {
    type = SemanticQuantityType::MOREOREQUALTHANNUMBER;
    nb = 2;
  }

  bool isPlural() const
  {
    return (type == SemanticQuantityType::MOREOREQUALTHANNUMBER && nb >= 2) ||
        (type == SemanticQuantityType::NUMBER && nb >= 2) ||
        type == SemanticQuantityType::MAXNUMBER;
  }

  bool isUnknown() const
  {
    return type == SemanticQuantityType::UNKNOWN && nb == 0 && subjectiveValue == SemanticSubjectiveQuantity::UNKNOWN;
  }

  bool isEqualToInit() const
  {
    return type == SemanticQuantityType::UNKNOWN && nb == 0 && paramSpec.empty() && subjectiveValue == SemanticSubjectiveQuantity::UNKNOWN;
  }

  bool isEqualTo(int pNb) const
  {
    return type == SemanticQuantityType::NUMBER && nb == pNb;
  }

  bool isEqualToZero() const
  {
    return isEqualTo(0);
  }

  bool isEqualToOne() const
  {
    return isEqualTo(1);
  }

  void clear()
  {
    type = SemanticQuantityType::UNKNOWN;
    nb = 0;
  }

  SemanticQuantityType type = SemanticQuantityType::UNKNOWN;
  int nb = 0;
  std::string paramSpec = std::string{""}; // if type is SemanticQuantityType::NUMBERTOFILL
  SemanticSubjectiveQuantity subjectiveValue = SemanticSubjectiveQuantity::UNKNOWN;
};



struct ONSEM_TEXTTOSEMANTIC_API SemanticGenericGrounding : public SemanticGrounding
{
  SemanticGenericGrounding()
    : SemanticGrounding(SemanticGroudingType::GENERIC),
      referenceType(SemanticReferenceType::UNDEFINED),
      coreference(),
      entityType(SemanticEntityType::UNKNOWN),
      quantity(),
      word(),
      possibleGenders()
  {
  }

  SemanticGenericGrounding
  (SemanticReferenceType pReferenceType,
   SemanticEntityType pAgentType);

  const SemanticGenericGrounding& getGenericGrounding() const override { return *this; }
  SemanticGenericGrounding& getGenericGrounding() override { return *this; }
  const SemanticGenericGrounding* getGenericGroundingPtr() const override { return this; }
  SemanticGenericGrounding* getGenericGroundingPtr() override { return this; }

  bool operator==(const SemanticGenericGrounding& pOther) const;
  bool isEqual(const SemanticGenericGrounding& pOther) const;

  static std::unique_ptr<SemanticGenericGrounding> makeThingThatHasToBeCompletedFromContext();

  SemanticReferenceType referenceType;
  mystd::optional<Coreference> coreference;
  SemanticEntityType entityType; // TODO: remove this attribute
  SemanticQuantity quantity;
  SemanticWord word;
  std::set<SemanticGenderType> possibleGenders;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICGENERICGROUNDING_HPP
