#include <onsem/semantictotext/semanticmemory/semanticmemorybloc.hpp>
#include <sstream>
#include <fstream>
#include <onsem/common/binary/radixmapsaver.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexpsaver.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/texttosemantic/printer/expressionprinter.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/semanticmemory/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticbehaviordefinition.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../utility/semexpcreator.hpp"
#include "semanticmemoryblockprivate.hpp"
#include "semanticmemoryblockbinaryreader.hpp"


namespace onsem
{
namespace
{
static inline void _writeStringPtr(binarymasks::Ptr& pPtr,
                                  const std::string* pStr)
{
  bool hasValue = pStr != nullptr;
  binarysaver::writeBool(pPtr.pchar++, hasValue);
  if (hasValue)
    binarysaver::writeString(pPtr, *pStr);
}
}

const std::size_t SemanticMemoryBlock::infinteMemory = 0;



SemanticMemoryBlock::SemanticMemoryBlock
(std::size_t pMaxNbOfKnowledgesInAMemoryBloc)
  : maxNbOfExpressionsInAMemoryBlock(pMaxNbOfKnowledgesInAMemoryBloc),
    expressionThatWillBeRemoved(),
    semExpAdded(),
    expressionRemoved(),
    infActionAdded(),
    infActionChanged(),
    conditionToActionChanged(),
    subBlockPtr(nullptr),
    semanticTimesWithConditionsLinked(),
    actionProposalSignal(),
    disableOldContrarySentences(true),
    _hardCodedUserIdResolution(),
    _fallbacksBlockPtr(),
    _impl(std::make_unique<SemanticMemoryBlockPrivate>(*this))
{
}

SemanticMemoryBlock::~SemanticMemoryBlock()
{
}


void SemanticMemoryBlock::ensureFallbacksBlock()
{
  if (!_fallbacksBlockPtr)
    _fallbacksBlockPtr = mystd::make_unique_pc<SemanticMemoryBlock>();
}

void SemanticMemoryBlock::_pruneExceedingExpressions(const linguistics::LinguisticDatabase& pLingDb,
                                                     std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  if (maxNbOfExpressionsInAMemoryBlock == infinteMemory)
    return;
  while (_impl->expressionsMemories.size() > maxNbOfExpressionsInAMemoryBlock)
  {
    bool anExpressionasBeenRemoved = false;
    for (auto& currMemoryKnowledge : _impl->expressionsMemories)
    {
      // remove only the knowledges that are not tracked by somebody
      if (currMemoryKnowledge.unique())
      {
        removeExpression(*currMemoryKnowledge, pLingDb, pAxiomToConditionCurrentStatePtr);
        anExpressionasBeenRemoved = true;
        break; // we remove only one element
      }
    }
    if (!anExpressionasBeenRemoved)
      return; // if no element has been removed we get out of the loop
  }
}


void SemanticMemoryBlock::addExpHandleInMemory
(const std::shared_ptr<ExpressionWithLinks>& pNewSemMemKnowledge,
 const linguistics::LinguisticDatabase& pLingDb,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  _impl->expressionsMemories.emplace_back(pNewSemMemKnowledge);
  _impl->addLastExpressionToPtrToIteratorMap();
  semExpAdded(*pNewSemMemKnowledge->semExp);
  _pruneExceedingExpressions(pLingDb, pAxiomToConditionCurrentStatePtr);
}

std::shared_ptr<ExpressionWithLinks> SemanticMemoryBlock::_addRootSemExp
(UniqueSemanticExpression pNewRootSemExp,
 const mystd::radix_map_str<std::string>* pLinkedInfosPtr)
{
  auto expForMemory =
      std::make_shared<ExpressionWithLinks>(*this, std::move(pNewRootSemExp),
                                                   pLinkedInfosPtr);
  _impl->expressionsMemories.emplace_back(expForMemory);
  _impl->addLastExpressionToPtrToIteratorMap();
  semExpAdded(*expForMemory->semExp);
  return expForMemory;
}

std::shared_ptr<ExpressionWithLinks> SemanticMemoryBlock::addRootSemExp
(UniqueSemanticExpression pNewRootSemExp,
 const linguistics::LinguisticDatabase& pLingDb,
 const mystd::radix_map_str<std::string>* pLinkedInfosPtr,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  auto expForMemory = _addRootSemExp(std::move(pNewRootSemExp), pLinkedInfosPtr);
  _pruneExceedingExpressions(pLingDb, pAxiomToConditionCurrentStatePtr);
  return expForMemory;
}

void SemanticMemoryBlock::addTrackerSemExp
(UniqueSemanticExpression pNewRootSemExp,
 std::shared_ptr<SemanticTracker>& pSemTracker,
 const linguistics::LinguisticDatabase& pLingDb)
{
  auto memKnow = std::make_unique<ExpressionWithLinks>(*this,
                                                                std::move(pNewRootSemExp));
  memKnow->addAxiomListToMemory(*memKnow->semExp,
                                &pSemTracker, InformationType::INFORMATION, true, nullptr, nullptr, nullptr, pLingDb);
  _impl->trackerKnowledges.emplace(&pSemTracker, std::move(memKnow));
}

void SemanticMemoryBlock::removeTrackerSemExp
(std::shared_ptr<SemanticTracker>& pSemTracker,
 const linguistics::LinguisticDatabase& pLingDb,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  auto itElt = _impl->trackerKnowledges.find(&pSemTracker);
  if (itElt != _impl->trackerKnowledges.end())
  {
    itElt->second->clearWrappings(*this, pLingDb, pAxiomToConditionCurrentStatePtr);
    _impl->trackerKnowledges.erase(itElt);
  }
}


void SemanticMemoryBlock::addRootSemExpWithHisContent
(InformationType pInformationType,
 UniqueSemanticExpression pNewRootSemExp,
 const linguistics::LinguisticDatabase& pLingDb)
{
  auto expForMem = addRootSemExp(std::move(pNewRootSemExp), pLingDb);
  expForMem->addAxiomListToMemory(*expForMem->semExp, nullptr,
                                  pInformationType, false, nullptr, nullptr, nullptr, pLingDb);
}



bool SemanticMemoryBlock::empty() const
{
  return (!_fallbacksBlockPtr || _fallbacksBlockPtr->empty()) && _impl->empty();
}

void SemanticMemoryBlock::clearLocalInformationButNotTheSubBloc()
{
  if (_fallbacksBlockPtr)
    _fallbacksBlockPtr->clearLocalInformationButNotTheSubBloc();
  _impl->clearLocalInformationButNotTheSubBloc();
}

void SemanticMemoryBlock::clear()
{
  subBlockPtr = nullptr;
  if (_fallbacksBlockPtr)
    _fallbacksBlockPtr->clear();
  _impl->clear();
}


void SemanticMemoryBlock::clearKnowledgeByKnowledgeOnlyForTests(const linguistics::LinguisticDatabase& pLingDb,
                                                                std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  while (!_impl->expressionsMemoriesPtrToIt.empty())
  {
    _impl->removeExpression(_impl->expressionsMemoriesPtrToIt.begin(), pLingDb, pAxiomToConditionCurrentStatePtr);
  }
  assert(empty());
  if (_fallbacksBlockPtr)
    _fallbacksBlockPtr->clearKnowledgeByKnowledgeOnlyForTests(pLingDb, pAxiomToConditionCurrentStatePtr);
}

void SemanticMemoryBlock::removeLinkedActions()
{
  for (auto& currKnowledgeMemory : _impl->expressionsMemories)
    currKnowledgeMemory->removeContextAxiomsWithAnActionLinked();
}


void SemanticMemoryBlock::removeExpression(ExpressionWithLinks& pExpressionToRemove,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  auto itExpMem = _impl->expressionsMemoriesPtrToIt.find(&pExpressionToRemove);
  if (itExpMem != _impl->expressionsMemoriesPtrToIt.end())
    _impl->removeExpression(itExpMem, pLingDb, pAxiomToConditionCurrentStatePtr);
}


std::size_t SemanticMemoryBlock::nbOfKnowledges() const
{
  return _impl->expressionsMemories.size();
}


void SemanticMemoryBlock::print(std::list<SemLineToPrint>& pLines,
                                const mystd::optional<std::size_t>& pMaxNbOfSemanticExpressionsToPrint) const
{
  std::size_t idOfLoop = 1;
  for (auto it = _impl->expressionsMemories.rbegin(); it != _impl->expressionsMemories.rend(); ++it)
  {
    if (pMaxNbOfSemanticExpressionsToPrint)
    {
      if (idOfLoop > *pMaxNbOfSemanticExpressionsToPrint)
        break;
      ++idOfLoop;
    }
    pLines.emplace_front();
    pLines.emplace_front();
    std::list<SemLineToPrint> localLines;
    printer::prettyPrintSemExp(localLines, *(*it)->semExp);
    pLines.splice(pLines.begin(), localLines);
  }

  std::stringstream ss;
  ss << "nb of expressions in memory: "
     << _impl->expressionsMemories.size()
     << " (max: " << maxNbOfExpressionsInAMemoryBlock << ")";
  pLines.emplace_front(0, ss.str());
  pLines.emplace_front(0, "-------------------------------------------------");
  pLines.emplace_front();

  pLines.emplace_back();
  pLines.emplace_back();
}



SemanticGenderType SemanticMemoryBlock::getGender(const std::string& pUser) const
{
  auto res = _impl->userCenteredLinks.getGender(pUser);
  if (res != SemanticGenderType::UNKNOWN)
    return res;
  if (_impl->subBinaryBlockPtr)
  {
    res = _impl->subBinaryBlockPtr->getGender(pUser);
    if (res != SemanticGenderType::UNKNOWN)
      return res;
  }
  return subBlockPtr ? subBlockPtr->getGender(pUser) : SemanticGenderType::UNKNOWN;
}

std::string SemanticMemoryBlock::getName(const std::string& pUser) const
{
  std::string res = _impl->userCenteredLinks.getName(pUser);
  if (res.empty())
  {
    if (_impl->subBinaryBlockPtr)
      res = _impl->subBinaryBlockPtr->getName(pUser);
    if (subBlockPtr != nullptr && res.empty())
      return subBlockPtr->getName(pUser);
  }
  return res;
}



std::unique_ptr<GroundedExpressionContainer> SemanticMemoryBlock::getEquivalentGrdExpPtr(const std::string& pUserId,
                                                                                         const linguistics::LinguisticDatabase& pLingDb) const
{
  auto res = _impl->userCenteredLinks.getEquivalentGrdExpPtr(pUserId);
  if (!res)
  {
    if (_impl->subBinaryBlockPtr)
      return _impl->subBinaryBlockPtr->getEquivalentGrdExpPtr(pUserId, pLingDb);
    if (!res && subBlockPtr != nullptr)
      return subBlockPtr->getEquivalentGrdExpPtr(pUserId, pLingDb);
  }
  return res;
}


void SemanticMemoryBlock::getAllEquivalentUserIds(std::list<std::string>& pRes,
                                                  const std::string& pUser) const
{
  pRes.emplace_back(pUser);
  _impl->userCenteredLinks.getAllEquivalentUserIds(pRes, pUser);
  if (_impl->subBinaryBlockPtr)
    _impl->subBinaryBlockPtr->getAllEquivalentUserIds(pRes, pUser);
  if (subBlockPtr)
    subBlockPtr->getAllEquivalentUserIds(pRes, pUser);
}


bool SemanticMemoryBlock::_userIdToHardCodedUserId(std::string& pHardCodedUserId,
                                                   const std::string& pUserId) const
{
  auto* res = _hardCodedUserIdResolution.find_ptr(pUserId);
  if (res != nullptr)
  {
    pHardCodedUserId = *res;
    return true;
  }
  if (_impl->subBinaryBlockPtr)
  {
    pHardCodedUserId = _impl->subBinaryBlockPtr->userIdToHardCodedUserIdsPtr(pUserId);
    if (!pHardCodedUserId.empty())
      return true;
  }
  if (subBlockPtr)
    return subBlockPtr->_userIdToHardCodedUserId(pHardCodedUserId, pUserId);
  return false;
}


void SemanticMemoryBlock::_nameToUserIds(std::set<std::string>& pUserIds,
                                         const std::string& pName) const
{
  auto addToLocalUserIds = [&](const std::string& pUserId) { pUserIds.insert(pUserId); };
  _impl->userCenteredLinks.iterateOnUserIdLinkedToAName(pName, addToLocalUserIds);
  if (_impl->subBinaryBlockPtr)
    _impl->subBinaryBlockPtr->iterateOnUserIdLinkedToAName(pName, addToLocalUserIds);
  if (subBlockPtr)
    subBlockPtr->_nameToUserIds(pUserIds, pName);
}


std::string SemanticMemoryBlock::getUserId(const std::vector<std::string>& pNames) const
{
  const std::string rootNewUserIdNameCandidate = SemanticAgentGrounding::namesToUserId(pNames);
  std::string hardCodedUserId;
  if (_userIdToHardCodedUserId(hardCodedUserId, rootNewUserIdNameCandidate))
    return hardCodedUserId;

  std::set<std::string> userIds;
  bool firstLoop = true;
  for (const auto& currName : pNames)
  {
    std::set<std::string> localUserIds;
    _nameToUserIds(localUserIds, currName);
    if (!localUserIds.empty())
    {
      if (firstLoop)
      {
        userIds = std::move(localUserIds);
        firstLoop = false;
      }
      else
      {
        // userIds <-- userIds UNION localUserIds
        for (auto itUserIds = userIds.begin(); itUserIds != userIds.end(); )
        {
          if (localUserIds.count(*itUserIds) == 0)
            itUserIds = userIds.erase(itUserIds);
          else
            ++itUserIds;
        }
      }
    }
    else
    {
      userIds.clear();
      break;
    }
  }

  // return the smaller userId (because smaller is it, closer it is from the names list)
  {
    std::size_t minSize = 0;
    const std::string* resPtr = nullptr;
    for (const auto& currUserId : userIds)
    {
      std::size_t currSize = currUserId.size();
      if (resPtr == nullptr || currSize < minSize)
      {
        minSize = currSize;
        resPtr = &currUserId;
      }
    }
    if (resPtr != nullptr)
      return *resPtr;
  }
  return _generateNewUserId(rootNewUserIdNameCandidate);
}


std::string SemanticMemoryBlock::getUserIdFromGrdExp(const GroundedExpression& pGrdExp,
                                                     const linguistics::LinguisticDatabase& pLingDb) const
{
  auto res = _impl->userCenteredLinks.getUserIdFromGrdExp(pGrdExp, *this, pLingDb);
  if (!res.empty())
    return res;
  if (_impl->subBinaryBlockPtr)
  {
    res = _impl->subBinaryBlockPtr->getUserIdFromGrdExp(pGrdExp, *this, pLingDb);
    if (!res.empty())
      return res;
  }
  return subBlockPtr ? subBlockPtr->getUserIdFromGrdExp(pGrdExp, pLingDb) : SemanticAgentGrounding::userNotIdentified;
}

bool SemanticMemoryBlock::areSameUser(const std::string& pUserId1,
                                      const std::string& pUserId2,
                                      RelatedContextAxiom& pRelContextAxiom)
{
  if (pUserId1 == pUserId2)
    return true;
  SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>* linksPtr;
  if (_impl->userCenteredLinks.areSameUser(pUserId1, pUserId2, linksPtr) ||
      (_impl->subBinaryBlockPtr && _impl->subBinaryBlockPtr->areSameUser(pUserId1, pUserId2)))
  {
    if (linksPtr != nullptr)
    {
      for (auto& currSemMemGrdExp : linksPtr->assertions)
        for (auto& currSemId : currSemMemGrdExp.second)
          pRelContextAxiom.elts.emplace_back(&currSemId.getMemSentence().getContextAxiom());
      for (auto& currSemMemGrdExp : linksPtr->informations)
        for (auto& currSemId : currSemMemGrdExp.second)
          pRelContextAxiom.elts.emplace_back(&currSemId.getMemSentence().getContextAxiom());
    }
    return true;
  }
  return subBlockPtr ? subBlockPtr->areSameUserConst(pUserId1, pUserId2, &pRelContextAxiom) : false;
}


bool SemanticMemoryBlock::areSameUserConst(const std::string& pUserId1,
                                           const std::string& pUserId2,
                                           RelatedContextAxiom* pRelContextAxiomPtr) const
{
  if (pUserId1 == pUserId2)
    return true;
  const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>* linksPtr;
  if (_impl->userCenteredLinks.areSameUser(pUserId1, pUserId2, linksPtr) ||
      (_impl->subBinaryBlockPtr && _impl->subBinaryBlockPtr->areSameUser(pUserId1, pUserId2)))
  {
    if (linksPtr != nullptr &&
        pRelContextAxiomPtr != nullptr)
    {
      for (auto& currSemMemGrdExp : linksPtr->assertions)
        for (auto& currSemId : currSemMemGrdExp.second)
          pRelContextAxiomPtr->constElts.emplace_back(&currSemId.getMemSentence().getContextAxiom());
      for (auto& currSemMemGrdExp : linksPtr->informations)
        for (auto& currSemId : currSemMemGrdExp.second)
          pRelContextAxiomPtr->constElts.emplace_back(&currSemId.getMemSentence().getContextAxiom());
    }
    return true;
  }
  return subBlockPtr ? subBlockPtr->areSameUserConst(pUserId1, pUserId2, pRelContextAxiomPtr) : false;
}


bool SemanticMemoryBlock::isItMe(const std::string& pUserId) const
{
  return areSameUserConst(pUserId, SemanticAgentGrounding::me) ||
      (subBlockPtr && subBlockPtr->isItMe(pUserId));
}


bool SemanticMemoryBlock::_doesUserIdExist(const std::string& pUserId) const
{
  bool res = _impl->userCenteredLinks.doesUserIdExist(pUserId) ||
      (_impl->subBinaryBlockPtr &&
       _impl->subBinaryBlockPtr->doesUserIdExist(pUserId));
  if (res)
    return true;
  return subBlockPtr ? subBlockPtr->_doesUserIdExist(pUserId) : false;
}

std::string SemanticMemoryBlock::_generateNewUserId(const std::string& pRootNewUserIdNameCandidate) const
{
  std::string newUserIdNameCandidate = pRootNewUserIdNameCandidate;
  std::size_t idIteraction = 1;
  while (_doesUserIdExist(newUserIdNameCandidate))
  {
    std::stringstream ss;
    ss << ++idIteraction;
    newUserIdNameCandidate = pRootNewUserIdNameCandidate + "-" + ss.str();
  }
  return newUserIdNameCandidate;
}


std::unique_ptr<SemanticAgentGrounding> SemanticMemoryBlock::generateNewAgentGrd(const std::string& pName,
                                                                                 SemanticLanguageEnum pLanguage,
                                                                                 const linguistics::LinguisticDatabase& pLingDb)
{
  std::vector<std::string> names;
  linguistics::extractProperNouns(names, pName, pLanguage, pLingDb);
  return std::make_unique<SemanticAgentGrounding>(names);
}


std::list<const SemanticExpression*> SemanticMemoryBlock::getSemExps() const
{
  std::list<const SemanticExpression*> res;
  for (const auto& currExpMem : _impl->expressionsMemories)
    res.emplace_back(&*currExpMem->semExp);
  return res;
}


const std::map<intSemId, const SemanticMemorySentence*>& SemanticMemoryBlock::getInfActions() const
{
  return _impl->infActions;
}

const std::set<const SemanticMemorySentence*>& SemanticMemoryBlock::getConditionToActions() const
{
  return _impl->conditionToActions;
}


SemanticBehaviorDefinition SemanticMemoryBlock::extractActionFromMemorySentence(
    const SemanticMemorySentence& pMemorySentence)
{
  if (pMemorySentence.getContextAxiom().infCommandToDo != nullptr)
  {
    bool isAComposedAction = !SemExpGetter::hasGroundingType(*pMemorySentence.getContextAxiom().infCommandToDo,
                                                             {SemanticGroundingType::RESOURCE, SemanticGroundingType::META});
    if (isAComposedAction)
      return SemanticBehaviorDefinition(pMemorySentence.grdExp.clone(), pMemorySentence.getContextAxiom().infCommandToDo->clone());
    return SemanticBehaviorDefinition(pMemorySentence.grdExp.clone());
  }
  else
  {
    assert(false);
  }
  return SemanticBehaviorDefinition();
}

void SemanticMemoryBlock::extractActions(std::list<SemanticBehaviorDefinition>& pActionDefinitions,
                                         const std::map<intSemId, const SemanticMemorySentence*>& pInfActions)
{
  for (const auto& currElt : pInfActions)
    pActionDefinitions.push_back(extractActionFromMemorySentence(*currElt.second));
}


void SemanticMemoryBlock::extractConditionToActions(
    std::list<UniqueSemanticExpression>& pConditionToActionsSemExp,
    const std::set<const SemanticMemorySentence*>& pConditionToActionsMemSent)
{
  for (const SemanticMemorySentence* currConditionToActionsMemSent : pConditionToActionsMemSent)
  {
    auto& contextAxiom = currConditionToActionsMemSent->getContextAxiom();
    if (contextAxiom.semExpToDo != nullptr)
    {
      pConditionToActionsSemExp.emplace_back(
            SemExpCreator::formulateConditionToAction(
              currConditionToActionsMemSent->grdExp,
              *contextAxiom.semExpToDo, contextAxiom.semExpToDoElse,
              contextAxiom.semExpToDoIsAlwaysActive));
    }
    else
    {
      assert(false);
    }
  }
}


void SemanticMemoryBlock::writeInBinaryFile(
    const linguistics::LinguisticDatabase& pLingDb,
    const std::string& pFilename,
    const std::size_t pAllocSize) const
{
  //const std::size_t maxSize = 3000000000;
  binarymasks::Ptr mem = ::operator new(pAllocSize);
  binarymasks::Ptr beginPtr = mem;

  _writeInBinary(mem, pLingDb);
  std::ofstream outfile(pFilename, std::ofstream::binary);
  std::size_t memorySize = mem.val - beginPtr.val;
  outfile.write(reinterpret_cast<const char*>(beginPtr.ptr), memorySize);
  outfile.close();

  ::operator delete(beginPtr.pchar);
}


void SemanticMemoryBlock::_writeInBinary(binarymasks::Ptr& pPtr,
                                         const linguistics::LinguisticDatabase& pLingDb) const
{
  binarysaver::writeInt64(pPtr.puint64++, _impl->getId()); // memory bloc id
  binarymasks::Ptr headerAfterIdPtr = pPtr;
  ++pPtr.pint; // expression memory size
  ++pPtr.pint; // memory links size
  ++pPtr.pint; // user links size
  ++pPtr.pint; // user hard coded user links size
  binarymasks::Ptr beginOfSemExpPtr = pPtr;

  MemGrdExpPtrOffsets memGrdExpPtrOffsets(pPtr.puchar);
  semexpsaver::SemExpPtrOffsets semExpPtrOffsets(pPtr.puchar);
  for (const auto& expMem : _impl->expressionsMemories)
  {
    pPtr = radixmap::write<std::string>(expMem->linkedInfos,
                                        pPtr, &_writeStringPtr);
    semExpPtrOffsets.clearSemExps();
    semexpsaver::writeSemExp(pPtr, *expMem->semExp, pLingDb, &semExpPtrOffsets);
    for (const auto& currContextAxiom : expMem->contextAxioms)
    {
      binarysaver::writeBool(pPtr.pchar++, currContextAxiom.memorySentences.and_or);
      for (const SemanticMemorySentence& currMemSent : currContextAxiom.memorySentences.elts)
        currMemSent.writeInBinary(pPtr, memGrdExpPtrOffsets, semExpPtrOffsets);
    }
  }
  pPtr = binarysaver::alignMemory(pPtr);
  uint32_t expMemorySize = pPtr.val - beginOfSemExpPtr.val;
  binarysaver::writeInt(headerAfterIdPtr.pint++, expMemorySize);

  {
    binarymasks::Ptr beginOfLinksPtr = pPtr;
    _impl->answersLinks.writeInBinary(pPtr, memGrdExpPtrOffsets, pLingDb);
    _impl->conditionToInformationLinks.writeInBinary(pPtr, memGrdExpPtrOffsets, pLingDb);
    _impl->sentWithActionLinks.writeInBinary(pPtr, memGrdExpPtrOffsets, pLingDb);
    pPtr = binarysaver::alignMemory(pPtr);

    uint32_t memoryLinkSize = pPtr.val - beginOfLinksPtr.val;
    binarysaver::writeInt(headerAfterIdPtr.pint++, memoryLinkSize);
  }

  {
    binarymasks::Ptr beginOfUserLinksPtr = pPtr;
    writeUserCenteredLinksInBinary(pPtr, _impl->userCenteredLinks,
                                   memGrdExpPtrOffsets, semExpPtrOffsets, pLingDb);
    pPtr = binarysaver::alignMemory(pPtr);

    uint32_t userLinkSize = pPtr.val - beginOfUserLinksPtr.val;
    binarysaver::writeInt(headerAfterIdPtr.pint++, userLinkSize);
  }

  {
    binarymasks::Ptr beginOfHardCodedUserLinksPtr = pPtr;
    pPtr = radixmap::write<std::string>(_hardCodedUserIdResolution,
                                              pPtr, [] (binarymasks::Ptr& pSubPtr,
                                                        const std::string* pStrPtr)
                {
                  bool hasValue = pStrPtr != nullptr;
                  binarysaver::writeBool(pSubPtr.pchar++, hasValue);
                  if (hasValue)
                    binarysaver::writeString(pSubPtr, *pStrPtr);
                });
    pPtr = binarysaver::alignMemory(pPtr);

    uint32_t hardCodedUserIds = pPtr.val - beginOfHardCodedUserLinksPtr.val;
    binarysaver::writeInt(headerAfterIdPtr.pint++, hardCodedUserIds);
  }
}


void SemanticMemoryBlock::loadBinaryFile(const std::string& pPath)
{
  _impl->subBinaryBlockPtr = SemanticMemoryBlockBinaryReader::getInstance(pPath);
}


void SemanticMemoryBlock::addASetOfEquivalentNames(const std::vector<std::string>& pNames,
                                                   SemanticLanguageEnum pLanguage,
                                                   const linguistics::LinguisticDatabase& pLingDb)
{
  std::string refUserID;
  for (const auto& currName : pNames)
  {
    std::vector<std::string> properNouns;
    linguistics::extractProperNouns(properNouns, currName, pLanguage, pLingDb);
    if (properNouns.empty())
      properNouns.emplace_back(currName);
    auto currNameId = SemanticAgentGrounding::namesToUserId(properNouns);
    if (refUserID.empty())
      refUserID = currNameId;
    _hardCodedUserIdResolution.emplace(currNameId, refUserID);
  }
}

const SemanticExpression* SemanticMemoryBlock::getLastSemExpFromContext() const
{
  auto itExpMemory = _impl->expressionsMemories.end();
  while (itExpMemory != _impl->expressionsMemories.begin())
  {
    --itExpMemory;
    const auto& currSemExp = *(*itExpMemory)->semExp;
    if (SemExpGetter::doesContainADialogSource(currSemExp))
      return &currSemExp;
  }
  return nullptr;
}


const SemanticExpression* SemanticMemoryBlock::getBeforeLastSemExpOfAnAuthor(const SemanticExpression& pAuthor,
                                                                             const linguistics::LinguisticDatabase& pLingDb) const
{
  const SemanticExpression* lastPtr = nullptr;
  auto itExpMemory = _impl->expressionsMemories.end();
  while (itExpMemory != _impl->expressionsMemories.begin())
  {
    --itExpMemory;
    const auto& currSemExp = *(*itExpMemory)->semExp;
    if (SemExpGetter::doesContainADialogSource(currSemExp))
    {
      if (lastPtr == nullptr)
        lastPtr = &currSemExp;
      else
      {
        const SemanticExpression* contextAuthorPtr = SemExpGetter::extractAuthorSemExp(currSemExp);
        if (contextAuthorPtr != nullptr &&
            SemExpComparator::semExpsAreEqualFromMemBlock(pAuthor, *contextAuthorPtr,
                                                          *this, pLingDb, nullptr))
          return &currSemExp;
      }
    }
  }
  return nullptr;
}


const SemanticExpression* SemanticMemoryBlock::getBeforeLastSemExpOfNotAnAuthor(const SemanticExpression& pAuthor,
                                                                                const linguistics::LinguisticDatabase& pLingDb) const
{
  std::size_t nbOfLastMemories = 0;
  auto itExpMemory = _impl->expressionsMemories.end();
  while (itExpMemory != _impl->expressionsMemories.begin())
  {
    --itExpMemory;
    const auto& currSemExp = *(*itExpMemory)->semExp;
    if (SemExpGetter::doesContainADialogSource(currSemExp))
    {
      if (nbOfLastMemories < 2)
        ++nbOfLastMemories;
      else
      {
        const SemanticExpression* contextAuthorPtr = SemExpGetter::extractAuthorSemExp(currSemExp);
        if (contextAuthorPtr != nullptr &&
            !SemExpComparator::semExpsAreEqualFromMemBlock(pAuthor, *contextAuthorPtr,
                                                           *this, pLingDb, nullptr))
          return &currSemExp;
      }
    }
  }
  return nullptr;
}


void SemanticMemoryBlock::copySemExps(std::list<UniqueSemanticExpression>& pCopiedSemExps) const
{
  for (const auto& currKnowledge : _impl->expressionsMemories)
    pCopiedSemExps.push_back(currKnowledge->semExp->clone());
  if (_fallbacksBlockPtr)
    _fallbacksBlockPtr->copySemExps(pCopiedSemExps);
}


intSemId SemanticMemoryBlock::getNextSemId()
{
  return ++_impl->nextSemId;
}


const std::list<std::shared_ptr<ExpressionWithLinks>>& SemanticMemoryBlock::getExpressionHandleInMemories() const
{
  return _impl->expressionsMemories;
}


std::list<std::shared_ptr<ExpressionWithLinks>>& SemanticMemoryBlock::getExpressionHandleInMemories()
{
  return _impl->expressionsMemories;
}


} // End of namespace onsem

