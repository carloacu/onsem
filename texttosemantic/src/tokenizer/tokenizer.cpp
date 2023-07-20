#include "tokenizer.hpp"
#include <iostream>
#include <algorithm>
#include <onsem/common/utility/getendofparenthesis.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>
#include <onsem/texttosemantic/dbtype/resourcegroundingextractor.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include "../tool/isaurl.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{

struct StrReplacement
{
  StrReplacement(std::string pOldStr,
                 std::string pNewStr)
    : oldStr(pOldStr),
      newStr(pNewStr)
  {
  }

  std::string oldStr;
  std::string newStr;
};


struct TextForms
{
  TextForms(const std::string& pInputText)
    : inputText(pInputText),
      formattedText(),
      reformattedTextMappingToInputText()
  {
    // spot the offsets
    std::map<std::size_t, StrReplacement> offsetsToDo;

    static const std::string complexApostrophe = "’";
    std::size_t pos = inputText.find(complexApostrophe);
    while (pos != std::string::npos)
    {
      offsetsToDo.emplace(pos, StrReplacement(complexApostrophe, "'"));
      pos = inputText.find(complexApostrophe, ++pos);
    }

    std::size_t posBegin = 0;
    for (const auto& currOffset : offsetsToDo)
    {
      std::size_t newBegin = currOffset.first;
      if (posBegin < newBegin)
        _copy(posBegin, newBegin);
      posBegin = newBegin;
      _replace(posBegin, currOffset.second.newStr);
      posBegin += currOffset.second.oldStr.size();
    }

    if (posBegin < inputText.size())
      _copy(posBegin, inputText.size());
  }

  std::string getSubInputText(std::size_t pBegin,
                              std::size_t pLength) const
  {
    assert(pBegin <= reformattedTextMappingToInputText.size());
    auto endPos = pBegin + pLength;
    assert(endPos <= reformattedTextMappingToInputText.size());
    assert(pBegin <= endPos);
    std::size_t inTextBegin = reformattedTextMappingToInputText[pBegin];

    if (endPos < reformattedTextMappingToInputText.size())
    {
      std::size_t inTextEnd = reformattedTextMappingToInputText[endPos];
      assert(inTextBegin <= inputText.size());
      assert(inTextEnd <= inputText.size());
      assert(inTextBegin <= inTextEnd);
      return inputText.substr(inTextBegin, inTextEnd - inTextBegin);
    }
    else
    {
      return inputText.substr(inTextBegin, inputText.size() - inTextBegin);
    }
  }

  bool isInputTextAtUpperCase(std::size_t pPos) const
  {
    assert(pPos < reformattedTextMappingToInputText.size());
    std::size_t posInInputText = reformattedTextMappingToInputText[pPos];
    assert(posInInputText < inputText.size());
    return beginWithUppercase(inputText, posInInputText);
  }

  /// Input text.
  const std::string inputText;
  std::string formattedText;
  std::vector<std::size_t> reformattedTextMappingToInputText;

private:
  void _copy(std::size_t pBegin,
             std::size_t pEnd)
  {
    formattedText.append(inputText, pBegin, pEnd - pBegin);
    for (std::size_t i = pBegin; i < pEnd; ++i)
      reformattedTextMappingToInputText.emplace_back(i);
  }


  void _replace(std::size_t pBegin,
                const std::string& pNewSubStr)
  {
    formattedText += pNewSubStr;
    for (std::size_t i = 0; i < pNewSubStr.size(); ++i)
      reformattedTextMappingToInputText.emplace_back(pBegin);
  }
};


/// Struct to save the current state of our tokenization.
struct TokSent
{
  /**
   * @brief Constructor.
   * @param pTokens Vector of tokens.
   * @param pInputSentence Input sentence.
   * @param pCurrPos Current position in the sentence.
   * @param pPosAfterPreviousToken Position just after the previous token inserted.
   * @param pEndPosToTokenize End position in the sentence that we have to tokenize.
   */
  TokSent
  (std::list<Token>& pTokens,
   const std::string& pInputSentence)
    : tokens(pTokens),
      textForms(pInputSentence),
      canBeAWordAtUppercase(true),
      currPos(0),
      posAfterPreviousToken(0),
      endPosToTokenize(textForms.formattedText.size())
  {}
  /// vector of tokens.
  std::list<Token>& tokens;
  TextForms textForms;
  // If next token can have the first letter at uppercase.
  bool canBeAWordAtUppercase;
  /// Current position in the sentence.
  std::size_t currPos;
  /// Position just after the previous token inserted.
  std::size_t posAfterPreviousToken;
  /// End position in the sentence that we have to tokenize.
  std::size_t endPosToTokenize;
};

void _databaseTokenizer(TokSent& pTokSent,
                        const LinguisticDictionary& pLingDico,
                        const LinguisticDictionary& pCommonLingDico);

void _refreshReferencePositionsafterAnInsertionWithLength(TokSent& pTokSent, std::size_t pLength)
{
  pTokSent.posAfterPreviousToken += pLength;
  pTokSent.currPos = pTokSent.posAfterPreviousToken;
}

void _refreshReferencePositionsafterAnInsertionWithAbsolutePosition(TokSent& pTokSent, std::size_t pPosition)
{
  pTokSent.posAfterPreviousToken = pPosition;
  pTokSent.currPos = pTokSent.posAfterPreviousToken;
}


std::unique_ptr<NominalInflections> _getAllNounInflections(SemanticLanguageEnum pLanguage)
{
  if (pLanguage == SemanticLanguageEnum::ENGLISH)
    return NominalInflections::get_inflections_ns_np();
  return NominalInflections::get_inflections_ms_mp_fs_fp();
}


template <typename TOKEN_LIST>
void _listOfTokensToVectorOfTokens(std::vector<Token>& pVectorOfkens,
                                   TOKEN_LIST& pListOfTokens,
                                   const TokenPos* pParentTokenPos = nullptr)
{
  pVectorOfkens.clear();
  pVectorOfkens.reserve(pListOfTokens.size());
  std::size_t id = 0;
  for (const Token& currTok : pListOfTokens)
  {
    pVectorOfkens.emplace_back(currTok.str);
    Token& newTok = pVectorOfkens.back();
    newTok.inflWords = std::move(currTok.inflWords);
    if (pParentTokenPos == nullptr)
      newTok.tokenPos.tokenIndexes.emplace_back(id);
    else
      newTok.tokenPos.set(*pParentTokenPos, id);
    assert(currTok.linkedTokens.empty());
    if (currTok.subTokens)
    {
      newTok.subTokens = std::make_unique<TokensTree>();
      _listOfTokensToVectorOfTokens(newTok.subTokens->tokens,
                                    currTok.subTokens->tokens,
                                    &newTok.tokenPos);
    }
    // is empty
    newTok.thisTokenAlreadyCausedARestart = currTok.thisTokenAlreadyCausedARestart;
    ++id;
  }
}


bool _mergeIfItsAComposedWord(std::list<Token>& pTokens,
                              const std::string& pTokStr,
                              std::list<InflectedWord>& pInfosGram,
                              const std::function<bool(PartOfSpeech, PartOfSpeech)>& pAreTokensCompatible)
{
  std::list<Token>::reverse_iterator it = ++pTokens.rbegin();
  // if the token before the "-" is a word
  if (pAreTokensCompatible(it->inflWords.front().word.partOfSpeech, pInfosGram.front().word.partOfSpeech))
  {
    for (std::list<InflectedWord>::iterator itIGram = pInfosGram.begin();
         itIGram != pInfosGram.end(); ++itIGram)
    {
      switch (itIGram->word.partOfSpeech)
      {
      case PartOfSpeech::PROPER_NOUN:
      case PartOfSpeech::NOUN:
      case PartOfSpeech::UNKNOWN:
      case PartOfSpeech::ADJECTIVE:
      {
        if (ConceptSet::haveAConceptThatBeginWith(itIGram->infos.concepts, "number_"))
          return false;

        // add the "-" token and the adjective
        // to token before the "-"
        it->str += pTokens.back().str + pTokStr;
        // put only adjective as grammatical possibility
        delAllExept(pInfosGram, itIGram);

        if (pInfosGram.front().word.partOfSpeech != PartOfSpeech::PROPER_NOUN &&
            it->inflWords.front().word.partOfSpeech != PartOfSpeech::PROPER_NOUN)
          it->inflWords = pInfosGram;
        else
        {
          auto itInflWord = getInflWordWithASpecificPartOfSpeech(it->inflWords, itIGram->word.partOfSpeech);
          if (itInflWord != it->inflWords.end())
            delAllExept(it->inflWords, itInflWord);
        }
        for (auto& currInflWord : it->inflWords)
          currInflWord.word.lemma = it->str;

        // remove the "-" token
        pTokens.pop_back();
        return true;
      }
      case PartOfSpeech::PRONOUN_SUBJECT:
      case PartOfSpeech::PRONOUN:
        return false;
      default:
        break;
      }
    }
  }
  return false;
}


void _insertTokenWithStr(TokSent& pTokSent, std::size_t pLength,
                         std::list<InflectedWord>& pInfosGram,
                         const std::string& pTokStr,
                         SemanticLanguageEnum pLanguage)
{
  // if the token can be a proper noun,
  // we add this grammatical possibilities
  bool originalWordBeginWithUpperCase = pTokSent.textForms.isInputTextAtUpperCase
      (pTokSent.posAfterPreviousToken);
  if (originalWordBeginWithUpperCase &&
      pInfosGram.size() == 1 &&
      pInfosGram.front().word.partOfSpeech == PartOfSpeech::UNKNOWN)
  {
    pInfosGram.clear();
    pInfosGram.emplace_back();
    InflectedWord& properNounIgram = pInfosGram.back();
    properNounIgram.word.partOfSpeech = PartOfSpeech::PROPER_NOUN;
    properNounIgram.word.lemma = pTokStr;
  }
  else
  {
    std::list<InflectedWord>::iterator insertProperNoun = pInfosGram.begin();
    for (; insertProperNoun != pInfosGram.end(); ++insertProperNoun)
      if (insertProperNoun->word.partOfSpeech == PartOfSpeech::NOUN)
        break;

    bool canAddANewProperNoun = true;
    bool needToInsert = false;
    for (auto it = pInfosGram.begin(); it != pInfosGram.end(); )
    {
      switch (it->word.partOfSpeech)
      {
      case PartOfSpeech::PROPER_NOUN:
      {
        canAddANewProperNoun = false;
        if (needToInsert)
        {
          auto nextIt = it;
          ++nextIt;
          pInfosGram.splice(insertProperNoun, pInfosGram, it);
          it = nextIt;
          continue;
        }
        break;
      }
      case PartOfSpeech::NOUN:
      {
        if (ConceptSet::haveAConceptThatBeginWith(it->infos.concepts, "time_weekday_"))
          canAddANewProperNoun = false;
        break;
      }
      case PartOfSpeech::PRONOUN:
      {
        canAddANewProperNoun = false;
        break;
      }
      default:
        break;
      };
      if (it == insertProperNoun)
      {
        needToInsert = true;
      }
      ++it;
    }

    if (originalWordBeginWithUpperCase && canAddANewProperNoun &&
        !beginWithUppercase(pTokSent.textForms.formattedText, pTokSent.posAfterPreviousToken))
    {
      InflectedWord properNounIgram;
      properNounIgram.word.partOfSpeech = PartOfSpeech::PROPER_NOUN;
      properNounIgram.word.lemma = pTokStr;
      pInfosGram.emplace_back(std::move(properNounIgram));
    }
  }

  // put all noun flexions in unknown gram possibilities
  for (InflectedWord& currInfoGram : pInfosGram)
  {
    if (currInfoGram.word.partOfSpeech == PartOfSpeech::UNKNOWN)
    {
      currInfoGram.moveInflections(_getAllNounInflections(pLanguage));
      if (isAUrl(pTokStr))
        currInfoGram.infos.concepts.emplace("url_*", 4);
    }
  }

  bool hasBeenMergedWithPreviousToken = false;
  if (pTokSent.tokens.size() >= 2)
  {
    if (pTokSent.tokens.back().str == "-" || pTokSent.tokens.back().str == "/")
      hasBeenMergedWithPreviousToken =
          _mergeIfItsAComposedWord(pTokSent.tokens, pTokStr, pInfosGram,
                                   [](PartOfSpeech pOs1, PartOfSpeech pOs2) { return partOfSpeech_isAWord(pOs1) && (!partOfSpeech_isVerbal(pOs1) || pOs2 == PartOfSpeech::PROPER_NOUN); });
    else if (pTokSent.tokens.back().str == "'")
      hasBeenMergedWithPreviousToken =
          _mergeIfItsAComposedWord(pTokSent.tokens, pTokStr, pInfosGram,
                                   [](PartOfSpeech pOs, PartOfSpeech) { return pOs == PartOfSpeech::PROPER_NOUN; });
  }
  if (hasBeenMergedWithPreviousToken)
  {
    // refresh the reference positions
    _refreshReferencePositionsafterAnInsertionWithLength(pTokSent, pLength);
    pTokSent.canBeAWordAtUppercase = false;
    return;
  }

  // if next token can be a word with an upper case
  PartOfSpeech mainPartOfSpeech = pInfosGram.front().word.partOfSpeech;
  if (mainPartOfSpeech == PartOfSpeech::PUNCTUATION)
    pTokSent.canBeAWordAtUppercase = true;
  else if (partOfSpeech_isAWord(mainPartOfSpeech))
    pTokSent.canBeAWordAtUppercase = false;

  // add the token
  pTokSent.tokens.emplace_back(pTokStr);
  assert(!pInfosGram.empty());
  pTokSent.tokens.back().inflWords.splice(pTokSent.tokens.back().inflWords.begin(),
                                          pInfosGram);

  // refresh the reference positions
  _refreshReferencePositionsafterAnInsertionWithLength(pTokSent, pLength);
}


/**
 * @brief Function to add a token.
 * @param pTokSent Current tokenization infos.
 * @param pLength Length of the token.
 * @param pInfosGram Grammatical infos that we want to add to the new token.
 * @param pLingDico Lingistic dictionary.
 */
void _insertToken(TokSent& pTokSent, std::size_t pLength,
                  std::list<InflectedWord>& pInfosGram,
                  SemanticLanguageEnum pLanguage)
{
  const std::string tokStr = pTokSent.textForms.getSubInputText(pTokSent.posAfterPreviousToken, pLength);
  _insertTokenWithStr(pTokSent, pLength, pInfosGram, tokStr, pLanguage);
}

bool _tryToMergeUrlTokens(TokSent& pTokSent,
                          const std::string& pExtension)
{
  auto itBeforeBeginOfUrl = pTokSent.tokens.end();
  auto itBeginOfUrl = itBeforeBeginOfUrl;
  --itBeforeBeginOfUrl;
  bool firstLoop = true;
  while (itBeforeBeginOfUrl != pTokSent.tokens.begin())
  {
    const auto& word = itBeforeBeginOfUrl->inflWords.front().word;
    if (word.partOfSpeech == PartOfSpeech::PUNCTUATION)
    {
      if (word.lemma != ".")
        break;
    }
    if (word.partOfSpeech == PartOfSpeech::INTERSPACE)
      break;
    if (firstLoop)
    {
      firstLoop = false;
    }
    else
    {
      itBeforeBeginOfUrl->str = itBeforeBeginOfUrl->str + itBeginOfUrl->str;
      pTokSent.tokens.erase(itBeginOfUrl);
    }
    itBeginOfUrl = itBeforeBeginOfUrl;
    --itBeforeBeginOfUrl;
  }
  if (!firstLoop)
  {
    itBeginOfUrl->str = itBeginOfUrl->str + pExtension;
    InflectedWord inflUrl;
    inflUrl.word.lemma += itBeginOfUrl->str;
    inflUrl.infos.concepts.emplace("url_*", 4);
    itBeginOfUrl->inflWords.clear();
    itBeginOfUrl->inflWords.emplace_back(std::move(inflUrl));
    return true;
  }
  return false;
}

/**
 * @brief Add in a new token the characters until the previous token.
 * @param pTokSent Current tokenization infos.
 * @param pSearchGramType If we have to ask the database to know the grammatical type of the new token.
 * @param pLingDico Lingistic dictionary.
 */
void _fushPrevCharactersInAToken(TokSent& pTokSent,
                                 bool pSearchGramType,
                                 const LinguisticDictionary& pLingDico)
{
  std::list<InflectedWord> infosGram;
  // if we have to search the grammatical possibilites of the word in the database
  if (pSearchGramType)
  {
    pLingDico.getGramPossibilitiesAndPutUnknownIfNothingFound
        (infosGram,
         pTokSent.textForms.formattedText, pTokSent.posAfterPreviousToken,
         pTokSent.currPos - pTokSent.posAfterPreviousToken);
    if (!pTokSent.tokens.empty() && pTokSent.tokens.back().str == "." &&
        !infosGram.empty())
    {
      const auto& extension = infosGram.front().word.lemma;
      if (canBeTheExtensionOfAnUrl(extension) &&
          _tryToMergeUrlTokens(pTokSent, extension))
      {
        _refreshReferencePositionsafterAnInsertionWithLength(pTokSent, pTokSent.currPos - pTokSent.posAfterPreviousToken);
        return;
      }
    }
  }
  else
  {
    infosGram.emplace_back();
  }
  _insertToken(pTokSent, pTokSent.currPos - pTokSent.posAfterPreviousToken, infosGram,
               pLingDico.getLanguage());
}


void _inParenthesis
(TokSent& pTokSent,
 const LinguisticDictionary& pLingDico,
 const LinguisticDictionary& pCommonLingDico)
{
  const std::size_t endingFound = getEndOfParenthesis(pTokSent.textForms.formattedText, pTokSent.currPos,
                                                      pTokSent.textForms.formattedText.size());
  if (endingFound != std::string::npos)
  {
    char begOfParenthesisChar = pTokSent.textForms.formattedText[pTokSent.currPos];
    if (pTokSent.currPos > pTokSent.posAfterPreviousToken)
      _fushPrevCharactersInAToken(pTokSent, true, pLingDico);
    if (pTokSent.tokens.empty())
      pTokSent.tokens.emplace_back("");
    pTokSent.tokens.back().subTokens = std::make_unique<TokensTree>();

    std::size_t beginInsideParenthesis = pTokSent.currPos + 1;
    std::size_t lengthInsideParenthesis = endingFound - (pTokSent.currPos + 1);
    std::string subInputText = pTokSent.textForms.getSubInputText(beginInsideParenthesis, lengthInsideParenthesis);
    std::list<Token> tokenList;

    std::list<InflectedWord> parenthesisInflWord;
    parenthesisInflWord.emplace_back();
    parenthesisInflWord.back().word.partOfSpeech = PartOfSpeech::LINKBETWEENWORDS;
    tokenList.emplace_front(std::string(1, begOfParenthesisChar), parenthesisInflWord);
    if (begOfParenthesisChar == '(')
    {
      TokSent tokSent(tokenList, subInputText);
      tokSent.canBeAWordAtUppercase = false;
      _databaseTokenizer(tokSent, pLingDico, pCommonLingDico);
    }
    else
    {
      std::list<InflectedWord> textInflWord;
      textInflWord.emplace_back();
      textInflWord.back().word.lemma = pTokSent.textForms.formattedText.substr(beginInsideParenthesis, lengthInsideParenthesis);
      tokenList.emplace_back(subInputText, textInflWord);
    }
    tokenList.emplace_back(std::string(1, pTokSent.textForms.formattedText[endingFound]), parenthesisInflWord);

    TokensTree& tokensTree = *pTokSent.tokens.back().subTokens;
    _listOfTokensToVectorOfTokens(tokensTree.tokens, tokenList);
    _refreshReferencePositionsafterAnInsertionWithAbsolutePosition(pTokSent, endingFound + 1);
  }
}


std::size_t _getMaxLength(TokSent& pTokSent,
                          const LinguisticDictionary& pLingDico,
                          const LinguisticDictionary& pCommonLingDico)
{
  if (pTokSent.canBeAWordAtUppercase)
  {
    std::size_t res = pLingDico.getLengthOfLongestWord
        (pTokSent.textForms.formattedText, pTokSent.currPos);
    if (pLingDico.statDb.getLanguageType() != SemanticLanguageEnum::UNKNOWN &&
        !beginWithUppercase(pTokSent.textForms.formattedText, pTokSent.currPos))
    {
      std::size_t commonBinDicoLen = pCommonLingDico.getLengthOfLongestWord
          (pTokSent.textForms.formattedText, pTokSent.currPos);
      return std::max(res, commonBinDicoLen);
    }
    if (res > 0)
    {
      return res;
    }
    lowerCaseFirstLetter(pTokSent.textForms.formattedText, pTokSent.currPos);
  }
  std::size_t res = pLingDico.getLengthOfLongestWord
      (pTokSent.textForms.formattedText, pTokSent.currPos);
  if (pLingDico.statDb.getLanguageType() != SemanticLanguageEnum::UNKNOWN)
  {
    std::size_t commonBinDicoLen = pCommonLingDico.getLengthOfLongestWord
        (pTokSent.textForms.formattedText, pTokSent.currPos);
    return std::max(res, commonBinDicoLen);
  }
  return res;
}


void _flushText(TokSent& pTokSent,
                std::size_t pEndOfText,
                const LinguisticDictionary& pLingDico)
{
  if (pTokSent.currPos > pTokSent.posAfterPreviousToken)
    _fushPrevCharactersInAToken(pTokSent, true, pLingDico);

  std::string subInputText = pTokSent.textForms.getSubInputText(pTokSent.currPos, pEndOfText - pTokSent.currPos);
  pTokSent.tokens.emplace_back(subInputText);
  std::list<InflectedWord>& inflWords = pTokSent.tokens.back().inflWords;
  inflWords.emplace_back(PartOfSpeech::UNKNOWN, _getAllNounInflections(pLingDico.getLanguage()));
  InflectedWord& quotedNounIGram = inflWords.back();
  quotedNounIGram.word.lemma = subInputText;
  _refreshReferencePositionsafterAnInsertionWithAbsolutePosition(pTokSent, pEndOfText);
}


void _inQuotationMarks(TokSent& pTokSent,
                       const LinguisticDictionary& pBinDico)
{
  std::size_t endingFound = pTokSent.textForms.formattedText.find_first_of('"', pTokSent.currPos + 1);
  if (endingFound != std::string::npos)
  {
    ++endingFound;
    _flushText(pTokSent, endingFound, pBinDico);
  }
}


void _inComplexQuotationMarks(TokSent& pTokSent,
                              const LinguisticDictionary& pLingDico)
{
  static const std::string quotationMarkStr = "«";
  static const std::size_t quotationMarkStrSize = quotationMarkStr.size();
  if (pTokSent.textForms.formattedText.compare(pTokSent.currPos, quotationMarkStrSize, "«") != 0)
    return;

  std::size_t endingFound = pTokSent.textForms.formattedText.find("»", pTokSent.currPos + quotationMarkStrSize);
  if (endingFound != std::string::npos)
  {
    endingFound += quotationMarkStrSize;
    _flushText(pTokSent, endingFound, pLingDico);
  }
}


/**
 * @brief Add a token that have to be followed by a separator.
 * @param pTokSent Current tokenization infos.
 * @param pLongestWord End of the longest word that begin at the current position.
 * @param pInfosGramWord Grammatical infos of the longest word that begin at the current position.
 * @param pLingDico Lingistic dictionary.
 */
bool _addTokenFollowedByASeparator(TokSent& pTokSent,
                                   const std::size_t pLongestWord,
                                   std::list<InflectedWord>& pInfosGramWord,
                                   const LinguisticDictionary& pLingDico)
{
  auto language = pLingDico.getLanguage();
  auto wordPartOfSpeech = pInfosGramWord.begin()->word.partOfSpeech;
  // if the token is a word
  if (partOfSpeech_isAWord(wordPartOfSpeech))
  {
    // if are are just after the end of the previous token
    if (pTokSent.currPos == pTokSent.posAfterPreviousToken)
    {
      // if it's the end of the sentence
      if (pTokSent.currPos + pLongestWord == pTokSent.endPosToTokenize)
      {
        _insertToken(pTokSent, pLongestWord, pInfosGramWord, language);
        return true;
      }

      // Check that the next token is a separator
      const std::size_t longestSep = pLingDico.getLengthOfLongestWord
          (pTokSent.textForms.formattedText, pTokSent.currPos + pLongestWord);
      std::list<InflectedWord> infosGramSep;
      pLingDico.getGramPossibilitiesAndPutUnknownIfNothingFound
          (infosGramSep,
           pTokSent.textForms.formattedText, pTokSent.currPos + pLongestWord, longestSep);
      if (!partOfSpeech_isAWord(infosGramSep.begin()->word.partOfSpeech))
      {
        // add the word and the separator in 2 tokens
        _insertToken(pTokSent, pLongestWord, pInfosGramWord, language);
        _insertToken(pTokSent, longestSep, infosGramSep, language);
        return true;
      }
    }
    return false;
  }
  else // if the current token is a separator
  {
    char currChar = pTokSent.textForms.formattedText[pTokSent.currPos];
    if (currChar != ' ' &&
        (wordPartOfSpeech != PartOfSpeech::PUNCTUATION ||
         (pTokSent.currPos + 1 < pTokSent.endPosToTokenize &&
          pTokSent.textForms.formattedText[pTokSent.currPos + 1] != ' ')) &&
        isAUrl(pTokSent.textForms.formattedText, pTokSent.posAfterPreviousToken))
      return false;
    if (currChar == '.' &&
        pTokSent.currPos + 1 < pTokSent.endPosToTokenize &&
        pTokSent.textForms.formattedText[pTokSent.currPos + 1] == ',')
      return false;

    // add in a new token the characters until the current position
    if (pTokSent.currPos > pTokSent.posAfterPreviousToken)
      _fushPrevCharactersInAToken(pTokSent, true, pLingDico);

    _insertToken(pTokSent, pLongestWord, pInfosGramWord, language);
  }
  return true;
}


void _fillInflWordOfHour(std::list<InflectedWord>& pInflWord,
                         SemanticLanguageEnum pLanguage)
{
  // a number can be a determiner or a noun
  pInflWord.emplace_back([&]
  {
    InflectedWord nounIGram(PartOfSpeech::NOUN,
                            pLanguage == SemanticLanguageEnum::ENGLISH ?
                              NominalInflections::get_inflections_ns() :
                              NominalInflections::get_inflections_ms());
    nounIGram.infos.concepts.emplace("time_day_*", 4);
    return nounIGram;
  }());
}

void _fillInflWordOfNumber(std::list<InflectedWord>& pInflWord,
                           const std::string& pNumberStr,
                           SemanticLanguageEnum pLanguage,
                           bool pNumberOrRank)
{
  const std::string beginOfNumberConcept = pNumberOrRank ? "number_" : "rank_";
  SemanticFloat nb;
  if (nb.fromStr(pNumberStr, pLanguage))
  {
    auto numberStr = nb.toStr(SemanticLanguageEnum::ENGLISH);
    // a number can be a determiner or a noun
    pInflWord.emplace_back([&]
    {
      InflectedWord nounIGram(PartOfSpeech::NOUN,
                              pLanguage == SemanticLanguageEnum::ENGLISH ?
                                NominalInflections::get_inflections_ns() :
                                NominalInflections::get_inflections_ms());
      nounIGram.infos.concepts.emplace(beginOfNumberConcept + numberStr, 4);
      return nounIGram;
    }());
    pInflWord.emplace_back([&]
    {
      InflectedWord inflWord(PartOfSpeech::DETERMINER, _getAllNounInflections(pLanguage));
      inflWord.infos.concepts.emplace(beginOfNumberConcept + numberStr, 4);
      return inflWord;
    }());
    InflectedWord adjInflWord(PartOfSpeech::ADJECTIVE, _getAllNounInflections(pLanguage));
    adjInflWord.infos.concepts.emplace(beginOfNumberConcept + numberStr, 4);
    if (!pNumberOrRank && pLanguage == SemanticLanguageEnum::FRENCH)
    {
      adjInflWord.infos.contextualInfos.insert(WordContextualInfos::CANBEBEFORENOUN);
      pInflWord.emplace_front(std::move(adjInflWord));
    }
    else
    {
      pInflWord.emplace_back(std::move(adjInflWord));
    }
  }
  else
  {
    pInflWord.emplace_back(PartOfSpeech::UNKNOWN, _getAllNounInflections(pLanguage));
  }
}


void _addNumberOrHourToken(TokSent& pTokSent, std::size_t pPosBegin, std::size_t pPosEnd,
                           SemanticLanguageEnum pLanguage,
                           bool pIsAnHour,
                           const mystd::optional<std::size_t>& pEndOfNumberOpt = mystd::optional<std::size_t>())
{
  std::size_t length = pPosEnd - pPosBegin;
  const std::string tokStr = pTokSent.textForms.formattedText.substr(pTokSent.posAfterPreviousToken, length);
  std::list<InflectedWord> infosGram;
  if (pEndOfNumberOpt)
  {
    std::size_t numberLength = *pEndOfNumberOpt - pPosBegin;
    const std::string tokNumberStr = pTokSent.textForms.formattedText.substr(pTokSent.posAfterPreviousToken, numberLength);
    if (pIsAnHour)
      _fillInflWordOfHour(infosGram, pLanguage);
    else
      _fillInflWordOfNumber(infosGram, tokNumberStr, pLanguage, false);
  }
  else
  {
    if (pIsAnHour)
      _fillInflWordOfHour(infosGram, pLanguage);
    else
      _fillInflWordOfNumber(infosGram, tokStr, pLanguage, true);
  }
  _insertTokenWithStr(pTokSent, length, infosGram, tokStr, pLanguage);
}


/**
 * @brief Try to add a number token at the current position.
 * @param pTokSent Current tokenization infos.
 * @param pLingDico Lingistic dictionary.
 */
void _tryToTokenizeANumber(TokSent& pTokSent,
                           const LinguisticDictionary& pLingDico)
{
  auto languageType = pLingDico.statDb.getLanguageType();
  bool isAnHour = false;
  bool isAPercentage = false;
  // until we are not at the end of the sentence
  while (pTokSent.currPos < pTokSent.endPosToTokenize)
  {
    if (isDigit(pTokSent.textForms.formattedText[pTokSent.currPos]))
    {
      // advance in the sentence
      ++pTokSent.currPos;
      continue;
    }
    if (!isAPercentage  && !isAnHour)
    {
      if (pTokSent.textForms.formattedText[pTokSent.currPos] == '%')
      {
        isAPercentage = true;
        continue;
      }
      if (pTokSent.textForms.formattedText[pTokSent.currPos] == 'h')
      {
        isAnHour = true;
        // advance in the sentence
        ++pTokSent.currPos;
        continue;
      }
    }
    if ((pTokSent.textForms.formattedText[pTokSent.currPos] == ' ' ||
         pTokSent.textForms.formattedText[pTokSent.currPos] == ',' ||
         pTokSent.textForms.formattedText[pTokSent.currPos] == '.') &&
        pTokSent.currPos + 1 < pTokSent.endPosToTokenize &&
        isDigit(pTokSent.textForms.formattedText[pTokSent.currPos + 1]))
    {
      // advance in the sentence
      pTokSent.currPos += 2;
      continue;
    }
    // if the current position is not just after the previous token
    // (so we have found digits previously)
    if (pTokSent.currPos > pTokSent.posAfterPreviousToken)
    {
      mystd::optional<std::size_t> endOfNumber;
      if (languageType == SemanticLanguageEnum::ENGLISH)
      {
        if (pTokSent.currPos + 2 <= pTokSent.endPosToTokenize)
        {
          if (pTokSent.textForms.formattedText.compare(pTokSent.currPos, 2, "th") == 0)
          {
            endOfNumber.emplace(pTokSent.currPos);
            pTokSent.currPos = pTokSent.currPos + 2;
          }
          else if (pTokSent.textForms.formattedText.compare(pTokSent.currPos, 2, "pm") == 0 ||
                   pTokSent.textForms.formattedText.compare(pTokSent.currPos, 2, "am") == 0)
          {
            std::size_t afterSuffixPos = pTokSent.currPos + 2;

            if (afterSuffixPos == pTokSent.endPosToTokenize)
            {
              _addNumberOrHourToken(pTokSent, pTokSent.posAfterPreviousToken, pTokSent.currPos, languageType, isAnHour);
            }
            else
            {
              const std::size_t longestWord = pLingDico.getLengthOfLongestWord
                  (pTokSent.textForms.formattedText, afterSuffixPos);
              if (longestWord > 0)
              {
                std::list<InflectedWord> infosGramSep;
                pLingDico.getGramPossibilitiesAndPutUnknownIfNothingFound
                    (infosGramSep, pTokSent.textForms.formattedText, afterSuffixPos, longestWord);
                // if after the number we have a separator we add the number in a new token
                if (!partOfSpeech_isAWord(infosGramSep.begin()->word.partOfSpeech))
                  _addNumberOrHourToken(pTokSent, pTokSent.posAfterPreviousToken, pTokSent.currPos, languageType, isAnHour);
              }
            }
            return;
          }
        }
      }
      else if (languageType == SemanticLanguageEnum::FRENCH)
      {
        if (pTokSent.currPos + 1 <= pTokSent.endPosToTokenize &&
            pTokSent.textForms.formattedText[pTokSent.currPos] == 'e')
        {
          endOfNumber.emplace(pTokSent.currPos);
          pTokSent.currPos = pTokSent.currPos + 1;
        }
      }

      if (pTokSent.currPos == pTokSent.endPosToTokenize || isAPercentage)
      {
        _addNumberOrHourToken(pTokSent,
                        pTokSent.posAfterPreviousToken, pTokSent.currPos, languageType, isAnHour, endOfNumber);
      }
      else
      {
        const std::size_t longestWord = pLingDico.getLengthOfLongestWord
            (pTokSent.textForms.formattedText, pTokSent.currPos);
        if (longestWord > 0)
        {
          std::list<InflectedWord> infosGramSep;
          pLingDico.getGramPossibilitiesAndPutUnknownIfNothingFound
              (infosGramSep, pTokSent.textForms.formattedText, pTokSent.currPos, longestWord);
          // if after the number we have a separator we add the number in a new token
          if (!partOfSpeech_isAWord(infosGramSep.begin()->word.partOfSpeech))
          {
            _addNumberOrHourToken(pTokSent,
                            pTokSent.posAfterPreviousToken, pTokSent.currPos, languageType, isAnHour, endOfNumber);
            // Add the separator after the number
            _insertToken(pTokSent, longestWord, infosGramSep, languageType);
          }
        }
      }
    }
    return;
  }
  // in this case we have found the begin of a number but we have been stoped because
  // we are at the end of the sentence
  // so we put the number in a new token
  _addNumberOrHourToken(pTokSent, pTokSent.posAfterPreviousToken, pTokSent.currPos, languageType, isAnHour);
}


/**
 * @brief Add a token that don't have to be followed by a separator.
 * @param pTokSent Current tokenization infos.
 * @param pLongestWord End of the longest word that begin at the current position.
 * @param pInfosGramWord Grammatical infos of the longest word that begin at the current position.
 * @param pLingDico Lingistic dictionary.
 */
void _addTokenNotFollowedByASeparator(TokSent& pTokSent,
                                      const std::size_t pLongestWord,
                                      std::list<InflectedWord>& pInfosGramWord,
                                      const LinguisticDictionary& pLingDico)
{
  // add in a new token the characters until the previous token
  if (pTokSent.currPos > pTokSent.posAfterPreviousToken)
    _fushPrevCharactersInAToken(pTokSent, false, pLingDico);

  // add the token found in the database
  _insertToken(pTokSent, pLongestWord, pInfosGramWord, pLingDico.getLanguage());
}


/**
 * @brief Tokenize according to a database.
 * @param pTokSent Current tokenization infos.
 * @param pLingDico Lingistic dictionary.
 * @param pCommonLingDico Lingistic dictionary for words language independent (ex proper nouns).
 */
void _databaseTokenizer(TokSent& pTokSent,
                        const LinguisticDictionary& pLingDico,
                        const LinguisticDictionary& pCommonLingDico)
{
  if (pLingDico.getLanguage() == SemanticLanguageEnum::OTHER)
  {
    pTokSent.tokens.emplace_back(pTokSent.textForms.inputText);
    auto& inflWords = pTokSent.tokens.back().inflWords;
    InflectedWord inflWord(PartOfSpeech::UNKNOWN, std::make_unique<NominalInflections>());
    inflWord.infos.concepts.emplace(mystd::urlizeText(pTokSent.textForms.inputText, true), 4);
    inflWords.emplace_back(std::move(inflWord));
    return;
  }

  // advance in the sentence
  while (pTokSent.currPos < pTokSent.endPosToTokenize)
  {
    // if we encounter a text inside parenthesis
    switch (pTokSent.textForms.formattedText[pTokSent.currPos])
    {
    case '(':
    case '[':
    {
      if (!pTokSent.tokens.empty())
        _inParenthesis(pTokSent, pLingDico, pCommonLingDico);
      break;
    }
    case '"':
    {
      _inQuotationMarks(pTokSent, pLingDico);
      break;
    }
    case static_cast<char>("«"[0]):
    {
      _inComplexQuotationMarks(pTokSent, pLingDico);
      break;
    }
    }

    if (pTokSent.currPos >= pTokSent.endPosToTokenize)
    {
      break;
    }

    std::size_t prevCurrPos = pTokSent.currPos;
    // find the end position in the sentence of the longest word in the database
    // (the word has to start at position "i")
    const std::size_t longestWord = _getMaxLength(pTokSent, pLingDico, pCommonLingDico);
    // find the grammatical posibilities of the word that we found
    std::list<InflectedWord> infosGramWord;
    if (longestWord > 0)
    {
      pLingDico.getGramPossibilities
          (infosGramWord,
           pTokSent.textForms.formattedText, pTokSent.currPos, longestWord);
      if (pLingDico.statDb.getLanguageType() != SemanticLanguageEnum::UNKNOWN &&
          (infosGramWord.empty() || partOfSpeech_isAWord(infosGramWord.front().word.partOfSpeech)))
      {
        pCommonLingDico.getGramPossibilities
            (infosGramWord,
             pTokSent.textForms.formattedText, pTokSent.currPos, longestWord);
      }
    }
    if (infosGramWord.empty())
    {
      infosGramWord.emplace_back();
    }

    // if the words have to be followed by a separator
    if (pLingDico.statDb.haveSeparatorBetweenWords())
    {
      // if we have found a word candidate in the database BUT
      // he is not followed by a separator AND
      // we can lower his first character
      if (longestWord > 0 &&
          !_addTokenFollowedByASeparator(pTokSent, longestWord, infosGramWord, pLingDico) &&
          pTokSent.canBeAWordAtUppercase && lowerCaseFirstLetter(pTokSent.textForms.formattedText, pTokSent.currPos))
      {
        // we redo the iteration (but now the first letter is at lower case)
        continue;
      }
      // if no new token has been inserted from the begin of the loop AND
      // we are just after the previous token (if one exist)
      if (pTokSent.currPos == prevCurrPos &&
          pTokSent.currPos == pTokSent.posAfterPreviousToken)
      {
        // try to tokenize a number at this position
        _tryToTokenizeANumber(pTokSent, pLingDico);
      }
    }
    // else the words don't have to be followed by a separator
    else if (longestWord > 0) // if we have found a word candidate in the database
    {
      _addTokenNotFollowedByASeparator(pTokSent, longestWord,
                                       infosGramWord, pLingDico);
    }

    // if no new token has been inserted from the begin of the loop,
    // advance to the next character
    if (pTokSent.currPos == prevCurrPos)
    {
      ++pTokSent.currPos;
      pTokSent.canBeAWordAtUppercase = false;
    }
  }

  // add in a new token the characters until the previous token
  if (pTokSent.currPos > pTokSent.posAfterPreviousToken)
    _fushPrevCharactersInAToken(pTokSent, true, pLingDico);
}


/**
 * @brief Function to add a bookmark (a marker more precisely).
 * @param pTokens List of tokens in result.
 * @param pStr String that contain the bookmark.
 * @param pBegin Begin of the bookmark in "pStr".
 * @param pLength Length of the bookmark.
 * @param pCmdGrdExtractorPtr Name of command to extract.
 * @param pLanguage Language of the analysis.
 */
void _insertBookmark(std::list<Token>& pTokens,
                     const std::string& pStr, std::size_t pBegin, std::size_t pLength,
                     const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr,
                     SemanticLanguageEnum pLanguage)
{
  pTokens.emplace_back("");
  std::list<InflectedWord>& infosGram = pTokens.back().inflWords;
  infosGram.emplace_back();
  std::string& bookmark = pTokens.back().str;
  bookmark = pStr.substr(pBegin, pLength);

  InflectedWord& newInflWord = infosGram.back();
  if (SemanticMetaGrounding::isTheBeginOfAParam(bookmark))
  {
    newInflWord.word.partOfSpeech = PartOfSpeech::UNKNOWN;
    newInflWord.moveInflections(_getAllNounInflections(pLanguage));
    infosGram.emplace_back(PartOfSpeech::DETERMINER, _getAllNounInflections(pLanguage));
  }
  else if (pCmdGrdExtractorPtr &&
           pCmdGrdExtractorPtr->isBeginOfAResource(bookmark))
  {
    newInflWord.word.partOfSpeech = PartOfSpeech::UNKNOWN;
    newInflWord.moveInflections(_getAllNounInflections(pLanguage));
  }
  else
  {
    newInflWord.word.partOfSpeech = PartOfSpeech::BOOKMARK;
  }
}

}


namespace tokenizer
{

void tokenize
(std::vector<Token>& pTokens,
 const std::string& pInputSentence,
 const LinguisticDictionary& pLingDico,
 const LinguisticDictionary& pCommonLingDico,
 const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr)
{
  std::list<Token> tokensList;

  // This algo split the sentence by each bookmark and
  // call "_databaseTokenizer" between each bookmark
  // =================================================
  // begin of the future token
  std::size_t beginToken = 0;
  // iterate over all characters in the sentence
  for (std::size_t i = 0; i < pInputSentence.size(); ++i)
  {
    // if the begin of a bookmark
    if (pInputSentence[i] == '\\')
    {
      std::size_t beginBookmark = i;
      // go to the end of the bookmark
      while (++i < pInputSentence.size() && pInputSentence[i] != '\\')
        ;
      // if we have fund the end of the bookmark
      if (i < pInputSentence.size())
      {
        // tokenize the sentence until the begin of the bookmark
        if (beginBookmark > beginToken)
        {
          TokSent tokSent(tokensList, pInputSentence.substr(beginToken, beginBookmark - beginToken));
          _databaseTokenizer(tokSent, pLingDico, pCommonLingDico);
        }
        beginToken = i + 1;

        // put the bookmark in the list
        _insertBookmark(tokensList, pInputSentence, beginBookmark,
                        i - beginBookmark + 1, pCmdGrdExtractorPtr, pLingDico.getLanguage());
      }
      else
        i = beginBookmark;
    }
  }

  // tokenize the rest of the sentence
  if (beginToken < pInputSentence.size())
  {
    TokSent tokSent(tokensList, pInputSentence.substr(beginToken, pInputSentence.size() - beginToken));
    _databaseTokenizer(tokSent, pLingDico, pCommonLingDico);
  }

  _listOfTokensToVectorOfTokens(pTokens, tokensList);
}


}  // End of namespace tokenizer
} // End of namespace linguistics
} // End of namespace onsem
