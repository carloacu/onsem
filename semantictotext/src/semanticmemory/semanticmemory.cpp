#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>


namespace onsem
{

SemanticMemory::SemanticMemory()
  : memBloc(10000),
    defaultLanguage(SemanticLanguageEnum::ENGLISH),
    callbackToSentencesCanBeAnswered(),
    proativeSpecifications(),
    _newUserFocusedToSemExps(),
    _currUserId(SemanticAgentGrounding::currentUser),
    _externalFallback()
{
  memBloc.ensureFallbacksBlock();
}


void SemanticMemory::clearLocalInformationButNotTheSubBloc()
{
  memBloc.clearLocalInformationButNotTheSubBloc();
  _currUserId = SemanticAgentGrounding::currentUser;
}


void SemanticMemory::clear()
{
  memBloc.clear();
  callbackToSentencesCanBeAnswered.clear();
  _externalFallback.reset();

  _currUserId = SemanticAgentGrounding::currentUser;
}


void SemanticMemory::copySemExps(std::list<UniqueSemanticExpression>& pCopiedSemExps)
{
  memBloc.copySemExps(pCopiedSemExps);
}


void SemanticMemory::addNewUserFocusedToSemExp
(UniqueSemanticExpression pSemExp,
 const std::string& pUserId)
{
  _newUserFocusedToSemExps[pUserId].emplace_back(std::move(pSemExp));
}


void SemanticMemory::extractSemExpsForASpecificUser
(std::list<UniqueSemanticExpression>& pSemExps,
 const std::string& pUserId)
{
  std::list<std::string> userIds;
  memBloc.getAllEquivalentUserIds(userIds, pUserId);
  for (const auto& currUserId : userIds)
  {
    auto itCurrUser = _newUserFocusedToSemExps.find(currUserId);
    if (itCurrUser != _newUserFocusedToSemExps.end())
    {
      for (auto& currKnowledge : itCurrUser->second)
        pSemExps.push_back(std::move(currKnowledge));
      _newUserFocusedToSemExps.erase(itCurrUser);
    }
  }
}


void SemanticMemory::registerExternalFallback
(std::unique_ptr<ExternalFallback> pExtFallback)
{
  _externalFallback = std::move(pExtFallback);
}


void SemanticMemory::setCurrUserId
(const std::string& pNewUserId)
{
  _currUserId = pNewUserId;
}


std::string SemanticMemory::getCurrUserId() const
{
  return _currUserId;
}



void SemanticMemory::registerExternalInfosProvider
(std::unique_ptr<MemBlockAndExternalCallback> pExtCallbackPtr,
 const linguistics::LinguisticDatabase& pLingDb)
{
  MemBlockAndExternalCallback& pExtCallback = *pExtCallbackPtr;
  unregisterExternalInfosProvider(pExtCallback.idStr());

  pExtCallback.memBlock.maxNbOfExpressionsInAMemoryBlock = 10;
  for (auto& currSemExp : pExtCallback.getSemExpThatCanBeAnswered())
    pExtCallback.memBlock.addRootSemExpWithHisContent(InformationType::ASSERTION, std::move(currSemExp), pLingDb);
  pExtCallback.getSemExpThatCanBeAnswered().clear();

  pExtCallback.memBlockForTriggers.maxNbOfExpressionsInAMemoryBlock = 10;
  for (auto& currSemExp : pExtCallback.getTriggers())
    pExtCallback.memBlockForTriggers.addRootSemExpWithHisContent(InformationType::ASSERTION, std::move(currSemExp), pLingDb);
  pExtCallback.getTriggers().clear();

  callbackToSentencesCanBeAnswered.emplace_back(mystd::unique_propagate_const<MemBlockAndExternalCallback>(std::move(pExtCallbackPtr)));
}



void SemanticMemory::unregisterExternalInfosProvider
(const std::string& pIdStr)
{
  for (auto itSentCanBeAnsw = callbackToSentencesCanBeAnswered.begin();
       itSentCanBeAnsw != callbackToSentencesCanBeAnswered.end(); ++itSentCanBeAnsw)
  {
    if ((*itSentCanBeAnsw)->idStr() == pIdStr)
    {
      (*itSentCanBeAnsw)->memBlock.clear();
      callbackToSentencesCanBeAnswered.erase(itSentCanBeAnsw);
      return;
    }
  }
}


ExternalFallback::~ExternalFallback()
{
}

} // End of namespace onsem
