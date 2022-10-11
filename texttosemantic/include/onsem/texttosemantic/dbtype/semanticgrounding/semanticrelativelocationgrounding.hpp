#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVELOCATIONGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVELOCATIONGROUNDING_HPP

#include "semanticgrounding.hpp"
#include <onsem/common/enum/semanticrelativelocationtype.hpp>


namespace onsem
{

// Relative Location Grounding
// ===========================


struct ONSEM_TEXTTOSEMANTIC_API SemanticRelativeLocationGrounding : public SemanticGrounding
{
  SemanticRelativeLocationGrounding(SemanticRelativeLocationType pLocationType)
    : SemanticGrounding(SemanticGroundingType::RELATIVELOCATION),
      locationType(pLocationType)
  {
  }

  const SemanticRelativeLocationGrounding& getRelLocationGrounding() const { return *this; }
  SemanticRelativeLocationGrounding& getRelLocationGrounding() { return *this; }
  const SemanticRelativeLocationGrounding* getRelLocationGroundingPtr() const { return this; }
  SemanticRelativeLocationGrounding* getRelLocationGroundingPtr() { return this; }

  bool operator==(const SemanticRelativeLocationGrounding& pOther) const;
  bool isEqual(const SemanticRelativeLocationGrounding& pOther) const;

  SemanticRelativeLocationType locationType;
};




inline bool SemanticRelativeLocationGrounding::operator==(const SemanticRelativeLocationGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticRelativeLocationGrounding::isEqual(const SemanticRelativeLocationGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      locationType == pOther.locationType;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRELATIVELOCATIONGROUNDING_HPP
