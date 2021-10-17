#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVEDURATIONGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVEDURATIONGROUNDING_HPP

#include "semanticgrouding.hpp"
#include <onsem/common/enum/semanticrelativedurationtype.hpp>


namespace onsem
{

// Relative Duration Grounding
// ===========================


struct ONSEM_TEXTTOSEMANTIC_API SemanticRelativeDurationGrounding : public SemanticGrounding
{
  SemanticRelativeDurationGrounding(SemanticRelativeDurationType pDurationType)
    : SemanticGrounding(SemanticGroudingType::RELATIVEDURATION),
      durationType(pDurationType)
  {
  }

  const SemanticRelativeDurationGrounding& getRelDurationGrounding() const { return *this; }
  SemanticRelativeDurationGrounding& getRelDurationGrounding() { return *this; }
  const SemanticRelativeDurationGrounding* getRelDurationGroundingPtr() const { return this; }
  SemanticRelativeDurationGrounding* getRelDurationGroundingPtr() { return this; }

  bool operator==(const SemanticRelativeDurationGrounding& pOther) const;
  bool isEqual(const SemanticRelativeDurationGrounding& pOther) const;

  SemanticRelativeDurationType durationType;
};




inline bool SemanticRelativeDurationGrounding::operator==(const SemanticRelativeDurationGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticRelativeDurationGrounding::isEqual(const SemanticRelativeDurationGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      durationType == pOther.durationType;
}


} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVEDURATIONGROUNDING_HPP
