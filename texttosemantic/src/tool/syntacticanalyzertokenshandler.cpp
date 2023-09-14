#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <functional>
#include <onsem/common/linguisticsubordinateid.hpp>
#include <onsem/common/utility/number.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>

namespace onsem
{
namespace linguistics
{
namespace
{
bool _delAllExcept(
    std::list<InflectedWord>& pGrams,
    const std::function<bool(std::list<InflectedWord>::iterator&)>& pExceptCondition)
{
  std::size_t pGramsSize = pGrams.size();
  bool hasErasedAnItem = false;
  for (auto itGram = pGrams.begin(); itGram != pGrams.end(); )
  {
    if (pGramsSize > 1 && pExceptCondition(itGram))
    {
      itGram = pGrams.erase(itGram);
      hasErasedAnItem = true;
      --pGramsSize;
      continue;
    }
    ++itGram;
  }
  return hasErasedAnItem;
}
}

// TODO: take the linflWords instead of a token
bool tokenIsMoreProbablyAType
(const linguistics::Token& pToken,
 PartOfSpeech pPartOfSpeech)
{
  return pToken.inflWords.begin()->word.partOfSpeech == pPartOfSpeech;
}


bool tokenIsMoreProbablyAType(
    const linguistics::Token& pToken,
    const std::vector<PartOfSpeech>& pPartOfSpeechs)
{
  const auto& word = pToken.inflWords.front().word;
  return hasAPartOfSpeech(pPartOfSpeechs, word.partOfSpeech);
}

bool hasAPartOfSpeech(
    const std::vector<PartOfSpeech>& pPartOfSpeechs,
    PartOfSpeech pPartOfSpeech)
{
  for (const auto& currPartOfSpeech : pPartOfSpeechs)
    if (currPartOfSpeech == pPartOfSpeech)
      return true;
  return false;
}

bool hasAPartOfSpeech(
    const std::list<InflectedWord>& pInflWords,
    PartOfSpeech pPartOfSpeech)
{
  for (const auto& currInflWord : pInflWords)
    if (currInflWord.word.partOfSpeech == pPartOfSpeech)
      return true;
  return false;
}


std::list<InflectedWord>::iterator getInflWordWithASpecificPartOfSpeech
(std::list<InflectedWord>& pIGrams,
 PartOfSpeech pPartOfSpeech)
{
  for (auto it = pIGrams.begin(); it != pIGrams.end(); ++it)
    if (it->word.partOfSpeech == pPartOfSpeech)
      return it;
  return pIGrams.end();
}


std::list<InflectedWord>::iterator getInflWordWithAnyPartOfSeechOf
(linguistics::Token& pToken,
 const std::vector<PartOfSpeech>& pPartOfSpeechs)
{
  for (auto it = pToken.inflWords.begin(); it != pToken.inflWords.end(); ++it)
    for (std::size_t i = 0; i < pPartOfSpeechs.size(); ++i)
      if (it->word.partOfSpeech == pPartOfSpeechs[i])
        return it;
  return pToken.inflWords.end();
}


void delTopPartOfSpeech(std::list<InflectedWord>& pIGrams)
{
  pIGrams.erase(pIGrams.begin());
  if (pIGrams.empty())
    pIGrams.emplace_back();
}


void delAPartOfSpeech
(std::list<InflectedWord>& pIGrams,
 std::list<InflectedWord>::iterator& pItToDel)
{
  pItToDel = pIGrams.erase(pItToDel);
  if (pIGrams.empty())
  {
    pIGrams.emplace_back();
    pItToDel = pIGrams.end();
  }
}


bool delAPartOfSpeechfPossible
(std::list<InflectedWord>& pIGrams,
 std::list<InflectedWord>::iterator& pItToDel)
{
  if (pIGrams.size() <= 1)
  {
    return false;
  }
  pItToDel = pIGrams.erase(pItToDel);
  return true;
}



bool delAPartOfSpeech
(std::list<InflectedWord>& pIGrams,
 PartOfSpeech pPartOfSpeechToDel)
{
  if (pIGrams.size() <= 1)
  {
    return false;
  }
  for (std::list<InflectedWord>::iterator it = pIGrams.begin();
       it != pIGrams.end(); ++it)
  {
    if (it->word.partOfSpeech == pPartOfSpeechToDel)
    {
      pIGrams.erase(it);
      return true;
    }
  }
  return false;
}


bool delPartOfSpeechs
(std::list<InflectedWord>& pIGrams,
 const std::vector<PartOfSpeech>& pPartOfSpeechToDel)
{
  bool res = false;
  for (auto it = pIGrams.begin(); it != pIGrams.end(); )
  {
    if (pIGrams.size() <= 1)
      break;
    bool possibleGram = true;
    for (std::size_t i = 0; i < pPartOfSpeechToDel.size(); ++i)
    {
      if (pPartOfSpeechToDel[i] == it->word.partOfSpeech)
      {
        possibleGram = false;
        break;
      }
    }
    if (possibleGram)
    {
      ++it;
    }
    else
    {
      it = pIGrams.erase(it);
      return true;
    }
  }
  return res;
}


TokIt getNextWord
(TokIt it, const TokIt& endIt)
{
  if (it == endIt)
  {
    return endIt;
  }
  for (TokIt res = ++it; res != endIt; ++res)
  {
    if (partOfSpeech_isAWord(res->inflWords.begin()->word.partOfSpeech))
    {
      return res;
    }
  }
  return endIt;
}



TokIt getNextToken
(TokIt it, const TokIt& endIt,
 PartOfSpeech pUnwantedGramType)
{
  if (it == endIt)
  {
    return endIt;
  }
  for (TokIt res = ++it; res != endIt; ++res)
  {
    PartOfSpeech currPartOfSpeech = res->inflWords.begin()->word.partOfSpeech;
    if (currPartOfSpeech >= PartOfSpeech::LINKBETWEENWORDS &&
        currPartOfSpeech != pUnwantedGramType)
    {
      return res;
    }
  }
  return endIt;
}


TokIt getTheNextestToken
(TokIt it, const TokIt& endIt,
 SkipPartOfWord pSkipPartOfWord)
{
  if (it == endIt)
  {
    return endIt;
  }
  for (TokIt res = it; res != endIt; ++res)
  {
    if (res->inflWords.begin()->word.partOfSpeech >= PartOfSpeech::LINKBETWEENWORDS &&
        (pSkipPartOfWord == SkipPartOfWord::NO || res->getTokenLinkage() != TokenLinkage::PART_OF_WORD_GROUP))
    {
      return res;
    }
  }
  return endIt;
}


TokIt getTheNextestWord(TokIt it, const TokIt& endIt)
{
  if (it == endIt)
    return endIt;
  for (TokIt res = it; res != endIt; ++res)
    if (partOfSpeech_isAWord(res->inflWords.begin()->word.partOfSpeech))
      return res;
  return endIt;
}


bool delAllBefore
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt)
{
  bool hasRemovedAGram = false;
  for (std::list<InflectedWord>::iterator itGram = pGrams.begin();
       itGram != pGrams.end(); )
  {
    if (itGram == pIt)
    {
      return hasRemovedAGram;
    }
    hasRemovedAGram = true;
    itGram = pGrams.erase(itGram);
  }
  pGrams.emplace_back();
  return hasRemovedAGram;
}



bool delAllAfter
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt)
{
  bool hasRemovedAGram = false;
  bool foundTheRef = false;
  for (std::list<InflectedWord>::iterator itGram = pGrams.begin();
       itGram != pGrams.end(); )
  {
    if (itGram == pIt)
    {
      foundTheRef = true;
      ++itGram;
    }
    else if (foundTheRef)
    {
      itGram = pGrams.erase(itGram);
      hasRemovedAGram = true;
    }
    else
    {
      ++itGram;
    }
  }
  if (!hasRemovedAGram)
  {
    return false;
  }
  if (pGrams.empty())
  {
    pGrams.emplace_back();
  }
  return true;
}



bool delAllExept
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt)
{
  return _delAllExcept(pGrams, [&](std::list<InflectedWord>::iterator& pItCurrInfl) {
    return pItCurrInfl != pIt;
  });
}


bool delAllExept
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt,
 PartOfSpeech pPartOfSpeech)
{
  return _delAllExcept(pGrams, [&](std::list<InflectedWord>::iterator& pItCurrInfl) {
    return pItCurrInfl != pIt && pItCurrInfl->word.partOfSpeech != pPartOfSpeech;
  });
}


bool delAllExept(
    std::list<InflectedWord>& pGrams,
    PartOfSpeech pPartOfSpeech)
{
  return _delAllExcept(pGrams, [&](std::list<InflectedWord>::iterator& pItCurrInfl) {
    return pItCurrInfl->word.partOfSpeech != pPartOfSpeech;
  });
}


void putOnTop
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt)
{
  if (pGrams.begin() != pIt)
    pGrams.splice(pGrams.begin(), pGrams, pIt);
}

bool putOnTop(
    std::list<InflectedWord>& pGrams,
    PartOfSpeech pPartOfSpeech)
{
  auto itBegin = pGrams.begin();
  for (auto it = itBegin; it != pGrams.end(); ++it)
  {
    if (it->word.partOfSpeech == pPartOfSpeech)
    {
      if (it != itBegin)
        pGrams.splice(itBegin, pGrams, it);
      return true;
    }
  }
  return false;
}

void putOnBottom
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt)
{
  if (pGrams.empty())
    return;
  auto itLast = --pGrams.end();
  if (itLast != pIt)
    pGrams.splice(pGrams.end(), pGrams, pIt);
}


bool canBeAFrenchReflexiveVerb(const WordAssociatedInfos& pWordAssInfos)
{
  if (!pWordAssInfos.metaMeanings.empty())
  {
    const auto& linkedMeanings = pWordAssInfos.metaMeanings.front().linkedMeanings;
    if (!linkedMeanings.empty())
      return linkedMeanings.front().first->lemma == "se";
  }
  return false;
}


bool isAtBeginOfASentence(
    std::vector<Token>& pTokens,
    TokIt pItTok)
{
  auto itPrev = getPrevToken(pItTok, pTokens.begin(), pTokens.end());
  if (itPrev != pTokens.end())
    return itPrev->getPartOfSpeech() == PartOfSpeech::PUNCTUATION;
  return true;
}


bool hasBefore(std::vector<Token>& pTokens,
               TokIt pItTok,
               const std::set<PartOfSpeech>& pPartOfSpeechsToFind,
               const InflectionsChecker& pInflsCheker,
               PartOfSpeech pPartOfSpeechToSkip)
{
  auto itPrev = getPrevToken(pItTok, pTokens.begin(), pTokens.end());
  while (itPrev != pTokens.end())
  {
    PartOfSpeech prevPartOfSpeech = itPrev->getPartOfSpeech();
    if (pPartOfSpeechsToFind.count(prevPartOfSpeech) > 0 &&
        pInflsCheker.areCompatibles(itPrev->inflWords.front(), pItTok->inflWords.front()))
      return true;
    if (prevPartOfSpeech != pPartOfSpeechToSkip)
      return false;
    itPrev = getPrevToken(itPrev, pTokens.begin(), pTokens.end());
  }
  return false;
}


bool hasAfter(std::vector<Token>& pTokens,
              TokIt pItTok,
              const std::set<PartOfSpeech>& pPartOfSpeechsToFind,
              const std::optional<PartOfSpeech>& pPartOfSpeechToSkip)
{
  auto itNext = getNextToken(pItTok, pTokens.end());
  while (itNext != pTokens.end())
  {
    PartOfSpeech nextPartOfSpeech = itNext->getPartOfSpeech();
    if (pPartOfSpeechsToFind.count(nextPartOfSpeech) > 0)
      return true;
    if (pPartOfSpeechToSkip && nextPartOfSpeech == *pPartOfSpeechToSkip)
      itNext = getNextToken(itNext, pTokens.end());
    else
      return false;
  }
  return false;
}




void fillRelativeCharEncodedFromInflWord(LinguisticSubordinateId& pLinguisticSubordinateId,
                                         const InflectedWord& pInflWord)
{
  switch (pLinguisticSubordinateId.chunkLinkType)
  {
  case ChunkLinkType::LOCATION:
  {
    const std::string locRelBeginOfCptStr = "location_relative_";
    std::map<std::string, char> relLocationConcepts;
    ConceptSet::extractConceptsThatBeginWith(relLocationConcepts,
                                             pInflWord.infos.concepts,
                                             locRelBeginOfCptStr);
    if (!relLocationConcepts.empty())
    {
      const std::string& conceptName = relLocationConcepts.begin()->first;
      if (conceptName.size() > locRelBeginOfCptStr.size())
        pLinguisticSubordinateId.relativeSubodinate = semanticRelativeLocationType_toChar(
              semanticRelativeLocationType_fromStr(
                conceptName.substr(locRelBeginOfCptStr.size(),
                                   conceptName.size() - locRelBeginOfCptStr.size())));
    }
    break;
  }
  case ChunkLinkType::TIME:
  {
    const std::string timeRelBeginOfCptStr = "time_relative_";
    std::map<std::string, char> relTimeConcepts;
    ConceptSet::extractConceptsThatBeginWith(relTimeConcepts,
                                             pInflWord.infos.concepts,
                                             timeRelBeginOfCptStr);

    for (const auto& currRelTimeCpt : relTimeConcepts)
    {
      SemanticRelativeTimeType relTimeType = SemanticRelativeTimeType::AFTER;
      const std::string& conceptName = currRelTimeCpt.first;
      if (semanticRelativeTimeType_fromStr_ifFound(relTimeType,
                                                   conceptName.substr(timeRelBeginOfCptStr.size(),
                                                                      conceptName.size() - timeRelBeginOfCptStr.size())))
        pLinguisticSubordinateId.relativeSubodinate = semanticRelativeTimeType_toChar(relTimeType);
    }
    break;
  }
  case ChunkLinkType::DURATION:
  {
    const std::string durationRelBeginOfCptStr = "duration_relative_";
    std::map<std::string, char> relDurationConcepts;
    ConceptSet::extractConceptsThatBeginWith(relDurationConcepts,
                                             pInflWord.infos.concepts,
                                             durationRelBeginOfCptStr);

    for (const auto& currRelDurationCpt : relDurationConcepts)
    {
      SemanticRelativeDurationType relDurationType = SemanticRelativeDurationType::UNTIL;
      const std::string& conceptName = currRelDurationCpt.first;
      if (semanticRelativeDurationType_fromStr_ifFound(relDurationType,
                                                       conceptName.substr(durationRelBeginOfCptStr.size(),
                                                                          conceptName.size() - durationRelBeginOfCptStr.size())))
        pLinguisticSubordinateId.relativeSubodinate = semanticRelativeDurationType_toChar(relDurationType);
    }
    break;
  }
  default:
    break;
  }
}

std::size_t getSeparatorOfHourMinute(const std::string& pStr)
{
  std::size_t res = std::string::npos;
  for (std::size_t i = 0; i < pStr.size(); ++i)
  {
    if (isDigit(pStr[i]))
      continue;

    if ((pStr[i] == 'h' || pStr[i] == 'H') && i > 0 && res == std::string::npos)
      res = i;
    else
      return std::string::npos;
  }
  return res;
}


template<typename TOKITTEMP>
TOKITTEMP eatNumber(mystd::optional<SemanticFloat>& pNumber,
                    TOKITTEMP pToken,
                    const TOKITTEMP& pItEndToken,
                    const std::string& pBeginOfNumberCpt)
{
  SemanticFloat numberToAddAtTheEnd;
  TOKITTEMP res = pToken;
  const std::size_t beginOfNumberCpt_size = pBeginOfNumberCpt.size();
  while (pToken != pItEndToken)
  {
    bool isANumber = false;
    const InflectedWord& inflWord = pToken->inflWords.front();
    for (const auto& currCpt : inflWord.infos.concepts)
    {
      const std::string& currCptName = currCpt.first;
      if (ConceptSet::doesConceptBeginWith(currCptName, pBeginOfNumberCpt) &&
          currCptName.size() > beginOfNumberCpt_size)
      {
        std::string numberStr = currCptName.substr(beginOfNumberCpt_size, currCptName.size() - beginOfNumberCpt_size);
        mystd::optional<SemanticFloat> newNumber;
        newNumber.emplace();
        if (newNumber->fromStr(numberStr))
        {
          // if the number is already written in number we have no number assembling to do
          if (inflWord.word.lemma.empty())
          {
            pNumber = newNumber;
          }
          else
          {
            if (pNumber)
            {
              if (*newNumber >= 100)
                pNumber.emplace(*pNumber * *newNumber);
              else
                pNumber.emplace(*pNumber + *newNumber);
            }
            else
            {
              pNumber = newNumber;
            }
            if (*newNumber >= 1000)
            {
              numberToAddAtTheEnd += *pNumber;
              pNumber.emplace(0);
            }
          }
          isANumber = true;
          break;
        }
      }
    }
    if (isANumber && pNumber)
    {
      res = pToken;
      pToken = getNextToken(pToken, pItEndToken);
    }
    else
    {
      break;
    }
  }
  if (pNumber && numberToAddAtTheEnd > 0)
    pNumber.emplace(*pNumber + numberToAddAtTheEnd);
  return res;
}

template TokIt eatNumber<TokIt>(mystd::optional<SemanticFloat>&, TokIt, const TokIt&, const std::string&);
template TokCstIt eatNumber<TokCstIt>(mystd::optional<SemanticFloat>&, TokCstIt, const TokCstIt&, const std::string&);

template<typename TOKITTEMP>
bool getNumberHoldByTheInflWord(SemanticFloat& number,
                                TOKITTEMP pToken,
                                const TOKITTEMP& pItEndToken,
                                const std::string& pNumberCpt)
{
  mystd::optional<SemanticFloat> numberOpt;
  eatNumber(numberOpt, pToken, pItEndToken, pNumberCpt);
  if (numberOpt)
  {
    number = *numberOpt;
    return true;
  }
  return false;
}

template bool getNumberHoldByTheInflWord<TokIt>(SemanticFloat&, TokIt, const TokIt&, const std::string&);
template bool getNumberHoldByTheInflWord<TokCstIt>(SemanticFloat&, TokCstIt, const TokCstIt&, const std::string&);


template<typename TOKITTEMP, typename TOKENRANGE>
mystd::optional<SemanticDate> extractDate(const TOKITTEMP& pTokenIt,
                                          const TOKENRANGE& pTokRange)
{
  mystd::optional<SemanticDate> resOpt;
  const InflectedWord& iGram = pTokenIt->inflWords.front();
  std::map<std::string, char> monthConcepts;
  ConceptSet::extractConceptsThatBeginWith(monthConcepts, iGram.infos.concepts, "time_month_");
  if (!monthConcepts.empty())
  {
    SemanticDate res;
    res.month.emplace(semanticTimeMonth_toId(semanticTimeMonth_fromStr(monthConcepts.begin()->first)));

    SemanticFloat dayNumber;
    SemanticFloat yearNumber;

    auto itTokenEndChunk = pTokRange.getItEnd();
    if (iGram.word.language == SemanticLanguageEnum::ENGLISH)
    {
      auto dayToken = getNextToken(pTokenIt, itTokenEndChunk);
      if (dayToken != pTokenIt &&
          (getNumberHoldByTheInflWord(dayNumber, dayToken, itTokenEndChunk, "number_") ||
           getNumberHoldByTheInflWord(dayNumber, dayToken, itTokenEndChunk, "rank_")) &&
          dayNumber.isPositive() && dayNumber.isAnInteger() &&
          dayNumber.value >= 1 && dayNumber.value <= 31)
      {
        res.day.emplace(dayNumber.value);

        auto yearToken = getNextToken(dayToken, itTokenEndChunk);
        if (yearToken != pTokRange.getItEnd() &&
            getNumberHoldByTheInflWord(yearNumber, yearToken, itTokenEndChunk, "number_") &&
            yearNumber.isPositive() && yearNumber.isAnInteger())
          res.year.emplace(yearNumber.value);
        resOpt.emplace(res);
        return resOpt;
      }
    }

    auto dayToken = getPrevToken(pTokenIt, pTokRange.getItBegin(), pTokenIt);
    if (dayToken != pTokenIt && getNumberHoldByTheInflWord(dayNumber, dayToken,  pTokenIt, "number_") &&
        dayNumber.isPositive() && dayNumber.isAnInteger() &&
        dayNumber.value >= 1 && dayNumber.value <= 31)
      res.day.emplace(dayNumber.value);

    auto yearToken = getNextToken(pTokenIt, itTokenEndChunk);
    if (yearToken != pTokRange.getItEnd() &&
        getNumberHoldByTheInflWord(yearNumber, yearToken, itTokenEndChunk, "number_") &&
        yearNumber.isPositive() && yearNumber.isAnInteger() &&
        hasNotMoreThanANumberOfDigits(yearNumber.value, 4))
      res.year.emplace(yearNumber.value);
    resOpt.emplace(res);
    return resOpt;
  }
  return resOpt;
}

template mystd::optional<SemanticDate> extractDate<TokIt, TokenRange>(const TokIt&, const TokenRange&);
template mystd::optional<SemanticDate> extractDate<TokCstIt, ConstTokenRange>(const TokCstIt&, const ConstTokenRange&);



bool isAnHour(const linguistics::ConstTokenIterator& pNextToken)
{
  if (pNextToken.atEnd())
    return false;

  auto itToken = pNextToken.getTokenIt();
  if (itToken->inflWords.front().word.language == SemanticLanguageEnum::ENGLISH)
  {
    auto endIt = pNextToken.getItEnd();

    if (itToken != endIt)
      itToken = getNextToken(itToken, endIt);

    if (itToken != endIt)
    {
      const auto& tokenInfos = itToken->inflWords.front().infos;
      if (ConceptSet::haveAConcept(tokenInfos.concepts, "duration_minute") ||
          ConceptSet::haveAConceptThatBeginWith(tokenInfos.concepts, "time_hour_"))
        return true;
    }

    if (itToken != endIt)
    {
      const auto& tokenInfos = itToken->inflWords.front().infos;
      if (tokenInfos.hasContextualInfo(WordContextualInfos::EN_TIMEWORD))
        return true;
    }
  }
  return false;
}


bool canBeParentOfANominalGroup(const InflectedWord& pInflWord)
{
  return !partOfSpeech_isPronominal(pInflWord.word.partOfSpeech) ||
      (pInflWord.word.language == SemanticLanguageEnum::FRENCH &&
       !pInflWord.infos.hasContextualInfo(WordContextualInfos::REFTOAPERSON));
}

} // End of namespace linguistics
} // End of namespace onsem
