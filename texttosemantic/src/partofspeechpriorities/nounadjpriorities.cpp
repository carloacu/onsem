#include "nounadjpriorities.hpp"
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>

namespace onsem
{
namespace linguistics
{


void detNounPriorities(std::vector<Token>& pTokens,
                       const InflectionsChecker& pInflsCheker)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currInflWord = inflWords.front();
    if (currInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER)
    {
      auto itNext = getNextToken(itTok, pTokens.end());
      if (itNext != pTokens.end())
      {
        const InflectedWord& nextInflWord = itNext->inflWords.front();
        bool firstPartOfSpeechIsOk = true;
        if (nextInflWord.word.partOfSpeech == PartOfSpeech::NOUN)
          firstPartOfSpeechIsOk = pInflsCheker.areCompatibles(currInflWord, nextInflWord);
        else if (nextInflWord.word.partOfSpeech == PartOfSpeech::VERB)
          firstPartOfSpeechIsOk = false;

        if (!firstPartOfSpeechIsOk)
        {
          auto itOtherInfl = itNext->inflWords.begin();
          if (itOtherInfl != itNext->inflWords.end())
          {
            ++itOtherInfl;
            for (; itOtherInfl != itNext->inflWords.end(); ++itOtherInfl)
            {
              if (pInflsCheker.areCompatibles(currInflWord, *itOtherInfl))
              {
                putOnTop(itNext->inflWords, itOtherInfl);
                break;
              }
            }
          }
        }
      }
    }
  }
}



void pronNounPriorities(std::vector<Token>& pTokens,
                        const InflectionsChecker& pInflsCheker)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currInflWord = inflWords.front();
    if (currInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN)
    {
      auto itNext = getNextToken(itTok, pTokens.end());
      if (itNext != pTokens.end())
      {
        const InflectedWord& nextInflWord = itNext->inflWords.front();
        if (nextInflWord.word.partOfSpeech == PartOfSpeech::NOUN &&
            !pInflsCheker.areCompatibles(currInflWord, nextInflWord))
        {
          bool prioritiesChanged = false;

          auto itPronOtherInfl = inflWords.begin();
          if (itPronOtherInfl != inflWords.end())
          {
            ++itPronOtherInfl;
            for (; itPronOtherInfl != inflWords.end(); ++itPronOtherInfl)
            {
              if (pInflsCheker.areCompatibles(*itPronOtherInfl, nextInflWord))
              {
                putOnTop(inflWords, itPronOtherInfl);
                prioritiesChanged = true;
                break;
              }
            }
          }


          if (!prioritiesChanged)
          {
            auto itOtherInfl = itNext->inflWords.begin();
            if (itOtherInfl != itNext->inflWords.end())
            {
              ++itOtherInfl;
              for (; itOtherInfl != itNext->inflWords.end(); ++itOtherInfl)
              {
                if (pInflsCheker.areCompatibles(currInflWord, *itOtherInfl))
                {
                  putOnTop(itNext->inflWords, itOtherInfl);
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
}

void nounDetPriorities(std::vector<Token>& pTokens,
                       const InflectionsChecker& pInflsCheker)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currInflWord = inflWords.front();
    if (partOfSpeech_isNominal(currInflWord.word.partOfSpeech))
    {
      auto itNext = getNextToken(itTok, pTokens.end());
      if (itNext != pTokens.end())
      {
        const InflectedWord& nextInflWord = itNext->inflWords.front();
        if (nextInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER)
        {
          auto itOtherInfl = inflWords.begin();
          if (itOtherInfl != inflWords.end())
          {
            ++itOtherInfl;
            for (; itOtherInfl != inflWords.end(); ++itOtherInfl)
            {
              if (pInflsCheker.areCompatibles(*itOtherInfl, nextInflWord))
              {
                putOnTop(inflWords, itOtherInfl);
                break;
              }
            }
          }
        }
      }
    }
  }
}


void verbPriorities(std::vector<Token>& pTokens,
                    const InflectionsChecker& pInflsCheker)
{
  const InflectedWord* prevInflWordPtr = nullptr;
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& firstVerbInflWord = inflWords.front();
    if (firstVerbInflWord.word.partOfSpeech == PartOfSpeech::VERB &&
        inflWords.size() > 1)
    {
      const InflectedWord* secondVerbInflWord = nullptr;
      for (auto itInflWord = ++inflWords.begin(); itInflWord != inflWords.end(); ++itInflWord)
      {
        if (itInflWord->word.partOfSpeech == PartOfSpeech::VERB)
        {
          secondVerbInflWord = &*itInflWord;
          break;
        }
      }

      if (secondVerbInflWord != nullptr &&
          !pInflsCheker.verbIsConjAtPerson(firstVerbInflWord, RelativePerson::THIRD_SING))
      {
        putOnBottom(inflWords, inflWords.begin());
      }
      else if (prevInflWordPtr != nullptr &&
               prevInflWordPtr->word.partOfSpeech == PartOfSpeech::VERB &&
               pInflsCheker.verbIsAtInfinitive(*prevInflWordPtr) &&
               pInflsCheker.verbCanBeAtImperative(firstVerbInflWord))
      {
        auto itNextTok = getNextToken(itTok, pTokens.end());
        if (itNextTok != pTokens.end() &&
            itNextTok->inflWords.front().word.partOfSpeech == PartOfSpeech::VERB &&
            pInflsCheker.verbCantHaveSubject(itNextTok->inflWords.front()))
          putOnBottom(inflWords, inflWords.begin());
      }
    }
    prevInflWordPtr = &itTok->inflWords.front();
  }
}


void verbPrioritiesFr(std::vector<Token>& pTokens,
                      const InflectionsChecker& pInflsCheker)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& firstVerbInflWord = inflWords.front();
    if (firstVerbInflWord.word.partOfSpeech == PartOfSpeech::VERB &&
        inflWords.size() > 1)
    {
      auto itNextTok = getNextToken(itTok, pTokens.end());
      if (itNextTok != pTokens.end() &&
          itNextTok->inflWords.front().word.partOfSpeech == PartOfSpeech::VERB &&
          (!pInflsCheker.verbIsAtInfinitive(itNextTok->inflWords.front()) &&
           !pInflsCheker.verbIsOnlyAtPresentOrPastParticiple(itNextTok->inflWords.front()) &&
           (!pInflsCheker.verbCanBeAtImperative(itNextTok->inflWords.front()) ||
            !ConceptSet::haveAConcept(firstVerbInflWord.infos.concepts, "verb_action_say"))))
        putOnBottom(inflWords, inflWords.begin());
    }
  }
}


void nounAdjPriorities(std::vector<Token>& pTokens)
{
  bool insideANounAdjGroup = false;
  std::list<InflectedWord>* inflWordOfAPotentialNoun = nullptr;
  std::list<std::list<InflectedWord>*> inflWordOfAPotentialAdj;

  for (TokIt itTok = getPrevToken(pTokens.end(), pTokens.begin(), pTokens.end()); itTok != pTokens.end();
       itTok = getPrevToken(itTok, pTokens.begin(), pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currIGram = inflWords.front();
    if (currIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE ||
        partOfSpeech_isNominal(currIGram.word.partOfSpeech))
    {
      if (insideANounAdjGroup)
      {
        if (currIGram.word.partOfSpeech != PartOfSpeech::ADJECTIVE &&
            hasAPartOfSpeech(inflWords, PartOfSpeech::ADJECTIVE))
          inflWordOfAPotentialAdj.push_back(&inflWords);
      }
      else
      {
        if (currIGram.word.partOfSpeech != PartOfSpeech::NOUN &&
            hasAPartOfSpeech(inflWords, PartOfSpeech::NOUN))
          inflWordOfAPotentialNoun = &inflWords;
        insideANounAdjGroup = true;
      }
    }
    else if (currIGram.word.partOfSpeech != PartOfSpeech::CONJUNCTIVE)
    {
      if (currIGram.word.partOfSpeech == PartOfSpeech::DETERMINER)
      {
        if (inflWordOfAPotentialNoun != nullptr)
          putOnTop(*inflWordOfAPotentialNoun, PartOfSpeech::NOUN);
        for (auto& currInflWord : inflWordOfAPotentialAdj)
          putOnTop(*currInflWord, PartOfSpeech::ADJECTIVE);
      }

      insideANounAdjGroup = false;
      inflWordOfAPotentialNoun = nullptr;
      inflWordOfAPotentialAdj.clear();
    }
  }
}


void NounPrioritiesFr(std::vector<Token>& pTokens,
                      const InflectionsChecker& pInflsCheker,
                      bool pIsRootLevel)
{
  Token* tokenAfterPtr = nullptr;
  Token* tokenAfterAfterPtr = nullptr;
  auto updateTokenAfterPtrs = [&](TokIt& pTokIt)
  {
    tokenAfterAfterPtr = tokenAfterPtr;
    tokenAfterPtr = &*pTokIt;
  };

  for (TokIt itTok = getPrevToken(pTokens.end(), pTokens.begin(), pTokens.end()); itTok != pTokens.end();
       itTok = getPrevToken(itTok, pTokens.begin(), pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    auto itFirstInflWord = inflWords.begin();
    const InflectedWord& currIGram = *itFirstInflWord;
    if (partOfSpeech_isNominal(itFirstInflWord->word.partOfSpeech))
    {
      bool needToContinue = false;
      {
        auto itInflWord = itFirstInflWord;
        ++itInflWord;
        while (itInflWord != inflWords.end())
        {
          if ((itInflWord->word.partOfSpeech == PartOfSpeech::DETERMINER &&
               !ConceptSet::haveAConceptThatBeginWith(itInflWord->infos.concepts, "number_")) ||
              itInflWord->word.partOfSpeech == PartOfSpeech::INTERJECTION ||
              (isAtBeginOfASentence(pTokens, itTok) && pInflsCheker.verbCanBeAtImperative(*itInflWord)) ||
              pInflsCheker.verbIsAtInfinitive(*itInflWord))
          {
            putOnTop(inflWords, itInflWord);
            needToContinue = true;
            break;
          }
          ++itInflWord;
        }
      }

      if (needToContinue)
      {
        updateTokenAfterPtrs(itTok);
        continue;
      }

      bool tryToMoveProperNounHasHigherPriority = false;
      if (tokenAfterPtr != nullptr)
      {
        Token& tokenAfter = *tokenAfterPtr;
        // If a possible proper noun surrounded by 2 proper nouns
        if (currIGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN)
        {
          if (tokenAfterAfterPtr != nullptr &&
              tokenAfterAfterPtr->getPartOfSpeech() == PartOfSpeech::PROPER_NOUN)
          {
            auto itProperNoun = getInflWordWithASpecificPartOfSpeech
                (tokenAfter.inflWords, PartOfSpeech::PROPER_NOUN);
            if (itProperNoun != tokenAfter.inflWords.end())
            {
              putOnTop(tokenAfter.inflWords, itProperNoun);
              updateTokenAfterPtrs(itTok);
              continue;
            }
            PartOfSpeech& tokenAfterPos = tokenAfter.inflWords.front().word.partOfSpeech;
            if (tokenAfterPos == PartOfSpeech::UNKNOWN)
              tokenAfterPos = PartOfSpeech::PROPER_NOUN;
          }

          if (itTok != pTokens.begin())
          {
            auto itTokenBefore = getPrevToken(itTok, pTokens.begin(), pTokens.end());
            if (itTokenBefore != pTokens.end() &&
                itTokenBefore->getPartOfSpeech() == PartOfSpeech::ADJECTIVE)
            {
              auto itNoun = getInflWordWithASpecificPartOfSpeech(inflWords, PartOfSpeech::NOUN);
              if (itNoun != inflWords.end())
              {
                putOnTop(inflWords, itNoun);
                updateTokenAfterPtrs(itTok);
                continue;
              }

              auto itNounOfTokenBefore = getInflWordWithASpecificPartOfSpeech(itTokenBefore->inflWords, PartOfSpeech::NOUN);
              if (itNounOfTokenBefore != inflWords.end())
              {
                putOnTop(itTokenBefore->inflWords, itNounOfTokenBefore);
                updateTokenAfterPtrs(itTok);
                continue;
              }
            }
          }
        }

        PartOfSpeech partOfSpeechAfter = tokenAfter.inflWords.front().word.partOfSpeech;
        if (partOfSpeechAfter == PartOfSpeech::PRONOUN)
        {
          auto itInflWord = itFirstInflWord;
          ++itInflWord;
          while (itInflWord != inflWords.end())
          {
            if (pInflsCheker.verbCanBeAtImperative(*itInflWord))
            {
              putOnTop(inflWords, itInflWord);
              break;
            }
            ++itInflWord;
          }
        }
        else if (partOfSpeech_isNominal(partOfSpeechAfter))
        {
          auto itDet = getInflWordWithASpecificPartOfSpeech(inflWords, PartOfSpeech::DETERMINER);
          if (itDet != inflWords.end())
          {
            putOnTop(inflWords, itDet);
            for (auto itInfl = tokenAfter.inflWords.begin(); itInfl != tokenAfter.inflWords.end(); ++itInfl)
            {
              if (pInflsCheker.areCompatibles(*itDet, *itInfl))
              {
                putOnTop(tokenAfter.inflWords, itInfl);
                break;
              }
            }
          }
          else
          {
            for (auto itInfl = tokenAfter.inflWords.begin(); itInfl != tokenAfter.inflWords.end(); ++itInfl)
            {
              if (itInfl->word.partOfSpeech == PartOfSpeech::PROPER_NOUN)
              {
                if (currIGram.word.partOfSpeech == PartOfSpeech::NOUN)
                {
                  putOnTop(inflWords, PartOfSpeech::PROPER_NOUN);
                  continue;
                }
              }
            }
            if (tokenAfter.inflWords.size() > 1)
            {
              putOnBottom(tokenAfter.inflWords, tokenAfter.inflWords.begin());
              for (auto itInfl = tokenAfter.inflWords.begin(); itInfl != tokenAfter.inflWords.end(); ++itInfl)
              {
                if (itInfl->word.partOfSpeech == PartOfSpeech::ADJECTIVE &&
                    pInflsCheker.areCompatibles(*itTok->inflWords.begin(), *itInfl))
                {
                  putOnTop(tokenAfter.inflWords, itInfl);
                  break;
                }
              }
            }
          }
        }
        else if (partOfSpeechAfter != PartOfSpeech::PARTITIVE &&
                 partOfSpeechAfter != PartOfSpeech::PREPOSITION)
        {
          tryToMoveProperNounHasHigherPriority = true;
        }
      }
      else
      {
        tryToMoveProperNounHasHigherPriority = true;
      }

      if (tryToMoveProperNounHasHigherPriority &&
          currIGram.word.partOfSpeech != PartOfSpeech::PROPER_NOUN)
      {
        auto itInflWord = itFirstInflWord;
        ++itInflWord;
        while (itInflWord != inflWords.end())
        {
          if (itInflWord->word.partOfSpeech == PartOfSpeech::PROPER_NOUN)
          {
            putOnTop(inflWords, itInflWord);
            break;
          }
          ++itInflWord;
        }
      }
    }
    // If the current word is an adjective, sometimes we will prefer to modify the main part of speech of the current word
    if (currIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE && pIsRootLevel)
    {
      // If the next word is a noun for exemple
      if (tokenAfterPtr != nullptr &&
          partOfSpeech_isNominal(tokenAfterPtr->getPartOfSpeech()))
      {
        updateTokenAfterPtrs(itTok);
        continue;
      }

      // Or if the current word can be a noun
      {
        auto itInflWord = itFirstInflWord;
        ++itInflWord;
        while (itInflWord != inflWords.end())
        {
          if (partOfSpeech_isNominal(itInflWord->word.partOfSpeech))
          {
            putOnTop(inflWords, itInflWord);
            break;
          }
          ++itInflWord;
        }
      }
    }
    updateTokenAfterPtrs(itTok);
  }
}


void nounNounPrioritiesEn(std::vector<Token>& pTokens,
                          const InflectionsChecker& pInflsCheker)
{
  Token* prevNounTokenPtr = nullptr;
  for (TokIt itTok = getPrevToken(pTokens.end(), pTokens.begin(), pTokens.end()); itTok != pTokens.end();
       itTok = getPrevToken(itTok, pTokens.begin(), pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currIGram = inflWords.front();
    if (partOfSpeech_isNominal(currIGram.word.partOfSpeech))
    {
      if (prevNounTokenPtr != nullptr)
      {
        Token& nounAfterToken = *prevNounTokenPtr;
        auto itDet = getInflWordWithASpecificPartOfSpeech(inflWords, PartOfSpeech::DETERMINER);
        if (itDet != inflWords.end())
        {
          putOnTop(inflWords, itDet);
          for (auto itInfl = nounAfterToken.inflWords.begin(); itInfl != nounAfterToken.inflWords.end(); ++itInfl)
          {
            if (pInflsCheker.areCompatibles(*itDet, *itInfl))
            {
              putOnTop(nounAfterToken.inflWords, itInfl);
              break;
            }
          }
        }
      }
      else
      {
        prevNounTokenPtr = &*itTok;
      }
    }
    else
    {
      prevNounTokenPtr = nullptr;
    }
  }
}


void pronounPriorities(std::vector<Token>& pTokens,
                       const InflectionsChecker& pInflsCheker)
{
  static const std::set<PartOfSpeech> verbAuxpPartOfSpeechsToFind{PartOfSpeech::AUX, PartOfSpeech::VERB};
  static const std::set<PartOfSpeech> nounPartOfSpeechsToFind{PartOfSpeech::NOUN};
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currInflWord = inflWords.front();
    if (currInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT)
    {
      auto itPronCompl = getInflWordWithASpecificPartOfSpeech(inflWords, PartOfSpeech::PRONOUN_COMPLEMENT);
      if (itPronCompl != inflWords.end())
      {
        auto itPrev = getPrevToken(itTok, pTokens.begin(), pTokens.end());
        if (itPrev == pTokens.end())
          continue;
        if (partOfSpeech_isNominal(itPrev->getPartOfSpeech()))
        {
          putOnTop(inflWords, itPronCompl);
        }
        else
        {
          while (itPrev != pTokens.end())
          {
            PartOfSpeech prevPartOfSpeech = itPrev->getPartOfSpeech();
            if (prevPartOfSpeech == PartOfSpeech::PUNCTUATION)
              break;
            if (prevPartOfSpeech != PartOfSpeech::VERB &&
                prevPartOfSpeech != PartOfSpeech::ADVERB)
            {
              itTok = getNextToken(itTok, pTokens.end());
              if (itTok == pTokens.end())
                return;
              const InflectedWord& nextInflWord = itTok->inflWords.front();
              if (pInflsCheker.verbIsAtInfinitive(nextInflWord))
                putOnTop(inflWords, itPronCompl);
              break;
            }
            if (prevPartOfSpeech == PartOfSpeech::VERB)
            {
              auto itPrevPrev = getPrevToken(itPrev, pTokens.begin(), pTokens.end());
              if (itPrevPrev == pTokens.end() ||
                  !pInflsCheker.areCompatibles(itPrevPrev->inflWords.front(),
                                               itPrev->inflWords.front()))
                break;
              itPrev = itPrevPrev;
            }
            else
            {
              itPrev = getPrevToken(itPrev, pTokens.begin(), pTokens.end());
            }
          }
        }
      }
    }
    else if (currInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT &&
             inflWords.size() > 1)
    {
      if (!hasBefore(pTokens, itTok, verbAuxpPartOfSpeechsToFind, pInflsCheker, PartOfSpeech::ADVERB) &&
          !hasAfter(pTokens, itTok, verbAuxpPartOfSpeechsToFind))
        putOnBottom(inflWords, inflWords.begin());
      else if (hasAfter(pTokens, itTok, nounPartOfSpeechsToFind, PartOfSpeech::ADVERB))
        putOnBottom(inflWords, inflWords.begin());
    }
    else if (currInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN &&
             inflWords.size() > 1)
    {
      auto itPrev = getPrevToken(itTok, pTokens.begin(), pTokens.end());
      if (itPrev == pTokens.end())
        continue;
      if (itPrev->getPartOfSpeech() == PartOfSpeech::ADJECTIVE)
      {
        putOnBottom(inflWords, inflWords.begin());
      }
    }
  }
}


void adjPriorities(std::vector<Token>& pTokens)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currInflWord = inflWords.front();
    if (currInflWord.word.partOfSpeech == PartOfSpeech::ADJECTIVE && inflWords.size() > 1)
    {
      auto itNext = getNextToken(itTok, pTokens.end());
      if (itNext != pTokens.end() &&
          itNext->getPartOfSpeech() == PartOfSpeech::PROPER_NOUN)
        putOnTop(inflWords, PartOfSpeech::PROPER_NOUN);
    }
  }
}


void advPriorities(std::vector<Token>& pTokens)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end(); )
  {
    auto itNext = getNextToken(itTok, pTokens.end());
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    const InflectedWord& currInflWord = inflWords.front();
    if (currInflWord.word.partOfSpeech == PartOfSpeech::ADVERB &&
        ConceptSet::haveAConcept(currInflWord.infos.concepts, "comparison_more"))
    {
      if (itNext != pTokens.end() &&
          itNext->getPartOfSpeech() == PartOfSpeech::ADVERB)
        putOnTop(itNext->inflWords, PartOfSpeech::ADJECTIVE);
    }
    itTok = itNext;
  }
}

void partitivePrioritiesFr(std::vector<Token>& pTokens,
                           const InflectionsChecker& pInflsCheker)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    auto itFirstInflWord = inflWords.begin();
    const InflectedWord& currInflWord = *itFirstInflWord;
    if (currInflWord.word.partOfSpeech == PartOfSpeech::PARTITIVE)
    {
      auto itNext = getNextToken(itTok, pTokens.end());
      if (itNext != pTokens.end())
      {
        auto itNextFirstInfl = itNext->inflWords.begin();
        if (itNextFirstInfl->word.partOfSpeech == PartOfSpeech::VERB &&
            pInflsCheker.verbIsConjugated(*itNextFirstInfl) &&
            itNext->inflWords.size() > 1)
        {
          putOnBottom(itNext->inflWords, itNextFirstInfl);
        }
      }

      auto itPrev = getPrevToken(itTok, pTokens.begin(), pTokens.end());
      if (itPrev != pTokens.end() &&
          itPrev->inflWords.size() > 1)
      {
        auto itPrevFirstInfl = itPrev->inflWords.begin();
        if (itPrevFirstInfl->word.partOfSpeech == PartOfSpeech::VERB)
        {
          if (!pInflsCheker.nounCanBePlural(currInflWord) &&
              itNext != pTokens.end())
          {
            PartOfSpeech nextPartOfSpeech = itNext->inflWords.front().word.partOfSpeech;
            if (nextPartOfSpeech != PartOfSpeech::DETERMINER &&
                nextPartOfSpeech != PartOfSpeech::PRONOUN_COMPLEMENT &&
                !ConceptSet::haveAConcept(currInflWord.infos.concepts, "reference_definite"))
            {
              delAPartOfSpeechfPossible(itPrev->inflWords, itPrevFirstInfl);
            }
          }
        }
      }
    }
  }
}


void putApproximatelyConceptInTopPrioritiesIfNecessary(std::vector<Token>& pTokens)
{
  for (TokIt itTok = pTokens.begin(); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    std::list<InflectedWord>& inflWords = itTok->inflWords;
    if (inflWords.size() > 1)
    {
      for (auto itInflWord = ++inflWords.begin(); itInflWord != inflWords.end(); ++itInflWord)
      {
        const InflectedWord& currInflWord = *itInflWord;
        if (currInflWord.word.partOfSpeech == PartOfSpeech::ADVERB &&
            currInflWord.infos.concepts.count("approximately") > 0)
        {
          TokIt nextNextTok = getNextToken(itTok, pTokens.end());
          if (nextNextTok != pTokens.end() &&
              ConceptSet::haveAConceptThatBeginWith(nextNextTok->inflWords.front().infos.concepts, "number_"))
          {
            putOnTop(inflWords, itInflWord);
            break;
          }
        }
      }
    }
  }

}



} // End of namespace linguistics
} // End of namespace onsem

