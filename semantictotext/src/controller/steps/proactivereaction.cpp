#include "proactivereaction.hpp"
#include <onsem/common/enum/timeweekdayenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/feedbackexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/tool/semexpagreementdetector.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticcontextaxiom.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../type/referencesfiller.hpp"
#include "../../type/semanticdetailledanswer.hpp"
#include "../../utility/semexpcreator.hpp"
#include "../semexpcontroller.hpp"
#include "similaritieswithmemoryfinder.hpp"
#include "unknowninfosgetter.hpp"


namespace onsem
{
namespace proactiveReaction
{

namespace
{

bool _isTheReverseTrue(std::unique_ptr<GroundedExpression>& pReverseGrdExp,
                       RelatedContextAxiom& pRelatedContextAxioms,
                       const SemControllerWorkingStruct& pWorkStruct,
                       const SemanticMemoryBlockViewer& pMemViewer,
                       const GroundedExpression& pGrdExp)
{
  if (pGrdExp.children.count(GrammaticalType::SUBJECT) == 0 ||
      pGrdExp.children.count(GrammaticalType::OBJECT) == 0)
    return false;

  pReverseGrdExp = pGrdExp.clone();
  pReverseGrdExp->children.find(GrammaticalType::SUBJECT)->second.swap
      (pReverseGrdExp->children.find(GrammaticalType::OBJECT)->second);

  SemControllerWorkingStruct subWorkStruct(pWorkStruct);
  if (subWorkStruct.askForNewRecursion())
  {
    subWorkStruct.reactOperator = SemanticOperatorEnum::CHECK;
    SemanticMemoryBlockViewer subMemView(pMemViewer);
    controller::applyOperatorOnGrdExp(subWorkStruct, subMemView, *pReverseGrdExp,
                                      std::list<const GroundedExpression*>(),
                                      *pReverseGrdExp);
    subWorkStruct.getSourceContextAxiom(pRelatedContextAxioms);
    return subWorkStruct.agreementTypeOfTheAnswer() == TruenessValue::VAL_TRUE;
  }
  return false;
}


bool _sayDayOfBirthdate(SemControllerWorkingStruct& pWorkStruct,
                        const GroundedExpression& pGrdExp,
                        const std::string& pCurrentUserId)
{
  auto itTime = pGrdExp.children.find(GrammaticalType::TIME);
  if (itTime == pGrdExp.children.end())
  {
    return false;
  }

  auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
  if (itObject == pGrdExp.children.end())
  {
    return false;
  }
  const GroundedExpression* grdExpObject = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpObject != nullptr)
  {
    const SemanticAgentGrounding* agentObject = (*grdExpObject)->getAgentGroundingPtr();
    if (agentObject != nullptr &&
        agentObject->userId == pCurrentUserId)
    {
      const GroundedExpression* grdExpTime = itTime->second->getGrdExpPtr_SkipWrapperPtrs();
      if (grdExpTime != nullptr)
      {
        const SemanticTimeGrounding* timeGrd = (*grdExpTime)->getTimeGroundingPtr();
        if (timeGrd != nullptr)
        {
          const auto& cpts = pGrdExp.grounding().concepts;
          if (cpts.find("verb_born") != cpts.end())
          {
            TimeWeekdayEnum weekDay = timeGrd->date.getWeekDay();
            if (weekDay != TimeWeekdayEnum::UNKNOWN)
            {
              pWorkStruct.addAnswerWithoutReferences
                  (ContextualAnnotation::FEEDBACK,
                   SemExpCreator::formulateWeekDay(semanticTimeWeekdayEnum_toStr(weekDay)));
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}


bool _isChildOfSemExpIsUnknown(const GroundedExpression& pGrdExp,
                               GrammaticalType pChildType)
{
  auto itChild = pGrdExp.children.find(pChildType);
  if (itChild != pGrdExp.children.end())
  {
    const GroundedExpression* grdExpChild = itChild->second->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpChild != nullptr)
    {
      const SemanticGenericGrounding* genGrdChild = (*grdExpChild)->getGenericGroundingPtr();
      return genGrdChild != nullptr &&
          genGrdChild->entityType == SemanticEntityType::THING &&
          !genGrdChild->word.lemma.empty() &&
          genGrdChild->word.partOfSpeech == PartOfSpeech::UNKNOWN;
    }
  }
  return false;
}

bool _sayItsNotFalseFromFrenchSentenceWhenObjectIsUnknown(SemControllerWorkingStruct& pWorkStruct,
                                                          const GroundedExpression& pGrdExp)
{
  if (pWorkStruct.fromLanguage == SemanticLanguageEnum::FRENCH &&
      (_isChildOfSemExpIsUnknown(pGrdExp, GrammaticalType::OBJECT) ||
       _isChildOfSemExpIsUnknown(pGrdExp, GrammaticalType::SUBJECT)))
  {
    pWorkStruct.addAnswerWithoutReferences
        (ContextualAnnotation::FEEDBACK,
         mystd::make_unique<GroundedExpression>
          (mystd::make_unique<SemanticTextGrounding>("C'est pas faux.")));
    return true;
  }
  return false;
}


bool _sayOkIfTheUserIsTalkingAboutHim(SemControllerWorkingStruct& pWorkStruct,
                                      const GroundedExpression& pGrdExp)
{
  // keep consider only sentence where the user is the subject
  if (pWorkStruct.author != nullptr &&
      SemExpGetter::agentIsTheSubject(pGrdExp, pWorkStruct.author->userId))
  {
    // don't consider some verbs
    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr ||
        ConceptSet::haveAConcept(statGrdPtr->concepts, "mentalState_know"))
      return false;

    pWorkStruct.addAnswerWithoutReferences
        (ContextualAnnotation::FEEDBACK,
         mystd::make_unique<FeedbackExpression>
         (SemExpCreator::sayOk(), pGrdExp.clone()));
    return true;
  }
  return false;
}



std::unique_ptr<SemanticExpression> _answerNiceToMeetYouIfTheUserSaysHisName(SemControllerWorkingStruct& pWorkStruct,
                                                                             SemanticContextAxiom* pNewContextAxiom,
                                                                             const SemanticMemoryBlock& pMemBlock,
                                                                             const GroundedExpression& pGrdExp)
{
  if (pWorkStruct.author != nullptr &&
      pNewContextAxiom != nullptr)
  {
    const auto& authorUserId = pWorkStruct.author->userId;
    if (!pNewContextAxiom->memorySentences.hasEquivalentUserIds(authorUserId))
      return {};
    std::string name = pNewContextAxiom->memorySentences.getName(authorUserId);
    if (name.empty())
      return {};
    return mystd::make_unique<FeedbackExpression>(mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticConceptualGrounding>("niceToMeetYou")),
                                                  mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticTextGrounding>(name)));
  }
  return {};
}


bool _reactOnSentiments(SemControllerWorkingStruct& pWorkStruct,
                        const SemanticMemoryBlockViewer& pMemViewer,
                        const GroundedExpression& pGrdExp)
{
  if (pWorkStruct.author == nullptr)
    return false;
  bool reactOnSentiment = pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
      (pWorkStruct.reactOperator == SemanticOperatorEnum::FEEDBACK && pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::SENTIMENT);
  auto sentimentSpec = sentimentDetector::extractMainSentiment
      (pGrdExp, *pWorkStruct.author, pWorkStruct.lingDb.conceptSet);
  if (sentimentSpec && sentimentSpec->sentimentStrengh > 1 && // 1 and not 0 because we don't want to react on too weak sentiments
      !ConceptSet::doesConceptBeginWith(sentimentSpec->sentiment, "sentiment_neutral_"))
  {
    if (SemExpGetter::doSemExpHoldUserId(*sentimentSpec->author, pWorkStruct.author->userId))
    {
      if (SemExpGetter::doSemExpHoldUserId(*sentimentSpec->receiver, SemanticAgentGrounding::me))
      {
        if (reactOnSentiment)
        {
          if (ConceptSet::doesConceptBeginWith(sentimentSpec->sentiment, "sentiment_positive_"))
          {
            std::unique_ptr<GroundedExpression> reverseGrdExp;
            RelatedContextAxiom relatedContextAxioms;
            if (_isTheReverseTrue(reverseGrdExp, relatedContextAxioms, pWorkStruct, pMemViewer, pGrdExp))
            {
              SemExpModifier::addChild(*reverseGrdExp, GrammaticalType::SPECIFIER,
                                       SemExpCreator::sayAlso(), ListExpressionType::UNRELATED, false, false);
              pWorkStruct.addAnswer
                  (ContextualAnnotation::FEEDBACK,
                   mystd::make_unique<FeedbackExpression>(SemExpCreator::sayThanks(),
                                                          std::move(reverseGrdExp)),
                   ReferencesFiller(relatedContextAxioms));
            }
            else
            {
              pWorkStruct.addAnswerWithoutReferences
                  (ContextualAnnotation::FEEDBACK,
                   SemExpCreator::sayThanksThatsCool(*pWorkStruct.author));
            }
            return true;
          }
          if (ConceptSet::doesConceptBeginWith(sentimentSpec->sentiment, "sentiment_negative_"))
          {
            pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::FEEDBACK, SemExpCreator::itIsNotKind());
            return true;
          }
        }
      }
      else if (SemExpGetter::isACoreference(*sentimentSpec->receiver, CoreferenceDirectionEnum::BEFORE, true))
      {
        if (reactOnSentiment)
        {
          if (sentimentSpec->sentiment == "sentiment_positive_joy")
          {
            auto interjectionSemExp = [&]() -> UniqueSemanticExpression
            {
              auto itObjectSemExp = pGrdExp.children.find(GrammaticalType::OBJECT);
              if (itObjectSemExp != pGrdExp.children.end())
              {
                const auto& obectGrdExpPtr = itObjectSemExp->second->getGrdExpPtr_SkipWrapperPtrs();
                if (obectGrdExpPtr != nullptr)
                  return SemExpGetter::returnAPositiveSemExpBasedOnAnInput(*obectGrdExpPtr);
              }
              return mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticConceptualGrounding>("sentiment_positive_joy"));
            }();

            pWorkStruct.addAnswerWithoutReferences
                (ContextualAnnotation::FEEDBACK,
                 SemExpCreator::niceYouLikeIt(std::move(interjectionSemExp), *pWorkStruct.author));
            return true;
          }
          if (sentimentSpec->sentiment == "sentiment_negative_*")
          {
            pWorkStruct.addAnswerWithoutReferences
                (ContextualAnnotation::FEEDBACK, SemExpCreator::sorryIWillTryToImproveMyself());
            return true;
          }
        }
      }
      if (pGrdExp.children.find(GrammaticalType::CAUSE) == pGrdExp.children.end())
      {
        bool res = false;
        if (SemExpGetter::doSemExpHoldUserId(*sentimentSpec->receiver, pWorkStruct.author->userId))
        {
          if (ConceptSet::doesConceptBeginWith(sentimentSpec->sentiment, "sentiment_positive_"))
          {
            if (reactOnSentiment)
            {
              pWorkStruct.addAnswerWithoutReferences
                  (ContextualAnnotation::FEEDBACK, SemExpCreator::sayIAmHappyToHearThat());
              res = true;
            }
          }
          else if (ConceptSet::doesConceptBeginWith(sentimentSpec->sentiment, "sentiment_negative_"))
          {
            if (reactOnSentiment)
            {
              if (pWorkStruct.fromLanguage == SemanticLanguageEnum::ENGLISH)
                pWorkStruct.addAnswerWithoutReferences
                    (ContextualAnnotation::FEEDBACK, SemExpCreator::iAmSorryToHearThat());
              else
                pWorkStruct.addAnswerWithoutReferences
                    (ContextualAnnotation::FEEDBACK, SemExpCreator::itIsABadNews());
              res = true;
            }
          }
        }
        auto askWyGrdExp = pGrdExp.clone();
        SemExpModifier::addRequest(*askWyGrdExp, SemanticRequestType::CAUSE);
        if (unknownInfosGetter::isItfUnknown(pWorkStruct, pMemViewer, *askWyGrdExp))
        {
          pWorkStruct.addAnswerWithoutReferences
              (ContextualAnnotation::QUESTION, std::move(askWyGrdExp));
          return true;
        }
        return res;
      }
    }
  }
  return false;
}


bool _askAboutLocationOrTimeOfAPastActions(SemControllerWorkingStruct& pWorkStruct,
                                           const SemanticMemoryBlockViewer& pMemViewer,
                                           const GroundedExpression& pGrdExp)
{
  const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr &&
      (isAPastTense(statGrdPtr->verbTense) || statGrdPtr->verbTense == SemanticVerbTense::FUTURE))
  {
    // don't consider sentence where the robot is the subject
    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject != pGrdExp.children.end())
    {
      const GroundedExpression* subjectGrdExpPtr = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
      if (subjectGrdExpPtr != nullptr)
      {
        const SemanticAgentGrounding* agentGrdPtr = subjectGrdExpPtr->grounding().getAgentGroundingPtr();
        if (agentGrdPtr != nullptr &&
            agentGrdPtr->userId == SemanticAgentGrounding::me)
          return false;
      }
    }

    std::list<SemanticRequestType> requestToAsk;

    if (pGrdExp.children.count(GrammaticalType::TIME) == 0)
      requestToAsk.emplace_back(SemanticRequestType::TIME);
    if (pGrdExp.children.count(GrammaticalType::LOCATION) == 0)
      requestToAsk.emplace_back(SemanticRequestType::LOCATION);

    if (!requestToAsk.empty())
    {
      auto questionAboutTheLocation = pGrdExp.clone();
      for (const auto& currRequestToAsk : requestToAsk)
      {
        SemExpModifier::setRequest(*questionAboutTheLocation, currRequestToAsk);
        if (unknownInfosGetter::isItfUnknown(pWorkStruct, pMemViewer, *questionAboutTheLocation))
        {
          pWorkStruct.addQuestion(std::move(questionAboutTheLocation));
          return true;
        }
      }
    }
  }
  return false;
}

void _confirmOrCorrect(SemControllerWorkingStruct& pWorkStruct,
                       const GroundedExpression& pGrdExp,
                       TruenessValue pTruenessValue,
                       const RelatedContextAxiom& pAnswersContextAxioms)
{
  auto res = SemExpCreator::sayThatTheAssertionIsTrueOrFalse
      (pGrdExp, pTruenessValue == TruenessValue::VAL_TRUE);
  pWorkStruct.addAnswer(ContextualAnnotation::FEEDBACK, std::move(res),
                        ReferencesFiller(pAnswersContextAxioms));
}

}


bool process(bool& pResThatCanHaveAdditionalFeedbacks,
             SemControllerWorkingStruct& pWorkStruct,
             SemanticContextAxiom* pNewContextAxiom,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp,
             TruenessValue pTruenessValue,
             bool pAnswerIsAnAssertion,
             const RelatedContextAxiom& pAnswersContextAxioms)
{
  if ((pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
       pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::ASK_FOR_ADDITIONAL_INFORMATION ||
       pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::SENTIMENT) &&
      _reactOnSentiments(pWorkStruct, pMemViewer, pGrdExp))
    return true;

  if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT)
  {
    auto niceToMeetTouExp = _answerNiceToMeetYouIfTheUserSaysHisName(pWorkStruct, pNewContextAxiom, pMemViewer.constView, pGrdExp);
    if (niceToMeetTouExp)
    {
      pWorkStruct.addAnswerWithoutReferences
          (ContextualAnnotation::FEEDBACK, std::move(niceToMeetTouExp));
      return true;
    }
  }

  if (pTruenessValue != TruenessValue::UNKNOWN && !pWorkStruct.haveAnAnswer())
  {
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
        pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::REACT_ON_SIMILARITIES)
    {
      // Don't react if the answer come from our own semantic expression
      if (pWorkStruct.expHandleInMemory != nullptr &&
          pAnswersContextAxioms.haveThisExpHandleInMemory(pWorkStruct.expHandleInMemory))
        return false;

      if (pAnswerIsAnAssertion)
      {
        _confirmOrCorrect(pWorkStruct, pGrdExp, pTruenessValue, pAnswersContextAxioms);
        return true;
      }
      if (pTruenessValue == TruenessValue::VAL_FALSE)
      {
        auto reactionOfOppositeInformation = [&](const SemanticContextAxiom& pCurrContextAxiom)
        {
          ImbricationType res = SemExpComparator::getSemExpsImbrications
              (*pCurrContextAxiom.getSemExpWrappedForMemory().semExp, pGrdExp, pMemViewer.constView, pWorkStruct.lingDb, nullptr);
          if (res == ImbricationType::OPPOSES)
            return false;

          UniqueSemanticExpression sentenceThatWeShougth(pCurrContextAxiom.getSemExpWrappedForMemory().semExp->clone());
          SemExpModifier::modifyVerbTenseOfSemExp(*sentenceThatWeShougth,
                                                  SemanticVerbTense::PAST);
          auto fdkExp = SemExpCreator::sayIThoughtThat(std::move(sentenceThatWeShougth));
          pWorkStruct.addAnswer(ContextualAnnotation::FEEDBACK, std::move(fdkExp),
                                ReferencesFiller(pCurrContextAxiom));
          return true;
        };

        for (const SemanticContextAxiom* currContextAxiom : pAnswersContextAxioms.elts)
        {
          if (reactionOfOppositeInformation(*currContextAxiom))
          {
            pResThatCanHaveAdditionalFeedbacks = true;
            break;
          }
        }
        if (!pResThatCanHaveAdditionalFeedbacks)
        {
          for (const SemanticContextAxiom* currContextAxiom : pAnswersContextAxioms.constElts)
          {
            if (reactionOfOppositeInformation(*currContextAxiom))
            {
              pResThatCanHaveAdditionalFeedbacks = true;
              break;
            }
          }
        }

        if (!pResThatCanHaveAdditionalFeedbacks)
        {
          pWorkStruct.addAnswer
              (ContextualAnnotation::FEEDBACK,
               SemExpCreator::sayThatWeThoughtTheContrary(),
               ReferencesFiller(pAnswersContextAxioms));
          pResThatCanHaveAdditionalFeedbacks = true;
        }
      }
      else
      {
        _confirmOrCorrect(pWorkStruct, pGrdExp, pTruenessValue, pAnswersContextAxioms);
        pResThatCanHaveAdditionalFeedbacks = true;
      }
    }
    else if (pAnswerIsAnAssertion)
    {
      return false;
    }
  }
  return false;
}


void processWithUpdatedMemory(bool& pRes,
                              SemControllerWorkingStruct& pWorkStruct,
                              SemanticContextAxiom* pNewContextAxiom,
                              SemanticMemoryBlockViewer& pMemViewer,
                              const GroundedExpression& pGrdExp)
{
  if (pNewContextAxiom != nullptr)
    pNewContextAxiom->setEnabled(false);

  if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
      pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::REACT_ON_SIMILARITIES)
    pRes = similaritesWithMemoryFinder::reactOnSimilarities(pWorkStruct, pMemViewer, pGrdExp) || pRes;

  if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
      pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::ASK_FOR_ADDITIONAL_INFORMATION)
    pRes = pRes || _askAboutLocationOrTimeOfAPastActions(pWorkStruct, pMemViewer, pGrdExp);

  if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT && !pWorkStruct.haveAnAnswer())
    pRes = pRes || _sayDayOfBirthdate(pWorkStruct, pGrdExp, pMemViewer.currentUserId) ||
        (pWorkStruct.reactionOptions.canSayOkToAnAffirmation && _sayOkIfTheUserIsTalkingAboutHim(pWorkStruct, pGrdExp)) ||
        _sayItsNotFalseFromFrenchSentenceWhenObjectIsUnknown(pWorkStruct, pGrdExp);

  if (pNewContextAxiom != nullptr)
    pNewContextAxiom->setEnabled(true);
}


} // End of namespace proactiveReaction
} // End of namespace onsem
