#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticpercentagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticunitygrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>

namespace onsem {
namespace linguistics {

std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::xFillPercentageStruct(
    const ToGenRepContext& pContext) const {
    switch (pContext.chunk.type) {
        case ChunkType::NOMINAL_CHUNK:
        case ChunkType::PREPOSITIONAL_CHUNK: {
            const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
            if (ConceptSet::haveAConcept(iGram.infos.concepts, "percent")) {
                std::unique_ptr<SemanticPercentageGrounding> newPercentage;
                for (TokIt itToken =
                         getPrevToken(pContext.chunk.head, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head);
                     itToken != pContext.chunk.head;
                     itToken = getPrevToken(itToken, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head)) {
                    SemanticFloat number;
                    if (getNumberHoldByTheInflWord(number, itToken, pContext.chunk.head, "number_")) {
                        if (!newPercentage)
                            newPercentage = std::make_unique<SemanticPercentageGrounding>();
                        newPercentage->value = number;
                    } else if (itToken->getPartOfSpeech() == PartOfSpeech::DETERMINER) {
                        if (newPercentage)
                            return std::make_unique<GroundedExpression>(std::move(newPercentage));
                        return {};
                    }
                }

                if (newPercentage)
                    return std::make_unique<GroundedExpression>(std::move(newPercentage));
                return {};
            } else if (ConceptSet::haveAConcept(iGram.infos.concepts, "percentage")) {
                return std::make_unique<GroundedExpression>(
                    std::make_unique<SemanticUnityGrounding>(TypeOfUnity::PERCENTAGE, ""));
            }
            break;
        }
        default: {
            break;
        }
    }
    return {};
}

}    // End of namespace linguistics
}    // End of namespace onsem
