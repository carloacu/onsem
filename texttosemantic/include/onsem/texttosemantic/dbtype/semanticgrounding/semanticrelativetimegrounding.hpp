#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVETIMEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVETIMEGROUNDING_HPP

#include "semanticgrounding.hpp"
#include <onsem/common/enum/semanticrelativetimetype.hpp>


namespace onsem
{

// Relative Time Grounding
// =======================


struct ONSEM_TEXTTOSEMANTIC_API SemanticRelativeTimeGrounding : public SemanticGrounding
{
  SemanticRelativeTimeGrounding(SemanticRelativeTimeType pTimeType)
    : SemanticGrounding(SemanticGroundingType::RELATIVETIME),
      timeType(pTimeType)
  {
  }

  const SemanticRelativeTimeGrounding& getRelTimeGrounding() const { return *this; }
  SemanticRelativeTimeGrounding& getRelTimeGrounding() { return *this; }
  const SemanticRelativeTimeGrounding* getRelTimeGroundingPtr() const { return this; }
  SemanticRelativeTimeGrounding* getRelTimeGroundingPtr() { return this; }

  bool operator==(const SemanticRelativeTimeGrounding& pOther) const;
  bool isEqual(const SemanticRelativeTimeGrounding& pOther) const;

  SemanticRelativeTimeType timeType;
};




inline bool SemanticRelativeTimeGrounding::operator==(const SemanticRelativeTimeGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticRelativeTimeGrounding::isEqual(const SemanticRelativeTimeGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      timeType == pOther.timeType;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVETIMEGROUNDING_HPP
