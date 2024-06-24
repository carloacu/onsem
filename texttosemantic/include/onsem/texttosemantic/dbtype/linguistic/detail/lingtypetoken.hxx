#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_LINGTYPETOKEN_HXX
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_LINGTYPETOKEN_HXX

#include "../lingtypetoken.hpp"
#include <sstream>
#include <iostream>

namespace onsem
{
namespace linguistics
{


inline Token::Token(const std::string& pStr,
                    const std::list<InflectedWord>& pInfosGram)
  : str(pStr),
    inflWords(pInfosGram),
    tokenPos(),
    linkedTokens(),
    subTokens(),
    thisTokenAlreadyCausedARestart(false)
{
}


inline Token::~Token()
{
}


inline Token::Token(Token&& pOther)
  : str(std::move(pOther.str)),
    inflWords(std::move(pOther.inflWords)),
    tokenPos(std::move(pOther.tokenPos)),
    linkedTokens(),
    subTokens(std::move(pOther.subTokens)),
    thisTokenAlreadyCausedARestart(std::move(pOther.thisTokenAlreadyCausedARestart))
{
  assert(pOther.linkedTokens.empty());
}


inline Token& Token::operator=(Token&& pOther)
{
  str = std::move(pOther.str);
  inflWords = std::move(pOther.inflWords);
  tokenPos = std::move(pOther.tokenPos);
  assert(pOther.linkedTokens.empty());
  subTokens = std::move(pOther.subTokens);
  thisTokenAlreadyCausedARestart = std::move(pOther.thisTokenAlreadyCausedARestart);
  return *this;
}

inline bool Token::haveSameInflectedFormThan
(const Token& pOther) const
{
  if (str == pOther.str &&
      inflWords.size() == pOther.inflWords.size())
  {
    for (auto it = inflWords.begin(),
         itOther = pOther.inflWords.begin();
         it != inflWords.end(); ++it, ++itOther)
      if (!it->isSameInflectedFormThan(*itOther))
        return false;
    return true;
  }
  return false;
}

inline TokenLinkage Token::getTokenLinkage() const
{
  std::size_t size = linkedTokens.size();
  if (size == 0)
    return TokenLinkage::STANDALONE;
  if (size > 1)
    return TokenLinkage::HEAD_OF_WORD_GROUP;
  return TokenLinkage::PART_OF_WORD_GROUP;
}

inline const Token* Token::getHeadToken() const
{
  if (linkedTokens.size() == 1)
    return &*linkedTokens.front();
  return nullptr;
}

inline PartOfSpeech Token::getPartOfSpeech() const
{
  return inflWords.front().word.partOfSpeech;
}


/**
 * @brief Print a token in a stream.
 * @param pOs The stream.
 * @param pToken The token to print.
 * @return The modified stream.
 */
inline std::ostream& operator<<
(std::ostream& pOs, const Token& pToken)
{
  return pOs << "Token: \"" << pToken.str << "\"";
}


template<typename TOKENSVECTOR, typename TOKENIT>
TOKENIT iteratorToSubList(const TOKENIT& pItBegin, const TOKENIT& pItEnd, const TOKENSVECTOR* pSubList) {
    for (TOKENIT it = pItBegin; it != pItEnd; ++it)
    {
        if (!it->subTokens)
            continue;
        if (&it->subTokens->tokens == pSubList)
            return it;

        auto subItEnd = it->subTokens->tokens.end();
        auto subIt = iteratorToSubList(it->subTokens->tokens.begin(), subItEnd, pSubList);
        if (subIt != subItEnd)
            return it;
    }
    return pItEnd;
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::TokenRangeTemplate
(TOKENSVECTOR& pTokList)
  : tokList(&pTokList),
    itBegin(pTokList.end()),
    itEnd(pTokList.end())
{
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::TokenRangeTemplate
(TOKENSVECTOR& pTokList,
 const TOKENIT pItBegin,
 const TOKENIT pItEnd)
  : tokList(&pTokList),
    itBegin(pItBegin),
    itEnd(pItEnd)
{
}

template<typename TOKENSVECTOR, typename TOKENIT>
inline TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::TokenRangeTemplate
(const TokenRangeTemplate& pOther)
  : tokList(pOther.tokList),
    itBegin(pOther.itBegin),
    itEnd(pOther.itEnd)
{
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline TOKENIT TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::getLastWordIt() const
{
  return getPrevToken(itEnd, itBegin, itEnd);
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline TokenRangeTemplate<TOKENSVECTOR, TOKENIT>& TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::operator=
(const TokenRangeTemplate<TOKENSVECTOR, TOKENIT>& pOther)
{
  tokList = pOther.tokList;
  itBegin = pOther.itBegin;
  itEnd = pOther.itEnd;
  return *this;
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline void TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::getStr(std::string& pRes) const
{
  for (TokCstIt it = itBegin; it != itEnd; ++it) {
      std::string subRes;
    xReplaceNewLineBySpace(subRes, it->str);
    pRes += subRes;
  }
}

template<typename TOKENSVECTOR, typename TOKENIT>
inline void TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::toStrInplace(std::string& pRes) const
{
  for (TokCstIt it = itBegin; it != itEnd; ++it) {
    pRes += it->str;
    if (it->subTokens)
        it->subTokens->extractText(pRes);
  }
}

template<typename TOKENSVECTOR, typename TOKENIT>
std::string TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::toStr() const {
    std::string res;
    toStrInplace(res);
    return res;
}

template<typename TOKENSVECTOR, typename TOKENIT>
bool TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::doesContain(const TokenPos& pPos) const
{
  if (itBegin == itEnd ||
      pPos < itBegin->tokenPos)
    return false;
  auto itLast = itEnd;
  --itLast;
  return pPos <= itLast->tokenPos;
}


template<typename TOKENSVECTOR, typename TOKENIT>
void TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::mergeWith(const TokenRangeTemplate& pOther) {
    if (tokList != pOther.tokList) {
        auto res = iteratorToSubList(itBegin, itEnd, pOther.tokList);
        if (res != itEnd) {

            if (itBegin == tokList->end()) {
                itBegin = res;
                ++res;
                itEnd = res;
                return;
            }
            if (res->tokenPos < itBegin->tokenPos) {
                itBegin = res;
                return;
            }

            if (itEnd == tokList->end()) {
                itEnd = tokList->end();
                return;
            }

            if (itEnd->tokenPos < res->tokenPos)
                itEnd = res;
            return;
        }
        std::cerr << "Failed to merge 2 TokenRangeTemplate because they does not come from the same list" << std::endl;
        return;
    }

    if (itBegin == tokList->end() || pOther.itBegin == tokList->end()) {
        itBegin = tokList->end();
        itEnd = tokList->end();
        return;
    }

    if (pOther.itBegin->tokenPos < itBegin->tokenPos)
        itBegin = pOther.itBegin;

    if (itEnd == tokList->end() || pOther.itEnd == tokList->end()) {
        itEnd = tokList->end();
        return;
    }

    if (itEnd->tokenPos < pOther.itEnd->tokenPos)
        itEnd = pOther.itEnd;
}




template<typename TOKENSVECTOR, typename TOKENIT>
inline std::size_t TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::size() const
{
  TokIt firstToken = getTheNextestToken(itBegin, itEnd);
  std::size_t res = 0;
  for (TokCstIt it = firstToken; it != itEnd;
       it = getNextToken(it, itEnd))
  {
    ++res;
  }
  return res;
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline void TokenRangeTemplate<TOKENSVECTOR, TOKENIT>::xReplaceNewLineBySpace
(std::string& pResult,
 const std::string& pInputString) const
{
  // replace '\n' by ' '
  pResult = pInputString;
  std::size_t found = pResult.find_first_of('\n');
  while (found != std::string::npos)
  {
    pResult[found] = ' ';
    found = pResult.find_first_of('\n', found + 1);
  }
}



inline ConstTokenIterator TokensTree::beginToken() const
{
  return ConstTokenIterator(tokens, 0);
}


inline std::string TokensTree::getText() const
{
  std::stringstream ss;
  for (ConstTokenIterator itToken = beginToken(); !itToken.atEnd(); ++itToken)
    ss << itToken.getToken().str;
  return ss.str();
}

inline void TokensTree::extractText(std::string& pText) const
{
    for (ConstTokenIterator itToken = beginToken(); !itToken.atEnd(); ++itToken) {
        auto& currToken = itToken.getToken();
        pText += currToken.str;
        if (currToken.subTokens)
            currToken.subTokens->extractText(pText);
    }
}

inline std::size_t TokensTree::size() const
{
  std::size_t res = 0;
  for (const Token& currToken : tokens)
  {
    ++res;
    if (currToken.subTokens)
      res += currToken.subTokens->size();
  }
  return res;
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline const Token& TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::getToken() const
{
  if (_subIterator)
    return _subIterator->getToken();
  return *_itToken;
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline TOKENIT TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::getTokenIt() const
{
  if (_subIterator)
    return _subIterator->getTokenIt();
  return _itToken;
}

template<typename TOKENSVECTOR, typename TOKENIT>
inline void TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::setTokenIt(TOKENIT pItToken)
{
  _subIterator.reset();
  _itToken = pItToken;
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline std::size_t TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::getOffset()
{
  if (_subIterator)
    return _subIterator->getOffset();
  return _offset;
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline void TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::setAtBegin()
{
  _itToken = _tokenRange.getItBegin();
 _subIterator.reset();
}

template<typename TOKENSVECTOR, typename TOKENIT>
inline void TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::setAtEnd()
{
  _itToken = _tokenRange.getItEnd();
 _subIterator.reset();
}

template<typename TOKENSVECTOR, typename TOKENIT>
inline bool TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::atBegin() const
{
  return _itToken == _tokenRange.getItBegin() && !_subIterator;
}

template<typename TOKENSVECTOR, typename TOKENIT>
inline bool TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::atEnd() const
{
  return _itToken == _tokenRange.getItEnd() &&
      (!_subIterator || _subIterator->atEnd());
}


template<typename TOKENSVECTOR, typename TOKENIT>
inline TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>& TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>::operator++()
{
  if (_subIterator)
  {
    _subIterator->operator++();
    if (!_subIterator->atEnd())
      return *this;
    _subIterator.reset();
  }
  else if (_itToken->subTokens != nullptr &&
           !_itToken->subTokens->tokens.empty())
  {
    _subIterator = std::make_unique<TokenIteratorTemplate<TOKENSVECTOR, TOKENIT>>(_itToken->subTokens->tokens, _offset + 1);
    return *this;
  }

  ++_itToken;
  return *this;
}



inline bool tokListHaveSameInflectedFormThan
(const std::vector<Token>& pTokVect1,
 const std::vector<Token>& pTokVect2)
{
  if (pTokVect1.size() != pTokVect2.size())
    return false;
  auto it2 = pTokVect2.begin();
  for (auto it1 = pTokVect1.begin(); it1 != pTokVect1.end(); ++it1, ++it2)
    if (!it1->haveSameInflectedFormThan(*it2))
      return false;
  return true;
}



} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_LINGTYPETOKEN_HXX
