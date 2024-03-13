#include "semexpcontroller.hpp"
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>
#include "../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../utility/semexpcreator.hpp"
#include "../type/semanticdetailledanswer.hpp"
#include "../interpretation/addagentinterpretation.hpp"
#include "../operator/semanticcategorizer.hpp"
#include "steps/proactivereaction.hpp"
#include "steps/proactivereactionfromnominalgroups.hpp"
#include "steps/answertospecificquestions.hpp"
#include "steps/managecondition.hpp"
#include "steps/semanticmemorylinker.hpp"
#include "steps/similaritieswithmemoryfinder.hpp"
#include "steps/specificactionshandler.hpp"
#include "steps/answertospecificassertions.hpp"
#include "steps/answertospecificquestions.hpp"

namespace onsem {
namespace controller {
namespace {
const SemanticTriggerAxiomId _emptyAxiomId;

void _informOrReactToTheCauseChild(SemControllerWorkingStruct& pWorkStruct,
                                   SemanticMemoryBlockViewer& pMemViewer,
                                   const SemanticExpression& pSemExp) {
    // do the operation also on the cause child
    const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        const GroundedExpression& grdExp = *grdExpPtr;
        auto itCauseChild = grdExp.children.find(GrammaticalType::CAUSE);
        if (itCauseChild != grdExp.children.end())
            applyOperatorOnSemExp(pWorkStruct, pMemViewer, *itCauseChild->second);
        return;
    }

    // if it's a list recursively do the same stuff for each element
    const ListExpression* listExpPtr = pSemExp.getListExpPtr();
    if (listExpPtr != nullptr) {
        const ListExpression& listExp = *listExpPtr;
        for (const auto& currElt : listExp.elts)
            _informOrReactToTheCauseChild(pWorkStruct, pMemViewer, *currElt);
        return;
    }
}

void _applyOperatorResolveSecondPersonOfSingular(SemanticExpression& pSemExp, const SemanticMemory& pSemanticMemory) {
    const std::string newUserId = pSemanticMemory.getCurrUserId();
    SemExpModifier::replaceAgentOfSemExp(pSemExp, newUserId, SemanticAgentGrounding::currentUser);
}

}

void applyOperatorOnExpHandleInMemory(
    std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
    ExpressionWithLinks& pExpressionWithLinks,
    SemanticOperatorEnum pReactionOperator,
    InformationType pInformationType,
    SemanticMemory& pSemanticMemory,
    std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
    const linguistics::LinguisticDatabase& pLingDb,
    const ReactionOptions* pReactionOptions) {
    const auto& semExp = *pExpressionWithLinks.semExp;
    SemControllerWorkingStruct workStruct(pInformationType,
                                          nullptr,
                                          SemanticLanguageEnum::UNKNOWN,
                                          &pExpressionWithLinks,
                                          pReactionOperator,
                                          &pSemanticMemory.proativeSpecifications,
                                          pSemanticMemory.getExternalFallback(),
                                          &pSemanticMemory.callbackToSentencesCanBeAnswered,
                                          pAxiomToConditionCurrentStatePtr,
                                          pLingDb);

    if (pReactionOperator == SemanticOperatorEnum::REACT && pSemanticMemory.interactionContextContainer) {
        auto& interactionContextContainer = *pSemanticMemory.interactionContextContainer;
        auto* currentInteractionContextPtr = interactionContextContainer.getCurrentInteractionContextPtr();
        if (currentInteractionContextPtr != nullptr) {
            auto& currentInteractionContext = *currentInteractionContextPtr;
            for (auto& currAnswPoss : currentInteractionContext.answerPossibilities) {
                auto* intPtr = semExp.getIntExpPtr_SkipWrapperPtrs();
                if ((intPtr != nullptr
                     && SemExpComparator::semExpsAreEqual(
                         *currAnswPoss.first, *intPtr->originalExp, pSemanticMemory.memBloc, pLingDb))
                    || SemExpComparator::semExpsAreEqual(
                        *currAnswPoss.first, semExp, pSemanticMemory.memBloc, pLingDb)) {
                    auto* answerInteractionContextPtr =
                        interactionContextContainer.getInteractionContextPtr(currAnswPoss.second);
                    if (answerInteractionContextPtr != nullptr) {
                        workStruct.addAnswerWithoutReferences(ContextualAnnotation::ANSWER,
                                                              answerInteractionContextPtr->textToSay->clone());
                        if (answerInteractionContextPtr->answerPossibilities.empty())
                            pSemanticMemory.interactionContextContainer.reset();
                        else
                            interactionContextContainer.currentPosition.emplace(currAnswPoss.second);
                        break;
                    }
                }
            }
        }
    }

    if (workStruct.compositeSemAnswers) {
        if (pReactionOptions != nullptr)
            workStruct.reactionOptions = *pReactionOptions;
        SemanticMemoryBlockViewer memViewer(
            &pSemanticMemory.memBloc, pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId());
        applyOperatorOnSemExp(workStruct, memViewer, semExp);
    }
    pCompositeSemAnswers = std::move(workStruct.compositeSemAnswers);
}

void applyOperatorResolveAgentAccordingToTheContext(UniqueSemanticExpression& pSemExp,
                                                    const SemanticMemory& pSemanticMemory,
                                                    const linguistics::LinguisticDatabase& pLingDb) {
    _applyOperatorResolveSecondPersonOfSingular(*pSemExp, pSemanticMemory);
    agentInterpretations::addAgentInterpretations(pSemExp, pSemanticMemory, pLingDb);
}

void _applyOperatorOnComparisonExp(SemControllerWorkingStruct& pWorkStruct,
                                   const SemanticMemoryBlockViewer& pMemViewer,
                                   const ComparisonExpression& pCompExp) {
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::CHECK && pCompExp.rightOperandExp) {
        bool answerFound = false;
        bool resPolarity = true;
        if (pCompExp.op == ComparisonOperator::EQUAL || pCompExp.op == ComparisonOperator::DIFFERENT) {
            bool isEqual = SemExpComparator::semExpsAreEqualFromMemBlock(*pCompExp.leftOperandExp,
                                                                         **pCompExp.rightOperandExp,
                                                                         pMemViewer.constView,
                                                                         pWorkStruct.lingDb,
                                                                         nullptr);
            resPolarity = pCompExp.op == ComparisonOperator::EQUAL ? isEqual : !isEqual;
            answerFound = true;
        } else {
            ComparisonOperator polarity =
                SemExpComparator::numberComparisonOfSemExps(*pCompExp.leftOperandExp, **pCompExp.rightOperandExp, true);
            if (polarity != ComparisonOperator::DIFFERENT) {
                resPolarity = pCompExp.op == polarity
                           || (pCompExp.op == ComparisonOperator::DIFFERENT && polarity != ComparisonOperator::EQUAL);
                answerFound = true;
            }
        }

        if (answerFound) {
            pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&resPolarity] {
                auto newAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                newAnsw->answerElts[SemanticRequestType::YESORNO].answersGenerated.emplace_back(
                    SemExpCreator::sayYesOrNo(resPolarity));
                return newAnsw;
            }());
        }
    }
}

void _applyOperatorIfOnListExp(SemControllerWorkingStruct& pWorkStruct,
                               SemanticMemoryBlockViewer& pMemViewer,
                               const ListExpression& pListExp) {
    bool aThrowOccurs = false;
    SemControllerWorkingStruct ansSubWorkStruct(pWorkStruct);

    for (const auto& currElt : pListExp.elts) {
        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
        applyOperatorOnSemExp(subWorkStruct, pMemViewer, *currElt);

        TruenessValue agreementVal = subWorkStruct.agreementTypeOfTheAnswer();

        if (pListExp.listType == ListExpressionType::OR && agreementVal == TruenessValue::VAL_TRUE) {
            pWorkStruct.addAnswers(subWorkStruct);
            return;
        }

        if (pListExp.listType == ListExpressionType::AND && agreementVal == TruenessValue::VAL_FALSE) {
            pWorkStruct.addAnswers(subWorkStruct);
            return;
        }

        if (agreementVal == TruenessValue::UNKNOWN) {
            ansSubWorkStruct.compositeSemAnswers->semAnswers.clear();
            ansSubWorkStruct.addAnswers(subWorkStruct);
            aThrowOccurs = true;
        } else if (!aThrowOccurs) {
            ansSubWorkStruct.compositeSemAnswers->semAnswers.clear();
            ansSubWorkStruct.addAnswers(subWorkStruct);
        }
    }

    if (!ansSubWorkStruct.compositeSemAnswers->semAnswers.empty()) {
        pWorkStruct.addAnswers(ansSubWorkStruct);
    }
}

void _filterDetailledAnsw(std::list<std::unique_ptr<SemAnswer>>& pSemAnwers,
                          const std::list<const GroundedExpression*>& pNewGrdExpAnswers,
                          bool pAnd_Or,
                          const SemanticMemoryBlock& pMemBlock,
                          const linguistics::LinguisticDatabase& pLingDb) {
    auto ifGrdStillExist = [&](const GroundedExpression& pGrdExp) {
        for (const auto& currNewGrdExp : pNewGrdExpAnswers) {
            if (SemExpComparator::grdExpsAreEqual(pGrdExp, *currNewGrdExp, pMemBlock, pLingDb)) {
                return true;
            }
        }
        return false;
    };

    for (auto itDetAns = pSemAnwers.begin(); itDetAns != pSemAnwers.end();) {
        LeafSemAnswer* leafPtr = (*itDetAns)->getLeafPtr();
        if (leafPtr != nullptr) {
            LeafSemAnswer& leafAnsw = *leafPtr;

            for (auto itAnsForARequ = leafAnsw.answerElts.begin(); itAnsForARequ != leafAnsw.answerElts.end();) {
                AllAnswerElts& answerElt = itAnsForARequ->second;
                for (auto itKnoAndGrdExp = answerElt.answersFromMemory.begin();
                     itKnoAndGrdExp != answerElt.answersFromMemory.end();) {
                    if (pAnd_Or == ifGrdStillExist(itKnoAndGrdExp->getGrdExp()))
                        ++itKnoAndGrdExp;
                    else
                        itKnoAndGrdExp = answerElt.answersFromMemory.erase(itKnoAndGrdExp);
                }

                for (auto itKnowAns = answerElt.answersGenerated.begin();
                     itKnowAns != answerElt.answersGenerated.end();) {
                    const GroundedExpression* grdExp = itKnowAns->genSemExp->getGrdExpPtr_SkipWrapperPtrs();
                    if (grdExp != nullptr && pAnd_Or == ifGrdStillExist(*grdExp))
                        ++itKnowAns;
                    else
                        itKnowAns = answerElt.answersGenerated.erase(itKnowAns);
                }

                if (!answerElt.isEmpty())
                    ++itAnsForARequ;
                else
                    itAnsForARequ = leafAnsw.answerElts.erase(itAnsForARequ);
            }

            if (!leafAnsw.isEmpty())
                ++itDetAns;
            else
                itDetAns = pSemAnwers.erase(itDetAns);
            continue;
        }
        ++itDetAns;
    }
}

void _applyOperatorGetOnListExp(SemControllerWorkingStruct& pWorkStruct,
                                SemanticMemoryBlockViewer& pMemViewer,
                                const ListExpression& pListExp) {
    bool firstIteration = true;
    SemControllerWorkingStruct ansSubWorkStruct(pWorkStruct);
    for (const auto& currElt : pListExp.elts) {
        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
        applyOperatorOnSemExp(subWorkStruct, pMemViewer, *currElt);

        if (pListExp.listType == ListExpressionType::OR) {
            if (!subWorkStruct.compositeSemAnswers->semAnswers.empty()) {
                pWorkStruct.addAnswers(subWorkStruct);
                return;
            }
        } else if (firstIteration) {
            ansSubWorkStruct.addAnswers(subWorkStruct);
            firstIteration = false;
        } else if (pListExp.listType == ListExpressionType::AND) {
            std::list<const GroundedExpression*> newGrdExpAnswers;
            CompositeSemAnswer::getGrdExps(newGrdExpAnswers, subWorkStruct.compositeSemAnswers->semAnswers);
            _filterDetailledAnsw(ansSubWorkStruct.compositeSemAnswers->semAnswers,
                                 newGrdExpAnswers,
                                 true,
                                 pMemViewer.constView,
                                 pWorkStruct.lingDb);

        } else {
            std::list<const GroundedExpression*> existingGrdExpAnswers;
            CompositeSemAnswer::getGrdExps(existingGrdExpAnswers, ansSubWorkStruct.compositeSemAnswers->semAnswers);
            _filterDetailledAnsw(subWorkStruct.compositeSemAnswers->semAnswers,
                                 existingGrdExpAnswers,
                                 false,
                                 pMemViewer.constView,
                                 pWorkStruct.lingDb);

            ansSubWorkStruct.addAnswers(subWorkStruct);
        }
    }

    if (!ansSubWorkStruct.compositeSemAnswers->semAnswers.empty())
        pWorkStruct.addAnswers(ansSubWorkStruct);
}

void _applyOperatorOnListExp(SemControllerWorkingStruct& pWorkStruct,
                             SemanticMemoryBlockViewer& pMemViewer,
                             const ListExpression& pListExp) {
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::CHECK
        && pListExp.listType != ListExpressionType::UNRELATED) {
        _applyOperatorIfOnListExp(pWorkStruct, pMemViewer, pListExp);
    } else if (pWorkStruct.reactOperator == SemanticOperatorEnum::GET) {
        _applyOperatorGetOnListExp(pWorkStruct, pMemViewer, pListExp);
    } else {
        if (pWorkStruct.reactionOptions.canAnswerWithATrigger
            && semanticMemoryLinker::addTriggerListExp(pWorkStruct, pMemViewer, pListExp))
            return;

        for (const auto& currElt : pListExp.elts) {
            SemControllerWorkingStruct subWorkStruct(pWorkStruct);
            applyOperatorOnSemExp(subWorkStruct, pMemViewer, *currElt);
            pWorkStruct.addAnswers(pListExp.listType, subWorkStruct);
        }
    }
}

void _applyOperatorOnFeedbackExp(SemControllerWorkingStruct& pWorkStruct,
                                 SemanticMemoryBlockViewer& pMemViewer,
                                 const FeedbackExpression& pFdkExp) {
    const SemanticExpression& concernedSemExp = *pFdkExp.concernedExp;
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
        {
            SemControllerWorkingStruct subWorkStruct(pWorkStruct);
            subWorkStruct.reactOperator = SemanticOperatorEnum::REACTFROMTRIGGER;
            applyOperatorOnSemExp(subWorkStruct, pMemViewer, concernedSemExp);
            if (subWorkStruct.haveAnAnswer()) {
                pWorkStruct.addAnswers(subWorkStruct);
                return;
            }
        }

        // answer "hello <user name>" to "hello <robot name>"
        // answer "bye <user name>" to "bye <robot name>"
        const GroundedExpression* fdkGrdExpPtr = pFdkExp.feedbackExp->getGrdExpPtr_SkipWrapperPtrs();
        if (fdkGrdExpPtr != nullptr
            && ConceptSet::haveAConceptThatBeginWith(fdkGrdExpPtr->grounding().concepts, "engagement_")) {
            const GroundedExpression* concernedGrdExpPtr = concernedSemExp.getGrdExpPtr_SkipWrapperPtrs();
            if (concernedGrdExpPtr != nullptr) {
                const GroundedExpression& concernedGrdExp = *concernedGrdExpPtr;
                const SemanticAgentGrounding* concernedAgentPtr = concernedGrdExp->getAgentGroundingPtr();
                if (concernedAgentPtr != nullptr && pMemViewer.constView.isItMe(concernedAgentPtr->userId)) {
                    // we will answer "hello/bye <user name>"
                    std::unique_ptr<GroundedExpression> userName;
                    if (pWorkStruct.author != nullptr) {
                        auto whatIsYourName = SemExpCreator::askWhatIsYourName(pWorkStruct.author->userId);
                        converter::addDifferentForms(whatIsYourName, pWorkStruct.lingDb);
                        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
                        if (subWorkStruct.askForNewRecursion()) {
                            subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
                            SemanticMemoryBlockViewer subMemView(pMemViewer);
                            controller::applyOperatorOnSemExp(subWorkStruct, subMemView, *whatIsYourName);
                            if (subWorkStruct.compositeSemAnswers) {
                                std::list<const GroundedExpression*> grdExpAnswers;
                                CompositeSemAnswer::getGrdExps(grdExpAnswers,
                                                               subWorkStruct.compositeSemAnswers->semAnswers);
                                if (!grdExpAnswers.empty())
                                    userName = grdExpAnswers.front()->clone();
                            }
                        }
                    }
                    if (!userName) {
                        userName = std::make_unique<GroundedExpression>([] {
                            auto res = std::make_unique<SemanticGenericGrounding>();
                            res->quantity.setNumber(1);
                            res->word.setContent(SemanticLanguageEnum::ENGLISH, "human", PartOfSpeech::NOUN);
                            return res;
                        }());
                    }

                    pWorkStruct.addAnswerWithoutReferences(
                        ContextualAnnotation::FEEDBACK,
                        std::make_unique<FeedbackExpression>(pFdkExp.feedbackExp->clone(), std::move(userName)));
                    return;
                }
            }
        }
    }

    applyOperatorOnSemExp(pWorkStruct, pMemViewer, concernedSemExp);
}

std::unique_ptr<SemanticExpression> _getSourceInPresent(const MetadataExpression& pMetadataExp) {
    auto presentTenseSource = (*pMetadataExp.source)->clone();
    SemExpModifier::modifyVerbTenseOfSemExp(*presentTenseSource, SemanticVerbTense::PUNCTUALPRESENT);
    return presentTenseSource;
}

void _punctuallyAssertAboutTheSource(SemControllerWorkingStruct& pWorkStruct,
                                     SemanticMemoryBlockViewer& pMemViewer,
                                     const SemanticExpression& pSemExp,
                                     SemanticLanguageEnum pLanguage) {
    auto robotAgent = std::make_unique<GroundedExpression>(SemanticAgentGrounding::getRobotAgentPtr());
    SemControllerWorkingStruct subWorkStruct(InformationType::ASSERTION,
                                             &*robotAgent,
                                             pLanguage,
                                             nullptr,
                                             SemanticOperatorEnum::INFORM,
                                             pWorkStruct.proativeSpecificationsPtr,
                                             pWorkStruct.externalFallbackPtr,
                                             pWorkStruct.callbackToSentencesCanBeAnsweredPtr,
                                             pWorkStruct.axiomToConditionCurrentStatePtr,
                                             pWorkStruct.lingDb);
    applyOperatorOnSemExp(subWorkStruct, pMemViewer, pSemExp);
    pWorkStruct.addAnswers(subWorkStruct);
}

void _permanentAssertAboutTheSource(SemControllerWorkingStruct& pWorkStruct,
                                    SemanticMemoryBlockViewer& pMemViewer,
                                    SemanticLanguageEnum pFromLanguage,
                                    const SemanticExpression& pSource) {
    SemControllerWorkingStruct subWorkStruct(InformationType::ASSERTION,
                                             nullptr,
                                             pFromLanguage,
                                             pWorkStruct.expHandleInMemory,
                                             SemanticOperatorEnum::INFORM,
                                             pWorkStruct.proativeSpecificationsPtr,
                                             pWorkStruct.externalFallbackPtr,
                                             pWorkStruct.callbackToSentencesCanBeAnsweredPtr,
                                             pWorkStruct.axiomToConditionCurrentStatePtr,
                                             pWorkStruct.lingDb);
    applyOperatorOnSemExp(subWorkStruct, pMemViewer, pSource);
    pWorkStruct.addAnswers(subWorkStruct);
}

void applyOperatorOnSemExp(SemControllerWorkingStruct& pWorkStruct,
                           SemanticMemoryBlockViewer& pMemViewer,
                           const SemanticExpression& pSemExp) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            const GroundedExpression& grdExp = pSemExp.getGrdExp();
            std::list<const GroundedExpression*> otherGrdExps;
            const auto& originalGrdExp = pWorkStruct.getOriginalGrdExp(grdExp);
            applyOperatorOnGrdExp(pWorkStruct, pMemViewer, grdExp, otherGrdExps, originalGrdExp);
            break;
        }
        case SemanticExpressionType::CONDITION: {
            const ConditionExpression& condExp = pSemExp.getCondExp();
            const auto* originalGrdExpPtr = pWorkStruct.getOriginalGrdExpPtr();
            manageCondition(pWorkStruct, pMemViewer, condExp, originalGrdExpPtr);
            break;
        }
        case SemanticExpressionType::COMPARISON: {
            const ComparisonExpression& compExp = pSemExp.getCompExp();
            _applyOperatorOnComparisonExp(pWorkStruct, pMemViewer, compExp);
            break;
        }
        case SemanticExpressionType::LIST: {
            const auto& listExp = pSemExp.getListExp();
            _applyOperatorOnListExp(pWorkStruct, pMemViewer, listExp);
            break;
        }
        case SemanticExpressionType::SETOFFORMS: {
            const SetOfFormsExpression& setOfFormsExp = pSemExp.getSetOfFormsExp();
            const GroundedExpression* originalGrdExpForm = SemExpGetter::getOriginalGrdExpForm(setOfFormsExp);
            bool canAnswerIDontKnow = pWorkStruct.reactionOptions.canAnswerIDontKnow;
            bool canAnswerWithATrigger = pWorkStruct.reactionOptions.canAnswerWithATrigger;
            bool canAnswerWithExternalEngines = pWorkStruct.reactionOptions.canAnswerWithExternalEngines;
            if (originalGrdExpForm != nullptr) {
                std::list<SemControllerWorkingStruct> textAnalyzes;
                SemControllerWorkingStruct* textAnalyzesOfOriginalFromPtr = nullptr;
                for (const auto& currForms : setOfFormsExp.prioToForms) {
                    std::list<const GroundedExpression*> otherGrdExps;
                    bool hasOriginalForm = false;
                    const GroundedExpression* mainGrdExp =
                        SemExpGetter::splitMainGrdAndOtherOnes(otherGrdExps, hasOriginalForm, currForms.second);

                    textAnalyzes.emplace_back(pWorkStruct);
                    SemControllerWorkingStruct& lastTextAnalyze = textAnalyzes.back();
                    if (textAnalyzesOfOriginalFromPtr == nullptr || hasOriginalForm)
                        textAnalyzesOfOriginalFromPtr = &lastTextAnalyze;
                    lastTextAnalyze.reactionOptions.canAnswerIDontKnow = canAnswerIDontKnow;
                    lastTextAnalyze.reactionOptions.canAnswerWithATrigger = canAnswerWithATrigger;
                    lastTextAnalyze.reactionOptions.canAnswerWithExternalEngines = canAnswerWithExternalEngines;

                    if (mainGrdExp != nullptr) {
                        const auto& originalGrdExp = pWorkStruct.getOriginalGrdExp(*originalGrdExpForm);
                        applyOperatorOnGrdExp(lastTextAnalyze, pMemViewer, *mainGrdExp, otherGrdExps, originalGrdExp);
                    } else if (!currForms.second.empty()) {
                        applyOperatorOnSemExp(lastTextAnalyze, pMemViewer, *currForms.second.front()->exp);
                    }
                    canAnswerWithATrigger = false;
                    canAnswerWithExternalEngines = false;

                    for (auto& currDetailledAnwer : lastTextAnalyze.compositeSemAnswers->semAnswers) {
                        const LeafSemAnswer* leafPtr = currDetailledAnwer->getLeafPtr();
                        if (leafPtr != nullptr && leafPtr->type != ContextualAnnotation::ANSWERNOTFOUND
                            && leafPtr->type != ContextualAnnotation::QUESTION) {
                            canAnswerIDontKnow = false;
                            pWorkStruct.addAnswers(lastTextAnalyze);
                            if (pWorkStruct.isFinished() || !pWorkStruct.canHaveAnotherTextualAnswer())
                                return;
                            break;
                        }
                    }
                }
                if (!pWorkStruct.haveAnAnswer() && textAnalyzesOfOriginalFromPtr != nullptr) {
                    pWorkStruct.addAnswers(*textAnalyzesOfOriginalFromPtr);
                    return;
                }
            } else {
                throw std::runtime_error("OriginalGrdExpForm not found");
            }
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            const InterpretationExpression& intExp = pSemExp.getIntExp();
            pWorkStruct.originalSemExpPtr = &*intExp.originalExp;
            auto& intSemExp =
                *intExp
                     .interpretedExp;    // This variable is to allow to enter directly on the folllowing call with gdb
            applyOperatorOnSemExp(pWorkStruct, pMemViewer, intSemExp);
            break;
        }
        case SemanticExpressionType::FEEDBACK: {
            const FeedbackExpression& fdkExp = pSemExp.getFdkExp();
            _applyOperatorOnFeedbackExp(pWorkStruct, pMemViewer, fdkExp);
            break;
        }
        case SemanticExpressionType::ANNOTATED: {
            const AnnotatedExpression& annExp = pSemExp.getAnnExp();
            for (const auto& currAnnotation : annExp.annotations)
                pWorkStruct.annotatedExps[currAnnotation.first] = &*currAnnotation.second;
            applyOperatorOnSemExp(pWorkStruct, pMemViewer, *annExp.semExp);
            break;
        }
        case SemanticExpressionType::METADATA: {
            const MetadataExpression& metadataExp = pSemExp.getMetadataExp();

            bool informOrReact = pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                              || pWorkStruct.reactOperator == SemanticOperatorEnum::INFORM
                              || pWorkStruct.reactOperator == SemanticOperatorEnum::EXECUTEFROMCONDITION;
            const bool isSourceASentence = metadataExp.isSourceASentence();
            bool storeSource = informOrReact && isSourceASentence;
            pWorkStruct.fromLanguage = metadataExp.fromLanguage;

            // Notify about the source without storing anything to the memory.
            // (source = "the robot say blabla" or "the user says blabla")
            // The goal is only to trigger existing conditions
            std::unique_ptr<SemanticExpression> sourceInPresent;
            if (storeSource) {
                sourceInPresent = _getSourceInPresent(metadataExp);
                _punctuallyAssertAboutTheSource(pWorkStruct, pMemViewer, *sourceInPresent, metadataExp.fromLanguage);
            }

            pWorkStruct.authorSemExp = metadataExp.getAuthorSemExpPtr();
            pWorkStruct.author = pWorkStruct.authorSemExp != nullptr
                                   ? SemExpGetter::extractAgentGrdPtr(*pWorkStruct.authorSemExp)
                                   : nullptr;
            const auto& semExp = *metadataExp.semExp;

            // Consider the information contained in the cause child
            if (informOrReact)
                _informOrReactToTheCauseChild(pWorkStruct, pMemViewer, semExp);

            // Main operation
            ExpressionWithLinks* memKnowledge = pWorkStruct.expHandleInMemory;
            if (pWorkStruct.expHandleInMemory != nullptr
                && SemExpGetter::extractSource(*pWorkStruct.expHandleInMemory->semExp)
                       == SemanticSourceEnum::SEMREACTION) {
                // We don't save the sentence in the memory if it comes the robot
                pWorkStruct.expHandleInMemory = nullptr;
            }
            SemControllerWorkingStruct subWorkStruct(pWorkStruct);
            subWorkStruct.isAtRoot = pWorkStruct.isAtRoot;
            applyOperatorOnSemExp(subWorkStruct, pMemViewer, semExp);
            if (!pWorkStruct.haveAnAnswer())
                pWorkStruct.addAnswers(subWorkStruct);
            pWorkStruct.expHandleInMemory = memKnowledge;

            // Inform about the source.
            // (source = "the robot say blabla" or "the user says blabla")
            // The goal is to remember that it occured in the past.
            if (sourceInPresent) {
                SemExpModifier::invertPolarity(*sourceInPresent);
                _punctuallyAssertAboutTheSource(pWorkStruct, pMemViewer, *sourceInPresent, metadataExp.fromLanguage);
            }
            if (isSourceASentence
                && (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                    || pWorkStruct.reactOperator == SemanticOperatorEnum::INFORM
                    || pWorkStruct.reactOperator == SemanticOperatorEnum::TEACHINFORMATION)) {
                _permanentAssertAboutTheSource(pWorkStruct, pMemViewer, metadataExp.fromLanguage, **metadataExp.source);
            }
            break;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS: {
            const FixedSynthesisExpression& fSynthExp = pSemExp.getFSynthExp();
            applyOperatorOnSemExp(pWorkStruct, pMemViewer, fSynthExp.getSemExp());
            break;
        }
        case SemanticExpressionType::COMMAND: break;
    }
}

void applyOperatorOnSemExp(std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
                           const SemanticExpression& pSemExp,
                           SemanticOperatorEnum pReactionOperator,
                           InformationType pInformationType,
                           SemanticMemory& pSemanticMemory,
                           const linguistics::LinguisticDatabase& pLingDb) {
    SemControllerWorkingStruct workStruct(pInformationType,
                                          nullptr,
                                          SemanticLanguageEnum::UNKNOWN,
                                          nullptr,
                                          pReactionOperator,
                                          &pSemanticMemory.proativeSpecifications,
                                          pSemanticMemory.getExternalFallback(),
                                          &pSemanticMemory.callbackToSentencesCanBeAnswered,
                                          nullptr,
                                          pLingDb);
    SemanticMemoryBlockViewer memViewer(
        &pSemanticMemory.memBloc, pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId());
    applyOperatorOnSemExp(workStruct, memViewer, pSemExp);
    pCompositeSemAnswers = std::move(workStruct.compositeSemAnswers);
}

void applyOperatorOnSemExpConstMem(
    std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
    const SemanticExpression& pSemExp,
    SemanticOperatorEnum pReactionOperator,
    InformationType pInformationType,
    const SemanticMemoryBlock& pConstMemBlock,
    const std::string& pCurrentUserId,
    const ProativeSpecifications* pProativeSpecificationsPtr,
    const ExternalFallback* pExternalFallbackPtr,
    const std::list<mystd::unique_propagate_const<MemBlockAndExternalCallback>>* pCallbackToSentencesCanBeAnsweredPtr,
    std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentState,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticTypeOfFeedback* pTypeOfFeedback,
    bool* pCanAnswerIDontKnowPtr) {
    SemControllerWorkingStruct workStruct(pInformationType,
                                          nullptr,
                                          SemanticLanguageEnum::UNKNOWN,
                                          nullptr,
                                          pReactionOperator,
                                          pProativeSpecificationsPtr,
                                          pExternalFallbackPtr,
                                          pCallbackToSentencesCanBeAnsweredPtr,
                                          pAxiomToConditionCurrentState,
                                          pLingDb);
    if (pTypeOfFeedback != nullptr)
        workStruct.typeOfFeedback = *pTypeOfFeedback;
    if (pCanAnswerIDontKnowPtr != nullptr)
        workStruct.reactionOptions.canAnswerIDontKnow = *pCanAnswerIDontKnowPtr;
    SemanticMemoryBlockViewer memViewer(nullptr, pConstMemBlock, pCurrentUserId);
    applyOperatorOnSemExp(workStruct, memViewer, pSemExp);
    pCompositeSemAnswers = std::move(workStruct.compositeSemAnswers);
}

void manageAction(SemControllerWorkingStruct& pWorkStruct,
                  SemanticMemoryBlockViewer& pMemViewer,
                  const SemanticStatementGrounding& pStatementStruct,
                  const GroundedExpression& pGrdExp,
                  const GroundedExpression& pOriginalGrdExp) {
    switch (pWorkStruct.reactOperator) {
        case SemanticOperatorEnum::REACT: {
            if (!semanticMemoryLinker::satisfyAnAction(pWorkStruct, pMemViewer, pGrdExp, pStatementStruct)
                && pWorkStruct.reactionOptions.canAnswerIDontKnow)
                pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::BEHAVIORNOTFOUND,
                                                       SemExpCreator::sayThatTheRobotCannotDoIt(pOriginalGrdExp));
            break;
        }
        case SemanticOperatorEnum::REACTFROMTRIGGER: {
            // get links of the current sentence
            semanticMemoryLinker::RequestLinks reqLinks;
            getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);

            bool anAnswerHasBeenAdded = false;
            if (addTriggerSentencesAnswer(pWorkStruct,
                                          anAnswerHasBeenAdded,
                                          pMemViewer,
                                          reqLinks,
                                          SemanticExpressionCategory::COMMAND,
                                          _emptyAxiomId,
                                          pGrdExp,
                                          ContextualAnnotation::BEHAVIOR))
                break;

            specificActionsHandler::process(pWorkStruct, pMemViewer, pGrdExp, pStatementStruct);
            break;
        }
        case SemanticOperatorEnum::EXECUTEBEHAVIOR:
        case SemanticOperatorEnum::RESOLVECOMMAND: {
            semanticMemoryLinker::satisfyAnAction(pWorkStruct, pMemViewer, pGrdExp, pStatementStruct);
            break;
        }
        case SemanticOperatorEnum::SHOW: {
            specificActionsHandler::process_forShowOperator(pWorkStruct, pMemViewer, pGrdExp, pStatementStruct);
            break;
        }
        case SemanticOperatorEnum::ANSWER:
        case SemanticOperatorEnum::CHECK:
        case SemanticOperatorEnum::GET:
        case SemanticOperatorEnum::HOWYOUKNOW:
        case SemanticOperatorEnum::FEEDBACK:
        case SemanticOperatorEnum::FIND:
        case SemanticOperatorEnum::INFORM:
        case SemanticOperatorEnum::EXECUTEFROMCONDITION:
        case SemanticOperatorEnum::TEACHBEHAVIOR:
        case SemanticOperatorEnum::TEACHCONDITION:
        case SemanticOperatorEnum::TEACHINFORMATION:
        case SemanticOperatorEnum::UNINFORM: break;
    }
}

void manageQuestion(SemControllerWorkingStruct& pWorkStruct,
                    SemanticMemoryBlockViewer& pMemViewer,
                    const SemanticStatementGrounding& pStatementGrd,
                    const GroundedExpression& pGrdExp,
                    const std::list<const GroundedExpression*>& pOtherGrdExps,
                    const GroundedExpression& pOriginalGrdExp) {
    switch (pWorkStruct.reactOperator) {
        case SemanticOperatorEnum::REACT: {
            if (!semanticMemoryLinker::satisfyAQuestion(
                    pWorkStruct, pMemViewer, pGrdExp, pOtherGrdExps, pOriginalGrdExp, pStatementGrd.requests)
                && pWorkStruct.reactionOptions.canAnswerIDontKnow) {
                bool reAskTheQuestion = pWorkStruct.author != nullptr && pWorkStruct.externalFallbackPtr == nullptr
                                     && SemExpGetter::agentIsTheSubject(pGrdExp, pWorkStruct.author->userId)
                                     && SemExpGetter::getGoal(pGrdExp) == VerbGoalEnum::NOTIFICATION;
                auto knowledgeWeCannotDoIt = [&]() -> UniqueSemanticExpression {
                    if (reAskTheQuestion)
                        return SemExpCreator::sayIKnow(false);
                    return SemExpCreator::sayThatWeDontKnowTheAnswer(pOriginalGrdExp);
                }();
                if (pWorkStruct.externalFallbackPtr != nullptr)
                    pWorkStruct.externalFallbackPtr->addFallback(
                        knowledgeWeCannotDoIt, pMemViewer.currentUserId, pOriginalGrdExp);
                pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::ANSWERNOTFOUND,
                                                       std::move(knowledgeWeCannotDoIt));
                if (reAskTheQuestion)
                    pWorkStruct.addQuestion(SemExpCreator::copyAndReformateGrdExpToPutItInAnAnswer(pGrdExp));
            }
            break;
        }
        case SemanticOperatorEnum::REACTFROMTRIGGER: {
            // get the triggers
            bool anAnswerHasBeenAdded = false;
            // TODO: get the links if the there is some triggers to optimize!
            semanticMemoryLinker::RequestLinks reqLinksOfOriginalGrdExp;
            getLinksOfAGrdExp(reqLinksOfOriginalGrdExp, pWorkStruct, pMemViewer, pOriginalGrdExp, false);
            if (addTriggerSentencesAnswer(pWorkStruct,
                                          anAnswerHasBeenAdded,
                                          pMemViewer,
                                          reqLinksOfOriginalGrdExp,
                                          SemanticExpressionCategory::QUESTION,
                                          _emptyAxiomId,
                                          pOriginalGrdExp,
                                          ContextualAnnotation::ANSWER))
                break;
            answerToSpecificQuestions::process(
                pWorkStruct, pMemViewer, pOriginalGrdExp, pStatementGrd.requests.firstOrNothing());
            break;
        }
        case SemanticOperatorEnum::ANSWER: {
            if (!semanticMemoryLinker::satisfyAQuestion(
                    pWorkStruct, pMemViewer, pGrdExp, pOtherGrdExps, pOriginalGrdExp, pStatementGrd.requests)
                && pWorkStruct.reactionOptions.canAnswerIDontKnow)
                pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::ANSWERNOTFOUND,
                                                       SemExpCreator::sayThatWeDontKnowTheAnswer(pOriginalGrdExp));
            break;
        }
        case SemanticOperatorEnum::CHECK:
        case SemanticOperatorEnum::GET:
        case SemanticOperatorEnum::HOWYOUKNOW: {
            semanticMemoryLinker::satisfyAQuestion(
                pWorkStruct, pMemViewer, pGrdExp, pOtherGrdExps, pOriginalGrdExp, pStatementGrd.requests);
            break;
        }
        case SemanticOperatorEnum::FEEDBACK: {
            if (pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::ASK_FOR_ADDITIONAL_INFORMATION
                && !answerToSpecificQuestions::process(
                    pWorkStruct, pMemViewer, pGrdExp, pStatementGrd.requests.firstOrNothing())) {
                bool reAskTheQuestion = pWorkStruct.author != nullptr
                                     && SemExpGetter::agentIsTheSubject(pGrdExp, pWorkStruct.author->userId);
                if (reAskTheQuestion)
                    pWorkStruct.addQuestion(pGrdExp.clone());
            }
            break;
        }
        case SemanticOperatorEnum::FIND:
        case SemanticOperatorEnum::INFORM:
        case SemanticOperatorEnum::RESOLVECOMMAND:
        case SemanticOperatorEnum::EXECUTEBEHAVIOR:
        case SemanticOperatorEnum::EXECUTEFROMCONDITION:
        case SemanticOperatorEnum::SHOW:
        case SemanticOperatorEnum::TEACHBEHAVIOR:
        case SemanticOperatorEnum::TEACHCONDITION:
        case SemanticOperatorEnum::TEACHINFORMATION:
        case SemanticOperatorEnum::UNINFORM: break;
    }
}

template<typename AXIOMTYPE>
bool _keepOnlyWhatIsLessDetailledFromAxiomList(std::list<AXIOMTYPE*>& pAxiomList,
                                               SemanticMemoryBlockViewer& pMemViewer,
                                               const linguistics::LinguisticDatabase& pLingDb,
                                               const GroundedExpression& pGrdExp) {
    bool res = false;
    for (auto it = pAxiomList.begin(); it != pAxiomList.end();) {
        bool eraseIt = false;
        for (const auto& currElt : (*it)->memorySentences.elts) {
            SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
            auto imbr = SemExpComparator::getGrdExpsImbrications(
                currElt.grdExp, pGrdExp, pMemViewer.constView, pLingDb, nullptr, &comparisonErrorReporting);
            if ((imbr == ImbricationType::MORE_DETAILED || imbr == ImbricationType::HYPONYM)
                && comparisonErrorReporting.getErrorCoef().type == SemExpComparator::ComparisonTypeOfError::NORMAL) {
                eraseIt = true;
                break;
            }
        }
        if (eraseIt) {
            it = pAxiomList.erase(it);
            res = true;
        } else {
            ++it;
        }
    }
    return res;
}

bool _keepOnlyWhatISLessDetailled(RelatedContextAxiom& pAnswersContextAxioms,
                                  SemanticMemoryBlockViewer& pMemViewer,
                                  const linguistics::LinguisticDatabase& pLingDb,
                                  const GroundedExpression& pGrdExp) {
    bool res = _keepOnlyWhatIsLessDetailledFromAxiomList(pAnswersContextAxioms.elts, pMemViewer, pLingDb, pGrdExp);
    return _keepOnlyWhatIsLessDetailledFromAxiomList(pAnswersContextAxioms.constElts, pMemViewer, pLingDb, pGrdExp)
        || res;
}

TruenessValue _checkIfItsTrue(RelatedContextAxiom& pAnswersContextAxioms,
                              SemanticMemoryBlockViewer& pMemViewer,
                              const SemControllerWorkingStruct& pWorkStruct,
                              const GroundedExpression& pGrdExp) {
    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion()) {
        subWorkStruct.reactOperator = SemanticOperatorEnum::CHECK;
        static const std::list<const GroundedExpression*> emptyListOfGrdExps;
        controller::applyOperatorOnGrdExp(subWorkStruct, pMemViewer, pGrdExp, emptyListOfGrdExps, pGrdExp);
        subWorkStruct.getSourceContextAxiom(pAnswersContextAxioms);
        return subWorkStruct.agreementTypeOfTheAnswer();
    }
    return TruenessValue::UNKNOWN;
}

void _updateConditionValidity(const GroundedExpWithLinks& pMemSentenceToUpdate,
                              SemControllerWorkingStruct& pWorkStruct,
                              SemanticMemoryBlockViewer& pMemViewer) {
    semanticMemoryLinker::RequestLinks reqLinks;
    semanticMemoryLinker::getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pMemSentenceToUpdate.grdExp, false);

    // get the pointers to the existing information in memory
    RelatedContextAxiom answersContextAxioms;    // the pointers to the existing information in memory
    if (pMemSentenceToUpdate.isEnabled()
        && (pWorkStruct.author == nullptr || !pMemViewer.constView.isItMe(pWorkStruct.author->userId)))
        _checkIfItsTrue(answersContextAxioms, pMemViewer, pWorkStruct, pMemSentenceToUpdate.grdExp);

    semanticMemoryLinker::checkForConditionsLinkedToStatement(
        pWorkStruct, pMemViewer, reqLinks, pMemSentenceToUpdate.grdExp);

    // unlink the old memories that are not revelant anymore
    if (pWorkStruct.informationType != InformationType::FALLBACK)
        for (SentenceWithLinks* currContextAxiomPtr : answersContextAxioms.elts)
            if (currContextAxiomPtr != nullptr && currContextAxiomPtr != &pMemSentenceToUpdate.getContextAxiom()
                && currContextAxiomPtr->informationType != InformationType::FALLBACK
                && currContextAxiomPtr->canOtherInformationTypeBeMoreRevelant(pWorkStruct.informationType))
                currContextAxiomPtr->setEnabled(false);
}

void _manageAssertion(SemControllerWorkingStruct& pWorkStruct,
                      SemanticMemoryBlockViewer& pMemViewer,
                      const GroundedExpression& pGrdExp,
                      const SemanticStatementGrounding& pStatementGrd,
                      const GroundedExpression& pOriginalGrdExp) {
    switch (pWorkStruct.reactOperator) {
        case SemanticOperatorEnum::CHECK:
        case SemanticOperatorEnum::HOWYOUKNOW: {
            // ask if it's true or false
            if (!pStatementGrd.isAtInfinitive())
                applyOperatorOnSemExp(pWorkStruct, pMemViewer, *SemExpCreator::askIfTrue(pGrdExp, pWorkStruct.lingDb));
            break;
        }
        case SemanticOperatorEnum::REACT:
        case SemanticOperatorEnum::INFORM:
        case SemanticOperatorEnum::FEEDBACK:
        case SemanticOperatorEnum::EXECUTEFROMCONDITION:
        case SemanticOperatorEnum::TEACHBEHAVIOR:
        case SemanticOperatorEnum::TEACHCONDITION:
        case SemanticOperatorEnum::TEACHINFORMATION: {
            // get links of the input grounded expression
            semanticMemoryLinker::RequestLinks reqLinks;
            semanticMemoryLinker::getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);

            // try to react according to the triggers
            bool isAnswered = false;
            if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
                if (pWorkStruct.reactionOptions.canAnswerWithATrigger) {
                    if (&pOriginalGrdExp != &pGrdExp) {
                        semanticMemoryLinker::RequestLinks reqLinksForTrigger;
                        semanticMemoryLinker::getLinksOfAGrdExp(
                            reqLinksForTrigger, pWorkStruct, pMemViewer, pOriginalGrdExp, false);
                        if (semanticMemoryLinker::matchAffirmationTrigger(
                                pWorkStruct, pMemViewer, reqLinksForTrigger, pOriginalGrdExp))
                            isAnswered = true;
                    }

                    if (!isAnswered
                        && semanticMemoryLinker::matchAffirmationTrigger(pWorkStruct, pMemViewer, reqLinks, pGrdExp))
                        isAnswered = true;

                    if (!isAnswered && &pOriginalGrdExp != &pGrdExp
                        && answerToSpecificAssertions::process(pWorkStruct, pMemViewer, pOriginalGrdExp))
                        break;
                }

                if (!isAnswered && answerToSpecificAssertions::process(pWorkStruct, pMemViewer, pGrdExp))
                    break;
            }

            if (!SemExpGetter::isGrdExpComplete(pGrdExp))
                break;

            // know if the input is an assertion and get the pointers to the existing information in memory
            RelatedContextAxiom answersContextAxioms;    // the pointers to the existing information in memory
            TruenessValue truenessValue = TruenessValue::UNKNOWN;    // if it's true according to the memory
            bool infoIsAlreadyAnAssertion = false;    // if the information in memory is an assertion or not
            bool canSaveNewGrdExp = true;
            if (pMemViewer.constView.disableOldContrarySentences
                && pWorkStruct.annotatedExps.count(GrammaticalType::TIME) == 0) {
                truenessValue = _checkIfItsTrue(answersContextAxioms, pMemViewer, pWorkStruct, pGrdExp);
                canSaveNewGrdExp =
                    !_keepOnlyWhatISLessDetailled(answersContextAxioms, pMemViewer, pWorkStruct.lingDb, pGrdExp);
                infoIsAlreadyAnAssertion = answersContextAxioms.isAnAssertion();
            }

            // if it's a new information link it to the memory
            SentenceWithLinks* newContextAxiom = nullptr;
            bool replacementNotified = false;
            auto setEnabledPreviousInformation = [&](bool pEnabled) {
                if (newContextAxiom != nullptr && pWorkStruct.informationType != InformationType::FALLBACK) {
                    for (SentenceWithLinks* currContextAxiomPtr : answersContextAxioms.elts) {
                        if (currContextAxiomPtr != nullptr
                            && currContextAxiomPtr->informationType != InformationType::FALLBACK
                            && currContextAxiomPtr->canOtherInformationTypeBeMoreRevelant(
                                pWorkStruct.informationType)) {
                            currContextAxiomPtr->setEnabled(pEnabled);
                            if (!pEnabled && !replacementNotified) {
                                replacementNotified = true;
                                auto& memBloc = currContextAxiomPtr->getSemExpWrappedForMemory().getParentMemBloc();
                                std::list<const GroundedExpression*> grdExpsDisabled;
                                for (const auto& currMemSent : currContextAxiomPtr->memorySentences.elts)
                                    grdExpsDisabled.emplace_back(&currMemSent.grdExp);
                                memBloc.grdExpReplacedGrdExps(pGrdExp, grdExpsDisabled);
                            }
                        }
                    }
                }
            };

            bool isNewInformationRevelant =
                (pWorkStruct.informationType == InformationType::ASSERTION
                 || pWorkStruct.informationType == InformationType::FALLBACK || !infoIsAlreadyAnAssertion)
                && canSaveNewGrdExp;

            if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                || pWorkStruct.reactOperator == SemanticOperatorEnum::INFORM
                || (pWorkStruct.reactOperator == SemanticOperatorEnum::TEACHBEHAVIOR
                    && privateImplem::categorizeGrdExp(pGrdExp, pWorkStruct.getAuthorUserId())
                           == SemanticExpressionCategory::ACTIONDEFINITION)
                || (pWorkStruct.reactOperator == SemanticOperatorEnum::TEACHINFORMATION
                    && privateImplem::categorizeGrdExp(pGrdExp, pWorkStruct.getAuthorUserId())
                           == SemanticExpressionCategory::AFFIRMATION)) {
                if (pWorkStruct.expHandleInMemory != nullptr && isNewInformationRevelant)
                    newContextAxiom = pWorkStruct.expHandleInMemory->addAxiomFromGrdExp(
                        pWorkStruct.informationType, pGrdExp, pWorkStruct.annotatedExps, pWorkStruct.lingDb, false);

                // Consider the new informations that become true because they are linked to a condition that is
                // satistied by the input. Recurssively this new informations can trigger new answers.
                {
                    std::set<const GroundedExpWithLinks*> newInformations;
                    getInformationsLinkedToCondition(newInformations, pWorkStruct, pMemViewer, reqLinks);
                    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
                    if (subWorkStruct.askForNewRecursion()) {
                        subWorkStruct.reactOperator = SemanticOperatorEnum::INFORM;
                        for (const auto& currNewInfo : newInformations)
                            _updateConditionValidity(*currNewInfo, subWorkStruct, pMemViewer);
                        if (subWorkStruct.haveAnAnswer()) {
                            isAnswered = true;
                            pWorkStruct.addAnswers(subWorkStruct);
                        }
                    }
                }

                // unlink the old memories that are not revelant anymore
                setEnabledPreviousInformation(false);
            }

            // Consider the new actions to do because they are linked to a condition that is satistied by the input.
            if ((pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                 || pWorkStruct.reactOperator == SemanticOperatorEnum::INFORM
                 || pWorkStruct.reactOperator == SemanticOperatorEnum::EXECUTEFROMCONDITION)
                && isNewInformationRevelant
                && semanticMemoryLinker::checkForConditionsLinkedToStatement(
                    pWorkStruct, pMemViewer, reqLinks, pGrdExp))
                isAnswered = true;

            if (isAnswered)
                break;

            // Proactively react to the input information
            if (pWorkStruct.reactionOptions.canDoAProactivity) {
                if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                    || pWorkStruct.reactOperator == SemanticOperatorEnum::TEACHBEHAVIOR) {
                    // check if the sentence was about learning a command
                    if (newContextAxiom != nullptr && newContextAxiom->infCommandToDo != nullptr) {
                        pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::TEACHINGFEEDBACK,
                                                               SemExpCreator::confirmInformation(pGrdExp));
                    }
                    // else do a proactive reaction
                    else if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                             && pWorkStruct.contAnnotationOfPreviousAnswers != ContextualAnnotation::QUESTION) {
                        setEnabledPreviousInformation(true);    // TODO: do it only when necessary
                        bool res = false;
                        bool cannotHaveOtherFeedbacks = proactiveReaction::process(res,
                                                                                   pWorkStruct,
                                                                                   newContextAxiom,
                                                                                   pMemViewer,
                                                                                   pGrdExp,
                                                                                   truenessValue,
                                                                                   infoIsAlreadyAnAssertion,
                                                                                   answersContextAxioms);
                        setEnabledPreviousInformation(false);
                        if (!cannotHaveOtherFeedbacks)
                            proactiveReaction::processWithUpdatedMemory(
                                res, pWorkStruct, newContextAxiom, pMemViewer, pGrdExp);
                    }
                } else if (pWorkStruct.reactOperator == SemanticOperatorEnum::FEEDBACK
                           && pWorkStruct.contAnnotationOfPreviousAnswers != ContextualAnnotation::QUESTION) {
                    bool res = false;
                    bool cannotHaveOtherFeedbacks = proactiveReaction::process(res,
                                                                               pWorkStruct,
                                                                               newContextAxiom,
                                                                               pMemViewer,
                                                                               pGrdExp,
                                                                               truenessValue,
                                                                               infoIsAlreadyAnAssertion,
                                                                               answersContextAxioms);
                    // TODO have the list of memory to not consider
                    if (!cannotHaveOtherFeedbacks)
                        proactiveReaction::processWithUpdatedMemory(
                            res, pWorkStruct, newContextAxiom, pMemViewer, pGrdExp);
                }
            }
            break;
        }
        case SemanticOperatorEnum::REACTFROMTRIGGER: {
            // get links of the input grounded expression
            semanticMemoryLinker::RequestLinks reqLinks;
            semanticMemoryLinker::getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);
            if (&pOriginalGrdExp != &pGrdExp) {
                semanticMemoryLinker::RequestLinks reqLinksForTrigger;
                semanticMemoryLinker::getLinksOfAGrdExp(
                    reqLinksForTrigger, pWorkStruct, pMemViewer, pOriginalGrdExp, false);
                if (semanticMemoryLinker::matchAffirmationTrigger(
                        pWorkStruct, pMemViewer, reqLinksForTrigger, pOriginalGrdExp))
                    break;
            }

            // try to react according to the triggers
            if (semanticMemoryLinker::matchAffirmationTrigger(pWorkStruct, pMemViewer, reqLinks, pGrdExp))
                break;

            if (&pOriginalGrdExp != &pGrdExp
                && answerToSpecificAssertions::process(pWorkStruct, pMemViewer, pOriginalGrdExp))
                break;
            answerToSpecificAssertions::process(pWorkStruct, pMemViewer, pGrdExp);
            break;
        }
        case SemanticOperatorEnum::GET: {
            // if it's an action label we replace it by his content
            const SemanticExpression* actionDefinitionPtr =
                semanticMemoryLinker::getActionComposition(pWorkStruct, pMemViewer, pGrdExp);
            if (actionDefinitionPtr != nullptr) {
                auto newAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                newAnsw->answerElts[SemanticRequestType::YESORNO].answersGenerated.emplace_back(
                    actionDefinitionPtr->clone(nullptr, true));
                pWorkStruct.compositeSemAnswers->semAnswers.emplace_back(std::move(newAnsw));
            }
            break;
        }
        case SemanticOperatorEnum::RESOLVECOMMAND: {
            answerToSpecificAssertions::process(pWorkStruct, pMemViewer, pGrdExp);
            break;
        }
        default: break;
    }
}

void _applyOpShowOnGrdExpWithoutVerb(SemControllerWorkingStruct& pWorkStruct,
                                     SemanticMemoryBlockViewer& pMemViewer,
                                     const GroundedExpression& pGrdExp) {
    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion()) {
        subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
        auto whatIsItQuest = SemExpCreator::askWhatIs(pGrdExp);
        converter::addDifferentForms(whatIsItQuest, pWorkStruct.lingDb);
        applyOperatorOnSemExp(subWorkStruct, pMemViewer, *whatIsItQuest);
        subWorkStruct.compositeSemAnswers->keepOnlyTheResourcesOrTexts();
        if (!subWorkStruct.compositeSemAnswers->semAnswers.empty())
            pWorkStruct.addAnswers(subWorkStruct);
    }
}

void applyOperatorOnGrdExp(SemControllerWorkingStruct& pWorkStruct,
                           SemanticMemoryBlockViewer& pMemViewer,
                           const GroundedExpression& pGrdExp,
                           const std::list<const GroundedExpression*>& pOtherGrdExps,
                           const GroundedExpression& pOriginalGrdExp) {
    switch (pGrdExp->type) {
        case SemanticGroundingType::STATEMENT: {
            const auto& statementGrd = pGrdExp->getStatementGrounding();

            // handle the requests
            if (statementGrd.requests.has(SemanticRequestType::ACTION)) {
                manageAction(pWorkStruct, pMemViewer, statementGrd, pGrdExp, pOriginalGrdExp);
                break;
            }
            if (!statementGrd.requests.empty()) {
                manageQuestion(pWorkStruct, pMemViewer, statementGrd, pGrdExp, pOtherGrdExps, pOriginalGrdExp);
                break;
            }

            // handle the assertion
            _manageAssertion(pWorkStruct, pMemViewer, pGrdExp, statementGrd, pOriginalGrdExp);
            break;
        }

        default:    // if it has no verb
        {
            // Grounding empty is a specific case for the not undertood children at root of a semantic expression.
            // In this case we do no reactions except the triggers matches.
            if (pGrdExp.grounding().isEmpty()) {
                if (pWorkStruct.reactionOptions.canAnswerWithATrigger
                    && (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                        || pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER)) {
                    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
                    subWorkStruct.reactOperator = SemanticOperatorEnum::REACTFROMTRIGGER;
                    for (const auto& currChild : pGrdExp.children)
                        applyOperatorOnSemExp(subWorkStruct, pMemViewer, *currChild.second);
                    pWorkStruct.addAnswers(subWorkStruct);
                } else if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACTFROMTRIGGER) {
                    for (const auto& currChild : pGrdExp.children)
                        applyOperatorOnSemExp(pWorkStruct, pMemViewer, *currChild.second);
                }

                if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                    || pWorkStruct.reactOperator == SemanticOperatorEnum::TEACHBEHAVIOR) {
                    auto* newContextAxiom = pWorkStruct.expHandleInMemory->tryToAddTeachFormulation(
                        pWorkStruct.informationType, pGrdExp, pWorkStruct.annotatedExps, pWorkStruct.lingDb, false);
                    // check if the sentence was about learning a command
                    if (newContextAxiom != nullptr && newContextAxiom->infCommandToDo != nullptr) {
                        pWorkStruct.addAnswerWithoutReferences(
                            ContextualAnnotation::TEACHINGFEEDBACK,
                            SemExpCreator::mergeInAList(SemExpCreator::confirmInformation(pGrdExp),
                                                        SemExpCreator::wrapWithStatementWithRequest(
                                                            std::make_unique<ListExpression>(ListExpressionType::THEN),
                                                            SemanticRequests(SemanticRequestType::YESORNO))));
                    }
                }
                break;
            }

            switch (pWorkStruct.reactOperator) {
                case SemanticOperatorEnum::REACT: {
                    semanticMemoryLinker::RequestLinks reqLinks;
                    semanticMemoryLinker::getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);
                    bool anAnswerHasBeenAdded = false;
                    if (pWorkStruct.reactionOptions.canAnswerWithATrigger
                        && semanticMemoryLinker::addTriggerSentencesAnswer(pWorkStruct,
                                                                           anAnswerHasBeenAdded,
                                                                           pMemViewer,
                                                                           reqLinks,
                                                                           SemanticExpressionCategory::NOMINALGROUP,
                                                                           _emptyAxiomId,
                                                                           pGrdExp,
                                                                           ContextualAnnotation::ANSWER))
                        break;

                    proactiveReactionFromNominalGroup::react(pWorkStruct, pMemViewer, pGrdExp);
                    break;
                }
                case SemanticOperatorEnum::REACTFROMTRIGGER: {
                    semanticMemoryLinker::RequestLinks reqLinks;
                    semanticMemoryLinker::getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);
                    bool anAnswerHasBeenAdded = false;
                    semanticMemoryLinker::addTriggerSentencesAnswer(pWorkStruct,
                                                                    anAnswerHasBeenAdded,
                                                                    pMemViewer,
                                                                    reqLinks,
                                                                    SemanticExpressionCategory::NOMINALGROUP,
                                                                    _emptyAxiomId,
                                                                    pGrdExp,
                                                                    ContextualAnnotation::ANSWER);
                    break;
                }
                case SemanticOperatorEnum::FEEDBACK: {
                    if (pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::SENTIMENT) {
                        semanticMemoryLinker::RequestLinks reqLinks;
                        semanticMemoryLinker::getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);
                        proactiveReactionFromNominalGroup::reactOnSentimentsFromNominalGroup(pWorkStruct, pGrdExp);
                    }
                    break;
                }
                case SemanticOperatorEnum::GET:
                case SemanticOperatorEnum::HOWYOUKNOW: {
                    const SemanticGenericGrounding* genGrdPtr = pGrdExp->getGenericGroundingPtr();
                    if (genGrdPtr != nullptr) {
                        if (genGrdPtr->referenceType == SemanticReferenceType::DEFINITE) {
                            auto whatIsQuestion = SemExpCreator::askWhatIs(pGrdExp);
                            converter::addDifferentForms(whatIsQuestion, pWorkStruct.lingDb);
                            applyOperatorOnSemExp(pWorkStruct, pMemViewer, *whatIsQuestion);
                        }
                        break;
                    }

                    const SemanticAgentGrounding* agentGrdPtr = pGrdExp->getAgentGroundingPtr();
                    if (agentGrdPtr != nullptr) {
                        auto whoIsQuestion = SemExpCreator::askWhoIs(pGrdExp);
                        converter::addDifferentForms(whoIsQuestion, pWorkStruct.lingDb);
                        applyOperatorOnSemExp(pWorkStruct, pMemViewer, *whoIsQuestion);
                    }
                    break;
                }
                case SemanticOperatorEnum::CHECK: {
                    semanticMemoryLinker::checkNominalGrdExp(pWorkStruct, pGrdExp);
                    break;
                }
                case SemanticOperatorEnum::SHOW: {
                    _applyOpShowOnGrdExpWithoutVerb(pWorkStruct, pMemViewer, pGrdExp);
                    break;
                }

                case SemanticOperatorEnum::ANSWER:
                case SemanticOperatorEnum::RESOLVECOMMAND:
                case SemanticOperatorEnum::EXECUTEBEHAVIOR:
                case SemanticOperatorEnum::EXECUTEFROMCONDITION:
                case SemanticOperatorEnum::FIND:
                case SemanticOperatorEnum::INFORM:
                case SemanticOperatorEnum::TEACHBEHAVIOR:
                case SemanticOperatorEnum::TEACHCONDITION:
                case SemanticOperatorEnum::TEACHINFORMATION:
                case SemanticOperatorEnum::UNINFORM: break;
            }
            break;
        }
    }
}

void compAnswerToSemExp(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                        CompositeSemAnswer& pCompositeSemAnswer) {
    for (auto& currDetAnsw : pCompositeSemAnswer.semAnswers) {
        auto currReaction = [&]() mutable {
            LeafSemAnswer* leafPtr = currDetAnsw->getLeafPtr();
            if (leafPtr != nullptr) {
                LeafSemAnswer& leafAnswer = *leafPtr;
                if (leafAnswer.reaction) {
                    auto metadataExp = std::make_unique<MetadataExpression>(std::move(*leafAnswer.reaction));
                    metadataExp->from = SemanticSourceEnum::SEMREACTION;
                    metadataExp->contextualAnnotation = leafAnswer.type;
                    metadataExp->interactionContextContainer = std::move(leafAnswer.interactionContextContainer);
                    return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(metadataExp));
                }
            } else {
                CompositeSemAnswer* compPtr = currDetAnsw->getCompositePtr();
                if (compPtr != nullptr) {
                    mystd::unique_propagate_const<UniqueSemanticExpression> subReaction;
                    compAnswerToSemExp(subReaction, *compPtr);
                    return subReaction;
                }
            }
            return mystd::unique_propagate_const<UniqueSemanticExpression>();
        }();

        if (currReaction) {
            if (!pReaction)
                pReaction = std::move(currReaction);
            else
                SemExpModifier::addListElt(*pReaction, pCompositeSemAnswer.listType, *currReaction);
        }
    }
}

ContextualAnnotation compAnswerToContextualAnnotation(CompositeSemAnswer& pCompositeSemAnswer) {
    auto res = ContextualAnnotation::ANSWERNOTFOUND;
    for (auto& currDetAnsw : pCompositeSemAnswer.semAnswers) {
        LeafSemAnswer* leafPtr = currDetAnsw->getLeafPtr();
        if (leafPtr != nullptr) {
            LeafSemAnswer& leafAnswer = *leafPtr;
            if (leafAnswer.reaction && leafAnswer.type < res)
                res = leafAnswer.type;
        }
    }
    return res;
}

TruenessValue operator_check_semExp(const SemanticExpression& pSemExp,
                                    const SemanticMemoryBlock& pConstMemBlock,
                                    const std::string& pCurrentUserId,
                                    const linguistics::LinguisticDatabase& pLingDb) {
    std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
    applyOperatorOnSemExpConstMem(compositeSemAnswers,
                                  pSemExp,
                                  SemanticOperatorEnum::CHECK,
                                  InformationType::INFORMATION,
                                  pConstMemBlock,
                                  pCurrentUserId,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  pLingDb);
    if (compositeSemAnswers)
        return compositeSemAnswers->getAgreementValue();
    return TruenessValue::UNKNOWN;
}

void linkConditionalReactions(std::list<std::unique_ptr<SemAnswer>>& pSemAnswers,
                              ExpressionWithLinks& pMemKnowledge,
                              SemanticMemory& pSemanticMemory,
                              const linguistics::LinguisticDatabase& pLingDb,
                              InformationType pInformationType) {
    for (auto& currAnswer : pSemAnswers) {
        LeafSemAnswer* leafPtr = currAnswer->getLeafPtr();
        if (leafPtr == nullptr)
            continue;
        LeafSemAnswer& leafAnswer = *leafPtr;

        if (leafAnswer.type == ContextualAnnotation::NOTIFYSOMETHINGWILLBEDONE) {
            if (leafAnswer.condition) {
                const ConditionResult& condRes = *leafAnswer.condition;
                if (condRes.thenCategory == SemanticExpressionCategory::COMMAND) {
                    if (SemExpGetter::isDoNothingSemExp(condRes.condition.thenExp)) {
                        const GroundedExpression* conditionGrdExpPtr =
                            condRes.condition.conditionExp.getGrdExpPtr_SkipWrapperPtrs();
                        if (conditionGrdExpPtr != nullptr)
                            semanticMemoryLinker::disableActionsLinkedToASentence(
                                pSemanticMemory, *conditionGrdExpPtr, pLingDb);
                        continue;
                    }
                    pMemKnowledge.addConditionToAnAction(pInformationType, condRes.condition, pLingDb);
                    continue;
                } else if (condRes.thenCategory == SemanticExpressionCategory::AFFIRMATION) {
                    pMemKnowledge.addConditionToAnInfo(pInformationType, condRes.condition, pLingDb);
                    continue;
                }
            }

            if (leafAnswer.conditionForAUser.thenSemExp && !leafAnswer.conditionForAUser.user.empty()) {
                pSemanticMemory.addNewUserFocusedToSemExp(std::move(leafAnswer.conditionForAUser.thenSemExp),
                                                          leafAnswer.conditionForAUser.user);
                continue;
            }
        } else if (leafAnswer.type == ContextualAnnotation::REMOVEALLCONDITIONS) {
            pSemanticMemory.memBloc.removeLinkedActions();
        }
    }
}

void uninform(const SentenceWithLinks& pContextAxiom,
              SemanticMemoryBlock& pMemBlock,
              const linguistics::LinguisticDatabase& pLingDb,
              std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr) {
    SemControllerWorkingStruct workStruct(pContextAxiom.informationType,
                                          nullptr,
                                          SemanticLanguageEnum::UNKNOWN,
                                          nullptr,
                                          SemanticOperatorEnum::UNINFORM,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          pAxiomToConditionCurrentStatePtr,
                                          pLingDb);
    SemanticMemoryBlockViewer memViewer(&pMemBlock, pMemBlock, SemanticAgentGrounding::userNotIdentified);
    for (const GroundedExpWithLinks& currMemSent : pContextAxiom.memorySentences.elts)
        if (!currMemSent.isANoun() && !currMemSent.isAConditionToSatisfy())
            _updateConditionValidity(currMemSent, workStruct, memViewer);
    if (workStruct.compositeSemAnswers)
        controller::sendActionProposalIfNecessary(*workStruct.compositeSemAnswers, pMemBlock);
}

void sendActionProposalIfNecessary(CompositeSemAnswer& pCompSemAnswer, SemanticMemoryBlock& pMemBlock) {
    if (!pCompSemAnswer.semAnswers.empty()) {
        // remove detailed answer that correspond to a feedback
        for (auto itDetAnsw = pCompSemAnswer.semAnswers.begin(); itDetAnsw != pCompSemAnswer.semAnswers.end();) {
            LeafSemAnswer* leafPtr = (*itDetAnsw)->getLeafPtr();
            if (leafPtr != nullptr) {
                if (leafPtr->type == ContextualAnnotation::NOTIFYSOMETHINGWILLBEDONE
                    || leafPtr->type == ContextualAnnotation::BEHAVIORNOTFOUND)
                    itDetAnsw = pCompSemAnswer.semAnswers.erase(itDetAnsw);
                else
                    ++itDetAnsw;
            }
        }

        mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
        controller::compAnswerToSemExp(reaction, pCompSemAnswer);
        if (reaction)
            pMemBlock.actionProposalSignal(*reaction);
    }
}

void convertToDetalledAnswer(std::list<std::unique_ptr<SemAnswer>>& pDetailledAnswers,
                             SemControllerWorkingStruct& pWorkStruct) {
    if (!pWorkStruct.compositeSemAnswers->semAnswers.empty())
        pDetailledAnswers.splice(pDetailledAnswers.end(), pWorkStruct.compositeSemAnswers->semAnswers);
}

void notifyCurrentTime(std::unique_ptr<CompositeSemAnswer>& pCompositeSemAnswers,
                       SemanticMemory& pSemanticMemory,
                       std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
                       const SemanticDuration& pNowTimeDuration,
                       const linguistics::LinguisticDatabase& pLingDb) {
    SemControllerWorkingStruct workStruct(InformationType::ASSERTION,
                                          nullptr,
                                          SemanticLanguageEnum::UNKNOWN,
                                          nullptr,
                                          SemanticOperatorEnum::INFORM,
                                          &pSemanticMemory.proativeSpecifications,
                                          pSemanticMemory.getExternalFallback(),
                                          &pSemanticMemory.callbackToSentencesCanBeAnswered,
                                          pAxiomToConditionCurrentStatePtr,
                                          pLingDb);
    SemanticMemoryBlockViewer memViewer(
        &pSemanticMemory.memBloc, pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId());

    semanticMemoryLinker::getNowConditions(workStruct, memViewer, pNowTimeDuration, workStruct.lingDb);

    // TODO: factorize this if
    SemControllerWorkingStruct subWorkStruct(workStruct);
    if (subWorkStruct.askForNewRecursion()) {
        subWorkStruct.reactOperator = SemanticOperatorEnum::INFORM;
        if (subWorkStruct.haveAnAnswer())
            workStruct.addAnswers(subWorkStruct);
    }

    pCompositeSemAnswers = std::move(workStruct.compositeSemAnswers);
}

}    // End of namespace controller
}    // End of namespace onsem
