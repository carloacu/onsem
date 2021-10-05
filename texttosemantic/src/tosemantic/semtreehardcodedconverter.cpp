#include "semtreehardcodedconverter.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/comparisonexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace semTreeHardCodedConverter
{

namespace
{

void _addAbilityVerbGoal(SemanticStatementGrounding& pStatementGrd)
{
  pStatementGrd.verbGoal = VerbGoalEnum::ABILITY;
}

void _addMandatoryVerbGoal(SemanticStatementGrounding& pStatementGrd)
{
  pStatementGrd.verbGoal = VerbGoalEnum::MANDATORY;
}

void _addAdviceVerbGoal(SemanticStatementGrounding& pStatementGrd)
{
  pStatementGrd.verbGoal = VerbGoalEnum::ADVICE;
}

void _addFutureVerbTense(SemanticStatementGrounding& pStatementGrd)
{
  pStatementGrd.verbTense = SemanticVerbTense::FUTURE;
}


void _putConditionAtRoot
(std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
 GrammaticalType pGramTypeOfCondition)
{
  auto it = pChildTypeToCondition.find(pGramTypeOfCondition);
  if (it != pChildTypeToCondition.end())
  {
    pChildTypeToCondition.emplace(GrammaticalType::UNKNOWN, std::move(it->second));
    pChildTypeToCondition.erase(it);
  }
}


bool _applySemExpModifier
(std::unique_ptr<SemanticExpression>& pSemExp,
 std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
 GroundedExpression& pGrdExpRoot,
 SemanticStatementGrounding& pStatementGrdRoot,
 void (*statementGrdModifier)(SemanticStatementGrounding&),
 bool pIsPassive)
{
  // get the object of "aller" (the object has to be a verb)
  auto itObjectChild = pGrdExpRoot.children.find(pIsPassive ? GrammaticalType::SUBJECT : GrammaticalType::OBJECT);
  if (itObjectChild != pGrdExpRoot.children.end())
  {
    // keep the child alive
    auto saveOfObject = itObjectChild->second.getSharedPtr();

    GroundedExpression* grdExpObject = saveOfObject->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpObject != nullptr)
    {
      SemanticStatementGrounding* statementGrdObject = (*grdExpObject)->getStatementGroundingPtr();
      if (statementGrdObject != nullptr &&
          !statementGrdObject->coreference &&
          statementGrdObject->requests.empty())
      {
        statementGrdObject->requests = pStatementGrdRoot.requests;
        statementGrdObject->verbTense = pStatementGrdRoot.verbTense;
        statementGrdObject->polarity = pStatementGrdRoot.polarity;
        bool hasToInvertSubjectAndObject = statementGrdObject->isPassive != pStatementGrdRoot.isPassive;
        (*statementGrdModifier)(*statementGrdObject);
        // "aller" grounding become the <verb object> grounding
        pGrdExpRoot.moveGrounding(grdExpObject->cloneGrounding());
        _putConditionAtRoot(pChildTypeToCondition, itObjectChild->first);
        // remove the <verb object> node (but it has been saved in a shared_ptr)
        pGrdExpRoot.children.erase(itObjectChild);
        // invert subject and object if necessary
        if (hasToInvertSubjectAndObject)
          SemExpModifier::invertSubjectAndObjectGrdExp(pGrdExpRoot);
        // add the <verb object> children in the "aller" child
        SemExpModifier::moveChildrenOfAGrdExp(pGrdExpRoot, *grdExpObject);
        return true;
      }
      return false;
    }

    ListExpression* listExpObject = saveOfObject->getListExpPtr();
    if (listExpObject != nullptr)
    {
      bool currObjectHasBenRemoved = false;
      for (auto& currElt : listExpObject->elts)
      {
        GroundedExpression* eltGrdExp = currElt->getGrdExpPtr_SkipWrapperPtrs();
        if (eltGrdExp == nullptr)
          return false;
        SemanticStatementGrounding* statementEltGrdObject = (*eltGrdExp)->getStatementGroundingPtr();
        if (statementEltGrdObject == nullptr)
          return false;
        statementEltGrdObject->requests = pStatementGrdRoot.requests;
        statementEltGrdObject->verbTense = pStatementGrdRoot.verbTense;
        statementEltGrdObject->polarity = pStatementGrdRoot.polarity;
        (*statementGrdModifier)(*statementEltGrdObject);
        if (!currObjectHasBenRemoved)
        {
          currObjectHasBenRemoved = true;
          _putConditionAtRoot(pChildTypeToCondition, itObjectChild->first);
          // remove the <verb object> node (but it has been saved in a shared_ptr)
          pGrdExpRoot.children.erase(itObjectChild);
        }

        for (const auto& currChild : pGrdExpRoot.children)
          SemExpModifier::addChild(*eltGrdExp, currChild.first,
                                   currChild.second->clone());
      }
      pSemExp = saveOfObject->clone();
      return true;
    }
  }
  return false;
}


void _refactorCaVaFrenchSentence(GroundedExpression& pGrdExp,
                                 SemanticStatementGrounding& pStatementGrd,
                                 const TextProcessingContext& pTextProcContext)
{
  if (pStatementGrd.verbTense != SemanticVerbTense::PUNCTUALPRESENT)
    return;

  // /!\ We already checked that the main verb is "aller"
  // replace "ça" from "ça va" sentence by the author or the receiver of the sentence
  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject != pGrdExp.children.end())
  {
    GroundedExpression* subjectGrdExpPtr = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
    if (subjectGrdExpPtr != nullptr)
    {
      if (pGrdExp.children.count(GrammaticalType::MANNER) == 0)
      {
        if (pGrdExp.children.size() > 1)
          return;
        if (pStatementGrd.requests.empty() ||
            pStatementGrd.requests.has(SemanticRequestType::YESORNO))
        {
          pGrdExp.children.emplace(GrammaticalType::MANNER,
                                   mystd::make_unique<GroundedExpression>
                                   ([]
          {
            auto res = mystd::make_unique<SemanticGenericGrounding>();
            res->word = SemanticWord(SemanticLanguageEnum::FRENCH, "bien", PartOfSpeech::ADVERB);
            res->concepts.emplace("manner_well", 4);
            return res;
          }()));
        }
      }
      else if (pGrdExp.children.count(GrammaticalType::OBJECT) > 0)
      {
        return;
      }

      pStatementGrd.concepts.clear();
      pStatementGrd.concepts.emplace("verb_go", 4);
      if (pStatementGrd.requests.empty())
        subjectGrdExpPtr->moveGrounding(mystd::make_unique<SemanticAgentGrounding>(pTextProcContext.author));
      else
        subjectGrdExpPtr->moveGrounding(mystd::make_unique<SemanticAgentGrounding>(pTextProcContext.receiver));
    }
  }
}

}


bool manageEnglishAuxiliaries
(GroundedExpression& pGrdExp,
 const std::map<std::string, char>& pAuxConcepts)
{
  if (ConceptSet::haveAConcept(pAuxConcepts, "verb_can"))
  {
    SemExpModifier::fillVerbGoal(pGrdExp, VerbGoalEnum::ABILITY);
    return true;
  }
  else if (ConceptSet::haveAConcept(pAuxConcepts, "verb_haveto"))
  {
    SemExpModifier::fillVerbGoal(pGrdExp, VerbGoalEnum::MANDATORY);
    return true;
  }
  else if (ConceptSet::haveAConcept(pAuxConcepts, "verb_should"))
  {
    SemExpModifier::fillVerbGoal(pGrdExp, VerbGoalEnum::ADVICE);
    return true;
  }
  else if (ConceptSet::haveAConcept(pAuxConcepts, "verb_would"))
  {
    SemExpModifier::fillVerbGoal(pGrdExp, VerbGoalEnum::CONDITIONAL);
    return true;
  }
  return false;
}



std::unique_ptr<SemanticExpression> convertEnglishSentenceToASemExp
(std::unique_ptr<GroundedExpression> pGrdExp)
{
  if (pGrdExp)
  {
    SemanticStatementGrounding* statGrdPtr = (*pGrdExp)->getStatementGroundingPtr();
    if (statGrdPtr != nullptr)
    {
      SemanticStatementGrounding& statGrd = *statGrdPtr;
      // handle equal verb
      if (ConceptSet::haveAConceptThatBeginWith(statGrd.concepts, "comparison_"))
      {
        ComparisonOperator compOperator = ComparisonOperator::EQUAL;
        if (statGrd.concepts.find("comparison_equal") != statGrd.concepts.end())
          compOperator = ComparisonOperator::EQUAL;
        else if (statGrd.concepts.find("comparison_different") != statGrd.concepts.end())
          compOperator = ComparisonOperator::DIFFERENT;
        else
          return std::move(pGrdExp);

        auto itSubjectChild = pGrdExp->children.find(GrammaticalType::SUBJECT);
        if (itSubjectChild != pGrdExp->children.end())
        {
          auto itObjectChild = pGrdExp->children.find(GrammaticalType::OBJECT);
          if (itObjectChild != pGrdExp->children.end())
          {
            auto compExp = mystd::make_unique<ComparisonExpression>
                (compOperator,
                 std::move(itSubjectChild->second));
            compExp->rightOperandExp.emplace(std::move(itObjectChild->second));
            compExp->tense = statGrd.verbTense;
            compExp->request = statGrd.requests.firstOrNothing();
            return std::move(compExp);
          }
        }
      }
      else if (!statGrd.polarity)
      {
        auto itObjectChild = pGrdExp->children.find(GrammaticalType::OBJECT);
        if (itObjectChild != pGrdExp->children.end())
        {
          GroundedExpression* objectGrdExpPtr = itObjectChild->second->getGrdExpPtr_SkipWrapperPtrs();
          if (objectGrdExpPtr != nullptr)
          {
            SemanticGenericGrounding* genGrdObjectPtr = objectGrdExpPtr->grounding().getGenericGroundingPtr();
            if (genGrdObjectPtr != nullptr &&
                genGrdObjectPtr->quantity.type == SemanticQuantityType::ANYTHING)
            {
              statGrd.polarity = !statGrd.polarity;
              genGrdObjectPtr->quantity.setNumber(0);
            }
          }
        }
      }
    }
  }
  return std::move(pGrdExp);
}


void refactorEnglishSentencesWithAGoal(std::unique_ptr<SemanticExpression>& pSemExp,
                                       std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition)
{
  GroundedExpression* grdExpPtr = pSemExp->getGrdExpPtr();
  if (grdExpPtr != nullptr)
  {
    GroundedExpression& grdExp = *grdExpPtr;
    SemanticStatementGrounding* statementGrdPtr = grdExp->getStatementGroundingPtr();
    if (statementGrdPtr != nullptr)
    {
      SemanticStatementGrounding& statementGrd = *statementGrdPtr;
      // check that the root verb is "going"
      if (statementGrd.verbTense == SemanticVerbTense::PUNCTUALPRESENT &&
          statementGrd.concepts.count("verb_action_go") != 0)
      {
        /**
         * replace stucts:
         *            going
         *            /    \
         *        <subj>   <infinitive verb>
         *
         * by:
         *       <verb in future>
         *         /
         *      <subj>
         */
        _applySemExpModifier(pSemExp, pChildTypeToCondition, grdExp, statementGrd,
                             &_addFutureVerbTense, false);
        return;
      }

      if (statementGrd.concepts.count("verb_have") != 0)
      {
        _applySemExpModifier(pSemExp, pChildTypeToCondition, grdExp, statementGrd,
                             &_addMandatoryVerbGoal, false);
        return;
      }
    }
  }
}


void refactorFrenchSentencesWithAGoal(std::unique_ptr<SemanticExpression>& pSemExp,
                                      std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
                                      const TextProcessingContext& pTextProcContext,
                                      bool pIsPassive)
{
  GroundedExpression* grdExpPtr = pSemExp->getGrdExpPtr();
  if (grdExpPtr != nullptr)
  {
    GroundedExpression& grdExp = *grdExpPtr;
    SemanticStatementGrounding* statementGrdPtr = grdExp->getStatementGroundingPtr();
    if (statementGrdPtr != nullptr)
    {
      SemanticStatementGrounding& statementGrd = *statementGrdPtr;

      // check that the root verb is "aller"
      if (statementGrd.verbTense == SemanticVerbTense::PUNCTUALPRESENT &&
          statementGrd.concepts.count("verb_action_go") != 0)
      {
        /**
         * replace stucts:
         *            aller
         *            /    \
         *        <subj>   <verb>
         *
         * by:
         *       <verb in future>
         *         /
         *      <subj>
         */
        if (!_applySemExpModifier(pSemExp, pChildTypeToCondition, grdExp, statementGrd,
                                  &_addFutureVerbTense, false))
        {
          _refactorCaVaFrenchSentence(grdExp, statementGrd, pTextProcContext);
        }
        return;
      }

      if (statementGrd.concepts.count("verb_haveto") != 0)
      {
        const bool isFalloirVerb = statementGrd.word == SemanticWord(SemanticLanguageEnum::FRENCH, "falloir", PartOfSpeech::VERB);
        bool modifDone = false;
        if (statementGrd.verbGoal == VerbGoalEnum::CONDITIONAL)
          modifDone = _applySemExpModifier(pSemExp, pChildTypeToCondition, grdExp, statementGrd,
                                           &_addAdviceVerbGoal, false);
        else
          modifDone = _applySemExpModifier(pSemExp, pChildTypeToCondition, grdExp, statementGrd,
                                           &_addMandatoryVerbGoal, false);
        if (isFalloirVerb && modifDone)
        {
          GroundedExpression* newGrdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
          if (newGrdExpPtr != nullptr)
          {
            auto itSubject = newGrdExpPtr->children.find(GrammaticalType::SUBJECT);
            if (itSubject != newGrdExpPtr->children.end() &&
                SemExpGetter::isACoreference(*itSubject->second, CoreferenceDirectionEnum::BEFORE))
              itSubject->second = pTextProcContext.usSemExp->clone();
          }
        }
        return;
      }

      if (ConceptSet::haveAnyOfConcepts(statementGrd.concepts, {"verb_can", "verb_able"}))
      {
        _applySemExpModifier(pSemExp, pChildTypeToCondition, grdExp, statementGrd,
                             &_addAbilityVerbGoal, pIsPassive);
        return;
      }
    }
  }
}

} // End of namespace semTreeHardCodedConverter
} // End of namespace onsem
