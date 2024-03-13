#include "managechoice.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/feedbackexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticconceptualgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpsimplifer.hpp>
#include "../semexpcontroller.hpp"
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../type/referencesfiller.hpp"
#include "../../utility/semexpcreator.hpp"

namespace onsem {

namespace {

void _splitChoice(std::list<UniqueSemanticExpression>& pSemExps,
                  const GroundedExpression& pGrdExp,
                  GrammaticalType pGramTypeOfTheChoice,
                  const ListExpression& pChoiceListExp) {
    for (const auto& currListElt : pChoiceListExp.elts) {
        auto grdExpPtr = SemExpGetter::getCopyExceptChild(pGrdExp, pGramTypeOfTheChoice);
        grdExpPtr->children.emplace(pGramTypeOfTheChoice, currListElt->clone());
        pSemExps.emplace_back(std::move(grdExpPtr));
    }
}

bool _areAllPolaritiesFalse(const std::list<TruenessValue>& pAnswersPolarities) {
    for (const auto& currElt : pAnswersPolarities)
        if (currElt != TruenessValue::VAL_FALSE)
            return false;
    return true;
}

bool _areAllPolaritiesTrue(const std::list<TruenessValue>& pAnswersPolarities) {
    for (const auto& currElt : pAnswersPolarities)
        if (currElt != TruenessValue::VAL_TRUE)
            return false;
    return true;
}

bool _isOnePolarityTrue(const std::list<TruenessValue>& pAnswersPolarities) {
    for (const auto& currElt : pAnswersPolarities)
        if (currElt == TruenessValue::VAL_TRUE)
            return true;
    return false;
}

UniqueSemanticExpression _neitherSemExp(std::size_t pNbOfElts) {
    return std::make_unique<GroundedExpression>(std::make_unique<SemanticConceptualGrounding>(
        pNbOfElts == 2 ? "accordance_disagreement_neither" : "accordance_disagreement_noneOfThem"));
}

UniqueSemanticExpression _bothSemExp(std::size_t pNbOfElts) {
    return std::make_unique<GroundedExpression>(std::make_unique<SemanticConceptualGrounding>(
        pNbOfElts == 2 ? "accordance_agreement_both" : "accordance_agreement_allOfThem"));
}

void _addNeitherInterjection(SemAnswer& pSemAnswer, std::size_t pNbOfElts) {
    LeafSemAnswer* leafPtr = pSemAnswer.getLeafPtr();
    if (leafPtr == nullptr)
        return;
    auto& leafAnsw = *leafPtr;

    if (leafAnsw.reaction)
        leafAnsw.reaction.emplace(
            std::make_unique<FeedbackExpression>(_neitherSemExp(pNbOfElts), std::move(*leafAnsw.reaction)));
}

}

bool manageChoice(SemControllerWorkingStruct& pWorkStruct,
                  SemanticMemoryBlockViewer& pMemViewer,
                  const GroundedExpression& pGrdExp,
                  GrammaticalType pGramTypeOfTheChoice,
                  const ListExpression& pChoiceListExp) {
    std::list<UniqueSemanticExpression> splittedQuestions;
    _splitChoice(splittedQuestions, pGrdExp, pGramTypeOfTheChoice, pChoiceListExp);

    UniqueSemanticExpression res;
    UniqueSemanticExpression falseAnwers;
    std::list<UniqueSemanticExpression> answerElts;
    std::list<TruenessValue> answersPolarities;
    RelatedContextAxiom relatedContextAxiom;
    for (auto& currChoiceQuestion : splittedQuestions) {
        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
        if (subWorkStruct.askForNewRecursion()) {
            subWorkStruct.reactOperator = SemanticOperatorEnum::CHECK;

            SemExpModifier::setRequest(*currChoiceQuestion, SemanticRequestType::YESORNO);
            converter::addDifferentForms(currChoiceQuestion, pWorkStruct.lingDb);
            controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *currChoiceQuestion);

            if (subWorkStruct.compositeSemAnswers) {
                subWorkStruct.compositeSemAnswers->getSourceContextAxiom(relatedContextAxiom);
                TruenessValue agreementValue = subWorkStruct.compositeSemAnswers->getAgreementValue();
                switch (pWorkStruct.reactOperator) {
                    case SemanticOperatorEnum::REACT:
                    case SemanticOperatorEnum::ANSWER: {
                        answersPolarities.emplace_back(agreementValue);
                        if (agreementValue != TruenessValue::UNKNOWN) {
                            SemExpModifier::clearRequestListOfSemExp(*currChoiceQuestion);
                            if (agreementValue == TruenessValue::VAL_TRUE)
                                SemExpModifier::addASemExp(res, std::move(currChoiceQuestion));
                            else if (agreementValue == TruenessValue::VAL_FALSE) {
                                SemExpModifier::invertPolarity(*currChoiceQuestion);
                                SemExpModifier::addASemExp(falseAnwers, std::move(currChoiceQuestion));
                            }
                        }
                        break;
                    }
                    case SemanticOperatorEnum::GET: {
                        if (agreementValue == TruenessValue::VAL_TRUE) {
                            const SemanticExpression* currAnswerEltPtr =
                                SemExpGetter::getChildFromSemExp(*currChoiceQuestion, pGramTypeOfTheChoice);
                            if (currAnswerEltPtr != nullptr)
                                answerElts.emplace_back(currAnswerEltPtr->clone());
                        }
                        break;
                    }
                    case SemanticOperatorEnum::CHECK: {
                        answersPolarities.emplace_back(agreementValue);
                        break;
                    }
                    default: break;
                }
            }
        }
    }

    switch (pWorkStruct.reactOperator) {
        case SemanticOperatorEnum::REACT:
        case SemanticOperatorEnum::ANSWER: {
            std::size_t nbOfElts = answersPolarities.size();
            if (nbOfElts == 0)
                break;

            if (!res->isEmpty()) {
                simplifier::processFromMemBlock(res, pMemViewer.constView, pWorkStruct.lingDb, true);
                if (_areAllPolaritiesTrue(answersPolarities))
                    res = std::make_unique<FeedbackExpression>(_bothSemExp(nbOfElts), std::move(res));
                pWorkStruct.addAnswer(
                    ContextualAnnotation::ANSWER, std::move(res), ReferencesFiller(relatedContextAxiom));
                return true;
            }

            if (_areAllPolaritiesFalse(answersPolarities)) {
                SemControllerWorkingStruct subWorkStruct(pWorkStruct);
                if (subWorkStruct.askForNewRecursion()) {
                    subWorkStruct.reactOperator = SemanticOperatorEnum::ANSWER;
                    subWorkStruct.reactionOptions.canAnswerIDontKnow = false;

                    UniqueSemanticExpression objectQuestionSemExp =
                        SemExpGetter::getCopyExceptChild(pGrdExp, pGramTypeOfTheChoice);
                    converter::addDifferentForms(objectQuestionSemExp, pWorkStruct.lingDb);
                    controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *objectQuestionSemExp);
                    if (subWorkStruct.compositeSemAnswers && !subWorkStruct.compositeSemAnswers->semAnswers.empty()) {
                        for (auto& currSemEnswer : subWorkStruct.compositeSemAnswers->semAnswers)
                            _addNeitherInterjection(*currSemEnswer, nbOfElts);
                        pWorkStruct.addAnswers(subWorkStruct);
                        return true;
                    }
                    pWorkStruct.addAnswer(
                        ContextualAnnotation::ANSWER, _neitherSemExp(nbOfElts), ReferencesFiller(relatedContextAxiom));
                    return true;
                }
            }

            if (!falseAnwers->isEmpty()) {
                simplifier::processFromMemBlock(falseAnwers, pMemViewer.constView, pWorkStruct.lingDb, true);
                pWorkStruct.addAnswer(
                    ContextualAnnotation::ANSWER, std::move(falseAnwers), ReferencesFiller(relatedContextAxiom));
                return true;
            }
            break;
        }
        case SemanticOperatorEnum::GET: {
            if (!answerElts.empty()) {
                auto newAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                AllAnswerElts& allAnswElts = newAnsw->answerElts[SemanticRequestType::YESORNO];
                for (auto& currAnswerElt : answerElts)
                    allAnswElts.answersGenerated.emplace_back(std::move(currAnswerElt));
                pWorkStruct.compositeSemAnswers->semAnswers.emplace_back(std::move(newAnsw));
                return true;
            }
            break;
        }
        case SemanticOperatorEnum::CHECK: {
            bool isTrue = _isOnePolarityTrue(answersPolarities);
            if (isTrue || _areAllPolaritiesFalse(answersPolarities)) {
                auto newAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                AllAnswerElts& allAnswElts = newAnsw->answerElts[SemanticRequestType::YESORNO];
                allAnswElts.answersGenerated.emplace_back(SemExpCreator::sayYesOrNo(isTrue));
                pWorkStruct.compositeSemAnswers->semAnswers.emplace_back(std::move(newAnsw));
                return true;
            }
            break;
        }
        default: break;
    }

    return false;
}

}    // End of namespace onsem
