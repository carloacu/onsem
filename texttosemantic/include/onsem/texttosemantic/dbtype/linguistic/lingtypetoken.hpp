#ifndef ALLINGTYPETOKEN_H
#define ALLINGTYPETOKEN_H

#include <string>
#include <list>
#include <memory>
#include <assert.h>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include "../../api.hpp"

namespace onsem
{
namespace linguistics
{
struct Token;
using TokIt = std::vector<Token>::iterator;
using TokCstIt = std::vector<Token>::const_iterator;
struct TokensTree;


enum class TokenLinkage
{
  STANDALONE,
  HEAD_OF_WORD_GROUP,
  PART_OF_WORD_GROUP
};

enum class SkipPartOfWord
{
  YES,
  NO
};


struct ONSEM_TEXTTOSEMANTIC_API TokenPos
{
  void set(const TokenPos& pOther,
           std::size_t pPos)
  {
    tokenIndexes.reserve(pOther.tokenIndexes.size() + 1);
    tokenIndexes.insert(tokenIndexes.begin(),
                        pOther.tokenIndexes.begin(), pOther.tokenIndexes.end());
    tokenIndexes.emplace_back(pPos);
  }

  void fromStr(const std::string& pStr)
  {
    std::list<std::size_t> posOfSeparators;
    std::size_t currPos = 0;
    while (currPos < pStr.size())
    {
      currPos = pStr.find('_', currPos);

      if (currPos != std::string::npos)
        posOfSeparators.emplace_back(currPos++);
      else
        break;
    }
    posOfSeparators.emplace_back(pStr.size());

    tokenIndexes.reserve(posOfSeparators.size());
    currPos = 0;
    for (const auto& currSep : posOfSeparators)
    {
      tokenIndexes.emplace_back(mystd::lexical_cast_unigned<std::size_t>
                                (pStr.substr(currPos, currSep - currPos)));
      currPos = currSep + 1;
    }
  }

  std::string toStr() const
  {
    bool firstIteration = true;
    std::stringstream ss;
    for (const auto& currIndex : tokenIndexes)
    {
      if (firstIteration)
        firstIteration = false;
      else
        ss << "_";
      ss << currIndex;
    }
    return ss.str();
  }

  bool operator<(const TokenPos& pOther) const
  {
    auto itIndex = tokenIndexes.begin();
    auto itOtherIndex = pOther.tokenIndexes.begin();
    while (itIndex != tokenIndexes.end() && itOtherIndex != pOther.tokenIndexes.end())
    {
      if (*itIndex != *itOtherIndex)
        return *itIndex < *itOtherIndex;
      ++itIndex;
      ++itOtherIndex;
    }
    return tokenIndexes.size() < pOther.tokenIndexes.size();
  }

  bool operator<=(const TokenPos& pOther) const
  {
    auto itIndex = tokenIndexes.begin();
    auto itOtherIndex = pOther.tokenIndexes.begin();
    while (itIndex != tokenIndexes.end() && itOtherIndex != pOther.tokenIndexes.end())
    {
      if (*itIndex != *itOtherIndex)
        return *itIndex < *itOtherIndex;
      ++itIndex;
      ++itOtherIndex;
    }
    return tokenIndexes.size() == pOther.tokenIndexes.size();
  }

  // Returns true if the TokenPos is the first token of the text
  bool isAtBegin() const
  {
    return tokenIndexes.size() == 1 &&
        tokenIndexes[0] == 0;
  }

  // it can have more than 1 index if the token is the child of another one
  // eg1: "I am born in France"
  //      "I"  -> tokenIndexes = [0]
  //      " "  -> tokenIndexes = [1]
  //      "am" -> tokenIndexes = [2]
  //      ...
  //
  // eg2:"I am born in France (at Paris) in 1960"
  //      "I"       -> tokenIndexes = [0]
  //      " "       -> tokenIndexes = [1]
  //      "am"      -> tokenIndexes = [2]
  //      ...
  //      "Paris"  -> tokenIndexes = [8]
  //      " "      -> tokenIndexes = [9]
  //      "("      -> tokenIndexes = [9, 0]
  //      "at"     -> tokenIndexes = [9, 1]
  //      " "      -> tokenIndexes = [9, 2]
  //      "Paris"  -> tokenIndexes = [9, 3]
  //      ")"      -> tokenIndexes = [9, 4]
  //      " "      -> tokenIndexes = [10]
  //      "in"     -> tokenIndexes = [11]
  //      ...
  std::vector<std::size_t> tokenIndexes{};
};


/// Struct of a token.
struct ONSEM_TEXTTOSEMANTIC_API Token
{
  /**
   * @brief Constructor.
   * @param pStr Value of the token.
   * @param pInfosGram Grammatical possibilities of the token.
   */
  Token(const std::string& pStr,
        const std::list<InflectedWord>& pInfosGram = std::list<InflectedWord>());
  ~Token();

  Token(Token&& pOther);
  Token& operator=(Token&& pOther);
  Token(const Token&) = delete;
  Token& operator=(const Token&) = delete;

  bool haveSameInflectedFormThan(const Token& pOther) const;
  TokenLinkage getTokenLinkage() const;
  const Token* getHeadToken() const;
  PartOfSpeech getPartOfSpeech() const;

  /// Value of the token.
  std::string str;

  /**
   * List of all the inflected words possible for the token.
   * The front of the list is the more probable one.
   */
  std::list<InflectedWord> inflWords; // TODO: std::list<std::unique_ptr<LingInfosGram>>


  TokenPos tokenPos;

  /**
   * 1) If this token doesn't belong to a token group
   *    -> This list is empty (so size() == 0)
   * 2) If this token is the head of a token group
   *    -> This list contain himself and all the other sub tokens (so size() > 1)
   * 3) If this token belong to a token group, but it's not the head
   *    -> This list contain only the head (so size() == 1)
   *
   *  By "token group" we don't talk of the tokens inside a chunk but the expressions
   *  from the dictionary of the language
   */
  std::list<TokIt> linkedTokens;

  std::unique_ptr<TokensTree> subTokens;

  // We can restart only one time due to a problem dcetected by this token, it's to avoid potential infinite loop
  bool thisTokenAlreadyCausedARestart;
};



template<typename TOKENSVECTOR, typename TOKENIT>
struct TokenRangeTemplate
{
  TokenRangeTemplate(TOKENSVECTOR& pTokList);

  TokenRangeTemplate(TOKENSVECTOR& pTokList,
                     const TOKENIT pItBegin,
                     const TOKENIT pItEnd);

  TokenRangeTemplate(const TokenRangeTemplate& pOther);

  bool isEmpty() const { return itBegin == itEnd; }

  std::size_t size() const;

  TOKENIT getItBegin() const { return itBegin; }
  TOKENIT getItEnd() const { return itEnd; }

  void setItBegin(const TOKENIT pItBegin) { itBegin = pItBegin; }
  void setItEnd(const TOKENIT pItEnd) { itEnd = pItEnd; }

  TOKENIT getLastWordIt() const;
  TOKENSVECTOR& getTokList() const { return *tokList; }

   TokenRangeTemplate& operator=(const TokenRangeTemplate& pOther);
   void getStr(std::string& pRes) const;
   bool doesContain(const TokenPos& pPos) const;

private:
  TOKENSVECTOR* tokList;
  TOKENIT itBegin;
  TOKENIT itEnd;

  void xReplaceNewLineBySpace(std::string& pResult,
                              const std::string& pInputString) const;
};

using TokenRange = TokenRangeTemplate<std::vector<Token>, TokIt>;
using ConstTokenRange = TokenRangeTemplate<const std::vector<Token>, TokCstIt>;





template<typename TOKENSVECTOR, typename TOKENIT>
struct TokenIteratorTemplate
{
  TokenIteratorTemplate(TOKENSVECTOR& pTokenList,
                        std::size_t pOffset)
    : _tokenRange(pTokenList),
      _itToken(pTokenList.begin()),
      _subIterator(),
      _offset(pOffset)
  {
  }

  TokenIteratorTemplate(TokenRangeTemplate<TOKENSVECTOR, TOKENIT>& pTokenList,
                        std::size_t pOffset)
    : _tokenRange(pTokenList),
      _itToken(pTokenList.getItBegin()),
      _subIterator(),
      _offset(pOffset)
  {
  }

  void setAtBegin();
  void setAtEnd();
  bool atBegin() const;
  bool atEnd() const;

  TOKENIT getItBegin() const { return _tokenRange.getItBegin(); }
  TOKENIT getItEnd() const { return _tokenRange.getItEnd(); }

  TokenIteratorTemplate& operator++();

  void advanceToNextToken(SkipPartOfWord pSkipPartOfWord = SkipPartOfWord::NO)
  {
    _subIterator.reset();
    if (atEnd())
      return;
    auto endIt = _tokenRange.getItEnd();
    auto currToken = _itToken;
    for (auto res = ++currToken; res != endIt; ++res)
    {
      if (res->inflWords.begin()->word.partOfSpeech >= PartOfSpeech::LINKBETWEENWORDS &&
          (pSkipPartOfWord == SkipPartOfWord::NO || res->getTokenLinkage() != TokenLinkage::PART_OF_WORD_GROUP))
      {
        _itToken = res;
        return;
      }
    }
    _itToken = endIt;
  }

  const Token& getToken() const;

  TOKENIT getTokenIt() const;
  void setTokenIt(TOKENIT pItToken);

  /**
    * Offset of the current token from the main tokens tree
    * getOffset() == 0, means the current token is in the main level of tokens
    */
  std::size_t getOffset();

  TokenRangeTemplate<TOKENSVECTOR, TOKENIT>& getTokenRange() { return _tokenRange; }
  const TokenRangeTemplate<TOKENSVECTOR, TOKENIT>& getTokenRange() const { return _tokenRange; }


private:
  TokenRangeTemplate<TOKENSVECTOR, TOKENIT> _tokenRange;
  TOKENIT _itToken;
  std::unique_ptr<TokenIteratorTemplate> _subIterator;
  std::size_t _offset;
};

using TokenIterator = TokenIteratorTemplate<std::vector<Token>, TokIt>;
using ConstTokenIterator = TokenIteratorTemplate<const std::vector<Token>, TokCstIt>;



struct ONSEM_TEXTTOSEMANTIC_API TokensTree
{
  TokensTree() = default;

  TokensTree(const TokensTree&) = delete;
  TokensTree& operator=(const TokensTree&) = delete;

  ConstTokenIterator beginToken() const;
  std::string getText() const;
  std::size_t size() const;

  /// The list of tokens of the sentence.
  std::vector<Token> tokens{};
};


bool tokListHaveSameInflectedFormThan
(const std::vector<Token>& pTokVect1,
 const std::vector<Token>& pTokVect2);


} // End of namespace linguistics
} // End of namespace onsem

#include "detail/lingtypetoken.hxx"

#endif // ALLINGTYPETOKEN_H
