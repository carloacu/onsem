#include "proactivereactionfromnominalgroups.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticconceptualgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include <onsem/semantictotext/tool/semexpagreementdetector.hpp>
#include "type/semcontrollerworkingstruct.hpp"
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../utility/semexpcreator.hpp"
#include "../semexpcontroller.hpp"

namespace onsem {
namespace proactiveReactionFromNominalGroup {

namespace {

enum class GroundingEntity { THING, AGENT, UNKNOWN };

GroundingEntity _extractGroundingEntity(const GroundedExpression& pGrdExp) {
    const SemanticGrounding& grd = pGrdExp.grounding();
    const SemanticGenericGrounding* genGrdPtr = grd.getGenericGroundingPtr();
    if (genGrdPtr != nullptr) {
        if (!genGrdPtr->coreference && genGrdPtr->word.partOfSpeech != PartOfSpeech::ADVERB
            && genGrdPtr->word.partOfSpeech != PartOfSpeech::INTERJECTION) {
            if (genGrdPtr->entityType == SemanticEntityType::AGENTORTHING
                || genGrdPtr->entityType == SemanticEntityType::HUMAN
                || genGrdPtr->entityType == SemanticEntityType::ROBOT)
                return GroundingEntity::AGENT;
            if (genGrdPtr->entityType == SemanticEntityType::THING)
                return GroundingEntity::THING;
        }
    } else {
        if (grd.getNameGroundingPtr() != nullptr || grd.getAgentGroundingPtr() != nullptr)
            return GroundingEntity::AGENT;
    }
    return GroundingEntity::UNKNOWN;
}

void _answerTheQuestionOrAskIt(SemControllerWorkingStruct& pWorkStruct,
                               SemanticMemoryBlockViewer& pMemViewer,
                               UniqueSemanticExpression pQuestionSemExp) {
    bool reactOnNominalGroup = false;
    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion()) {
        subWorkStruct.reactOperator = SemanticOperatorEnum::ANSWER;
        subWorkStruct.reactionOptions.canAnswerIDontKnow = false;
        converter::addDifferentForms(pQuestionSemExp, pWorkStruct.lingDb);
        controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *pQuestionSemExp);
        if (!subWorkStruct.compositeSemAnswers->semAnswers.empty()) {
            if (pWorkStruct.isAtRoot)
                pWorkStruct.addAnswers(subWorkStruct);
            reactOnNominalGroup = true;
        }
    }
    if (!reactOnNominalGroup && pWorkStruct.reactionOptions.canDoAProactivity && !pWorkStruct.haveAnAnswer()) {
        converter::unsplitPossibilitiesOfQuestions(pQuestionSemExp);
        pWorkStruct.addQuestion(std::move(pQuestionSemExp));
    }
}

}

bool reactOnSentimentsFromNominalGroup(SemControllerWorkingStruct& pWorkStruct, const GroundedExpression& pGrdExp) {
    if (pWorkStruct.author == nullptr)
        return false;
    auto sentimentSpec =
        sentimentDetector::extractMainSentiment(pGrdExp, *pWorkStruct.author, pWorkStruct.lingDb.conceptSet);
    if (sentimentSpec
        && sentimentSpec->sentimentStrengh > 1)    // 1 and not 0 because we don't want to react on too weak sentiments
    {
        if (SemExpGetter::isACoreference(*sentimentSpec->receiver, CoreferenceDirectionEnum::BEFORE, true)) {
            if (sentimentSpec->sentiment == "sentiment_positive_joy") {
                pWorkStruct.addAnswerWithoutReferences(
                    ContextualAnnotation::FEEDBACK,
                    SemExpCreator::niceYouLikeIt(SemExpGetter::returnAPositiveSemExpBasedOnAnInput(pGrdExp),
                                                 *pWorkStruct.author));
                return true;
            }
            if (sentimentSpec->sentiment == "sentiment_positive_thanks") {
                pWorkStruct.addAnswerWithoutReferences(
                    ContextualAnnotation::FEEDBACK,
                    std::make_unique<GroundedExpression>(
                        std::make_unique<SemanticConceptualGrounding>("youAreWelcome")));
                return true;
            }
            if (sentimentSpec->sentiment == "sentiment_negative_*") {
                pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::FEEDBACK,
                                                       SemExpCreator::sorryIWillTryToImproveMyself());
                return true;
            }
        }
    }
    return false;
}

bool react(SemControllerWorkingStruct& pWorkStruct,
           SemanticMemoryBlockViewer& pMemViewer,
           const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactionOptions.canDoAProactivity) {
        if (pWorkStruct.authorSemExp != nullptr) {
            TruenessValue agrementRes = semExpAgreementDetector::getAgreementValue(pGrdExp);
            if (agrementRes != TruenessValue::UNKNOWN) {
                const SemanticExpression* semExpPtr = pMemViewer.constView.getBeforeLastSemExpOfNotAnAuthor(
                    *pWorkStruct.authorSemExp, pWorkStruct.lingDb);
                if (semExpPtr != nullptr) {
                    for (auto& currElt : SemExpGetter::iterateOnListOfGrdExps(*semExpPtr)) {
                        if (SemExpGetter::getMainRequestTypeFromGrdExp(*currElt) == SemanticRequestType::YESORNO) {
                            pWorkStruct.addQuestion(currElt->clone());
                            return true;
                        }
                    }
                }
            }
        }

        if (reactOnSentimentsFromNominalGroup(pWorkStruct, pGrdExp))
            return true;

        const SemanticAgentGrounding* agentGrdPtr = pGrdExp->getAgentGroundingPtr();
        if (agentGrdPtr != nullptr && pMemViewer.constView.isItMe(agentGrdPtr->userId)) {
            pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::FEEDBACK, SemExpCreator::itsMe());
            return true;
        }
    }

    if (pWorkStruct.reactionOptions.canReactToANoun) {
        const SemanticExpression* nominalSemExp = pWorkStruct.originalSemExpPtr;
        if (nominalSemExp == nullptr)
            nominalSemExp = &pGrdExp;
        GroundingEntity grdEntity = _extractGroundingEntity(pGrdExp);
        switch (grdEntity) {
            case GroundingEntity::THING: {
                _answerTheQuestionOrAskIt(pWorkStruct, pMemViewer, SemExpCreator::askWhatIs(*nominalSemExp));
                return true;
            }
            case GroundingEntity::AGENT: {
                _answerTheQuestionOrAskIt(pWorkStruct, pMemViewer, SemExpCreator::askWhoIs(*nominalSemExp));
                return true;
            }
            case GroundingEntity::UNKNOWN: break;
        }
    }
    return false;
}

}    // End of namespace proactiveReactionFromNominalGroup
}    // End of namespace onsem
