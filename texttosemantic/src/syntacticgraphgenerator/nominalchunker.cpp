#include "nominalchunker.hpp"
#include <iostream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include "listextractor.hpp"
#include "entityrecognizer.hpp"
#include "../tool/chunkshandler.hpp"



namespace onsem
{
namespace linguistics
{


NominalChunker::NominalChunker
(const AlgorithmSetForALanguage& pConfiguration)
  : fConf(pConfiguration),
    fLingDico(fConf.getLingDico())
{
}


void NominalChunker::process
(std::list<ChunkLink>& pFirstChildren) const
{
  if (fConf.getLanguageType() == SemanticLanguageEnum::ENGLISH)
    xExtractEnglishSPossessive(pFirstChildren);

  auto prevVerb = pFirstChildren.end();
  for (auto it = pFirstChildren.begin(); it != pFirstChildren.end(); ++it)
  {
    ChunkType chunkType = it->chunk->type;
    if (chunkTypeIsVerbal(chunkType))
    {
      xProcessBetween2VerbChunks(pFirstChildren, prevVerb, it);
      prevVerb = it;
    }
  }
  xProcessBetween2VerbChunks(pFirstChildren, prevVerb,
                             pFirstChildren.end());
}


void NominalChunker::xSplitPronounToken(ChunkLinkWorkingZone& pWorkingZone) const
{
  auto firstChunkLk = pWorkingZone.begin();
  if (firstChunkLk == pWorkingZone.end())
    return;

  TokIt endOfTokenList = firstChunkLk->chunk->tokRange.getItEnd();
  TokIt firstToken = getTheNextestToken(firstChunkLk->chunk->tokRange.getItBegin(), endOfTokenList, SkipPartOfWord::YES);
  if (firstToken != endOfTokenList &&
      partOfSpeech_isPronominal(firstToken->inflWords.front().word.partOfSpeech))
  {
    TokIt secondToken = getNextToken(firstToken, endOfTokenList);
    if (secondToken != endOfTokenList)
    {
      PartOfSpeech secondTokenPartOfSpeech = secondToken->inflWords.front().word.partOfSpeech;
      if (secondTokenPartOfSpeech == PartOfSpeech::PRONOUN ||
          secondTokenPartOfSpeech == PartOfSpeech::ADJECTIVE ||
          secondTokenPartOfSpeech == PartOfSpeech::ADVERB)
        separateBeginOfAChunk(pWorkingZone.syntTree(), firstChunkLk, secondToken,
                              firstChunkLk->chunk->type, fConf.getLanguageType());
    }
  }
}


void NominalChunker::xProcessBetween2VerbChunks
(std::list<ChunkLink>& pRootList,
 std::list<ChunkLink>::iterator pItFirstVerbChunk,
 std::list<ChunkLink>::iterator pItSecondVerbChunk) const
{
  auto afterFirstVerb = [&]
  {
    if (pItFirstVerbChunk == pRootList.end()) // if there is no first verb
      return pRootList.begin();
    return std::next(pItFirstVerbChunk);
  }();
  ChunkLinkWorkingZone workingZone(pRootList, afterFirstVerb, pItSecondVerbChunk);
  if (workingZone.empty())
    return;
  Chunk* firstVerbChunk =
      pItFirstVerbChunk == pRootList.end() ? nullptr : &*pItFirstVerbChunk->chunk;
  Chunk* secondVerbChunk =
      pItSecondVerbChunk == pRootList.end() ? nullptr : &*pItSecondVerbChunk->chunk;

  Chunk* subSecondVerbChunk = nullptr;
  if (secondVerbChunk != nullptr)
  {
    subSecondVerbChunk = linguistics::whereToLinkTheSubjectPtr(secondVerbChunk,
                                                               fConf.getFlsChecker());
    if (subSecondVerbChunk != nullptr)
      xSplitLastTokens(workingZone, {PartOfSpeech::PRONOUN_SUBJECT, PartOfSpeech::PRONOUN});
    else
      xSplitLastTokens(workingZone, {PartOfSpeech::PREPOSITION, PartOfSpeech::PARTITIVE});
  }
  xSplitPronounToken(workingZone);

  if (workingZone.empty())
    return;

  auto lastChunkLink = workingZone.end();
  --lastChunkLink;
  int nbOfNominalGroupsExpected =
      (firstVerbChunk != nullptr && firstVerbChunk->type != ChunkType::SEPARATOR_CHUNK) +
      (subSecondVerbChunk != nullptr && lastChunkLink->chunk->type != ChunkType::SEPARATOR_CHUNK);

  int nbMaxOfListElt = nbOfGNs(workingZone) - nbOfNominalGroupsExpected;
  bool canHaveAnotherList = false;
  if (nbMaxOfListElt > 0)
  {
    const auto& listExtractor = fConf.getListExtractor();
    canHaveAnotherList = listExtractor.extractLists(workingZone, true, false, nbMaxOfListElt);
  }

  if (canHaveAnotherList)
    fConf.getListExtractor().extractLists(workingZone, true, false, false, nbMaxOfListElt);

  for (auto itChunkLink = workingZone.beginListIter();
       itChunkLink != workingZone.endListIter(); ++itChunkLink)
    xSplitAChunk(itChunkLink);

  if (fConf.getLanguageType() == SemanticLanguageEnum::ENGLISH)
    xLinkOwners(workingZone);
  xLinkPartitivesOfWorkingZone(workingZone);

  for (auto itChunkLink = workingZone.beginListIter();
       itChunkLink != workingZone.endListIter(); ++itChunkLink)
  {
    if ((itChunkLink->chunk->type == ChunkType::NOMINAL_CHUNK ||
         itChunkLink->chunk->type == ChunkType::PREPOSITIONAL_CHUNK) &&
        itChunkLink->chunk->head->inflWords.front().word.partOfSpeech != PartOfSpeech::PRONOUN_SUBJECT)
    {
      auto itNextChunkLink = itChunkLink;
      ++itNextChunkLink;
      if (itNextChunkLink != workingZone.endListIter() &&
          itNextChunkLink->chunk->type == ChunkType::PREPOSITIONAL_CHUNK)
      {
        ChunkLinkType chkLkType = fConf.getEntityRecognizer().findNatureOfAChunkLink(*itNextChunkLink, nullptr, false);
        if (chkLkType == ChunkLinkType::BETWEEN)
          moveChildren(itChunkLink->chunk->children, itNextChunkLink, chkLkType);
      }
    }
  }

  if (nbMaxOfListElt != 0 && !workingZone.empty())
  {
    fConf.getListExtractor().extractLists(workingZone, false, false, true, nbMaxOfListElt);
    xLinkPartitivesOfWorkingZone(workingZone);
  }
}


void NominalChunker::xSplitAChunk(ChunkLinkIter& pItChunkLink) const
{
  static const std::vector<PartOfSpeech> subChunkSeps =
  {PartOfSpeech::PARTITIVE, PartOfSpeech::PREPOSITION, PartOfSpeech::DETERMINER,
   PartOfSpeech::ADVERB};
  SemanticLanguageEnum language = fConf.getLanguageType();

  Chunk& currChunk = *pItChunkLink->chunk;
  if (tokenIsMoreProbablyAType(*currChunk.tokRange.getItBegin(), PartOfSpeech::PREPOSITION) &&
      currChunk.type != ChunkType::INFINITVE_VERB_CHUNK)
    currChunk.type = ChunkType::PREPOSITIONAL_CHUNK;

  for (TokIt currIt = getTheNextestToken(currChunk.tokRange.getItBegin(), currChunk.tokRange.getItEnd(), SkipPartOfWord::YES);
       currIt != currChunk.tokRange.getItEnd();
       currIt = getNextToken(currIt, currChunk.tokRange.getItEnd(), SkipPartOfWord::YES))
  {
    TokIt itBeginOfWordGroup = goAtBeginOfWordGroup(currIt, currChunk.tokRange.getItBegin());
    if (itBeginOfWordGroup == currChunk.tokRange.getItBegin())
      continue;
    TokIt itTokenBefore = getPrevToken(itBeginOfWordGroup, currChunk.tokRange.getItBegin(), currChunk.tokRange.getItEnd());
    if (itTokenBefore == currChunk.tokRange.getItEnd())
      continue;
    bool canBeInANewChunk = !tokenIsMoreProbablyAType(*itTokenBefore, subChunkSeps);

    if (canBeInANewChunk)
    {
      const auto& tokenBeforeInflWord = *itTokenBefore->inflWords.begin();
      const auto& beginOfWordInflWord = *itBeginOfWordGroup->inflWords.begin();
      PartOfSpeech currentPartOfSpeech = beginOfWordInflWord.word.partOfSpeech;
      switch (currentPartOfSpeech)
      {
      case PartOfSpeech::PREPOSITION:
      {
        if (language == SemanticLanguageEnum::ENGLISH &&
            beginOfWordInflWord.infos.hasContextualInfo(WordContextualInfos::EN_TIMEWORD) &&
            (ConceptSet::haveAConceptThatBeginWith(tokenBeforeInflWord.infos.concepts, "number_") ||
             tokenBeforeInflWord.infos.hasContextualInfo(WordContextualInfos::EN_TIMEWORD)))
          break;
        TokIt currNext = getNextToken(currIt, currChunk.tokRange.getItEnd(), SkipPartOfWord::YES);
        separateBeginOfAChunk(pItChunkLink, itBeginOfWordGroup, currChunk.type, language);
        currChunk.type = ChunkType::PREPOSITIONAL_CHUNK;
        currIt = currNext;
        if (currIt == currChunk.tokRange.getItEnd())
          return;
        break;
      }
      case PartOfSpeech::PARTITIVE:
      {
        TokIt currNext = getNextToken(currIt, currChunk.tokRange.getItEnd(), SkipPartOfWord::YES);
        separateBeginOfAChunk(pItChunkLink, itBeginOfWordGroup, currChunk.type, language);
        currChunk.type = ChunkType::NOMINAL_CHUNK;
        currIt = currNext;
        if (currIt == currChunk.tokRange.getItEnd())
          return;
        break;
      }
      case PartOfSpeech::DETERMINER:
      {
        if (language == SemanticLanguageEnum::ENGLISH)
        {
          if (ConceptSet::haveAConceptThatBeginWith(beginOfWordInflWord.infos.concepts, "number_") &&
              (ConceptSet::haveAConceptThatBeginWith(tokenBeforeInflWord.infos.concepts, "number_") ||
               tokenBeforeInflWord.infos.hasContextualInfo(WordContextualInfos::EN_TIMEWORD)))
            break;
        }
        separateBeginOfAChunk(pItChunkLink, itBeginOfWordGroup, currChunk.type, language);
        currChunk.type = ChunkType::NOMINAL_CHUNK;
        xSplitAChunk(pItChunkLink);
        return;
      }
      case PartOfSpeech::ADVERB:
      {
        separateBeginOfAChunk(pItChunkLink, itBeginOfWordGroup, currChunk.type, language);
        currChunk.type = ChunkType::NOMINAL_CHUNK;
        xSplitAChunk(pItChunkLink);
        return;
      }
      case PartOfSpeech::NOUN:
      {
        if (tokenBeforeInflWord.word.partOfSpeech != PartOfSpeech::ADJECTIVE &&
            ConceptSet::haveAConceptThatBeginWith(currIt->inflWords.front().infos.concepts, "time_"))
        {
          separateBeginOfAChunk(pItChunkLink, itBeginOfWordGroup, currChunk.type, language);
          currChunk.type = ChunkType::NOMINAL_CHUNK;
          xSplitAChunk(pItChunkLink);
          return;
        }
        break;
      }
      default:
        break;
      }
    }
  }
}


void NominalChunker::xLinkPartitivesOfWorkingZone(ChunkLinkWorkingZone& pWorkingZone) const
{
  auto itChunkLink = pWorkingZone.endListIter();
  if (itChunkLink != pWorkingZone.beginListIter())
  {
    --itChunkLink;
    if (itChunkLink != pWorkingZone.beginListIter())
    {
      auto itNext = itChunkLink;
      while (itChunkLink != pWorkingZone.beginListIter())
      {
        itNext = itChunkLink;
        --itChunkLink;
        Chunk& currChunk = *itChunkLink->chunk;
        const auto& entityRecognizer = fConf.getEntityRecognizer();
        linkPartitives(currChunk, itNext, entityRecognizer);
      }
    }
  }
}


void NominalChunker::xLinkOwners(ChunkLinkWorkingZone& pWorkingZone) const
{
  auto itChunkLink = pWorkingZone.beginListIter();
  while (!itChunkLink.atEnd())
  {
    if (itChunkLink->type != ChunkLinkType::SIMPLE)
    {
      auto itOwner = itChunkLink;
      ++itChunkLink;
      if (itChunkLink.atEnd())
        return;
      itChunkLink->chunk->children.emplace_front(*itOwner);
      itOwner.eraseIt();
      continue;
    }
    ++itChunkLink;
  }
}




void NominalChunker::xSplitLastTokens
(ChunkLinkWorkingZone& pWorkingZone,
 const std::vector<PartOfSpeech>& pSplitLastToken) const
{
  auto lastChunk = --pWorkingZone.end();
  TokIt lastToken = getPrevToken(lastChunk->chunk->tokRange.getItEnd(),
                                 lastChunk->chunk->tokRange.getItBegin(),
                                 lastChunk->chunk->tokRange.getItEnd());
  if (lastToken != lastChunk->chunk->tokRange.getItEnd() &&
      lastToken != lastChunk->chunk->tokRange.getItBegin() &&
      tokenIsMoreProbablyAType(*lastToken, pSplitLastToken))
  {
    // don't split the last token if there is a preposition before
    TokIt beforeLastToken = getPrevToken(lastToken,
                                         lastChunk->chunk->tokRange.getItBegin(),
                                         lastChunk->chunk->tokRange.getItEnd());
    if (beforeLastToken != lastChunk->chunk->tokRange.getItEnd() &&
        beforeLastToken->inflWords.front().word.partOfSpeech == PartOfSpeech::PREPOSITION)
      return;

    if (lastToken->inflWords.front().word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT)
    {
      TokIt beforeLastToken = getPrevToken(lastToken,
                                           lastChunk->chunk->tokRange.getItBegin(),
                                           lastChunk->chunk->tokRange.getItEnd());
      if (beforeLastToken != lastChunk->chunk->tokRange.getItEnd() &&
          beforeLastToken->inflWords.front().word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT)
        return;
    }

    separateBeginOfAChunk(pWorkingZone.syntTree(), lastChunk, lastToken,
                          lastChunk->chunk->type, fConf.getLanguageType());
  }
}



void NominalChunker::xExtractEnglishSPossessive
(std::list<ChunkLink>& pFirstChildren) const
{
  if (pFirstChildren.empty())
  {
    return;
  }
  TokIt itLastToken = pFirstChildren.front().tokRange.getTokList().end();
  auto itLastChkLk = pFirstChildren.end();
  TokPosInAChunk itOwner(itLastToken, itLastChkLk); // it of owner
  TokPosInAChunk itPoss(itLastToken, itLastChkLk); // it of "'"
  TokPosInAChunk itSPoss(itLastToken, itLastChkLk); // it of "s",  if exist
  TokPosInAChunk itMain(itLastToken, itLastChkLk); // it of main word

  auto tryToAddAOwner = [&itOwner](
      TokIt pItTok,
      std::list<ChunkLink>::iterator pItChlds)
  {
    itOwner.itToken = pItTok;
    itOwner.itChkLk = pItChlds;
  };

  // the "'" character of the possessive structure
  for (auto itChlds = pFirstChildren.begin(); itChlds != pFirstChildren.end(); ++itChlds)
  {
    TokIt itFirstTok = itChlds->chunk->tokRange.getItBegin();
    for (TokIt itTok = itFirstTok;
         itTok != itChlds->chunk->tokRange.getItEnd(); ++itTok)
    {
      const InflectedWord& tokInflWord = *itTok->inflWords.begin();
      PartOfSpeech tokPartOfSpeech = tokInflWord.word.partOfSpeech;
      if (tokPartOfSpeech == PartOfSpeech::BOOKMARK)
        continue;

      if (itPoss.isEmpty())
      {
        if (tokPartOfSpeech == PartOfSpeech::INTERSPACE)
        {
          if (!itTok->str.empty() &&
              itTok->str[0] == '\'' &&
              !itOwner.isEmpty())
          {
            itPoss.itToken = itTok;
            itPoss.itChkLk = itChlds;
          }
        }
        else if (partOfSpeech_isNominal(tokPartOfSpeech))
        {
          if (itOwner.isEmpty())
            tryToAddAOwner(itTok, itChlds);
        }
        else
        {
          itOwner.clear();
        }
      }
      else
      {
        if (partOfSpeech_isAWord(tokPartOfSpeech))
        {
          if (itTok->str == "s")
          {
            itSPoss.itToken = itTok;
            itSPoss.itChkLk = itChlds;
          }
          else
          {
            itMain.itToken = itTok;
            itMain.itChkLk = itChlds;
            xEnglishSPossessiveAddAOwner(pFirstChildren, itOwner, itPoss,
                                         itSPoss, itMain);
            tryToAddAOwner(itTok, itChlds);
          }
        }
        else if (tokPartOfSpeech == PartOfSpeech::PUNCTUATION)
        {
          xEnglishSPossessiveClearContext(itOwner, itPoss,
                                          itSPoss, itMain);
        }
      }
    }
  }
}



void NominalChunker::xEnglishSPossessiveAddAOwner
(std::list<ChunkLink>& pFirstChildren,
 TokPosInAChunk& pItOwner,
 TokPosInAChunk& pItPoss,
 TokPosInAChunk& pItSPoss,
 TokPosInAChunk& pItMain) const
{
  if (pItSPoss.isEmpty())
  {
    // itSPoss has to point also to a token
    // even if it's the same (to simply algo)
    pItSPoss = pItPoss;
  }

  // separate chunks: possessive - main
  if (pItSPoss.itChkLk == pItMain.itChkLk)
  {
    TokIt itBeginMain = getNextToken(pItSPoss.itToken, pItMain.itToken);
    if (itBeginMain != pItMain.itChkLk->chunk->tokRange.getItEnd())
    {
      separateBeginOfAChunk(pFirstChildren, pItMain.itChkLk, itBeginMain,
                            pItMain.itChkLk->chunk->type,
                            SemanticLanguageEnum::ENGLISH);
      if (pItPoss.itChkLk == pItSPoss.itChkLk)
        --pItPoss.itChkLk;
      if (pItOwner.itChkLk == pItSPoss.itChkLk)
        --pItOwner.itChkLk;
      --pItSPoss.itChkLk;
    }
  }

  // separate chunks: owner - possessive
  if (pItOwner.itChkLk == pItPoss.itChkLk)
  {
    separateEndOfAChunk(pFirstChildren, pItOwner.itChkLk, pItPoss.itToken,
                        ChunkType::SEPARATOR_CHUNK, SemanticLanguageEnum::ENGLISH);
    if (pItSPoss.itChkLk == pItPoss.itChkLk)
      ++pItSPoss.itChkLk;
    ++pItPoss.itChkLk;
  }

  ChunkLink& ownerChkLk = *pItOwner.itChkLk;
  ownerChkLk.type = fConf.getEntityRecognizer().findNatureOfAChunkLink(ownerChkLk, nullptr, true);
  ownerChkLk.tokRange.setItBegin(ownerChkLk.chunk->tokRange.getItEnd());
  ownerChkLk.tokRange.setItEnd(pItMain.itChkLk->chunk->tokRange.getItBegin());
  if (pItSPoss.itChkLk != pItPoss.itChkLk)
    pFirstChildren.erase(pItSPoss.itChkLk);
  pFirstChildren.erase(pItPoss.itChkLk);
  xEnglishSPossessiveClearContext(pItOwner, pItPoss,
                                  pItSPoss, pItMain);

}


void NominalChunker::xEnglishSPossessiveClearContext
(TokPosInAChunk& pItOwner,
 TokPosInAChunk& pItPoss,
 TokPosInAChunk& pItSPoss,
 TokPosInAChunk& pItMain)
{
  pItOwner.clear();
  pItPoss.clear();
  pItSPoss.clear();
  pItMain.clear();
}


} // End of namespace linguistics
} // End of namespace onsem
