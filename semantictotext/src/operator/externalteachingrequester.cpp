#include "externalteachingrequester.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../semanticmemory/semanticmemoryblockviewer.hpp"

namespace onsem {
namespace privateImplem {

std::unique_ptr<SemanticExpression> reactToAnExternalTeachingRequest(const SemanticMemoryBlockViewer& pMemViewer,
                                                                     const GroundedExpression& pGrdExp,
                                                                     const SemanticStatementGrounding& pStatGrd,
                                                                     const std::string& pAuthorId,
                                                                     const linguistics::LinguisticDatabase& pLingDb) {
    const SemanticExpression* externalTeachingLabelSemExpPtr = nullptr;
    SemanticLanguageEnum pTeachingLanguage = SemanticLanguageEnum::UNKNOWN;
    if (!SemExpGetter::extractExternalTeachingLabel(
            pGrdExp, pStatGrd, externalTeachingLabelSemExpPtr, pTeachingLanguage, pAuthorId))
        return {};
    assert(externalTeachingLabelSemExpPtr != nullptr);

    TextProcessingContext textProcContext(
        SemanticAgentGrounding::me, SemanticAgentGrounding::currentUser, pTeachingLanguage);
    std::string labelOfActionToLearn;
    static const std::set<SemanticExpressionType> expressionTypesToSkip{SemanticExpressionType::SETOFFORMS};
    UniqueSemanticExpression semExpToSay(externalTeachingLabelSemExpPtr->clone(nullptr, false, &expressionTypesToSkip));
    SemExpModifier::clearRequestListOfSemExp(*semExpToSay);
    converter::semExpToText(labelOfActionToLearn,
                            std::move(semExpToSay),
                            textProcContext,
                            false,
                            pMemViewer.constView,
                            pAuthorId,
                            pLingDb,
                            nullptr);
    return std::make_unique<GroundedExpression>(
        std::make_unique<SemanticResourceGrounding>("learn", textProcContext.langType, labelOfActionToLearn));
}

mystd::unique_propagate_const<UniqueSemanticExpression> externalTeachingRequester(
    const SemanticExpression& pSemExp,
    const SemanticMemoryBlockViewer& pMemViewer,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::string& pAuthorUserId) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            const GroundedExpression& grdExp = pSemExp.getGrdExp();
            const SemanticStatementGrounding* statGrdPtr = grdExp->getStatementGroundingPtr();
            if (statGrdPtr == nullptr || !ConceptSet::haveAConcept(statGrdPtr->concepts, "verb_action_teach"))
                return {};
            auto res = reactToAnExternalTeachingRequest(pMemViewer, grdExp, *statGrdPtr, pAuthorUserId, pLingDb);
            if (res)
                return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(res));
            return {};
        }
        case SemanticExpressionType::LIST: {
            const ListExpression& listExp = pSemExp.getListExp();
            if (listExp.listType == ListExpressionType::UNRELATED || listExp.listType == ListExpressionType::AND) {
                for (const auto& currElt : listExp.elts) {
                    auto res = externalTeachingRequester(*currElt, pMemViewer, pLingDb, pAuthorUserId);
                    if (res)
                        return res;
                }
            }
            return {};
        }
        case SemanticExpressionType::INTERPRETATION: {
            return externalTeachingRequester(*pSemExp.getIntExp().interpretedExp, pMemViewer, pLingDb, pAuthorUserId);
        }
        case SemanticExpressionType::FEEDBACK: {
            return externalTeachingRequester(*pSemExp.getFdkExp().concernedExp, pMemViewer, pLingDb, pAuthorUserId);
        }
        case SemanticExpressionType::ANNOTATED: {
            return externalTeachingRequester(*pSemExp.getAnnExp().semExp, pMemViewer, pLingDb, pAuthorUserId);
        }
        case SemanticExpressionType::METADATA: {
            const MetadataExpression& metadataExp = pSemExp.getMetadataExp();
            const SemanticAgentGrounding* authorPtr = metadataExp.getAuthorPtr();
            if (authorPtr != nullptr)
                return externalTeachingRequester(*metadataExp.semExp, pMemViewer, pLingDb, authorPtr->userId);
            return externalTeachingRequester(*metadataExp.semExp, pMemViewer, pLingDb, pAuthorUserId);
        }
        case SemanticExpressionType::FIXEDSYNTHESIS: {
            return externalTeachingRequester(pSemExp.getFSynthExp().getSemExp(), pMemViewer, pLingDb, pAuthorUserId);
        }
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::CONDITION:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::SETOFFORMS: return {};
    }

    assert(false);
    return {};
}

}    // End of namespace privateImplem
}    // End of namespace onsem
