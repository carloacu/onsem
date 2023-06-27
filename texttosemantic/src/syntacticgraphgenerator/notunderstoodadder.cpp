#include "notunderstoodadder.hpp"
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include "../tool/chunkshandler.hpp"


namespace onsem
{
namespace linguistics
{

namespace
{

bool _tryToAddOtherPersonForPresentOfIndicative(Inflections& pInflections)
{
  bool res = false;
  bool hasFirstPersonOfIndicative = false;
  bool hasSecondPersonOfIndicative = false;
  bool hasThirdPersonOfIndicative = false;
  VerbalInflections* verbalIPtr = pInflections.getVerbalIPtr();
  if (verbalIPtr != nullptr)
  {
    VerbalInflections& verbalI = *verbalIPtr;
    for (const VerbalInflection& currVerbalI : verbalI.inflections)
    {
      if (currVerbalI.tense == LinguisticVerbTense::PRESENT_INDICATIVE)
      {
        switch (currVerbalI.person)
        {
        case RelativePerson::FIRST_SING:
          hasFirstPersonOfIndicative = true;
          break;
        case RelativePerson::SECOND_SING:
          hasSecondPersonOfIndicative = true;
          break;
        case RelativePerson::THIRD_SING:
          hasThirdPersonOfIndicative = true;
          break;
        default:
          break;
        }
      }
    }

    if (hasFirstPersonOfIndicative || hasSecondPersonOfIndicative ||
        hasThirdPersonOfIndicative)
    {
      if (!hasFirstPersonOfIndicative)
      {
        verbalI.inflections.emplace_back("P1s");
        res = true;
      }
      if (!hasSecondPersonOfIndicative)
      {
        verbalI.inflections.emplace_back("P2s");
        res = true;
      }
      if (!hasThirdPersonOfIndicative)
      {
        verbalI.inflections.emplace_back("P3s");
        res = true;
      }
    }
  }
  return res;
}

bool _tryToAddOtherPersonForPresentOfIndicativeFromInflWords(std::list<InflectedWord>& pInflWords)
{
  bool res = false;
  for (auto& currInflWord : pInflWords)
    if (_tryToAddOtherPersonForPresentOfIndicative(currInflWord.inflections()))
      res = true;
  return res;
}


bool _tryToIncreaseOriginalVerbInflections(Token& pVerbToken)
{
  bool res = false;
  if (!pVerbToken.thisTokenAlreadyCausedARestart)
  {
    pVerbToken.thisTokenAlreadyCausedARestart = true;
    res = _tryToAddOtherPersonForPresentOfIndicativeFromInflWords(pVerbToken.inflWords);
  }
  return res;
}


}



bool addNotUnderstood(std::list<ChunkLink>& pChunkList,
                      std::size_t& pNbOfNotUnderstood,
                      std::size_t& pNbOfSuspiciousChunks,
                      const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                      const InflectionsChecker& pInlfChecker,
                      const LinguisticDictionary& pLingDico)
{
  bool needToRestart = false;
  bool isFirstChunk = true;
  for (auto it = pChunkList.begin(); it != pChunkList.end(); ++it)
  {
    Chunk& currChunk = *it->chunk;

    switch (currChunk.type)
    {
    case ChunkType::VERB_CHUNK:
    {
      bool isNotUnderstood = false;
      if (!currChunk.requests.has(SemanticRequestType::SUBJECT) &&
          !currChunk.requests.has(SemanticRequestType::ACTION) &&
          !haveASubject(currChunk))
      {
        if (currChunk.requests.has(SemanticRequestType::OBJECT))
        {
          ChunkLink* doChkLkPtr = getDOChunkLink(currChunk);
          if (doChkLkPtr == nullptr ||
              checkOrder(currChunk, *doChkLkPtr->chunk))
            break;
        }
        isNotUnderstood = true;
        if (isFirstChunk && currChunk.requests.empty())
        {
          bool subjectCanBeInAPreviousText = pLingDico.getLanguage() == SemanticLanguageEnum::FRENCH ?
                !pInlfChecker.verbIsOnlyAtPresentOrPastParticiple(currChunk.head->inflWords.front()) :
                !pInlfChecker.verbIsOnlyAtPastParticiple(currChunk.head->inflWords.front());
          if (subjectCanBeInAPreviousText)
            isNotUnderstood = false;
        }
      }

      for (const auto& currChildChunkLink : currChunk.children)
      {
        Chunk& childChunk = *currChildChunkLink.chunk;
        TokIt itPrevTok = childChunk.tokRange.getItBegin();
        for (TokIt itTok = getNextToken(itPrevTok, childChunk.tokRange.getItEnd()); itTok != childChunk.tokRange.getItEnd();
             itTok = getNextToken(itTok, childChunk.tokRange.getItEnd()))
        {
          auto& prevTokInflWord = itPrevTok->inflWords.front();
          auto& tokInflWord = itTok->inflWords.front();
          if (!pInlfChecker.areCompatibles(prevTokInflWord, tokInflWord) ||
              !InflectionsChecker::areInflectionsCompatibles(prevTokInflWord.inflections(), tokInflWord.inflections()))
          {
            ++pNbOfSuspiciousChunks;
            break;
          }
          itPrevTok = itTok;
        }
      }

      if (isNotUnderstood)
      {
        ++pNbOfNotUnderstood;
        it->type = ChunkLinkType::NOTUNDERSTOOD;
        Token& verbToken = *currChunk.head;
        if (pSpellingMistakeTypesPossible.count(SpellingMistakeType::CONJUGATION) == 1 &&
            _tryToIncreaseOriginalVerbInflections(verbToken))
          needToRestart = true;

        if (it != pChunkList.begin())
        {
          auto itPrev = it;
          --itPrev;
          if (itPrev->chunk->type == ChunkType::NOMINAL_CHUNK)
          {
            itPrev->type = ChunkLinkType::NOTUNDERSTOOD;

            Token& prevToken = *itPrev->chunk->head;
            if (prevToken.getTokenLinkage() == TokenLinkage::STANDALONE)
            {
              std::list<InflectedWord> inflWords;
              std::string lowerCaseStr = prevToken.str;
              bool wasAtUpperCase = lowerCaseFirstLetter(lowerCaseStr, 0);
              pLingDico.getGramPossibilities(inflWords, lowerCaseStr, 0, lowerCaseStr.size());
              if (wasAtUpperCase)
              {
                InflectedWord properNounIgram;
                properNounIgram.word.partOfSpeech = PartOfSpeech::PROPER_NOUN;
                properNounIgram.word.lemma = prevToken.str;
                inflWords.emplace_back(std::move(properNounIgram));
              }
              if (!inflWords.empty())
                prevToken.inflWords.swap(inflWords);
            }
          }
        }
      }
      break;
    }
    case ChunkType::PREPOSITIONAL_CHUNK:
    {
      if (currChunk.getHeadPartOfSpeech() == PartOfSpeech::PRONOUN_SUBJECT)
        it->type = ChunkLinkType::NOTUNDERSTOOD;
      break;
    }
    case ChunkType::AND_CHUNK:
    {
      std::size_t subNbOfNotUnderstood = 0;
      needToRestart = addNotUnderstood(currChunk.children, subNbOfNotUnderstood,
                                       pNbOfSuspiciousChunks, pSpellingMistakeTypesPossible,
                                       pInlfChecker, pLingDico) || needToRestart;
      if (subNbOfNotUnderstood > 0)
      {
        pNbOfNotUnderstood += subNbOfNotUnderstood;
        it->type = ChunkLinkType::NOTUNDERSTOOD;
      }
      break;
    }
    default:
      break;
    }
    isFirstChunk = false;
  }

  return needToRestart;
}


} // End of namespace linguistics
} // End of namespace onsem
