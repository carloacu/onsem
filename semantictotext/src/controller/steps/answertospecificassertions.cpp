#include "answertospecificassertions.hpp"
#include <set>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../semexpcontroller.hpp"
#include "../../operator/externalteachingrequester.hpp"
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"

namespace onsem {
namespace answerToSpecificAssertions {

namespace {

bool _processWantToKnowSentences(SemControllerWorkingStruct& pWorkStruct,
                                 SemanticMemoryBlockViewer& pMemViewer,
                                 const GroundedExpression& pGrdExp) {
    if (pWorkStruct.author == nullptr || !SemExpGetter::agentIsTheSubject(pGrdExp, pWorkStruct.author->userId))
        return false;

    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject != pGrdExp.children.end()) {
        auto objGrdExPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
        if (objGrdExPtr == nullptr)
            return false;

        const SemanticStatementGrounding* objStatGrdPtr = objGrdExPtr->grounding().getStatementGroundingPtr();
        if (objStatGrdPtr == nullptr)
            return false;

        if (objStatGrdPtr->concepts.count("mentalState_know") != 0) {
            if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT)
                return false;
            if (objStatGrdPtr->verbTense != SemanticVerbTense::UNKNOWN)
                return false;

            auto itKnowObject = objGrdExPtr->children.find(GrammaticalType::OBJECT);
            if (itKnowObject != objGrdExPtr->children.end()) {
                auto questionGrdExPtr = itKnowObject->second->getGrdExpPtr_SkipWrapperPtrs();
                if (questionGrdExPtr == nullptr)
                    return false;

                const GroundedExpression& questionGrdExp = *questionGrdExPtr;
                const SemanticStatementGrounding* questionStatGrdPtr = questionGrdExp->getStatementGroundingPtr();
                if (questionStatGrdPtr != nullptr) {
                    auto& questionStatGrd = *questionStatGrdPtr;
                    if (questionStatGrd.requests.has(SemanticRequestType::NOTHING)
                        || questionStatGrd.requests.has(SemanticRequestType::ACTION))
                        return false;
                    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
                    if (subWorkStruct.askForNewRecursion()) {
                        controller::manageQuestion(
                            subWorkStruct, pMemViewer, questionStatGrd, questionGrdExp, {}, questionGrdExp);
                        pWorkStruct.addAnswers(subWorkStruct);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool _processTeachSentences(SemControllerWorkingStruct& pWorkStruct,
                            const SemanticMemoryBlockViewer& pMemViewer,
                            const GroundedExpression& pGrdExp,
                            const SemanticStatementGrounding& pStatGrd) {
    auto res = privateImplem::reactToAnExternalTeachingRequest(
        pMemViewer, pGrdExp, pStatGrd, pWorkStruct.author->userId, pWorkStruct.lingDb);
    if (!res)
        return false;
    pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::EXTERNALTEACHINGREQUEST, std::move(res));
    return true;
}

}

bool process(SemControllerWorkingStruct& pWorkStruct,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp) {
    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr)
        return false;
    auto& statGrd = *statGrdPtr;
    if (!statGrd.polarity)
        return false;

    if (SemExpGetter::isWishStatement(statGrd) &&
        _processWantToKnowSentences(pWorkStruct, pMemViewer, pGrdExp))
        return true;
    for (const auto& currCpt : statGrd.concepts) {
        if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT && pWorkStruct.proativeSpecificationsPtr != nullptr
            && pWorkStruct.proativeSpecificationsPtr->canLearnANewAxiomaticnAction
            && currCpt.first == "verb_action_teach")
            return _processTeachSentences(pWorkStruct, pMemViewer, pGrdExp, *statGrdPtr);
    }

    auto grdExpToDpPtr = SemExpGetter::getGrdExpToDo(pGrdExp, statGrd, pMemViewer.currentUserId);
    if (grdExpToDpPtr != nullptr) {
        const SemanticStatementGrounding* statGrdToDoPtr = grdExpToDpPtr->grounding().getStatementGroundingPtr();
        if (statGrdToDoPtr != nullptr) {
            SemControllerWorkingStruct subWorkStruct(pWorkStruct);
            if (subWorkStruct.askForNewRecursion()) {
                subWorkStruct.comparisonExceptions.request = true;
                subWorkStruct.comparisonExceptions.verbTense = true;
                controller::manageAction(subWorkStruct, pMemViewer, *statGrdToDoPtr, *grdExpToDpPtr, *grdExpToDpPtr);
                pWorkStruct.addAnswers(subWorkStruct);
                return true;
            }
        }
    }

    return false;
}

}    // End of namespace answerToSpecificAssertions
}    // End of namespace onsem
