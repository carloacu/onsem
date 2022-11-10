#include "semanticmemoryblockprivate.hpp"
#include <chrono>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/semantictotext/semanticmemory/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/sentencewithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentence.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorybloc.hpp>
#include "semanticmemoryblockbinaryreader.hpp"

namespace onsem
{
namespace
{
const SemanticMemoryGrdExp* _getLastGrdExpPtr(const MemoryGrdExpLinks& pLinks)
{
  if (pLinks.empty())
    return nullptr;
  auto& grdExps = (--pLinks.end())->second;
  if (grdExps.empty())
    return nullptr;
  return &*(--grdExps.end());
}
}


SemanticMemoryBlockPrivate::SemanticMemoryBlockPrivate(SemanticMemoryBlock& pMemBlock)
 : expressionsMemories(),
   trackerKnowledges(),
   expressionsMemoriesPtrToIt(),
   answersLinks(),
   conditionToInformationLinks(),
   sentWithActionLinks(),
   conditionToActions(),
   sentWithInfActionLinks(),
   infActions(),
   subBinaryBlockPtr(),
   userCenteredLinks(),
   nextSemId(0),
   _memBlock(pMemBlock),
   _triggerLinks(),
   _id(std::chrono::duration_cast<std::chrono::duration<int64_t, std::micro>>(
       std::chrono::system_clock::now().time_since_epoch()).count())
{
}


bool SemanticMemoryBlockPrivate::empty() const
{
  return expressionsMemories.empty() &&
      expressionsMemoriesPtrToIt.empty() &&
      answersLinks.empty() &&
      conditionToInformationLinks.empty() &&
      sentWithActionLinks.empty() &&
      conditionToActions.empty() &&
      sentWithInfActionLinks.empty() &&
      infActions.empty() &&
      userCenteredLinks.empty() &&
      _triggerLinks.empty();
}


void SemanticMemoryBlockPrivate::clearLocalInformationButNotTheSubBloc()
{
  expressionsMemories.clear();
  expressionsMemoriesPtrToIt.clear();
  answersLinks.clear();
  conditionToInformationLinks.clear();
  sentWithActionLinks.clear();
  conditionToActions.clear();
  sentWithInfActionLinks.clear();
  infActions.clear();
  userCenteredLinks.clear();
  nextSemId = 0;
  _triggerLinks.clear();
}

void SemanticMemoryBlockPrivate::clear()
{
  clearLocalInformationButNotTheSubBloc();
  subBinaryBlockPtr.reset();
}


SemanticMemoryLinksForAnyVerbTense& SemanticMemoryBlockPrivate::ensureSentenceTriggersLinks(SemanticExpressionCategory pSemExpCategory,
                                                                                            const SemanticTriggerAxiomId& pAxiomId)
{
  switch (pSemExpCategory)
  {
  case SemanticExpressionCategory::AFFIRMATION:
    return _triggerLinks[pAxiomId].affirmationLinks;
  case SemanticExpressionCategory::COMMAND:
    return _triggerLinks[pAxiomId].actionLinks;
  case SemanticExpressionCategory::QUESTION:
    return _triggerLinks[pAxiomId].questionLinks;
  default:
  {
    assert(false);
    return _triggerLinks[pAxiomId].questionLinks;
  }
  }
}


SemanticMemoryLinksForAnyVerbTense* SemanticMemoryBlockPrivate::getSentenceTriggersLinks
(SemanticExpressionCategory pSemExpCategory,
 const SemanticTriggerAxiomId& pAxiomId)
{
  auto it = _triggerLinks.find(pAxiomId);
  if (it != _triggerLinks.end())
  {
    switch (pSemExpCategory)
    {
    case SemanticExpressionCategory::AFFIRMATION:
      return &it->second.affirmationLinks;;
    case SemanticExpressionCategory::COMMAND:
      return &it->second.actionLinks;
    case SemanticExpressionCategory::QUESTION:
      return &it->second.questionLinks;
    default:
    {
      assert(false);
      return nullptr;
    }
    }
  }
  return nullptr;
}


const SemanticMemoryLinksForAnyVerbTense* SemanticMemoryBlockPrivate::getSentenceTriggersLinks
(SemanticExpressionCategory pSemExpCategory,
 const SemanticTriggerAxiomId& pAxiomId) const
{
  auto it = _triggerLinks.find(pAxiomId);
  if (it != _triggerLinks.end())
  {
    switch (pSemExpCategory)
    {
    case SemanticExpressionCategory::AFFIRMATION:
      return &it->second.affirmationLinks;;
    case SemanticExpressionCategory::COMMAND:
      return &it->second.actionLinks;
    case SemanticExpressionCategory::QUESTION:
      return &it->second.questionLinks;
    default:
    {
      assert(false);
      return nullptr;
    }
    }
  }
  return nullptr;
}


SemanticMemoryLinks& SemanticMemoryBlockPrivate::ensureNominalGroupsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId)
{
  return _triggerLinks[pAxiomId].nominalGroupsLinks;
}


SemanticMemoryLinks* SemanticMemoryBlockPrivate::getNominalGroupsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId)
{
  auto it = _triggerLinks.find(pAxiomId);
  if (it != _triggerLinks.end())
    return &it->second.nominalGroupsLinks;
  return nullptr;
}


const SemanticMemoryLinks* SemanticMemoryBlockPrivate::getNominalGroupsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId) const
{
  auto it = _triggerLinks.find(pAxiomId);
  if (it != _triggerLinks.end())
    return &it->second.nominalGroupsLinks;
  return nullptr;
}


std::unique_ptr<SemanticNameGrounding> SemanticMemoryBlockPrivate::getNameGrd
(const std::string& pUserId,
 const SemanticMemoryGrdExp*& pSemMemoryGrdExpPtr) const
{
  const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>* linksPtr = nullptr;
  auto res = userCenteredLinks.getNameGrd(pUserId, linksPtr);
  if (res)
  {
    if (linksPtr != nullptr)
    {
      auto& links = *linksPtr;
      pSemMemoryGrdExpPtr = _getLastGrdExpPtr(links.assertions);
      if (pSemMemoryGrdExpPtr == nullptr)
        pSemMemoryGrdExpPtr = _getLastGrdExpPtr(links.informations);
    }
    return res;
  }
  if (subBinaryBlockPtr)
  {
    res = subBinaryBlockPtr->getNameGrd(pUserId);
    if (res)
      return res;
  }
  if (_memBlock.subBlockPtr != nullptr)
    return _memBlock.subBlockPtr->_impl->getNameGrd(pUserId, pSemMemoryGrdExpPtr);
  return {};
}


void SemanticMemoryBlockPrivate::removeExpression
(MapMemoryPtrToIterator::iterator pItExpMem,
 const linguistics::LinguisticDatabase& pLingDb,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  _memBlock.expressionThatWillBeRemoved(*pItExpMem->first);
  auto itToRemove = pItExpMem->second;
  (*itToRemove)->clearWrappings(_memBlock, pLingDb, pAxiomToConditionCurrentStatePtr);
  _memBlock.expressionRemoved(*pItExpMem->first->semExp);
  expressionsMemories.erase(itToRemove);
  expressionsMemoriesPtrToIt.erase(pItExpMem);
}


void SemanticMemoryBlockPrivate::addLastExpressionToPtrToIteratorMap()
{
  auto& memKnowledge = *expressionsMemories.back();
  expressionsMemoriesPtrToIt.emplace(&memKnowledge, --expressionsMemories.end());
}


void SemanticMemoryBlockPrivate::addConditionToAnAction(const SemanticMemorySentence& pMemSent,
                                                        const SemanticMemoryLinksForAMemSentence& pMemSentLinks)
{
  if (pMemSent.getContextAxiom().isAnActionLinked()) // because it can be only a tracker
  {
    conditionToActions.insert(&pMemSent);
    _memBlock.conditionToActionChanged(conditionToActions);
  }

  std::set<intSemId> beginOfTimes;
  for (auto& currReq : pMemSentLinks.reqToGrdExps)
  {
    currReq.second.timeToSemExps.for_each([&](const std::string& pKey,
                                              const mystd::vector_of_refs<SemanticMemoryGrdExp>& pValue)
    {
      for (const auto& currMemoryGrdExp : pValue)
      {
        auto& memSentId = currMemoryGrdExp.getMemSentence().id;
        if (beginOfTimes.count(memSentId) == 0)
        {
          beginOfTimes.insert(memSentId);
          SemanticDuration duration = SemanticDuration::fromRadixMapStr(pKey);
          _memBlock.semanticTimesWithConditionsLinked(duration);
          break;
        }
      }
    });
  }
}


void SemanticMemoryBlockPrivate::addInfAction(const SemanticMemorySentence& pMemSent)
{
  infActions.emplace(pMemSent.id, &pMemSent);
  _memBlock.infActionAdded(pMemSent.id, &pMemSent);
  _memBlock.infActionChanged(infActions);
}

void SemanticMemoryBlockPrivate::removeInfAction(const SemanticMemorySentence& pMemSent)
{
  infActions.erase(pMemSent.id);
  _memBlock.infActionChanged(infActions);
}


void SemanticMemoryBlockPrivate::removeConditionToAnAction(const SemanticMemorySentence& pMemSent)
{
  if (pMemSent.getContextAxiom().isAnActionLinked()) // because it can be only a tracker
  {
    conditionToActions.erase(&pMemSent);
    _memBlock.conditionToActionChanged(conditionToActions);
  }
}

void SemanticMemoryBlockPrivate::removeLinksIfEmpty(const SemanticTriggerAxiomId& pAxiomId)
{
  auto it = _triggerLinks.find(pAxiomId);
  if (it != _triggerLinks.end() &&
      it->second.empty())
    _triggerLinks.erase(it);
}


RequestToMemoryLinks<true> SemanticMemoryBlockPrivate::getLinks(
    SemanticTypeOfLinks pTypeOfLinks,
    SemanticVerbTense pTense,
    VerbGoalEnum pVerbGoal)
{
  auto* binaryOffset = subBinaryBlockPtr ? subBinaryBlockPtr->getLinks(pTypeOfLinks, pTense, pVerbGoal) : nullptr;
  switch (pTypeOfLinks)
  {
  case SemanticTypeOfLinks::ANSWER:
    break;
  case SemanticTypeOfLinks::CONDITION_INFORMATION:
    return {conditionToInformationLinks.getLinks(pTense, pVerbGoal).reqToGrdExps, binaryOffset};
  case SemanticTypeOfLinks::SENT_WITH_ACTION:
    return {sentWithActionLinks.getLinks(pTense, pVerbGoal).reqToGrdExps, binaryOffset};
  }
  return {answersLinks.getLinks(pTense, pVerbGoal).reqToGrdExps, binaryOffset};
}

RequestToMemoryLinks<false> SemanticMemoryBlockPrivate::getLinks(
    SemanticTypeOfLinks pTypeOfLinks,
    SemanticVerbTense pTense,
    VerbGoalEnum pVerbGoal) const
{
  auto* binaryOffset = subBinaryBlockPtr ? subBinaryBlockPtr->getLinks(pTypeOfLinks, pTense, pVerbGoal) : nullptr;
  switch (pTypeOfLinks)
  {
  case SemanticTypeOfLinks::ANSWER:
    break;
  case SemanticTypeOfLinks::CONDITION_INFORMATION:
    return {conditionToInformationLinks.getLinks(pTense, pVerbGoal).reqToGrdExps, binaryOffset};
  case SemanticTypeOfLinks::SENT_WITH_ACTION:
    return {sentWithActionLinks.getLinks(pTense, pVerbGoal).reqToGrdExps, binaryOffset};
  }
  return {answersLinks.getLinks(pTense, pVerbGoal).reqToGrdExps, binaryOffset};
}


} // End of namespace onsem

