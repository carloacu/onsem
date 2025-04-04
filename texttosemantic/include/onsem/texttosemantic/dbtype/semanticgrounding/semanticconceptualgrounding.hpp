#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICCONCEPTUALGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICCONCEPTUALGROUNDING_HPP

#include <list>
#include "semanticgrounding.hpp"
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API SemanticConceptualGrounding : public SemanticGrounding {
    SemanticConceptualGrounding()
        : SemanticGrounding(SemanticGroundingType::CONCEPTUAL) {}
    SemanticConceptualGrounding(const std::map<std::string, char>& pConcepts)
        : SemanticGrounding(SemanticGroundingType::CONCEPTUAL, pConcepts) {}
    SemanticConceptualGrounding(const std::string& pConceptStr)
        : SemanticGrounding(SemanticGroundingType::CONCEPTUAL, std::map<std::string, char>{{pConceptStr, 4}}) {}

    const SemanticConceptualGrounding& getConceptualGrounding() const override { return *this; }
    SemanticConceptualGrounding& getConceptualGrounding() override { return *this; }
    const SemanticConceptualGrounding* getConceptualGroundingPtr() const override { return this; }
    SemanticConceptualGrounding* getConceptualGroundingPtr() override { return this; }

    bool operator==(const SemanticConceptualGrounding& pOther) const { return this->isEqual(pOther); }
    bool isEqual(const SemanticConceptualGrounding& pOther) const { return _isMotherClassEqual(pOther); }
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICCONCEPTUALGROUNDING_HPP
