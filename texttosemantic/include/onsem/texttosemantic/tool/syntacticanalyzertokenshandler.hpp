#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SYNTACTICANALYZERTOKENSHANDLER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SYNTACTICANALYZERTOKENSHANDLER_HPP

#include <list>
#include <vector>
#include <memory>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include "../api.hpp"

namespace onsem
{
struct SemanticDate;
struct SemanticWord;
struct SemanticFloat;
namespace linguistics
{
struct LinguisticSubordinateId;
struct InflectedWord;
class InflectionsChecker;


ONSEM_TEXTTOSEMANTIC_API bool tokenIsMoreProbablyAType
(const Token& pToken,
 PartOfSpeech pPartOfSpeech);

ONSEM_TEXTTOSEMANTIC_API bool tokenIsMoreProbablyAType(
    const linguistics::Token& pToken,
    const std::vector<PartOfSpeech>& pPartOfSpeechs);

ONSEM_TEXTTOSEMANTIC_API bool hasAPartOfSpeech(
    const std::vector<PartOfSpeech>& pPartOfSpeechs,
    PartOfSpeech pPartOfSpeech);

ONSEM_TEXTTOSEMANTIC_API bool hasAPartOfSpeech(
    const std::list<InflectedWord>& pInflWords,
    PartOfSpeech pPartOfSpeech);

ONSEM_TEXTTOSEMANTIC_API std::list<InflectedWord>::iterator getInflWordWithASpecificPartOfSpeech
(std::list<InflectedWord>& pIGrams,
 PartOfSpeech pPartOfSpeech);

ONSEM_TEXTTOSEMANTIC_API std::list<InflectedWord>::iterator getInflWordWithAnyPartOfSeechOf
(Token& pToken,
 const std::vector<PartOfSpeech>& pPartOfSpeechs);

ONSEM_TEXTTOSEMANTIC_API void delTopPartOfSpeech(std::list<InflectedWord>& pIGrams);

ONSEM_TEXTTOSEMANTIC_API void delAPartOfSpeech
(std::list<InflectedWord>& pIGrams,
 std::list<InflectedWord>::iterator& pItToDel);

ONSEM_TEXTTOSEMANTIC_API bool delAPartOfSpeechfPossible
(std::list<InflectedWord>& pIGrams,
 std::list<InflectedWord>::iterator& pItToDel);

ONSEM_TEXTTOSEMANTIC_API bool delAPartOfSpeech
(std::list<InflectedWord>& pIGrams,
 PartOfSpeech pPartOfSpeechToDel);

ONSEM_TEXTTOSEMANTIC_API bool delPartOfSpeechs
(std::list<InflectedWord>& pIGrams,
 const std::vector<PartOfSpeech>& pPartOfSpeechToDel);

ONSEM_TEXTTOSEMANTIC_API TokIt getNextWord
(TokIt it, const TokIt& endIt);

ONSEM_TEXTTOSEMANTIC_API TokIt getNextToken
(TokIt it, const TokIt& endIt,
 PartOfSpeech pUnwantedGramType);

ONSEM_TEXTTOSEMANTIC_API TokIt getTheNextestToken
(TokIt it, const TokIt& endIt,
 SkipPartOfWord pSkipPartOfWord = SkipPartOfWord::NO);

ONSEM_TEXTTOSEMANTIC_API TokIt getTheNextestWord(TokIt it, const TokIt& endIt);

ONSEM_TEXTTOSEMANTIC_API bool delAllBefore
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt);

ONSEM_TEXTTOSEMANTIC_API bool delAllAfter
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt);

ONSEM_TEXTTOSEMANTIC_API bool delAllExept
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt);

ONSEM_TEXTTOSEMANTIC_API bool delAllExept
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt,
 PartOfSpeech pPartOfSpeech);

ONSEM_TEXTTOSEMANTIC_API bool delAllExept(
    std::list<InflectedWord>& pGrams,
    PartOfSpeech pPartOfSpeech);

ONSEM_TEXTTOSEMANTIC_API void putOnTop
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt);

ONSEM_TEXTTOSEMANTIC_API bool putOnTop(
    std::list<InflectedWord>& pGrams,
    PartOfSpeech pPartOfSpeech);

ONSEM_TEXTTOSEMANTIC_API void putOnBottom
(std::list<InflectedWord>& pGrams,
 std::list<InflectedWord>::iterator pIt);

ONSEM_TEXTTOSEMANTIC_API bool canBeAFrenchReflexiveVerb(const WordAssociatedInfos& pWordAssInfos);

ONSEM_TEXTTOSEMANTIC_API
bool isAtBeginOfASentence(
    std::vector<Token>& pTokens,
    TokIt pItTok);

ONSEM_TEXTTOSEMANTIC_API
bool hasBefore(std::vector<Token>& pTokens,
               TokIt pItTok,
               const std::set<PartOfSpeech>& pPartOfSpeechsToFind,
               const InflectionsChecker& pInflsCheker,
               PartOfSpeech pPartOfSpeechToSkip);

ONSEM_TEXTTOSEMANTIC_API
bool hasAfter(std::vector<Token>& pTokens,
              TokIt pItTok,
              const std::set<PartOfSpeech>& pPartOfSpeechsToFind,
              PartOfSpeech pPartOfSpeechToSkip);

ONSEM_TEXTTOSEMANTIC_API
void fillRelativeCharEncodedFromInflWord(LinguisticSubordinateId& pLinguisticSubordinateId,
                                         const InflectedWord& pInflWord);

template<typename TOKITTEMP>
ONSEM_TEXTTOSEMANTIC_API
TOKITTEMP eatNumber(mystd::optional<SemanticFloat>& pNumber,
                    TOKITTEMP pToken,
                    const TOKITTEMP& pItEndToken,
                    const std::string& pBeginOfNumberCpt);

template<typename TOKITTEMP>
ONSEM_TEXTTOSEMANTIC_API
bool getNumberHoldByTheInflWord(SemanticFloat& number,
                                TOKITTEMP pToken,
                                const TOKITTEMP& pItEndToken,
                                const std::string& pNumberCpt);

template<typename TOKITTEMP, typename TOKENRANGE>
ONSEM_TEXTTOSEMANTIC_API
mystd::optional<SemanticDate> extractDate(const TOKITTEMP& pTokenIt,
                                          const TOKENRANGE& pTokRange);

ONSEM_TEXTTOSEMANTIC_API
bool isAnHour(const linguistics::ConstTokenIterator& pNextToken);

ONSEM_TEXTTOSEMANTIC_API
bool canBeParentOfANominalGroup(const InflectedWord& pInflWord);


template<typename T, typename T2>
T getNextToken(T it, const T2& endIt, SkipPartOfWord pSkipPartOfWord = SkipPartOfWord::NO)
{
  if (it == endIt)
    return endIt;
  for (T res = ++it; res != endIt; ++res)
    if (res->inflWords.begin()->word.partOfSpeech >= PartOfSpeech::LINKBETWEENWORDS &&
        (pSkipPartOfWord == SkipPartOfWord::NO || res->getTokenLinkage() != TokenLinkage::PART_OF_WORD_GROUP))
      return res;
  return endIt;
}


template<typename T>
T getPrevToken(T pIt, const T& pBeginIt, const T& pEndIt, SkipPartOfWord pSkipPartOfWord = SkipPartOfWord::NO)
{
  if (pIt == pBeginIt)
    return pEndIt;
  auto res = --pIt;
  while (true)
  {
    const Token& currToken = *res;
    if (currToken.inflWords.front().word.partOfSpeech >= PartOfSpeech::LINKBETWEENWORDS &&
        (pSkipPartOfWord == SkipPartOfWord::NO || currToken.getTokenLinkage() != TokenLinkage::PART_OF_WORD_GROUP))
      return res;
    if (res == pBeginIt)
      return pEndIt;
    --res;
  }
  return pEndIt;
}

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SYNTACTICANALYZERTOKENSHANDLER_HPP
