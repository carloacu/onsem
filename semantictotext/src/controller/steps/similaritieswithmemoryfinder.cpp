#include "similaritieswithmemoryfinder.hpp"
#include <onsem/common/utility/container.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/feedbackexpression.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../type/semanticdetailledanswer.hpp"
#include "../../type/answerexp.hpp"
#include "../../type/referencesfiller.hpp"
#include "../../utility/semexpcreator.hpp"
#include "semanticmemorylinker.hpp"


namespace onsem
{
namespace similaritesWithMemoryFinder
{
namespace
{

struct SimilaritiesAnswers
{
  SimilaritiesAnswers(SemanticVerbTense pInputVerbTense)
    : answers(),
      inputVerbTense(pInputVerbTense),
      verbTense()
  {
  }

  std::list<const AnswerExp*> answers;
  SemanticVerbTense inputVerbTense;
  SemanticVerbTense verbTense;
};


void _filterRevelantSimilarities(GroundedExpression& pGrdExp,
                                 std::list<const AnswerExp*>& pAnswers,
                                 SemControllerWorkingStruct& pWorkStruct,
                                 SemanticMemoryBlockViewer& pMemViewer,
                                 const SemanticRequests& pRequests)
{
  semanticMemoryLinker::satisfyAQuestion(pWorkStruct, pMemViewer, pGrdExp, {}, pGrdExp, pRequests);
  pWorkStruct.getAnswersForRequest(pAnswers, pRequests);
  for (auto itAnsw = pAnswers.begin(); itAnsw != pAnswers.end(); )
  {
    const auto& currAnswer = **itAnsw;
    // Don't react if the answer come from our own semantic expression
    if (pWorkStruct.expHandleInMemory != nullptr &&
        currAnswer.relatedContextAxioms.haveThisExpHandleInMemory(pWorkStruct.expHandleInMemory))
    {
      itAnsw = pAnswers.erase(itAnsw);
      continue;
    }

    const GroundedExpression& grdExp = currAnswer.getGrdExp();
    switch (grdExp->type)
    {
    case SemanticGroudingType::AGENT:
    {
      // remove if it's not a specific user
      const auto& agentGrd = grdExp->getAgentGrounding();
      if (!agentGrd.isSpecificUser())
      {
        itAnsw = pAnswers.erase(itAnsw);
        continue;
      }
      break;
    }
    case SemanticGroudingType::GENERIC:
    {
      const auto& genGrd = grdExp->getGenericGrounding();
      if (genGrd.quantity.isEqualToZero())
      {
        itAnsw = pAnswers.erase(itAnsw);
        continue;
      }
      break;
    }
    case SemanticGroudingType::STATEMENT:
    {
      itAnsw = pAnswers.erase(itAnsw);
      continue;
    }
    default:
      break;
    }
    ++itAnsw;
  }
  keepOnlyTheFirstElements(pAnswers, 3);
}


void _filterRevelantSimilaritiesForDifferentTimePrecision(GroundedExpression& pGrdExp,
                                                          std::list<const AnswerExp*>& pAnswers,
                                                          SemControllerWorkingStruct& pWorkStruct,
                                                          SemanticMemoryBlockViewer& pMemViewer,
                                                          const SemanticRequests& pRequests)
{
  _filterRevelantSimilarities(pGrdExp, pAnswers, pWorkStruct, pMemViewer, pRequests);
  if (!pAnswers.empty() ||
      pRequests.has(SemanticRequestType::TIME))
    return;
  auto itTimeChild = pGrdExp.children.find(GrammaticalType::TIME);
  if (itTimeChild == pGrdExp.children.end())
    return;
  // time without year
  auto timeChildCloned = itTimeChild->second->clone();
  bool haveYearInfo = SemExpModifier::removeYearInformation(*itTimeChild->second);
  if (haveYearInfo)
    _filterRevelantSimilarities(pGrdExp, pAnswers, pWorkStruct, pMemViewer, pRequests);
  if (!pAnswers.empty())
    return;
  auto timeWithoutYear = std::move(itTimeChild->second);
  itTimeChild->second = timeChildCloned->clone();

  // time without day
  bool haveDayInfo = SemExpModifier::removeDayInformation(*itTimeChild->second);
  if (haveDayInfo)
    _filterRevelantSimilarities(pGrdExp, pAnswers, pWorkStruct, pMemViewer, pRequests);
  if (!pAnswers.empty())
    return;

  // time without year and day
  if (haveYearInfo && haveDayInfo &&
      SemExpModifier::removeDayInformation(*timeWithoutYear))
  {
    itTimeChild->second = std::move(timeWithoutYear);
    _filterRevelantSimilarities(pGrdExp, pAnswers, pWorkStruct, pMemViewer, pRequests);
    if (!pAnswers.empty())
      return;
  }
  itTimeChild->second = std::move(timeChildCloned);
}


void _getRevelantSimilaritiesForDifferentTense(GroundedExpression& pGrdExp,
                                               SimilaritiesAnswers& pAnswers,
                                               SemControllerWorkingStruct& pWorkStruct,
                                               SemanticMemoryBlockViewer& pMemViewer,
                                               SemanticRequestType pRequest)
{
  _filterRevelantSimilaritiesForDifferentTimePrecision(pGrdExp, pAnswers.answers,
                                                       pWorkStruct, pMemViewer, pRequest);
  if (pAnswers.answers.empty())
  {
    if (isAPresentTense(pAnswers.inputVerbTense))
    {
      pAnswers.verbTense = SemanticVerbTense::PUNCTUALPAST;
      SemExpModifier::modifyVerbTense(pGrdExp, pAnswers.verbTense);
      _filterRevelantSimilaritiesForDifferentTimePrecision(pGrdExp, pAnswers.answers,
                                                           pWorkStruct, pMemViewer, pRequest);
      SemExpModifier::modifyVerbTense(pGrdExp, pAnswers.inputVerbTense);
    }
  }
  else
  {
    pAnswers.verbTense = pAnswers.inputVerbTense;
  }
}



bool _reactToSimilarAnswers(SemControllerWorkingStruct& pWorkStruct,
                            const SemanticMemoryBlockViewer& pMemViewer,
                            std::unique_ptr<GroundedExpression> pNewRootExp,
                            const GroundedExpression& pGrdExp,
                            const std::list<const AnswerExp*>& pAnswers,
                            GrammaticalType pChildGrammaticalType,
                            GrammaticalType pOtherChildGrammaticalType,
                            const GroundedExpression& pSubjectGrd)
{
  std::list<std::string> references;
  SemExpModifier::clearRequestList(*pNewRootExp);
  std::size_t numberOfAnswers = 0;
  for (const auto& currAnsw : pAnswers)
  {
    if (!SemExpComparator::grdExpsReferToSameInstance(currAnsw->getGrdExp(), pSubjectGrd, pMemViewer.constView, pWorkStruct.lingDb))
    {
      currAnsw->relatedContextAxioms.getReferences(references);
      auto answerClonedGrdExp = currAnsw->getGrdExp().clone();
      SemExpModifier::removeSpecificationsNotNecessaryForAnAnswer(*answerClonedGrdExp);
      SemExpModifier::addChild(*pNewRootExp, pChildGrammaticalType,
                               std::move(answerClonedGrdExp),
                               ListExpressionType::AND);
      ++numberOfAnswers;
    }
  }
  if (numberOfAnswers == 0)
    return false;

  if (pOtherChildGrammaticalType != GrammaticalType::UNKNOWN &&
      numberOfAnswers > 1)
  {
    auto* nexExpStatementPtr = pNewRootExp->grounding().getStatementGroundingPtr();
    if (nexExpStatementPtr != nullptr &&
        ConceptSet::haveAConceptThatBeginWith(nexExpStatementPtr->concepts, "verb_equal_"))
    {
      auto itOtherChild = pNewRootExp->children.find(pOtherChildGrammaticalType);
      if (itOtherChild != pNewRootExp->children.end())
        SemExpModifier::setAtPluralFromSemExp(*itOtherChild->second);
    }
  }

  SemExpModifier::addChild(*pNewRootExp, GrammaticalType::SPECIFIER,
                           SemExpCreator::sayAlso(), ListExpressionType::UNRELATED, false, false);
  pWorkStruct.addAnswer(ContextualAnnotation::FEEDBACK, std::move(pNewRootExp), ReferencesFiller(references));
  return true;
}


bool _reactOnChildSimilarities(SemControllerWorkingStruct& pWorkStruct,
                               SemanticMemoryBlockViewer& pMemViewer,
                               const GroundedExpression& pGrdExp,
                               SemanticRequestType pRequest)
{
  GrammaticalType childGramType = semanticRequestType_toSemGram(pRequest);

  // grdExp has to have the child
  auto itChild = pGrdExp.children.find(childGramType);
  if (itChild == pGrdExp.children.end())
    return false;

  GrammaticalType otherChildGrammaticalType = GrammaticalType::UNKNOWN;
  if (childGramType == GrammaticalType::OBJECT)
    otherChildGrammaticalType = GrammaticalType::SUBJECT;
  else if (childGramType == GrammaticalType::SUBJECT)
    otherChildGrammaticalType = GrammaticalType::OBJECT;
  if (otherChildGrammaticalType != GrammaticalType::UNKNOWN)
  {
    auto itGrdExpOtherChild = pGrdExp.children.find(otherChildGrammaticalType);
    if (itGrdExpOtherChild != pGrdExp.children.end())
    {
      if (SemExpGetter::isAnythingFromSemExp(*itGrdExpOtherChild->second))
          return false;
    }
    else if (pGrdExp.children.count(GrammaticalType::LOCATION) == 0 &&
             pGrdExp.children.count(GrammaticalType::TIME) == 0)
    {
      return false;
    }
  }

  // deal only with simple subjects that just contain a grounding
  const GroundedExpression* childGrdExpPtr = itChild->second->getGrdExpPtr_SkipWrapperPtrs();
  if (childGrdExpPtr == nullptr)
    return false;
  auto& childGrdExp = *childGrdExpPtr;
  switch (childGrdExp->type)
  {
  case SemanticGroudingType::AGENT:
  case SemanticGroudingType::NAME:
  {
    break;
  }
  case SemanticGroudingType::GENERIC:
  {
    const auto& childGenGrd = childGrdExp->getGenericGrounding();
    if (childGenGrd.coreference || childGenGrd.quantity.isEqualToZero() ||
        childGenGrd.referenceType != SemanticReferenceType::DEFINITE ||
        !ConceptSet::haveAConceptThatBeginWith(childGenGrd.concepts, "agent_"))
      return false;
    break;
  }
  default:
    return false;
  }

  // recussive loop (to avoid a potential infinite loop, check that we didn't already do too many recurssive loops)
  SemControllerWorkingStruct subWorkStruct(pWorkStruct);
  if (subWorkStruct.askForNewRecursion())
  {
    // ask about the subject in a new semExp
    auto newRootExp = pGrdExp.clone();
    SemExpModifier::addRequest(*newRootExp, pRequest);
    SemExpModifier::removeChild(*newRootExp, childGramType);

    // the subjects from memory
    subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
    SimilaritiesAnswers allAnswers(SemExpGetter::getVerbTense(*newRootExp));
    _getRevelantSimilaritiesForDifferentTense(*newRootExp, allAnswers, subWorkStruct, pMemViewer, pRequest);

    if (allAnswers.answers.empty())
    {
      if (newRootExp->children.count(GrammaticalType::LOCATION) != 0 &&
          newRootExp->children.count(GrammaticalType::TIME) != 0)
      {
        SimilaritiesAnswers locationAnswers(allAnswers.inputVerbTense);
        auto timeChild = std::move(newRootExp->children[GrammaticalType::TIME]);
        newRootExp->children.erase(GrammaticalType::TIME);
        _getRevelantSimilaritiesForDifferentTense(*newRootExp, locationAnswers, subWorkStruct, pMemViewer, pRequest);
        if (!locationAnswers.answers.empty())
        {
          allAnswers = std::move(locationAnswers);
        }
        else
        {
          newRootExp->children.erase(GrammaticalType::LOCATION);
          newRootExp->children.emplace(GrammaticalType::TIME, std::move(timeChild));
          _getRevelantSimilaritiesForDifferentTense(*newRootExp, allAnswers, subWorkStruct, pMemViewer, pRequest);
        }
      }
    }

    if (!allAnswers.answers.empty())
    {
      if (allAnswers.inputVerbTense != allAnswers.verbTense)
        SemExpModifier::modifyVerbTense(*newRootExp, allAnswers.verbTense);
      // react according to the answers found
      return _reactToSimilarAnswers(pWorkStruct, pMemViewer, std::move(newRootExp),
                                    pGrdExp, allAnswers.answers, childGramType,
                                    otherChildGrammaticalType, childGrdExp);
    }
  }
  return false;
}

}



bool reactOnSimilarities(SemControllerWorkingStruct& pWorkStruct,
                         SemanticMemoryBlockViewer& pMemViewer,
                         const GroundedExpression& pGrdExp)
{
  return _reactOnChildSimilarities(pWorkStruct, pMemViewer, pGrdExp, SemanticRequestType::SUBJECT) ||
      _reactOnChildSimilarities(pWorkStruct, pMemViewer, pGrdExp, SemanticRequestType::OBJECT);
}



} // End of namespace similaritesWithMemoryFinder
} // End of namespace onsem
