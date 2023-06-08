#include "unknowninfosgetter.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinks.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../semexpcontroller.hpp"
#include "../../outputter/grdexptooutputterinformation.hpp"
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../type/referencesfiller.hpp"
#include "../../type/semanticdetailledanswer.hpp"
#include "../../utility/semexpcreator.hpp"
#include "type/semcontrollerworkingstruct.hpp"
#include "semanticmemorylinker.hpp"

namespace onsem
{
namespace unknownInfosGetter
{

namespace
{
enum class ReplacingParamStrategy
{
  RETURN_PARAM,
  ASK_WHAT_IS,
  RETURN_AN_INSTANCE
};


void _recCheckIfMatchAndGetParams(IndexToSubNameToParameterValue& pParams,
                                  std::unique_ptr<InteractionContextContainer>& pSubIntContext,
                                  GrdKnowToUnlinked* pIncompleteRelations,
                                  bool& pIsComplete,
                                  const GroundedExpWithLinks& pSemMemSent,
                                  GrammaticalType pMemChildType,
                                  const std::pair<const GrammaticalType, UniqueSemanticExpression>& pCurrMemChild,
                                  const GroundedExpression& pGrdExp,
                                  const SemControllerWorkingStruct& pWorkStruct,
                                  const SemanticMemoryBlockViewer& pMemViewer)
{
  auto itSemExpChild = pGrdExp.children.find(pCurrMemChild.first);

  // if a child is specified in the condition from memory but not in the current sentence
  if (itSemExpChild == pGrdExp.children.end())
  {
    // if it's a
    const GroundedExpression* childGrdExpPtr = pCurrMemChild.second->getGrdExpPtr_SkipWrapperPtrs();
    if (childGrdExpPtr != nullptr)
    {
      auto& childGrd = childGrdExpPtr->grounding();
      if (childGrd.type == SemanticGroundingType::META)
      {
        int idParam = childGrd.getMetaGroundingPtr()->paramId;
        if (idParam == SemanticMetaGrounding::returnId)
        {
          pParams[SemanticMetaGrounding::returnId][""];
          return;
        }
      }
      else if (ConceptSet::haveAConcept(childGrd.concepts, "stuff_informationToFill"))
      {
        return;
      }
    }

    // if the current sentence have a SPECIFIER, we look recurssively at his children
    auto itSpecSemExpChild = pGrdExp.children.find(GrammaticalType::SPECIFIER);
    if (itSpecSemExpChild != pGrdExp.children.end())
    {
      const GroundedExpression* specGrdExpChild = itSpecSemExpChild->second->getGrdExpPtr_SkipWrapperPtrs();
      if (specGrdExpChild != nullptr)
      {
        _recCheckIfMatchAndGetParams(pParams, pSubIntContext, pIncompleteRelations, pIsComplete, pSemMemSent,
                                     pMemChildType, pCurrMemChild, *specGrdExpChild, pWorkStruct, pMemViewer);
        return;
      }
    }

    if (pCurrMemChild.first == GrammaticalType::OWNER)
      return;

    // the relation is incomplete
    pIsComplete = false;
    if (pIncompleteRelations != nullptr)
    {
      (*pIncompleteRelations)[pSemMemSent.id].push_back(pMemChildType);
    }
  }
  else if (pIsComplete)
  {
    // add parameters
    const GroundedExpression* memGrdExpChild = pCurrMemChild.second->getGrdExpPtr_SkipWrapperPtrs();
    if (memGrdExpChild != nullptr)
    {
      if (memGrdExpChild->grounding().type == SemanticGroundingType::META)
      {
        const SemanticMetaGrounding& metaGr = (*memGrdExpChild)->getMetaGrounding();
        const SemanticExpression& childCorespondingToTheParam = *itSemExpChild->second;
        const GroundedExpression* childCorespondingToTheParamGrdExpPtr = childCorespondingToTheParam.getGrdExpPtr_SkipWrapperPtrs();
        ReplacingParamStrategy replacingParamStrategy = [&]
        {
          if (metaGr.attibuteName == "#dontanswer")
            return ReplacingParamStrategy::RETURN_PARAM;

          if (childCorespondingToTheParamGrdExpPtr != nullptr)
          {
            const GroundedExpression& childCorespondingToTheParamGrdExp = *childCorespondingToTheParamGrdExpPtr;
            const SemanticStatementGrounding* childCorespondingToTheParamStatGrdPtr = childCorespondingToTheParamGrdExp->getStatementGroundingPtr();
            if (childCorespondingToTheParamStatGrdPtr != nullptr)
            {
              if (childCorespondingToTheParamStatGrdPtr->requests.empty())
                return ReplacingParamStrategy::RETURN_PARAM;
            }
            else
            {
              const SemanticGenericGrounding* childCorespondingToTheParamGenGrdPtr = childCorespondingToTheParamGrdExp->getGenericGroundingPtr();
              if (childCorespondingToTheParamGenGrdPtr != nullptr)
              {
                const SemanticGenericGrounding& childCorespondingToTheParamGenGrd = *childCorespondingToTheParamGenGrdPtr;
                if (childCorespondingToTheParamGenGrd.referenceType == SemanticReferenceType::INDEFINITE)
                {
                  if (ConceptSet::haveAConceptThatBeginWith(childCorespondingToTheParamGenGrd.concepts, "story"))
                    return ReplacingParamStrategy::RETURN_AN_INSTANCE;
                  else
                    return ReplacingParamStrategy::RETURN_PARAM;
                }
                else if (childCorespondingToTheParamGenGrd.referenceType == SemanticReferenceType::UNDEFINED)
                {
                  return ReplacingParamStrategy::RETURN_PARAM;
                }
              }
              else
                return ReplacingParamStrategy::RETURN_PARAM;
            }
          }
          return ReplacingParamStrategy::ASK_WHAT_IS;
        }();

        switch (replacingParamStrategy)
        {
        case ReplacingParamStrategy::RETURN_PARAM:
        {
          pParams[metaGr.paramId].emplace(metaGr.attibuteName,
                                          std::make_unique<UniqueSemanticExpression>(childCorespondingToTheParam.clone()));
          break;
        }
        case ReplacingParamStrategy::ASK_WHAT_IS:
        {
          bool paramHasBeenInserted = false;
          // get value behind the param
          SemControllerWorkingStruct subWorkStruct(pWorkStruct);
          if (subWorkStruct.askForNewRecursion())
          {
            if (SemExpGetter::semExphasAStatementGrd(childCorespondingToTheParam))
              subWorkStruct.reactOperator = SemanticOperatorEnum::ANSWER;
            else
              subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
            SemanticMemoryBlockViewer subMemView(pMemViewer);
            controller::applyOperatorOnSemExp(subWorkStruct, subMemView, childCorespondingToTheParam);
            if (subWorkStruct.compositeSemAnswers)
            {
              auto paramSemExp = subWorkStruct.compositeSemAnswers->convertToSemExp();
              if (paramSemExp)
              {
                if (!pSubIntContext)
                  pSubIntContext = subWorkStruct.compositeSemAnswers->getInteractionContextContainer();
                pParams[metaGr.paramId].emplace(metaGr.attibuteName, std::move(paramSemExp));
                paramHasBeenInserted = true;
              }
            }

            if (!paramHasBeenInserted &&
                subWorkStruct.reactOperator == SemanticOperatorEnum::GET &&
                childCorespondingToTheParamGrdExpPtr != nullptr)
            {
              subWorkStruct.reactOperator = SemanticOperatorEnum::ANSWER;
              SemanticMemoryBlockViewer subMemView(pMemViewer);
              controller::applyOperatorOnSemExp(subWorkStruct, subMemView,
                                                *SemExpCreator::askWhatIs(*childCorespondingToTheParamGrdExpPtr));
              if (subWorkStruct.compositeSemAnswers)
              {
                auto paramSemExp = subWorkStruct.compositeSemAnswers->convertToSemExp();
                if (paramSemExp)
                {
                  pParams[metaGr.paramId].emplace(metaGr.attibuteName, std::move(paramSemExp));
                  paramHasBeenInserted = true;
                }
              }
            }
          }
          if (!paramHasBeenInserted)
            pParams[metaGr.paramId].emplace(metaGr.attibuteName,
                                            std::make_unique<UniqueSemanticExpression>(childCorespondingToTheParam.clone()));
          break;
        }
        case ReplacingParamStrategy::RETURN_AN_INSTANCE:
        {
          pParams[metaGr.paramId].emplace(metaGr.attibuteName,
                                          std::make_unique<UniqueSemanticExpression>(SemExpCreator::sayThatWeDontKnowAnInstanceOf(childCorespondingToTheParam)));
          break;
        }
        }
      }


      for (const auto& currMemSubChild : memGrdExpChild->children)
      {
        const GroundedExpression* grdExpChild = itSemExpChild->second->getGrdExpPtr_SkipWrapperPtrs();
        if (grdExpChild != nullptr)
          _recCheckIfMatchAndGetParams(pParams, pSubIntContext, pIncompleteRelations, pIsComplete,
                                       pSemMemSent, pMemChildType, currMemSubChild,
                                       *grdExpChild, pWorkStruct, pMemViewer);
      }
    }
  }
}

std::unique_ptr<UniqueSemanticExpression> convertToActionToDo(
    const GroundedExpression*& pActionThatCannotBeDone,
    const GroundedExpression& pActionGrdExp,
    const SemControllerWorkingStruct& pWorkStruct,
    SemanticMemoryBlockViewer& pMemViewer)
{
  const auto* statGrdExpPtr = pActionGrdExp->getStatementGroundingPtr();
  if (statGrdExpPtr != nullptr &&
      (statGrdExpPtr->isAtInfinitive() || statGrdExpPtr->isMandatoryInPresentTense()))
  {
    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion())
    {
      if (semanticMemoryLinker::satisfyAnAction(subWorkStruct, pMemViewer, pActionGrdExp, *statGrdExpPtr))
      {
        if (!subWorkStruct.compositeSemAnswers->semAnswers.empty())
        {
          auto& semAnswer = subWorkStruct.compositeSemAnswers->semAnswers.front();
          LeafSemAnswer* leafAnswPtr = semAnswer->getLeafPtr();
          if (leafAnswPtr != nullptr && leafAnswPtr->reaction)
          {
            auto& reaction = *leafAnswPtr->reaction;
            auto* reactionResourcePtr = SemExpGetter::semExpToResourceGrounding(reaction.getSemExp());
            if (reactionResourcePtr != nullptr)
            {
              auto& reactionResource = reactionResourcePtr->resource;
              auto semResource = std::make_unique<SemanticResourceGrounding>(
                    reactionResource.label, reactionResource.language, reactionResource.value);
              auto genericMandatoryForm = pActionGrdExp.clone();
              SemExpModifier::setChild(*genericMandatoryForm, GrammaticalType::SUBJECT,
                                       std::make_unique<GroundedExpression>(SemanticAgentGrounding::getRobotAgentPtr()));
              converter::extractParameters(semResource->resource.parametersLabelsToValue,
                                           reactionResource.parameterLabelsToQuestions,
                                           std::move(genericMandatoryForm), pWorkStruct.lingDb);
              return std::make_unique<UniqueSemanticExpression>(std::make_unique<GroundedExpression>(std::move(semResource)));
            }
            return std::make_unique<UniqueSemanticExpression>(std::move(reaction));
          }
        }
        return {};
      }
    }
    pActionThatCannotBeDone = &pActionGrdExp;
  }
  return {};
}

}


bool checkIfMatchAndGetParams(IndexToSubNameToParameterValue& pParams,
                              std::unique_ptr<InteractionContextContainer>& pSubIntContext,
                              GrdKnowToUnlinked* pIncompleteRelations,
                              const GroundedExpWithLinks& pSemMemSent,
                              const GroundedExpression& pGrdExp,
                              const SemControllerWorkingStruct& pWorkStruct,
                              const SemanticMemoryBlockViewer& pMemViewer)
{
  if (!SemExpComparator::haveSamePolarity(pGrdExp, pSemMemSent.grdExp,
                                          pWorkStruct.lingDb.conceptSet, true))
    return false;
  bool isComplete = true;
  for (const auto& currChild : pSemMemSent.grdExp.children)
    _recCheckIfMatchAndGetParams(pParams, pSubIntContext, pIncompleteRelations, isComplete,
                                 pSemMemSent, currChild.first, currChild,
                                 pGrdExp, pWorkStruct, pMemViewer);
  return isComplete;
}


std::unique_ptr<UniqueSemanticExpression> _extartActionToDoInBackground(
    const GroundedExpression*& pActionThatCannotBeDone,
    const GroundedExpression& pGrdExp,
    SemControllerWorkingStruct& pWorkStruct,
    SemanticMemoryBlockViewer& pMemViewer)
{
  auto itBackgroundChild = pGrdExp.children.find(GrammaticalType::IN_BACKGROUND);
  if (itBackgroundChild != pGrdExp.children.end())
  {
    auto* backgroundGrdExpPtr = itBackgroundChild->second->getGrdExpPtr_SkipWrapperPtrs();
    if (backgroundGrdExpPtr != nullptr)
    {
      return convertToActionToDo(pActionThatCannotBeDone, *backgroundGrdExpPtr,
                                 pWorkStruct, pMemViewer);
    }
  }
  return {};
}


bool splitCompeleteIncompleteOfActions(SemControllerWorkingStruct& pWorkStruct,
                                       SemanticMemoryBlockViewer& pMemViewer,
                                       GrdKnowToUnlinked& pIncompleteRelations,
                                       const GroundedExpWithLinks& pMemSent,
                                       const GroundedExpression& pGrdExp)
{
  auto params = std::make_unique<IndexToSubNameToParameterValue>();
  std::unique_ptr<InteractionContextContainer> subIntContext;
  if (!checkIfMatchAndGetParams(*params, subIntContext, &pIncompleteRelations, pMemSent, pGrdExp, pWorkStruct, pMemViewer))
    return false;

  // if it has a receiver -> say that we will do it when the receiver will be focused
  auto itSemExpReceiver = pGrdExp.children.find(GrammaticalType::RECEIVER);
  if (itSemExpReceiver != pGrdExp.children.end())
  {
    const GroundedExpression* receiverGrdExp = itSemExpReceiver->second->getGrdExpPtr_SkipWrapperPtrs();
    if (receiverGrdExp != nullptr)
    {
      if (receiverGrdExp->grounding().type == SemanticGroundingType::AGENT)
      {
        const SemanticAgentGrounding& agentGrd = (*receiverGrdExp)->getAgentGrounding();
        if (agentGrd.userId != SemanticAgentGrounding::me &&
            !pMemViewer.constView.areSameUserConst(agentGrd.userId, pMemViewer.currentUserId))
        {
          if (agentGrd.userId == SemanticAgentGrounding::userNotIdentified ||
              agentGrd.userId == SemanticAgentGrounding::anyUser ||
              pMemViewer.constView.getName(agentGrd.userId).empty())
          {
            pWorkStruct.addQuestion(SemExpCreator::getWhoIsSomebodyQuestion(*receiverGrdExp));
            return true;
          }
          else
          {
            pWorkStruct.addConditionForAUserAnswer
                (ContextualAnnotation::NOTIFYSOMETHINGWILLBEDONE,
                 SemExpCreator::sayWeWillDoIt_fromGrdExp(pGrdExp),
                 ConditionForAUser(agentGrd.userId,
                                   pGrdExp.clone()));
            return true;
          }
        }
      }
      else
      {
        pWorkStruct.addQuestion(SemExpCreator::getWhoIsSomebodyQuestion(*receiverGrdExp));
        return true;
      }
    }
  }

  // get the specified language for the reaction
  SemanticLanguageEnum behavLanguage = SemExpGetter::getLanguage(pGrdExp.children);

  // get the specified number of execution for the reaction
  const SemanticExpression* numberOfExcutionsSemExpPtr = nullptr;
  auto itSemExpRepetition = pGrdExp.children.find(GrammaticalType::REPETITION);
  if (itSemExpRepetition != pGrdExp.children.end())
    numberOfExcutionsSemExpPtr = &*itSemExpRepetition->second;

  // get the duration
  const SemanticExpression* durationSemExpPtr = nullptr;
  auto itSemExpDuration = pGrdExp.children.find(GrammaticalType::DURATION);
  if (itSemExpDuration != pGrdExp.children.end())
    durationSemExpPtr = &*itSemExpDuration->second;

  auto& contextAxiom = pMemSent.getContextAxiom();
  if (contextAxiom.infCommandToDo == nullptr)
    return false;
  auto& infCommandToDo = *contextAxiom.infCommandToDo;
  bool answerGenerated = false;
  const IndexToSubNameToParameterValue* paramsPtr = params ? &*params : nullptr;

  for (const auto& currListElts : SemExpGetter::iterateOnList(infCommandToDo))
  {
    const GroundedExpression* listEltGrdExpPtr =
        currListElts->getGrdExpPtr_SkipWrapperPtrs();
    if (listEltGrdExpPtr != nullptr)
    {
      const SemanticStatementGrounding* statGrdExpPtr =
          listEltGrdExpPtr->grounding().getStatementGroundingPtr();
      if (statGrdExpPtr != nullptr &&
          !statGrdExpPtr->isAtInfinitive() && !statGrdExpPtr->isMandatoryInPresentTense())
      {
        if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT && pWorkStruct.author != nullptr)
        {
          pWorkStruct.addAnswerWithoutReferences
              (ContextualAnnotation::TEACHINGFEEDBACK,
               SemExpCreator::mergeInAList(SemExpCreator::sayThatTheRobotIsNotAbleToDoIt(pGrdExp),
                                           SemExpCreator::doYouWantMeToSayToTellYouHowTo(*pWorkStruct.author, pGrdExp)));
        }
        return true;
      }
    }
  }

  const GroundedExpression* actionThatCannotBeDone = nullptr;
  if (!answerGenerated)
  {
    UniqueSemanticExpression actionSemExp = infCommandToDo.clone(paramsPtr);

    for (const auto& currListElts : SemExpGetter::iterateOnList(actionSemExp))
    {
      GroundedExpression* listEltGrdExpPtr =
          (*currListElts)->getGrdExpPtr_SkipWrapperPtrs();
      if (listEltGrdExpPtr != nullptr)
      {
        if (!hasGrdExpOutputInformations(*listEltGrdExpPtr))
        {
          auto actionToDoPtr = convertToActionToDo(actionThatCannotBeDone, *listEltGrdExpPtr, pWorkStruct, pMemViewer);
          if (actionThatCannotBeDone != nullptr)
            break;
          if (actionToDoPtr)
            *currListElts = std::move(*actionToDoPtr);
        }

        auto actionToDoInBackground = _extartActionToDoInBackground(actionThatCannotBeDone, *listEltGrdExpPtr, pWorkStruct, pMemViewer);
        if (actionThatCannotBeDone != nullptr)
          break;
        if (actionToDoInBackground)
        {
          listEltGrdExpPtr->children.erase(GrammaticalType::IN_BACKGROUND);
          listEltGrdExpPtr->children.emplace(GrammaticalType::IN_BACKGROUND, std::move(*actionToDoInBackground));
        }
      }
    }

    std::unique_ptr<UniqueSemanticExpression> actionToDoInBackground;
    if (!answerGenerated && actionThatCannotBeDone == nullptr)
      actionToDoInBackground = _extartActionToDoInBackground(actionThatCannotBeDone, pGrdExp, pWorkStruct, pMemViewer);

    if (actionThatCannotBeDone != nullptr)
    {
      if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT)
      {
        pWorkStruct.addAnswerWithoutReferences
            (ContextualAnnotation::BEHAVIORNOTFOUND,
             SemExpCreator::sayThatTheRobotCannotDoIt(*actionThatCannotBeDone));
      }
      answerGenerated = true;
    }

    if (!answerGenerated)
    {
      if (behavLanguage != SemanticLanguageEnum::UNKNOWN ||
          numberOfExcutionsSemExpPtr != nullptr ||
          durationSemExpPtr != nullptr ||
          actionToDoInBackground)
      {
        auto newAnnExp = std::make_unique<AnnotatedExpression>(std::move(actionSemExp));
        if (behavLanguage != SemanticLanguageEnum::UNKNOWN)
          newAnnExp->annotations.emplace
              (GrammaticalType::LANGUAGE,
               std::make_unique<GroundedExpression>(std::make_unique<SemanticLanguageGrounding>(behavLanguage)));
        if (numberOfExcutionsSemExpPtr != nullptr)
          newAnnExp->annotations.emplace
              (GrammaticalType::REPETITION, numberOfExcutionsSemExpPtr->clone());
        if (durationSemExpPtr != nullptr)
          newAnnExp->annotations.emplace
              (GrammaticalType::DURATION, durationSemExpPtr->clone());
        if (actionToDoInBackground)
          newAnnExp->annotations.emplace
              (GrammaticalType::IN_BACKGROUND, std::move(*actionToDoInBackground));
        actionSemExp = std::move(newAnnExp);
      }

      auto cmdExp = std::make_unique<CommandExpression>(std::move(actionSemExp));
      cmdExp->description.emplace(SemExpModifier::fromImperativeToActionDescription(pGrdExp));
      pWorkStruct.addAnswer(ContextualAnnotation::BEHAVIOR, std::move(cmdExp),
                            ReferencesFiller(contextAxiom));
      auto* leafPtr = pWorkStruct.compositeSemAnswers->semAnswers.back()->getLeafPtr();
      if (leafPtr != nullptr && subIntContext)
        leafPtr->interactionContextContainer = std::move(subIntContext);
    }
  }
  return true;
}


bool getRequestToAskForPrecision(SemanticRequestType& pRequestType,
                                 const GrdKnowToUnlinked& pIncompleteRelations)
{
  pRequestType = SemanticRequestType::NOTHING;
  for (GrdKnowToUnlinked::const_iterator itIncRel = pIncompleteRelations.begin();
       itIncRel != pIncompleteRelations.end(); ++itIncRel)
  {
    if (!itIncRel->second.empty())
    {
      if (pRequestType == SemanticRequestType::NOTHING)
      {
        if (itIncRel->second.front() == GrammaticalType::MANNER)
        {
          pRequestType = SemanticRequestType::MANNER;
        }
        else if (itIncRel->second.front() == GrammaticalType::OBJECT)
        {
          pRequestType = SemanticRequestType::OBJECT;
        }
      }
      break;
    }
  }
  return pRequestType != SemanticRequestType::NOTHING;
}


bool isItfUnknown(const SemControllerWorkingStruct& pWorkStruct,
                  const SemanticMemoryBlockViewer& pMemViewer,
                  const GroundedExpression& pGrdExp)
{
  SemControllerWorkingStruct subWorkStruct(pWorkStruct);
  if (subWorkStruct.askForNewRecursion())
  {
    subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
    SemanticMemoryBlockViewer subMemView(pMemViewer);
    controller::applyOperatorOnGrdExp(subWorkStruct, subMemView, pGrdExp,
                                      std::list<const GroundedExpression*>(),
                                      pGrdExp);
    return !subWorkStruct.haveAnAnswer();
  }
  return false;
}



} // End of namespace unknownInfosGetter
} // End of namespace onsem
