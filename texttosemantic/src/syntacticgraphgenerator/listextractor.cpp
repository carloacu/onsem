#include "listextractor.hpp"
#include <onsem/common/utility/pointer_from.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include "entityrecognizer.hpp"
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{

ListExtractor::ListExtractor
(const AlgorithmSetForALanguage& pConfiguration)
  : fFlschecker(pConfiguration.getFlsChecker()),
    fConf(pConfiguration)
{
}


bool _isAListSeparator(const InflectedWord& pIGram,
                       ChunkType pListType)
{
  if (pIGram.word.lemma == "," && pIGram.word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS)
    return true;
  if (pListType == ChunkType::AND_CHUNK)
    return ConceptSet::haveAConcept(pIGram.infos.concepts, "list_and");
  if (pListType == ChunkType::OR_CHUNK)
    return ConceptSet::haveAConcept(pIGram.infos.concepts, "list_or");
  if (pListType == ChunkType::THEN_CHUNK)
    return ConceptSet::haveAConcept(pIGram.infos.concepts, "list_then");
  return false;
}


mystd::optional<ChunkType> getListType
(const std::map<std::string, char>& pConcepts)
{
   mystd::optional<ChunkType> res;
  if (ConceptSet::haveAConceptThatBeginWith(pConcepts, "list_"))
  {
    if (ConceptSet::haveAConcept(pConcepts, "list_and"))
      res.emplace(ChunkType::AND_CHUNK);
    else if (ConceptSet::haveAConcept(pConcepts, "list_or"))
      res.emplace(ChunkType::OR_CHUNK);
    else if (ConceptSet::haveAConcept(pConcepts, "list_then"))
      res.emplace(ChunkType::THEN_CHUNK);
  }
  return res;
}


// List extractor
// ==================================================================================

bool ListExtractor::extractLists
(ChunkLinkWorkingZone& pWorkingZone,
 bool pOneWordList,
 bool pAllowToNotRepeatTheSubject,
 bool pCanCompletePreviousList,
 int pNbMaxOfListElt,
 const mystd::optional<ChunkType>& pExceptListType) const
{
  if (fConf.getLanguageType() == SemanticLanguageEnum::FRENCH)
    pAllowToNotRepeatTheSubject = true;

  bool canHaveAnotherList = false;
  // the current list infos
  ListInfo currList;
  std::list<ListToMove> listsToMove;

  // iterate over all the chunks
  ChunkLinkIter currIt(pWorkingZone, pWorkingZone.end());

  while (currIt.getIt() != pWorkingZone.begin())
  {
    --currIt;

    if (pNbMaxOfListElt == 0)
      break;

    // if we are not in a list yet
    if (!currList.listType)
    {
      if (currIt->chunk->type == ChunkType::SEPARATOR_CHUNK)
      {
        if (listsToMove.empty() &&
            _tryToExtractANewListAroundASeparator(currIt, currList, listsToMove,
                                                  canHaveAnotherList, pNbMaxOfListElt,
                                                  pWorkingZone,
                                                  pOneWordList, pAllowToNotRepeatTheSubject,
                                                  pExceptListType))
          continue;
      }
      else if (pCanCompletePreviousList &&
               chunkTypeIsAList(currIt->chunk->type) &&
               currIt->tokRange.isEmpty() &&
               !currIt->chunk->children.empty())
      {
        currList.listType.emplace(currIt->chunk->type);
        currList.listChunk = &*currIt->chunk;
        auto& firstEltHead = *currIt->chunk->children.front().chunk;
        currList.eltType = firstEltHead.type;
        currList.firstHeadPartOfSpeech = firstEltHead.getHeadPartOfSpeech();
        continue;
      }
    }
    else // else (we already are inside a list)
    {
      if (_tryToCompleteListInfoWithAChunk(currIt, currList, listsToMove,
                                           pNbMaxOfListElt, pWorkingZone,
                                           pOneWordList, pAllowToNotRepeatTheSubject))
        continue;
    }
    currList.listType.reset();
  }

  _moveSomeChunksAfterMAinAlgo(listsToMove, pWorkingZone);
  return canHaveAnotherList;
}


bool ListExtractor::_createNewList
(std::list<ListToMove>& pListsToMove,
 ChunkAndTokIt& pChunkEnd,
 ChunkAndTokIt& pChunkBegin,
 int& pNbMaxOfListElt,
 ListInfo& pCurrList,
 ChunkLinkIter& pPrevIt,
 ChunkLinkIter& pIt,
 ChunkLinkIter& pNextIt) const
{
  // convert the separator to an "and node"
  pIt.getIt()->chunk->type = *pCurrList.listType;

  // add the last element
  if (pChunkEnd.atEnd())
  {
    pPrevIt.getIt()->type = ChunkLinkType::SIMPLE;
    if (pCurrList.chunkToMoveInNextEltChunkLink &&
        !pCurrList.chunkToMoveInNextEltChunkLink->atEnd())
    {
      pPrevIt.getIt()->tokRange = (*pCurrList.chunkToMoveInNextEltChunkLink)->chunk->tokRange;
      pCurrList.chunkToMoveInNextEltChunkLink->eraseIt();
      pCurrList.chunkToMoveInNextEltChunkLink.reset();
    }
    pIt.getIt()->chunk->children.push_front(*pPrevIt.getIt());
  }
  else
  {
    ChunkLink newChunkEltTrunc(ChunkLinkType::SIMPLE, *pPrevIt.getIt()->chunk);
    newChunkEltTrunc.chunk->tokRange.setItEnd(pChunkEnd.tokIt);
    newChunkEltTrunc.chunk->head = getHeadOfNominalGroup(newChunkEltTrunc.chunk->tokRange,
                                                      fConf.getLanguageType());
    pIt.getIt()->chunk->children.push_front(newChunkEltTrunc);
    ChunkLink& complChunk = *pPrevIt.getIt();
    complChunk.chunk->tokRange.setItBegin(pChunkEnd.tokIt);
    complChunk.chunk->head = getHeadOfNominalGroup(complChunk.chunk->tokRange,
                                                   fConf.getLanguageType());
    newChunkEltTrunc.chunk->children.clear();
    auto newChunkTruncPartoSpeech = newChunkEltTrunc.chunk->getHeadPartOfSpeech();
    if (newChunkTruncPartoSpeech == PartOfSpeech::ADVERB)
    {
      auto nextIt = pIt.getIt();
      ++nextIt;
      pIt.getList()->insert(nextIt, complChunk);
    }
    else if (newChunkTruncPartoSpeech == PartOfSpeech::ADJECTIVE &&
             partOfSpeech_isNominal(complChunk.chunk->tokRange.getItBegin()->inflWords.front().word.partOfSpeech))
    {
      pIt.getList()->insert(pIt.getIt(), complChunk);
      ChunkLinkIter newParentChLk = pIt;
      --newParentChLk;
      _addAListToMove(pListsToMove, ListToMove(newParentChLk, pIt, false));
    }
    else
    {
      if (tokenIsMoreProbablyAType(*complChunk.chunk->tokRange.getItBegin(),
                                   PartOfSpeech::PARTITIVE))
      {
        complChunk.type = ChunkLinkType::SPECIFICATION;
      }
      newChunkEltTrunc.chunk->children.push_back(complChunk);
    }
  }
  if (!_itIsInListToMove(pPrevIt, pListsToMove)) // otherwise bug on windows
  {
    pPrevIt.eraseIt();
  }
  pPrevIt.setItAtEnd();
  pCurrList.listChunk = &*pIt.getIt()->chunk;

  // add the new element
  return _addListElt(pCurrList.listChunk, pNextIt, pNbMaxOfListElt, pChunkBegin.tokIt);
}


bool ListExtractor::_itIsInListToMove
(const ChunkLinkIter& pIt,
 const std::list<ListToMove>& pListsToMove)
{
  for (const auto& currLstToMv : pListsToMove)
  {
    if (currLstToMv.newParent == pIt ||
        currLstToMv.itRootList == pIt)
    {
      return true;
    }
  }
  return false;
}


void ListExtractor::_addAListToMove
(std::list<ListToMove>& pListsToMove,
 const ListToMove& pNewListToMove)
{
  for (auto& currElt : pListsToMove)
  {
    if (currElt.itRootList == pNewListToMove.itRootList)
    {
      if (currElt.removeIfAnotherChunkFound)
      {
        currElt = pNewListToMove;
      }
      else
      {
        pListsToMove.push_front(ListToMove(currElt.newParent, pNewListToMove.newParent, false));
      }
      return;
    }
  }
  pListsToMove.push_back(pNewListToMove);
}



bool ListExtractor::_addListElt
(Chunk* pListChunk,
 ChunkLinkIter& pNextIt,
 int& pNbMaxOfListElt,
 TokIt pNextChunkBegin) const
{
  bool res = false;

  // add the new element
  if (pNextIt.getIt()->chunk->tokRange.getItBegin() == pNextChunkBegin)
  {
    // add simple
    pNextIt.getIt()->type = ChunkLinkType::SIMPLE;
    pListChunk->children.push_front(*pNextIt.getIt());
    pNextIt.eraseIt();
    --pNbMaxOfListElt;
    res = true;
  }
  else
  {
    // split it before to add it in the list
    ChunkLink nextChunkTrunc(ChunkLinkType::SIMPLE, *pNextIt.getIt()->chunk);
    nextChunkTrunc.chunk->tokRange.setItBegin(pNextChunkBegin);
    nextChunkTrunc.chunk->head = getHeadOfNominalGroup(nextChunkTrunc.chunk->tokRange,
                                                       fConf.getLanguageType());
    pListChunk->children.push_front(nextChunkTrunc);

    Chunk& nextChunk = *pNextIt.getIt()->chunk;
    nextChunk.children.clear();
    nextChunk.tokRange.setItEnd(pNextChunkBegin);
    nextChunk.head = getHeadOfNominalGroup(pNextIt.getIt()->chunk->tokRange,
                                           fConf.getLanguageType());
    setChunkType(nextChunk);
  }

  // if we are on a transitive verb without direct object,
  // the transitive verb will share the direct of object of the next verb of the list
  Chunk& newlyAddedChunk = *pListChunk->children.front().chunk;
  if (newlyAddedChunk.type == ChunkType::INFINITVE_VERB_CHUNK &&
      !haveADO(newlyAddedChunk))
  {
    const InflectedWord& headInfosGram = newlyAddedChunk.head->inflWords.front();
    if (headInfosGram.word.partOfSpeech == PartOfSpeech::VERB &&
        headInfosGram.infos.hasContextualInfo(WordContextualInfos::TRANSITIVEVERB) &&
        pListChunk->children.size() > 1)
    {
      Chunk& nextChunkInTheList = *(++pListChunk->children.begin())->chunk;
      ChunkLink* nextChkDOChkLk = getDOChunkLink(nextChunkInTheList);
      if (nextChkDOChkLk != nullptr &&
          checkOrder(nextChunkInTheList, *nextChkDOChkLk->chunk))
      {
        newlyAddedChunk.children.emplace_back(*nextChkDOChkLk);
      }
    }
  }
  return res;
}


bool ListExtractor::_addANewMultiWordsList
(ListInfo& pCurrList,
 std::list<ListToMove>& pListsToMove,
 int& pNbMaxOfListElt,
 ChunkLinkIter& pPrevIt,
 ChunkLinkIter& pIt,
 ChunkLinkIter& pNextIt,
 bool pAllowToNotRepeatTheSubject) const
{
  if (haveOtherEltsBetterToLinkInAList(pNextIt, pPrevIt))
    return false;

  const Chunk& prevChunk = *pPrevIt.getIt()->chunk;
  std::list<ChunkLink>::iterator nextChunk = pNextIt.getIt();
  const Chunk& nextChunkFirstElt = getFirstListEltChunk(*nextChunk->chunk);
  if (_isNewEltCompatibleWithTheList(pCurrList, nextChunkFirstElt, pAllowToNotRepeatTheSubject))
  {
    bool createList = false;
    const Chunk& prevLastChunkOfList = getFirstListEltChunk(prevChunk);
    auto prevHeadIt = prevLastChunkOfList.head;
    auto prevBeginIt = prevLastChunkOfList.tokRange.getItBegin();
    if (fFlschecker.canBeAssociatedInAList(nextChunkFirstElt.head->inflWords.front(),
                                           prevHeadIt->inflWords.front()))
    {
      createList = true;
    }
    else
    {
      if (prevChunk.type == ChunkType::PREPOSITIONAL_CHUNK)
      {
        if (prevBeginIt->inflWords.front().word == nextChunk->chunk->tokRange.getItBegin()->inflWords.front().word)
          createList = true;
      }
      else if (fFlschecker.canBeAssociatedInAList(prevBeginIt->inflWords.front(),
                                                  nextChunk->chunk->tokRange.getItBegin()->inflWords.front()))
      {
        createList = true;
      }
    }

    if (createList)
    {
      ChunkAndTokIt chunkEnd(*pPrevIt.getIt()->chunk);
      chunkEnd.tokIt = chunkEnd.getItEnd();
      ChunkAndTokIt chunkBegin(*nextChunk->chunk);
      bool chunkHasBeenRemoved = _createNewList
          (pListsToMove, chunkEnd, chunkBegin, pNbMaxOfListElt,
           pCurrList, pPrevIt, pIt, pNextIt);
      assert(chunkHasBeenRemoved);
      return chunkHasBeenRemoved;
    }
  }

  return false;
}


bool ListExtractor::_addANewOneWordList
(ListInfo& pCurrList,
 std::list<ListToMove>& pListsToMove,
 int& pNbMaxOfListElt,
 ChunkLinkIter& pPrevIt,
 ChunkLinkIter& pIt,
 ChunkLinkIter& pNextIt) const
{
  Chunk& oldChunk = *pPrevIt.getIt()->chunk;
  bool chunkHasBeenRemoved = false;
  bool addANewList = false;

  // get delimitators of the new list
  ChunkAndTokIt chunkBegin(*pNextIt.getIt()->chunk);
  ChunkAndTokIt chunkEnd(oldChunk);
  if (_getBeginNewEltAndEndOldElt(chunkBegin, chunkEnd, pCurrList.partOfSpeechAtBegining))
  {
    chunkHasBeenRemoved = _createNewList
        (pListsToMove, chunkEnd, chunkBegin, pNbMaxOfListElt,
         pCurrList, pPrevIt, pIt, pNextIt);
    addANewList = true;

    // if there is a chunk before the list
    while (!chunkHasBeenRemoved &&
           pNbMaxOfListElt != 0)
    {
      ChunkAndTokIt chunkBegin(*pNextIt.getIt()->chunk);
      ChunkAndTokIt chunkEnd(*pCurrList.listChunk->children.front().chunk);
      if (_getBeginNewEltAndEndOldElt(chunkBegin, chunkEnd, pCurrList.partOfSpeechAtBegining))
        chunkHasBeenRemoved = _addListElt(pCurrList.listChunk, pNextIt, pNbMaxOfListElt, chunkBegin.tokIt);
      else
        break;
    }

    // if there is a chunk before the list
    if (!chunkHasBeenRemoved &&
        canBeParentOfANominalGroup(pNextIt->chunk->head->inflWords.front()))
    {
      _addAListToMove(pListsToMove, ListToMove(pNextIt, pIt, false));
    }
  }

  if (!chunkHasBeenRemoved)
    pCurrList.listType.reset();
  return addANewList;
}


bool ListExtractor::_isNewEltCompatibleWithTheList(
    ListInfo& pCurrList,
    const Chunk& pChunk,
    bool pAllowToNotRepeatTheSubject) const
{
  ChunkType newChunkType = pChunk.type;
  bool doesBecomeAListOfInfinitiveVerbs =
      _doesBecomeAListOfInfinitiveVerbs(pCurrList, newChunkType);
  if ((newChunkType == pCurrList.eltType &&
       (pAllowToNotRepeatTheSubject || !(pCurrList.eltType == ChunkType::VERB_CHUNK && !pCurrList.hasASubject && haveASubject(pChunk)))) ||
      (newChunkType == ChunkType::PREPOSITIONAL_CHUNK && pCurrList.eltType == ChunkType::NOMINAL_CHUNK) ||
      doesBecomeAListOfInfinitiveVerbs)
  {
    PartOfSpeech headPartOfSpeech = pChunk.head->inflWords.front().word.partOfSpeech;
    // don't allow a pronoun followed by a noun in a list
    if ((headPartOfSpeech == PartOfSpeech::PRONOUN ||
         headPartOfSpeech == PartOfSpeech::PRONOUN_SUBJECT) &&
        partOfSpeech_isNominal(pCurrList.firstHeadPartOfSpeech))
      return false;
    return _updateExistingListInfoAccordingToTheNewElt(pCurrList, pChunk,
                                                       doesBecomeAListOfInfinitiveVerbs);
  }
  else if (newChunkType == ChunkType::NOMINAL_CHUNK &&
           pCurrList.eltType == ChunkType::VERB_CHUNK &&
           InflectionsChecker::verbalInflAreAtPresentParticiple(pCurrList.commonVerbInflections) &&
           pChunk.head->inflWords.front().word.partOfSpeech == PartOfSpeech::ADJECTIVE)
  {
    pCurrList.eltType = ChunkType::NOMINAL_CHUNK;
    return true;
  }
  return false;
}


void ListExtractor::_getVerbAndAuxInflections
(VerbalInflections& pVerbInflections,
 const Chunk& pVerbChunk) const
{
  const Chunk* auxChunk = getAuxiliaryChunk(pVerbChunk);
  if (auxChunk != nullptr)
  {
    fFlschecker.unionOfSameVerbTenses(pVerbInflections,
                                      pVerbChunk.head->inflWords.front().inflections(),
                                      auxChunk->head->inflWords.front().inflections());
  }
  else
  {
    const Inflections& verbInfls = pVerbChunk.head->inflWords.front().inflections();
    if (verbInfls.type == InflectionType::VERBAL)
      pVerbInflections = verbInfls.getVerbalI();
  }
}


bool ListExtractor::_getBeginNewEltAndEndOldElt
(ChunkAndTokIt& pChunkBegin,
 ChunkAndTokIt& pChunkEnd,
 PartOfSpeech pPartOfSpeechAtBegining) const
{
  const Chunk& oldChunk = pChunkEnd.chunk;
  const InflectedWord& firstIGramOfFirstTokenOfTheOldChunk = oldChunk.tokRange.getItBegin()->inflWords.front();
  Chunk& newChunk = pChunkBegin.chunk;
  TokIt lastTokNewChunk = getPrevToken(newChunk.tokRange.getItEnd(),
                                       newChunk.tokRange.getItBegin(),
                                       newChunk.tokRange.getItEnd(), SkipPartOfWord::YES);

  if (lastTokNewChunk != newChunk.tokRange.getItEnd())
  {
    // check that 2 chunks contains: new:DET + GRAMX  old:DET + GRAMX
    if (pPartOfSpeechAtBegining == PartOfSpeech::DETERMINER)
    {
      // checks the DETs
      if (firstIGramOfFirstTokenOfTheOldChunk.word.partOfSpeech == pPartOfSpeechAtBegining)
      {
        pChunkEnd.tokIt = getNextToken(pChunkEnd.getItBegin(), pChunkEnd.getItEnd());
        if (!pChunkEnd.atEnd())
        {
          const auto& chunkBeginFirstInflWord = lastTokNewChunk->inflWords.front();
          const auto& chunkEndFirstInflWord = pChunkEnd.tokIt->inflWords.front();
          bool haveListEltCompatibles = fFlschecker.canBeAssociatedInAList(chunkBeginFirstInflWord,
                                                                           chunkEndFirstInflWord);
          if (!haveListEltCompatibles &&
              chunkEndFirstInflWord.word.partOfSpeech == PartOfSpeech::NOUN &&
              chunkBeginFirstInflWord.word.partOfSpeech == PartOfSpeech::ADJECTIVE)
          {
            lastTokNewChunk = getPrevToken(lastTokNewChunk,
                                           newChunk.tokRange.getItBegin(),
                                           newChunk.tokRange.getItEnd(), SkipPartOfWord::YES);
            haveListEltCompatibles = fFlschecker.canBeAssociatedInAList(lastTokNewChunk->inflWords.front(),
                                                                        chunkEndFirstInflWord);
          }
          if (haveListEltCompatibles)
          {
            // checks the GRAMXs
            TokIt detTokNewChunk = getPrevToken(lastTokNewChunk,
                                                newChunk.tokRange.getItBegin(),
                                                newChunk.tokRange.getItEnd());
            if (detTokNewChunk != newChunk.tokRange.getItEnd() &&
                detTokNewChunk->inflWords.front().word.partOfSpeech == pPartOfSpeechAtBegining)
            {
              pChunkBegin.tokIt = detTokNewChunk;
              pChunkEnd.tokIt = getNextToken(pChunkEnd.tokIt, pChunkEnd.getItEnd());
              return true;
            }
          }
        }
      }
    }
    // check that 2 chunks contains the same grammatical type
    else if (fFlschecker.canBeAssociatedInAList(lastTokNewChunk->inflWords.front(),
                                                firstIGramOfFirstTokenOfTheOldChunk) ||
             _tryAssociateNewChunkByTruncateTheEnd(lastTokNewChunk, newChunk,
                                                   firstIGramOfFirstTokenOfTheOldChunk))
    {
      pChunkBegin.tokIt = lastTokNewChunk;
      pChunkEnd.tokIt = getNextToken(pChunkEnd.getItBegin(), pChunkEnd.getItEnd());

      if (lastTokNewChunk->getPartOfSpeech() != PartOfSpeech::ADVERB)
      {
        while (lastTokNewChunk != pChunkBegin.getItBegin())
        {
          lastTokNewChunk = getPrevToken(lastTokNewChunk, pChunkBegin.getItBegin(), pChunkBegin.getItEnd());
          if (lastTokNewChunk->getPartOfSpeech() == PartOfSpeech::ADVERB)
            pChunkBegin.tokIt = lastTokNewChunk;
          else
            break;
        }
      }

      // skip words that are part of the first word
      while (!pChunkEnd.atEnd() &&
             pChunkEnd.tokIt->linkedTokens.size() == 1 &&
             pChunkEnd.tokIt->linkedTokens.front() == pChunkEnd.getItBegin())
        pChunkEnd.tokIt = getNextToken(pChunkEnd.tokIt, pChunkEnd.getItEnd());

      if (firstIGramOfFirstTokenOfTheOldChunk.word.partOfSpeech == PartOfSpeech::VERB)
      {
        while (!pChunkEnd.atEnd() &&
               pChunkEnd.tokIt->inflWords.front().word.partOfSpeech == PartOfSpeech::ADVERB)
          pChunkEnd.tokIt = getNextToken(pChunkEnd.tokIt, pChunkEnd.getItEnd());
      }

      return pChunkEnd.atEnd() ||
          pChunkEnd.tokIt->getPartOfSpeech() != firstIGramOfFirstTokenOfTheOldChunk.word.partOfSpeech;
    }
  }
  return false;
}




bool ListExtractor::_tryAssociateNewChunkByTruncateTheEnd
(TokIt& pLastTokNewChunk,
 Chunk& pNewChunk,
 const InflectedWord& pIGramLastElt) const
{
  // we skip the adjectives, if we are tracking a list of simple nouns
  if (pIGramLastElt.word.partOfSpeech == PartOfSpeech::NOUN)
  {
    TokIt beginSubChunk = pLastTokNewChunk;
    while (pLastTokNewChunk != pNewChunk.tokRange.getItEnd() &&
           pLastTokNewChunk->inflWords.front().word.partOfSpeech == PartOfSpeech::ADJECTIVE)
    {
      beginSubChunk = pLastTokNewChunk;
      pLastTokNewChunk = getPrevToken(pLastTokNewChunk,
                                      pNewChunk.tokRange.getItBegin(),
                                      pNewChunk.tokRange.getItEnd());
    }
    if (beginSubChunk != pLastTokNewChunk &&
        pLastTokNewChunk != pNewChunk.tokRange.getItEnd() &&
        pLastTokNewChunk->inflWords.front().word.partOfSpeech == PartOfSpeech::NOUN)
    {
      putEndOfAChunkToHisChildren(pNewChunk, beginSubChunk,
                                fConf.getLanguageType());
      return true;
    }
  }
  return false;
}







// Subordonates extractors
// ---------------------------------------------------------------------


void ListExtractor::extractSubordonates
(ChunkLinkWorkingZone& pWorkingZone) const
{
  // the current list type
  mystd::optional<ChunkType> currListType;

  std::list<ListToMove> listsToMove;
  ListOfSubordinates listOfSubs;
  SemanticLanguageEnum language = fConf.getLanguageType();
  std::list<ChunkLink>::iterator endEltOfList = pWorkingZone.end();
  std::list<ChunkLink>::iterator prevPhrase = pWorkingZone.end();
  std::list<ChunkLink>::iterator it = pWorkingZone.end();
  // iterate over all the chunks (from the end)
  while (it != pWorkingZone.begin())
  {
    prevPhrase = it;
    --it;
    auto nextPhrase = it;
    if (nextPhrase == pWorkingZone.begin())
      nextPhrase = pWorkingZone.end();
    else
      --nextPhrase;
    const Chunk& currChunk = *it->chunk;
    PartOfSpeech currChunkHeadPartOfSpeech = currChunk.head->inflWords.begin()->word.partOfSpeech;

    if ((currChunk.type == ChunkType::SEPARATOR_CHUNK &&
         currChunkHeadPartOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION) ||
        (currChunk.type == ChunkType::PREPOSITIONAL_CHUNK &&
         currChunkHeadPartOfSpeech == PartOfSpeech::PREPOSITION) ||
        (currChunk.type == ChunkType::NOMINAL_CHUNK &&
         currChunkHeadPartOfSpeech == PartOfSpeech::PARTITIVE))
    {
      // if there is no phrase before, we skip this subordonate
      if (prevPhrase == pWorkingZone.end() ||
          (language == SemanticLanguageEnum::FRENCH && !recInListConst(chunkIsVerbalAndNotAYesOrNo, *prevPhrase->chunk)) ||
          (language == SemanticLanguageEnum::ENGLISH && !recInListConst(chunkIsVerbal, *prevPhrase->chunk)))
      {
        currListType.reset();
        listOfSubs.endOfList = false;
      }
      // if there is a phrase before, we add this subordonates the current list
      else
      {
        listOfSubs.addSub(prevPhrase, it);
        listOfSubs.endOfList = true;
      }
      continue;
    }
    else if (currChunk.type == ChunkType::SEPARATOR_CHUNK)
    {
      if (!currListType &&
          listOfSubs.subWithSeps.size() == 1)
      {
        currListType = getListType(currChunk.head->inflWords.front().infos.concepts);

        if (currListType)
        {
          endEltOfList = it;
          listOfSubs.subWithSeps.front().separators.push_front(it);
          listOfSubs.endOfList = false;
          continue;
        }
      }

      // if we found an inside list separator
      if (!listOfSubs.subWithSeps.empty() &&
          currListType &&
          _isAListSeparator(currChunk.head->inflWords.front(), *currListType))
      {
        listOfSubs.subWithSeps.front().separators.push_front(it);
        listOfSubs.endOfList = false;
        continue;
      }

      // if it's another separator, we cancel the current list of subordonates
      currListType.reset();
      listOfSubs.subWithSeps.clear();
      listOfSubs.endOfList = true;
    }
    // if the current chunk it's not a separator AND
    // if have at least one separator AND
    // if it can be the end of the list
    else if (listOfSubs.endOfList)
    {
      if (listOfSubs.subWithSeps.empty())
      {
        if (prevPhrase == pWorkingZone.end())
        {
          currListType.reset();
        }
        else if (recInListConst(chunkIsVerbal, currChunk))
        {
          bool needToSkip = false;
          if (language == SemanticLanguageEnum::ENGLISH)
          {
            if (!recInListConst(chunkIsVerbal, *prevPhrase->chunk))
            {
              needToSkip = true;
            }
            else if (!hasAChunkTypeOrAListOfChunkTypes(*prevPhrase->chunk, ChunkType::INFINITVE_VERB_CHUNK) &&
                     nextPhrase != pWorkingZone.end() &&
                     nextPhrase->chunk->type == ChunkType::SEPARATOR_CHUNK)
            {
              const auto& iGramOfHeadOfNextPhrase = nextPhrase->chunk->head->inflWords.front();
              if (iGramOfHeadOfNextPhrase.infos.hasContextualInfo(WordContextualInfos::CONDITION))
              {
                currListType.reset();
                needToSkip = true;
              }
            }
          }
          else if (language == SemanticLanguageEnum::FRENCH)
          {
            needToSkip = recInListConst([&](const Chunk& pChunk)
            {
              return pChunk.type != ChunkType::INFINITVE_VERB_CHUNK &&
                  (pChunk.type != ChunkType::VERB_CHUNK ||
                  (!pChunk.requests.isAQuestion() && !ConceptSet::haveAConcept(currChunk.getHeadConcepts(), "verb_action_say")));
            }, *prevPhrase->chunk);
          }

          // if there is a phrase before, we add this subordonates the current list
          if (!needToSkip)
          {
            listOfSubs.addSub(prevPhrase);
            listOfSubs.endOfList = true;
          }
        }
      }

      if (!listOfSubs.subWithSeps.empty())
      {
        if (_addSubordonates(it, listOfSubs.subWithSeps, endEltOfList, currListType))
        {
          _clear(pWorkingZone, listOfSubs.subWithSeps);
          it = _tryToCompleteAnExistingList(listsToMove, pWorkingZone, it);
        }
        currListType.reset();
        listOfSubs.subWithSeps.clear();
      }
    }
  }

  // TODO: need to fix windows before to uncomment that
  //_moveSomeChunksAfterMAinAlgo(listsToMove, pWorkingZone);
}



void ListExtractor::_clear
(ChunkLinkWorkingZone& pWorkingZone,
 const std::list<SubordinateWithSeparators>& pSubOfChunk)
{
  for (const auto& currSub : pSubOfChunk)
  {
    pWorkingZone.syntTree().erase(currSub.sub);
    for (const auto& currSep : currSub.separators)
      pWorkingZone.syntTree().erase(currSep);
  }
}




bool ListExtractor::_addSubordonates
(std::list<ChunkLink>::iterator& pParentChunkLk,
 std::list<SubordinateWithSeparators>& pSubOfChunk,
 std::list<ChunkLink>::iterator& pEndEltOfList,
 mystd::optional<ChunkType> pListType) const
{
  assert(pSubOfChunk.size() >= 1);
  if (pSubOfChunk.size() == 1) // TODO: have the same algo for one or many usbordinate!!!
  {
    SubordinateWithSeparators& subOfAChunk = pSubOfChunk.front();
    TokenRange tokRange([&subOfAChunk]
    {
      if (subOfAChunk.separators.empty())
        return TokenRange(subOfAChunk.sub->chunk->tokRange.getTokList());
      const auto& chkLksList = subOfAChunk.separators.front();
      return TokenRange(chkLksList->chunk->tokRange.getTokList(),
                        chkLksList->chunk->tokRange.getItBegin(),
                        subOfAChunk.separators.back()->chunk->tokRange.getItEnd());
    }());
    Chunk* parentChunkPtr = &*pParentChunkLk->chunk;
    if (chunkTypeIsAList(parentChunkPtr->type) &&
        !parentChunkPtr->children.empty())
      parentChunkPtr = &*parentChunkPtr->children.back().chunk;

    ChunkLink& secondVerb = *subOfAChunk.sub;
    if (parentChunkPtr->type == ChunkType::VERB_CHUNK)
      return _linkVerbChunk_VerbChunk(*parentChunkPtr, secondVerb, tokRange);

    if (parentChunkPtr->type == ChunkType::INFINITVE_VERB_CHUNK)
    {
      if (fConf.getLanguageType() == SemanticLanguageEnum::ENGLISH &&
          tokRange.isEmpty() &&
          recInListConst(chunkDontHaveASubjectAndCannotHaveNoSubject, *secondVerb.chunk) &&
          !doesTokRangeBeginsWithARequestSubordinate(tokRange, fConf.getLingDico().statDb))
        return false;
      return _linkVerbChunk_VerbChunk(*parentChunkPtr, secondVerb, tokRange);
    }

    return _linkChunk_VerbChunk(*parentChunkPtr, secondVerb, tokRange);
  }
  else if (pListType)
  {
    ChunkLink andChunkLink
        (ChunkLinkType::SUBORDINATE_CLAUSE,
         Chunk(pEndEltOfList->chunk->tokRange, *pListType));
    Chunk* andChunk = &*andChunkLink.chunk;
    Chunk& parentChunk = *pParentChunkLk->chunk;
    bool verbDOIsTheSubject = true;
    for (auto it = pSubOfChunk.begin(); it != pSubOfChunk.end(); ++it)
    {
      it->sub->tokRange = TokenRange(it->separators.front()->chunk->tokRange.getTokList(),
                                     it->separators.front()->chunk->tokRange.getItBegin(),
                                     it->separators.back()->chunk->tokRange.getItEnd());
      andChunk->children.push_back(*it->sub);
      if (verbDOIsTheSubject)
      {
        if (chunkTypeIsVerbal(parentChunk.type))
        {
          verbDOIsTheSubject = _canLinkDO_Subject(parentChunk, *it->sub->chunk);
        }
        else
        {
          verbDOIsTheSubject = canLinkVerbToASubject(whereToLinkTheSubject(*it->sub->chunk),
                                                     parentChunk, fFlschecker, false);
        }
      }
    }
    if (verbDOIsTheSubject)
      andChunkLink.type = ChunkLinkType::SUBJECT_OF;
    else
      andChunkLink.type =
          fConf.getEntityRecognizer().findNatureOfAChunkLink(andChunkLink, &*pParentChunkLk->chunk);

    if (pParentChunkLk->chunk->type == ChunkType::VERB_CHUNK)
    {
      Chunk* dObj = getDOComplOrSubjClauseChunk(*pParentChunkLk->chunk);
      if (dObj != nullptr &&
          !chunkTypeIsAList(dObj->type))
      {
        dObj->children.push_back(andChunkLink);
        return true;
      }
    }
    pParentChunkLk->chunk->children.push_back(andChunkLink);
    return true;
  }
  return false;
}


mystd::optional<ChunkLinkType> ListExtractor::_getAppropriateChunkLinkFromTokens
(InflectedWord* pVerbInflectedWord,
 mystd::optional<const SemanticWord*>& pIntroductingWord,
 const TokenRange& pTokRange,
 ChunkType pTokensChunkType) const
{
  auto itEnd = pTokRange.getItEnd();
  TokCstIt itFirstWord = getTheNextestWord(pTokRange.getItBegin(), itEnd);
  if (itFirstWord != itEnd)
  {
    const auto& firstWordIGram = itFirstWord->inflWords.front();
    ConstTokenIterator nextToken(pTokRange.getTokList(), 0);
    nextToken.setTokenIt(itFirstWord);
    nextToken.advanceToNextToken();
    return fConf.getEntityRecognizer().getAppropriateChunkLink(pVerbInflectedWord, pIntroductingWord, &firstWordIGram,
                                                               pTokensChunkType, &nextToken);
  }
  return mystd::optional<ChunkLinkType>();
}


bool ListExtractor::_linkVerbChunk_VerbChunk
(Chunk& pFirstVerb,
 ChunkLink& pSecondVerb,
 const TokenRange& pTokRange) const
{
  if (pFirstVerb.isPassive && !pSecondVerb.chunk->requests.empty())
    return false;

  bool isACondition = false; // if it's a condition subordinate
  SemanticLanguageEnum language = fConf.getLanguageType();
  if (language == SemanticLanguageEnum::ENGLISH && pTokRange.isEmpty())
  {
    // puprpose subordinate
    if (pFirstVerb.isPassive)
    {
      if (recInListConst(chunkIsAtInfinitive, *pSecondVerb.chunk))
      {
        pFirstVerb.children.emplace_back(pTokRange, ChunkLinkType::PURPOSE, pSecondVerb.chunk);
        return true;
      }
      return false;
    }
  }
  else
  {
    TokIt endIt = pTokRange.getItEnd();
    for (TokIt it = pTokRange.getItBegin(); it != endIt; it = getNextToken(it, endIt))
    {
      const InflectedWord& tokIGram = it->inflWords.front();
      if (tokIGram.infos.hasContextualInfo(WordContextualInfos::CONDITION) &&
          fConf.getLingDico().statDb.wordToSubordinateRequest(tokIGram.word) == SemanticRequestType::YESORNO &&
          !ConceptSet::haveAConcept(pFirstVerb.head->inflWords.front().infos.concepts, "verb_action_say_ask"))
      {
        isACondition = true;
      }
    }
  }

  mystd::optional<ChunkLinkType> newLkType;
  ChunkLink* subChunkLkToHoldTheSubordinatePtr = getChunkLinkWhereWeCanLinkASubodinate(pFirstVerb);
  bool secondChunkHasASubject = haveASubject(*pSecondVerb.chunk);
  if (!isACondition || pTokRange.isEmpty())
  {
    bool tryToPutSubordonateOnChildOfTheDO = true;

    // don't consider present particile without auxiliaries
    InflectedWord& iGramVerb = pFirstVerb.head->inflWords.front();
    if (!haveAnAuxiliary(pFirstVerb))
    {
      const VerbalInflections* verbalInflPtr = iGramVerb.inflections().getVerbalIPtr();
      if (verbalInflPtr != nullptr &&
          verbalInflPtr->inflections.size() == 1 &&
          verbalInflPtr->inflections.front().tense == LinguisticVerbTense::PRESENT_PARTICIPLE)
        return false;
    }

    // detect link type from first verb and introduction of the subordinate
    newLkType = _getAppropriateChunkLinkFromTokens(&iGramVerb, pSecondVerb.chunk->introductingWordToSaveForSynthesis,
                                                   pTokRange, pSecondVerb.chunk->type);
    if (newLkType && *newLkType != ChunkLinkType::OBJECT_OF && *newLkType != ChunkLinkType::DIRECTOBJECT)
      tryToPutSubordonateOnChildOfTheDO = false;

    if (tryToPutSubordonateOnChildOfTheDO)
    {
      if (subChunkLkToHoldTheSubordinatePtr == nullptr &&
          iGramVerb.infos.concepts.count("verb_equal_be") > 0)
      {
        subChunkLkToHoldTheSubordinatePtr = getSubjectChunkLinkWhereWeCanLinkASubodinate(pFirstVerb);
        if (subChunkLkToHoldTheSubordinatePtr != nullptr &&
            partOfSpeech_isPronominal(subChunkLkToHoldTheSubordinatePtr->chunk->head->inflWords.front().word.partOfSpeech))
          subChunkLkToHoldTheSubordinatePtr = nullptr;
      }
      if (subChunkLkToHoldTheSubordinatePtr != nullptr &&
          chunkTypeIsAList(subChunkLkToHoldTheSubordinatePtr->chunk->type) &&
          !subChunkLkToHoldTheSubordinatePtr->chunk->children.empty())
        subChunkLkToHoldTheSubordinatePtr = &*(--subChunkLkToHoldTheSubordinatePtr->chunk->children.end());

      if (subChunkLkToHoldTheSubordinatePtr != nullptr &&
          (haveADO(*subChunkLkToHoldTheSubordinatePtr->chunk) ||
           haveASubordonateClause(*subChunkLkToHoldTheSubordinatePtr->chunk)))
        return false;

      const InflectedWord& secondVerbInflWord = pSecondVerb.chunk->head->inflWords.front();
      ChunkLinkType newChunkLiknType = ChunkLinkType::SUBORDINATE_CLAUSE;
      if (newLkType && (*newLkType == ChunkLinkType::OBJECT_OF || *newLkType == ChunkLinkType::DIRECTOBJECT))
        newChunkLiknType = *newLkType;
      else
      {
        if (pFirstVerb.type == ChunkType::INFINITVE_VERB_CHUNK &&
            pTokRange.isEmpty() &&
            !ConceptSet::haveAConcept(pFirstVerb.getHeadConcepts(), "verb_action_say") &&
            recInListConst(chunkCanBeAtImperative, *pSecondVerb.chunk))
        {
          newLkType = ChunkLinkType::PURPOSE_OF;
        }
        else if (InflectionsChecker::verbIsAtPresentParticiple(secondVerbInflWord))
        {
          newChunkLiknType = ChunkLinkType::SUBJECT_OF;
        }
        else if (InflectionsChecker::verbIsAtPresentIndicative(secondVerbInflWord) &&
                 !haveASubject(*pSecondVerb.chunk))
        {
          if (subChunkLkToHoldTheSubordinatePtr != nullptr &&
              InflectionsChecker::verbIsAtInfinitive(subChunkLkToHoldTheSubordinatePtr->chunk->head->inflWords.front()))
          {
            if (pSecondVerb.chunk->requests.empty())
              pSecondVerb.chunk->requests.set(SemanticRequestType::ACTION);
          }
          else
          {
            newChunkLiknType = ChunkLinkType::SUBJECT_OF;
          }
        }
      }

      if (subChunkLkToHoldTheSubordinatePtr != nullptr)
      {
        if (chunkTypeIsVerbal(subChunkLkToHoldTheSubordinatePtr->chunk->type))
        {
          if (hasAChunkTypeOrAListOfChunkTypes(*subChunkLkToHoldTheSubordinatePtr->chunk, ChunkType::INFINITVE_VERB_CHUNK) ||
              hasAChunkTypeOrAListOfChunkTypes(*pSecondVerb.chunk, ChunkType::INFINITVE_VERB_CHUNK))
          {
            subChunkLkToHoldTheSubordinatePtr->chunk->children.emplace_back(pTokRange, newChunkLiknType, pSecondVerb.chunk);
            return true;
          }
        }
        else
        {
          if (hasAChunkTypeOrAListOfChunkTypes(*pSecondVerb.chunk, ChunkType::INFINITVE_VERB_CHUNK))
          {
            PartOfSpeech subChunkPartOfSpeech = subChunkLkToHoldTheSubordinatePtr->chunk->getHeadPartOfSpeech();
            if (subChunkPartOfSpeech == PartOfSpeech::PREPOSITION &&
                subChunkLkToHoldTheSubordinatePtr->chunk->children.empty())
            {
              subChunkLkToHoldTheSubordinatePtr->tokRange = subChunkLkToHoldTheSubordinatePtr->chunk->tokRange;
              subChunkLkToHoldTheSubordinatePtr->chunk = pSecondVerb.chunk;
            }
            else if (fConf.getLanguageType() == SemanticLanguageEnum::FRENCH &&
                     pTokRange.isEmpty())
            {
              if (getDOChunkLink(*pSecondVerb.chunk) != nullptr)
                pFirstVerb.children.emplace_front(ChunkLinkType::INDIRECTOBJECT, *subChunkLkToHoldTheSubordinatePtr->chunk);
              else
                pSecondVerb.chunk->children.emplace_front(ChunkLinkType::DIRECTOBJECT, *subChunkLkToHoldTheSubordinatePtr->chunk);
              subChunkLkToHoldTheSubordinatePtr->chunk = pSecondVerb.chunk;
            }
            else if (partOfSpeech_isPronominal(subChunkPartOfSpeech)) // replace by any human
            {
              subChunkLkToHoldTheSubordinatePtr->chunk->children.emplace_back(pTokRange, ChunkLinkType::SUBJECT_OF, pSecondVerb.chunk);
            }
            else
            {
              auto subChLkOpt = _getAppropriateChunkLinkFromTokens(nullptr,
                                                                   subChunkLkToHoldTheSubordinatePtr->chunk->introductingWordToSaveForSynthesis,
                                                                   pTokRange,
                                                                   subChunkLkToHoldTheSubordinatePtr->chunk->type);
              if (subChLkOpt)
                subChunkLkToHoldTheSubordinatePtr->chunk->children.emplace_back(pTokRange, *subChLkOpt, pSecondVerb.chunk);
              else
                subChunkLkToHoldTheSubordinatePtr->chunk->children.emplace_back(pTokRange, ChunkLinkType::DIRECTOBJECT, pSecondVerb.chunk);
            }
            return true;
          }
          if (!secondChunkHasASubject &&
              !pTokRange.isEmpty() &&
              canLinkVerbToASubject(whereToLinkTheSubject(*pSecondVerb.chunk), *subChunkLkToHoldTheSubordinatePtr->chunk, fFlschecker, false))
          {
            subChunkLkToHoldTheSubordinatePtr->chunk->children.emplace_back(pTokRange, newChunkLiknType, pSecondVerb.chunk);
            return true;
          }
        }
        if (!chunkTypeIsAList(subChunkLkToHoldTheSubordinatePtr->chunk->type) &&
            (secondChunkHasASubject || chunkIsAtPresentParticiple(*pSecondVerb.chunk)))
        {
          subChunkLkToHoldTheSubordinatePtr->chunk->children.emplace_back(pTokRange, newChunkLiknType, pSecondVerb.chunk);
          return true;
        }
      }
    }
  }

  if (isACondition)
    newLkType = ChunkLinkType::IF;
  else if (!newLkType)
  {
    const auto& entRecognizer = fConf.getEntityRecognizer();
    newLkType = entRecognizer.findNatureOfAChunkLink(pSecondVerb, &pFirstVerb);

    if (!secondChunkHasASubject &&
        fConf.getLanguageType() == SemanticLanguageEnum::FRENCH)
    {
      recInList([&](Chunk& pSecondVerbChunkElt)
      {
        _refactorANewSubordinate(pFirstVerb, pSecondVerbChunkElt, pTokRange);
      }, *pSecondVerb.chunk);
    }
  }

  if (subChunkLkToHoldTheSubordinatePtr != nullptr && pTokRange.isEmpty() && !secondChunkHasASubject)
    return false;
  if (fConf.getLanguageType() == SemanticLanguageEnum::ENGLISH &&
      !pSecondVerb.chunk->requests.has(SemanticRequestType::ACTION))
    pSecondVerb.chunk->requests.clear();
  pFirstVerb.children.emplace_back(pTokRange, *newLkType, pSecondVerb.chunk);
  return true;
}


void ListExtractor::_refactorANewSubordinate(Chunk& pFirstVerb,
                                             Chunk& pSecondVerb,
                                             const TokenRange& pTokRange) const
{
  const InflectionsChecker& inflChecker = fConf.getFlsChecker();
  auto itDObj = getChunkLink(pSecondVerb, ChunkLinkType::DIRECTOBJECT);
  if (itDObj != pSecondVerb.children.end())
  {
    if (ConceptSet::haveAConcept(pSecondVerb.head->inflWords.front().infos.concepts, "verb_equal_be") &&
        doesTokRangeBeginsWithARequestSubordinate(pTokRange, fConf.getLingDico().statDb) &&
        canLinkVerbToASubject(pSecondVerb, *itDObj->chunk, inflChecker, false))
    {
      itDObj->type = ChunkLinkType::SUBJECT;
    }
  }

  if (recInListConst(chunkIsAtInfinitive, pSecondVerb))
  {
    auto itSubj = getChunkLink(pFirstVerb, ChunkLinkType::SUBJECT);
    if (itSubj != pFirstVerb.children.end())
    {
      auto tryToConvertObjChildToReflexive = [&](ChunkLink& pObjChkLink)
      {
        const InflectedWord& objInflWord = pObjChkLink.chunk->head->inflWords.front();
        if (objInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT &&
            fConf.getEntityRecognizer().pronounCompIsReflexiveOfASubject(itSubj->chunk->head->inflWords.front(), objInflWord))
          pObjChkLink.type = ChunkLinkType::REFLEXIVE;
      };

      auto itIndirectObj = getChunkLink(pSecondVerb, ChunkLinkType::INDIRECTOBJECT);
      if (itIndirectObj != pSecondVerb.children.end())
        tryToConvertObjChildToReflexive(*itIndirectObj);
      else
        if (itDObj != pSecondVerb.children.end())
          tryToConvertObjChildToReflexive(*itDObj);

    }
    else if (itDObj != pSecondVerb.children.end() &&
             !pFirstVerb.requests.empty() &&
             canLinkVerbToASubject(pFirstVerb, *itDObj->chunk, inflChecker, false))
    {
      itDObj->type = ChunkLinkType::SUBJECT;
      pFirstVerb.children.splice(pFirstVerb.children.end(), pSecondVerb.children, itDObj);
    }
  }
}


bool ListExtractor::_linkChunk_VerbChunk
(Chunk& pFirstChunk,
 ChunkLink& pSecondVerb,
 const TokenRange& pTokRange) const
{
  mystd::optional<ChunkLinkType> subType = _getAppropriateChunkLinkFromTokens(nullptr, pSecondVerb.chunk->introductingWordToSaveForSynthesis,
                                                                              pTokRange, pSecondVerb.chunk->type);
  if (!subType)
  {
    if (pFirstChunk.type == ChunkType::INTERJECTION_CHUNK)
      return false;
    subType = ChunkLinkType::SUBORDINATE_CLAUSE;
    if (!haveASubject(*pSecondVerb.chunk) &&
        canLinkVerbToASubject(whereToLinkTheSubject(*pSecondVerb.chunk), pFirstChunk,
                              fFlschecker, false))
      subType = ChunkLinkType::SUBJECT_OF;
  }
  pFirstChunk.children.emplace_back(pTokRange, *subType, pSecondVerb.chunk);
  return true;
}



bool ListExtractor::_canLinkDO_Subject
(Chunk& pFirstVerb,
 Chunk& pSecondVerb) const
{
  if (pSecondVerb.type == ChunkType::INFINITVE_VERB_CHUNK)
    return false;
  Chunk* dObj = getNoneVerbalDOComplOrSubjClauseChunk(pFirstVerb);
  if (dObj != nullptr && !haveASubject(pSecondVerb))
  {
    return canLinkVerbToASubject(whereToLinkTheSubject(pSecondVerb),
                                 *dObj, fFlschecker, false);
  }
  return false;
}




// complete an existing list
// =========================

std::list<ChunkLink>::iterator ListExtractor::_tryToCompleteAnExistingList
(std::list<ListToMove>& pListsToMove,
 ChunkLinkWorkingZone& pWorkingZone,
 std::list<ChunkLink>::iterator pItChunkLink) const
{
  auto itExistingList = pItChunkLink;
  ++itExistingList;
  while (itExistingList != pWorkingZone.end() &&
         itExistingList->chunk->type == ChunkType::SEPARATOR_CHUNK &&
         itExistingList->chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS)
    ++itExistingList;
  if (itExistingList == pWorkingZone.end())
    return pItChunkLink;

  const Chunk& currExistingChunk = *itExistingList->chunk;
  // if we will try to fill an existing list
  if (chunkTypeIsAList(currExistingChunk.type))
  {
    // init list info with the existing list
    ListInfo currList;
    currList.listChunk = &*itExistingList->chunk;
    currList.listType.emplace(currList.listChunk->type);
    bool firstListElt = true;
    for (auto itListElt = currList.listChunk->children.rbegin();
         itListElt != currList.listChunk->children.rend(); ++itListElt)
    {
      if (itListElt->type != ChunkLinkType::SIMPLE)
        continue;
      const Chunk& currChunkElt = *itListElt->chunk;

      if (firstListElt)
      {
        _initNewListInfo(currList, currChunkElt);
        firstListElt = false;
      }
      else
      {
        bool doesBecomeAListOfInfinitiveVerbs =
            _doesBecomeAListOfInfinitiveVerbs(currList, currChunkElt.type);
        _updateExistingListInfoAccordingToTheNewElt
            (currList, currChunkElt,
             doesBecomeAListOfInfinitiveVerbs);
      }
    }

    // try to expend the current list
    ChunkLinkIter currIt(pWorkingZone, itExistingList);
    int nbMaxOfListElt = -1;
    while (currIt.getIt() != pWorkingZone.begin())
    {
      --currIt;
      if (!_tryToCompleteListInfoWithAChunk(currIt, currList, pListsToMove,
                                           nbMaxOfListElt, pWorkingZone, false, false))
        break;
    }
    return itExistingList;
  }
  else if (currExistingChunk.type == ChunkType::SEPARATOR_CHUNK)
  {
    bool canHaveAnotherList = false;
    int nbMaxOfListElt = -1;
    ChunkLinkIter currIt(pWorkingZone, itExistingList);
    ListInfo currList;
    mystd::optional<ChunkType> noExceptionListType;
    _tryToExtractANewListAroundASeparator(currIt, currList, pListsToMove,
                                           canHaveAnotherList, nbMaxOfListElt,
                                           pWorkingZone, false, false, noExceptionListType);
    while (currList.listType &&
           currIt.getIt() != pWorkingZone.begin())
    {
      --currIt;
      if (_tryToCompleteListInfoWithAChunk(currIt, currList, pListsToMove,
                                           nbMaxOfListElt, pWorkingZone, false, false))
        continue;
      break;
    }
    return itExistingList;
  }

  return pItChunkLink;
}


void ListExtractor::_initNewListInfo(
    ListInfo& pCurrList,
    const Chunk& pFirstListEltChunk) const
{
  // save the characterisitics of the possible new list
  const Chunk& firstEltChk = getFirstListEltChunk(pFirstListEltChunk);
  pCurrList.eltType = firstEltChk.type;
  const auto& headInlfWord = pFirstListEltChunk.head->inflWords.front();
  pCurrList.firstHeadPartOfSpeech = headInlfWord.word.partOfSpeech;
  for (const auto& currCpt : headInlfWord.infos.concepts)
    pCurrList.concepts.emplace(currCpt.first);
  if (pCurrList.eltType == ChunkType::VERB_CHUNK)
  {
    _getVerbAndAuxInflections(pCurrList.commonVerbInflections, firstEltChk);
    pCurrList.requestsOfElts = pFirstListEltChunk.requests;
    pCurrList.hasASubject = haveASubject(pFirstListEltChunk);
  }
  else
  {
    pCurrList.hasASubject = false;
  }
}


bool ListExtractor::_updateExistingListInfoAccordingToTheNewElt(
    ListInfo& pCurrList,
    const Chunk& pNewListEltChunk,
    bool pDoesBecomeAListOfInfinitiveVerbs) const
{
  if (pCurrList.eltType == ChunkType::VERB_CHUNK)
  {
    if (pCurrList.requestsOfElts != pNewListEltChunk.requests &&
        !(pCurrList.requestsOfElts.empty() &&
          pNewListEltChunk.requests.has(SemanticRequestType::ACTION)))
      return false;

    const auto& headInlfWord = pNewListEltChunk.head->inflWords.front();
    for (auto itCpt = pCurrList.concepts.begin();
         itCpt != pCurrList.concepts.end(); )
    {
      if (headInlfWord.infos.concepts.count(*itCpt) == 0)
        itCpt = pCurrList.concepts.erase(itCpt);
      else
        ++itCpt;
    }
    VerbalInflections verbFlexions;
    _getVerbAndAuxInflections(verbFlexions, pNewListEltChunk);
    if (pDoesBecomeAListOfInfinitiveVerbs)
      pCurrList.commonVerbInflections = verbFlexions;
    else
      fFlschecker.intersectionOfSameVerbTenses
          (pCurrList.commonVerbInflections, verbFlexions);
    return !pCurrList.commonVerbInflections.inflections.empty();
  }
  return true;
}


bool ListExtractor::_doesBecomeAListOfInfinitiveVerbs(
    const ListInfo& pCurrList,
    const ChunkType& pNewChunkType) const
{
  return fConf.getLanguageType() == SemanticLanguageEnum::ENGLISH &&
      pNewChunkType == ChunkType::INFINITVE_VERB_CHUNK && pCurrList.eltType == ChunkType::VERB_CHUNK;
}



bool ListExtractor::_tryToCompleteListInfoWithAChunk(
    ChunkLinkIter& pCurrIt,
    ListInfo& pCurrList,
    std::list<ListToMove>& pListsToMove,
    int& pNbMaxOfListElt,
    const ChunkLinkWorkingZone& pWorkingZone,
    bool pOneWordList,
    bool pAllowToNotRepeatTheSubject) const
{
  ChunkLinkIter* currItSeparatorPtr = nullptr;
  ChunkLinkIter newEltListChunk = pCurrIt;
  if (pCurrIt.getIt()->chunk->type == ChunkType::SEPARATOR_CHUNK &&
      _isAListSeparator(pCurrIt.getIt()->chunk->head->inflWords.front(), *pCurrList.listType) &&
      pCurrIt.getIt() != pWorkingZone.begin())
  {
    currItSeparatorPtr = &pCurrIt;
    --newEltListChunk;
  }

  bool haveASubConjBefore = false;
  if (newEltListChunk.getIt() != pWorkingZone.begin())
  {
    auto itNextNextChunk = newEltListChunk.getIt();
    --itNextNextChunk;
    haveASubConjBefore = itNextNextChunk->chunk->type == ChunkType::SEPARATOR_CHUNK &&
        itNextNextChunk->chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION &&
        itNextNextChunk->chunk->head->inflWords.front().infos.hasContextualInfo(WordContextualInfos::SENTENCECANBEGINWITH);
  }

  const Chunk& newEltListChunkFirstElt = getFirstListEltChunk(*newEltListChunk.getIt()->chunk);
  if (!haveASubConjBefore &&
      _isNewEltCompatibleWithTheList(pCurrList, newEltListChunkFirstElt, pAllowToNotRepeatTheSubject))
  {
    // if we consider a list with several words in each element
    if (!pOneWordList)
    {
      if (fFlschecker.canBeAssociatedInAList(newEltListChunkFirstElt.head->inflWords.front(),
                                             getFirstListEltChunk(*pCurrList.listChunk).head->inflWords.front()))
      {
        if (currItSeparatorPtr != nullptr)
          currItSeparatorPtr->eraseIt();
        pCurrList.listChunk->children.push_front(*newEltListChunk.getIt());
        newEltListChunk.eraseIt();
        pCurrIt = newEltListChunk;
        --pNbMaxOfListElt;
        return true;
      }
    }
    // if we consider a list of words
    else
    {
      ChunkAndTokIt chunkBegin(*newEltListChunk.getIt()->chunk);
      ChunkAndTokIt chunkEnd(*pCurrList.listChunk->children.front().chunk);
      if (_getBeginNewEltAndEndOldElt(chunkBegin, chunkEnd, pCurrList.partOfSpeechAtBegining))
      {
        if (!chunkBegin.atBegin() &&
            !pCurrList.hasAConceptInCommon(chunkBegin.tokIt->inflWords.front().infos.concepts))
        {
          for (auto itTok = chunkBegin.getItBegin();
               itTok != chunkBegin.tokIt;
               itTok = getNextToken(itTok, chunkBegin.tokIt))
            if (pCurrList.hasAConceptInCommon(itTok->inflWords.front().infos.concepts))
              return false;
        }

        if (currItSeparatorPtr != nullptr)
          currItSeparatorPtr->eraseIt();
        bool chunkHasBeenRemoved =
            _addListElt(pCurrList.listChunk, newEltListChunk, pNbMaxOfListElt, chunkBegin.tokIt);
        pCurrIt = newEltListChunk;

        // if there is a chunk before the list
        if (!chunkHasBeenRemoved)
        {
          ChunkLinkIter listChunkRoot(newEltListChunk);
          ++listChunkRoot;
          assert(!listChunkRoot.atEnd());

          // the list becomes a child of the chunk before
          _addAListToMove(pListsToMove, ListToMove(newEltListChunk, listChunkRoot, false));
        }
        return true;
      }
    }
  }
  return false;
}


void ListExtractor::_moveSomeChunksAfterMAinAlgo(
    std::list<ListToMove>& pListsToMove,
    const ChunkLinkWorkingZone& pWorkingZone)
{
  // put list that are not in the root of the synt graph
  for (auto& currElt : pListsToMove)
  {
    assert(pWorkingZone.syntTreePtr() == currElt.itRootList.getList());

    PartOfSpeech headPartOfSpeech =
        currElt.newParent->chunk->head->inflWords.front().word.partOfSpeech;
    if (headPartOfSpeech == PartOfSpeech::PREPOSITION || headPartOfSpeech == PartOfSpeech::PARTITIVE)
    {
      currElt.newParent->tokRange = currElt.newParent->chunk->tokRange;
      currElt.newParent->chunk = currElt.itRootList->chunk;
      currElt.itRootList.getList()->erase(currElt.itRootList.getIt());
    }
    else
    {
      currElt.newParent.getIt()->chunk->children.splice(currElt.newParent.getIt()->chunk->children.end(),
                                                        *currElt.itRootList.getList(), currElt.itRootList.getIt());
    }
  }
}


bool ListExtractor::_tryToExtractANewListAroundASeparator(
    ChunkLinkIter& pCurrIt,
    ListInfo& pCurrList,
    std::list<ListToMove>& pListsToMove,
    bool& pCanHaveAnotherList,
    int& pNbMaxOfListElt,
    const ChunkLinkWorkingZone& pWorkingZone,
    bool pOneWordList,
    bool pAllowToNotRepeatTheSubject,
    const mystd::optional<ChunkType>& pExceptListType) const
{
  const auto& currItHeadConcepts =
      pCurrIt.getIt()->chunk->head->inflWords.front().infos.concepts;
  pCurrList.listType = getListType(currItHeadConcepts);
  if (pExceptListType && pCurrList.listType == *pExceptListType)
    return false;

  ChunkLinkIter prevIt = pCurrIt;
  ++prevIt;

  if (!pOneWordList &&
      !prevIt.atEnd() &&
      prevIt.getIt()->chunk->type == ChunkType::SEPARATOR_CHUNK)
  {
    const InflectedWord& prevItHeadInflWord = prevIt.getIt()->chunk->head->inflWords.front();

    if (pCurrList.listType == ChunkType::AND_CHUNK)
    {
      auto listType = getListType(prevItHeadInflWord.infos.concepts);
      if (listType)
        pCurrList.listType = listType;
    }
    if (!pCurrList.chunkToMoveInNextEltChunkLink ||
        pCurrList.chunkToMoveInNextEltChunkLink->atEnd())
      pCurrList.chunkToMoveInNextEltChunkLink = prevIt;
    ++prevIt;
  }

  if (pCurrList.listType &&
      !prevIt.atEnd() &&
      prevIt->type == ChunkLinkType::SIMPLE &&
      prevIt->chunk->type != ChunkType::SEPARATOR_CHUNK)
  {
    const Chunk& firstListEltChunk = *prevIt.getIt()->chunk;
    _initNewListInfo(pCurrList, firstListEltChunk);

    // if we are not at the first word
    if (pCurrIt.getIt() != pWorkingZone.begin())
    {
      ChunkLinkIter nextChunk(pCurrIt);
      --nextChunk;
      // if we consider a list with several words in each element
      if (!pOneWordList)
      {
        if (_addANewMultiWordsList(pCurrList, pListsToMove,
                                   pNbMaxOfListElt, prevIt, pCurrIt, nextChunk, pAllowToNotRepeatTheSubject))
          return true;
      }
      // if we consider a list of words
      else if (pCurrList.eltType == ChunkType::NOMINAL_CHUNK ||
               pCurrList.eltType == ChunkType::INFINITVE_VERB_CHUNK ||
               pCurrList.eltType == ChunkType::VERB_CHUNK)
      {
        const Chunk& nextChunkFirstElt = getFirstListEltChunk(*nextChunk->chunk);
        if (_isNewEltCompatibleWithTheList(pCurrList, nextChunkFirstElt, pAllowToNotRepeatTheSubject))
        {
          pCurrList.partOfSpeechAtBegining = getFirstPartOfSpeechOfAChunk(firstListEltChunk);
          // try to create a new list of words
          if (_addANewOneWordList(pCurrList, pListsToMove, pNbMaxOfListElt,
                                  prevIt, pCurrIt, nextChunk))
            return true;
        }
      }
      pCanHaveAnotherList = true;
    }
  }

  pCurrList.listType.reset();
  return false;
}


} // End of namespace linguistics
} // End of namespace onsem
