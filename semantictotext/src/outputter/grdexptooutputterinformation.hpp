#ifndef ONSEM_SEMANTICTOTEXT_SRC_OUTPUTTER_GRDEXPTOOUTPUTINFORMATION_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_OUTPUTTER_GRDEXPTOOUTPUTINFORMATION_HPP

#include <optional>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>

namespace onsem {

struct OutputInformation {
    std::optional<int> nbOfTimes;
    std::optional<const SemanticExpression*> toDoInBackground;
};

bool grdExpToOutputInformation(OutputInformation& pOutputInformation, const GroundedExpression& pGrdExp);

bool hasGrdExpOutputInformations(const GroundedExpression& pGrdExp);

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SRC_OUTPUTTER_GRDEXPTOOUTPUTINFORMATION_HPP
