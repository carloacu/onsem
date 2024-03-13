#include <onsem/semantictotext/triggers.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "controller/semexpcontroller.hpp"
#include "conversion/conditionsadder.hpp"
#include "utility/utility.hpp"

namespace onsem {
namespace triggers {

void add(UniqueSemanticExpression pTriggerSemExp,
         UniqueSemanticExpression pReactionSemExp,
         SemanticMemory& pSemanticMemory,
         const linguistics::LinguisticDatabase& pLingDb) {
    conditionsAdder::addConditonsForSomeTimedGrdExp(pTriggerSemExp, "", false);
    memoryOperation::resolveAgentAccordingToTheContext(pTriggerSemExp, pSemanticMemory, pLingDb);
    converter::addBothDirectionForms(pTriggerSemExp, pLingDb);

    // Add anything children to the trigger for asked parameters
    std::set<GrammaticalType> askedChildren;
    SemExpGetter::extractAskedChildrenByAResource(askedChildren, *pReactionSemExp);
    for (const auto& currAskedChild : askedChildren)
        SemExpModifier::addAnythingChild(*pTriggerSemExp, currAskedChild);

    auto expForMem = pSemanticMemory.memBloc.addRootSemExp(std::move(pTriggerSemExp), pLingDb);
    expForMem->outputToAnswerIfTriggerHasMatched.emplace(std::move(pReactionSemExp));
    expForMem->addTriggerLinks(InformationType::ASSERTION, *expForMem->semExp, pLingDb);
}

std::shared_ptr<ExpressionWithLinks> match(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                                           SemanticMemory& pSemanticMemory,
                                           UniqueSemanticExpression pUtteranceSemExp,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           const ReactionOptions* pReactionOptions) {
    converter::addDifferentForms(pUtteranceSemExp, pLingDb);
    conditionsAdder::addConditonsForSomeTimedGrdExp(pUtteranceSemExp);

    static const InformationType informationType = InformationType::INFORMATION;
    std::unique_ptr<CompositeSemAnswer> compSemAnswers;
    auto expForMem = pSemanticMemory.memBloc.addRootSemExp(std::move(pUtteranceSemExp), pLingDb);
    ExpressionWithLinks& expForMemRef = *expForMem;
    controller::applyOperatorOnExpHandleInMemory(compSemAnswers,
                                                 expForMemRef,
                                                 SemanticOperatorEnum::REACTFROMTRIGGER,
                                                 informationType,
                                                 pSemanticMemory,
                                                 nullptr,
                                                 pLingDb,
                                                 pReactionOptions);

    if (compSemAnswers) {
        controller::linkConditionalReactions(
            compSemAnswers->semAnswers, expForMemRef, pSemanticMemory, pLingDb, informationType);
        utility::keepOnlyLastFeedback(*compSemAnswers);
        controller::compAnswerToSemExp(pReaction, *compSemAnswers);
    }
    return expForMem;
}

}    // End of namespace triggers
}    // End of namespace onsem
