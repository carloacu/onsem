#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_GROUNDING_SEMANTICCONCEPTUALGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_GROUNDING_SEMANTICCONCEPTUALGROUNDING_HPP

#include <list>
#include "semanticgrouding.hpp"
#include "../../api.hpp"

namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticConceptualGrounding : public SemanticGrounding
{
  SemanticConceptualGrounding()
    : SemanticGrounding(SemanticGroudingType::CONCEPTUAL)
  {}
  SemanticConceptualGrounding(const std::map<std::string, char>& pConcepts)
    : SemanticGrounding(SemanticGroudingType::CONCEPTUAL, pConcepts)
  {}
  SemanticConceptualGrounding(const std::string& pConceptStr)
    : SemanticGrounding(SemanticGroudingType::CONCEPTUAL, std::map<std::string, char>{{pConceptStr, 4}})
  {}

  const SemanticConceptualGrounding& getConceptualGrounding() const override { return *this; }
  SemanticConceptualGrounding& getConceptualGrounding() override { return *this; }
  const SemanticConceptualGrounding* getConceptualGroundingPtr() const override { return this; }
  SemanticConceptualGrounding* getConceptualGroundingPtr() override { return this; }

  bool operator==(const SemanticConceptualGrounding& pOther) const { return this->isEqual(pOther); }
  bool isEqual(const SemanticConceptualGrounding& pOther) const { return _isMotherClassEqual(pOther); }
};


} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_GROUNDING_SEMANTICCONCEPTUALGROUNDING_HPP
