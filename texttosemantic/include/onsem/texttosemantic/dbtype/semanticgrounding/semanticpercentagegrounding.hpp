#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICPERCENTAGEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICPERCENTAGEGROUNDING_HPP

#include "../../api.hpp"
#include "semanticgrounding.hpp"
#include "../semanticquantity.hpp"


namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticPercentageGrounding : public SemanticGrounding
{
  SemanticPercentageGrounding()
    : SemanticGrounding(SemanticGroundingType::PERCENTAGE),
      value()
  {
    concepts["percent_*"] = 4;
  }

  const SemanticPercentageGrounding& getPercentageGrounding() const override { return *this; }
  SemanticPercentageGrounding& getPercentageGrounding() override { return *this; }
  const SemanticPercentageGrounding* getPercentageGroundingPtr() const override { return this; }
  SemanticPercentageGrounding* getPercentageGroundingPtr() override { return this; }

  bool operator==(const SemanticPercentageGrounding& pOther) const;
  bool isEqual(const SemanticPercentageGrounding& pOther) const;

  SemanticFloat value;
};



inline bool SemanticPercentageGrounding::operator==(const SemanticPercentageGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticPercentageGrounding::isEqual(const SemanticPercentageGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      value == pOther.value;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICPERCENTAGEGROUNDING_HPP
