#include "conditionsadder.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticrelativetimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include "simplesentencesplitter.hpp"

namespace onsem {
namespace conditionsAdder {

void addConditonsForSomeTimedGrdExp(UniqueSemanticExpression& pSemExp,
                                    const std::string& pUserId,
                                    bool pAddInterpretations) {
    switch (pSemExp->type) {
        case SemanticExpressionType::GROUNDED: {
            GroundedExpression& grdExp = pSemExp->getGrdExp();
            if (grdExp->type == SemanticGroundingType::STATEMENT) {
                const SemanticStatementGrounding& statGrd = grdExp->getStatementGrounding();

                if (statGrd.verbTense == SemanticVerbTense::PUNCTUALPRESENT
                    && statGrd.requests.has(SemanticRequestType::ACTION)) {
                    auto itTimeChild = grdExp.children.find(GrammaticalType::TIME);
                    if (itTimeChild != grdExp.children.end()) {
                        const GroundedExpression* timeGrdExpPtr = itTimeChild->second->getGrdExpPtr_SkipWrapperPtrs();
                        if (timeGrdExpPtr != nullptr) {
                            auto timeGrdType = timeGrdExpPtr->grounding().type;
                            if (timeGrdType == SemanticGroundingType::STATEMENT
                                || timeGrdType == SemanticGroundingType::TIME) {
                                auto timeSemExp = itTimeChild->second.extractContent();
                                SemExpModifier::removeChildFromSemExp(*timeSemExp, GrammaticalType::INTRODUCTING_WORD);
                                splitter::splitInVerySimpleSentences(timeSemExp, false);
                                grdExp.children.erase(itTimeChild);
                                if (pAddInterpretations) {
                                    auto condSemExp = std::make_unique<ConditionExpression>(
                                        timeGrdType == SemanticGroundingType::STATEMENT,
                                        false,
                                        std::move(timeSemExp),
                                        pSemExp->clone());
                                    pSemExp = std::make_unique<InterpretationExpression>(
                                        InterpretationSource::CONDITION, std::move(condSemExp), std::move(pSemExp));
                                } else {
                                    pSemExp = std::make_unique<ConditionExpression>(
                                        timeGrdType == SemanticGroundingType::STATEMENT,
                                        false,
                                        std::move(timeSemExp),
                                        std::move(pSemExp));
                                }
                                break;
                            }

                            if (timeGrdType == SemanticGroundingType::RELATIVETIME) {
                                auto relTimeGrd = timeGrdExpPtr->grounding().getRelTimeGrounding();
                                if (relTimeGrd.timeType == SemanticRelativeTimeType::DELAYEDSTART) {
                                    auto durationSemExp = itTimeChild->second.extractContent();
                                    SemExpModifier::removeChildFromSemExp(*durationSemExp,
                                                                          GrammaticalType::INTRODUCTING_WORD);
                                    splitter::splitInVerySimpleSentences(durationSemExp, false);
                                    grdExp.children.erase(itTimeChild);

                                    if (pAddInterpretations) {
                                        auto condSemExp = std::make_unique<ConditionExpression>(
                                            false, false, std::move(durationSemExp), pSemExp->clone());
                                        pSemExp = std::make_unique<InterpretationExpression>(
                                            InterpretationSource::CONDITION, std::move(condSemExp), std::move(pSemExp));
                                    } else {
                                        pSemExp = std::make_unique<ConditionExpression>(
                                            false, false, std::move(durationSemExp), std::move(pSemExp));
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp->getListExp();
            for (auto& currElt : listExp.elts)
                addConditonsForSomeTimedGrdExp(currElt, pUserId);
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            addConditonsForSomeTimedGrdExp(pSemExp->getIntExp().interpretedExp, pUserId);
            break;
        }
        case SemanticExpressionType::FEEDBACK: {
            addConditonsForSomeTimedGrdExp(pSemExp->getFdkExp().concernedExp, pUserId);
            break;
        }
        case SemanticExpressionType::ANNOTATED: {
            addConditonsForSomeTimedGrdExp(pSemExp->getAnnExp().semExp, pUserId);
            break;
        }
        case SemanticExpressionType::METADATA: {
            auto& metadataExp = pSemExp->getMetadataExp();
            const SemanticAgentGrounding* agentPtr = nullptr;
            auto* authorSemExpPtr = metadataExp.getAuthorSemExpPtr();
            if (authorSemExpPtr != nullptr)
                agentPtr = SemExpGetter::extractAgentGrdPtr(*authorSemExpPtr);
            if (agentPtr != nullptr)
                addConditonsForSomeTimedGrdExp(metadataExp.semExp, agentPtr->userId);
            else
                addConditonsForSomeTimedGrdExp(metadataExp.semExp, pUserId);
            break;
        }
        case SemanticExpressionType::CONDITION: {
            auto& condExp = pSemExp->getCondExp();
            addConditonsForSomeTimedGrdExp(condExp.thenExp, pUserId);
            if (condExp.elseExp)
                addConditonsForSomeTimedGrdExp(*condExp.elseExp, pUserId);
            break;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::SETOFFORMS: break;
    }
}

}    // End of namespace conditionsAdder
}    // End of namespace onsem
