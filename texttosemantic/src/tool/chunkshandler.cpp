#include "chunkshandler.hpp"
#include <assert.h>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "listiter.hpp"
#include "../syntacticgraphgenerator/entityrecognizer.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{
ChunkLink* _getChunkLinkWhereWeCanLinkASubodinateFromChildChkLk(ChunkLink& pChkLk)
{
  for (auto& subChild : pChkLk.chunk->children)
    if (subChild.chunk->children.empty() &&
        subChild.chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::PREPOSITION)
      return &subChild;
  return &pChkLk;
}
}

ChunkLinkIter ChunkLinkWorkingZone::beginListIter() const
{
  if (_beginAtBeginOfList)
  {
    return ChunkLinkIter(*this, _syntTreePtr->begin());
  }
  std::list<ChunkLink>::iterator res = _beforeBegin;
  return ChunkLinkIter(*this, ++res);
}

ChunkLinkIter ChunkLinkWorkingZone::endListIter() const
{
  return ChunkLinkIter(*this, _end);
}


TokIt goAtBeginOfWordGroup(TokIt pTokenIt,
                           const TokIt& pTokenBeginIt)
{
  if (pTokenIt->linkedTokens.size() > 1)
  {
    auto it = pTokenIt->linkedTokens.begin();
    auto itHeadToken = *it;
    ++it;
    if (it != pTokenIt->linkedTokens.end() &&
        (*it)->tokenPos < itHeadToken->tokenPos &&
         pTokenBeginIt->tokenPos <= (*it)->tokenPos)
      return *it;
    return itHeadToken;
  }
  return pTokenIt;
}

TokIt goAtEndOfWordGroup(TokIt pTokenIt,
                         const TokenRange& pTokRangeContext)
{
  if (pTokenIt->linkedTokens.size() > 1)
  {
    const auto itEnd = pTokRangeContext.getItEnd();
    auto res = *pTokenIt->linkedTokens.begin();
    auto it = --pTokenIt->linkedTokens.end();
    if (it != pTokenIt->linkedTokens.begin() &&
        res->tokenPos < (*it)->tokenPos &&
        (itEnd == pTokRangeContext.getTokList().end() || (*it)->tokenPos <= itEnd->tokenPos))
      return *it;
    return res;
  }
  return pTokenIt;
}


TokenRange tokenToTokenRange(TokIt pTokenIt,
                             const TokenRange& pTokRangeContext)
{
  auto itResBegin = pTokenIt;
  auto itResEnd = pTokenIt;
  const auto itEnd = pTokRangeContext.getItEnd();
  if (pTokenIt->linkedTokens.size() > 1)
  {
    auto it = pTokenIt->linkedTokens.begin();
    auto itHeadToken = *it;
    ++it;
    if (it != pTokenIt->linkedTokens.end() &&
        (*it)->tokenPos < itHeadToken->tokenPos &&
        pTokRangeContext.getItBegin()->tokenPos <= (*it)->tokenPos)
      itResBegin = *it;
    it = --pTokenIt->linkedTokens.end();
    if (it != pTokenIt->linkedTokens.begin() &&
        itHeadToken->tokenPos < (*it)->tokenPos &&
        (itEnd == pTokRangeContext.getTokList().end() || (*it)->tokenPos <= itEnd->tokenPos))
      itResEnd = *it;
  }
  itResEnd = getNextToken(itResEnd, itEnd);
  return TokenRange(pTokRangeContext.getTokList(), itResBegin, itResEnd);
}


void advanceBeforeLastSeparator
(std::list<ChunkLink>::iterator& pPos,
 const ChunkLinkWorkingZone& pWorkingZone)
{
  for (auto it = pPos; it != pWorkingZone.end(); ++it)
  {
    if (it->chunk->type == ChunkType::SEPARATOR_CHUNK)
    {
      pPos = it;
      ++pPos;
    }
  }
}


void addNominalGroup
(std::list<ChunkLink>& pSyntTree,
 const TokenRange& pTokRange,
 SemanticLanguageEnum pLangType)
{
  assert(!pTokRange.isEmpty());

  auto prevIt = pTokRange.getItEnd();
  auto beginSubNomGroup = prevIt;
  auto prevGram = PartOfSpeech::BOOKMARK;
  for (auto itTok = pTokRange.getItBegin(); itTok != pTokRange.getItEnd();
       itTok = getNextToken(itTok, pTokRange.getItEnd()))
  {
    auto currPartOfSpeech = itTok->inflWords.front().word.partOfSpeech;
    if (pLangType == SemanticLanguageEnum::ENGLISH &&
        prevGram == PartOfSpeech::PROPER_NOUN &&
        currPartOfSpeech == PartOfSpeech::DETERMINER)
    {
      beginSubNomGroup = itTok;
      break;
    }
    prevIt = itTok;
    prevGram = currPartOfSpeech;
  }

  if (beginSubNomGroup == pTokRange.getItEnd())
  {
    pSyntTree.emplace_back(ChunkLinkType::SIMPLE,
                           Chunk(pTokRange,
                                 ChunkType::NOMINAL_CHUNK));
    pSyntTree.back().chunk->head = getHeadOfNominalGroup(pTokRange, pLangType);
  }
  else
  {
    {
      TokenRange firstToksRange = pTokRange;
      firstToksRange.setItEnd(beginSubNomGroup);
      pSyntTree.emplace_back(ChunkLinkType::SIMPLE,
                             Chunk(firstToksRange,
                                   ChunkType::NOMINAL_CHUNK));
      pSyntTree.back().chunk->head = getHeadOfNominalGroup(firstToksRange, pLangType);
    }

    {
      TokenRange secondToksRange = pTokRange;
      secondToksRange.setItBegin(beginSubNomGroup);

      pSyntTree.emplace_back(ChunkLinkType::SIMPLE,
                             Chunk(secondToksRange,
                                   ChunkType::NOMINAL_CHUNK));
      pSyntTree.back().chunk->head = getHeadOfNominalGroup(secondToksRange, pLangType);
    }
  }
}


bool doesHaveAPos(const TokenRange& pTokRange,
                  PartOfSpeech pPartOfSpeech)
{
  for (auto itTok = pTokRange.getItBegin(); itTok != pTokRange.getItEnd();
       itTok = getNextToken(itTok, pTokRange.getItEnd()))
    if (itTok->inflWords.front().word.partOfSpeech == pPartOfSpeech)
      return true;
  return false;
}


void separateBeginOfAChunk
(ChunkLinkIter& pChunkIt,
 TokIt pBeginNewChunk,
 ChunkType pChunkType,
 SemanticLanguageEnum pLanguage)
{
  separateBeginOfAChunk(*pChunkIt.getList(), pChunkIt.getIt(), pBeginNewChunk, pChunkType, pLanguage);
}

void separateBeginOfAChunk
(std::list<ChunkLink>& pSyntTree,
 std::list<ChunkLink>::iterator pChunkIt,
 TokIt pNewBeginChunk,
 ChunkType pTypeNewChunk,
 SemanticLanguageEnum pLanguage)
{
  assert(pChunkIt->chunk->tokRange.getItBegin() != pNewBeginChunk);
  assert(pChunkIt->chunk->tokRange.getItEnd() != pNewBeginChunk);
  pSyntTree.insert(pChunkIt,
                   ChunkLink(ChunkLinkType::SIMPLE,
                             Chunk
                             (TokenRange(pChunkIt->chunk->tokRange.getTokList(),
                                         pChunkIt->chunk->tokRange.getItBegin(), pNewBeginChunk),
                              pTypeNewChunk)));
  auto firstNewChunk = pChunkIt;
  --firstNewChunk;
  pChunkIt->chunk->tokRange.setItBegin(pNewBeginChunk);
  for (auto itChild = pChunkIt->chunk->children.begin();
       itChild != pChunkIt->chunk->children.end(); )
  {
    auto nextIt = itChild;
    ++nextIt;
    if (!checkOrder(*pChunkIt->chunk, *itChild->chunk))
      firstNewChunk->chunk->children.splice(firstNewChunk->chunk->children.end(),
                                            pChunkIt->chunk->children, itChild);
    itChild = nextIt;
  }
  if (!ifContainToken(pChunkIt->chunk->head, pChunkIt->chunk->tokRange))
  {
    pChunkIt->chunk->head = getHeadOfNominalGroup(pChunkIt->chunk->tokRange, pLanguage);
  }
  firstNewChunk->chunk->head = getHeadOfNominalGroup(firstNewChunk->chunk->tokRange, pLanguage);
}



void separateBeginOfAChunk
(std::list<ChunkLink>& pSyntTree,
 std::list<ChunkLink>::iterator pChunkIt,
 TokIt pNewBeginChunk,
 ChunkType pTypeNewChunk,
 TokIt pHeadFirstChunk,
 SemanticLanguageEnum pLanguage)
{
  assert(pChunkIt->chunk->tokRange.getItBegin() != pNewBeginChunk);
  assert(pChunkIt->chunk->tokRange.getItEnd() != pNewBeginChunk);
  pSyntTree.insert(pChunkIt,
                   ChunkLink(ChunkLinkType::SIMPLE,
                             Chunk
                             (TokenRange(pChunkIt->chunk->tokRange.getTokList(),
                                         pChunkIt->chunk->tokRange.getItBegin(),
                                         pNewBeginChunk), pTypeNewChunk)));
  auto firstNewChunk = pChunkIt;
  --firstNewChunk;
  pChunkIt->chunk->tokRange.setItBegin(pNewBeginChunk);
  for (auto itChild = pChunkIt->chunk->children.begin();
       itChild != pChunkIt->chunk->children.end(); )
  {
    auto nextIt = itChild;
    ++nextIt;
    if (!checkOrder(*pChunkIt->chunk, *itChild->chunk))
      firstNewChunk->chunk->children.splice(firstNewChunk->chunk->children.end(),
                                            pChunkIt->chunk->children, itChild);
    itChild = nextIt;
  }
  if (!ifContainToken(pChunkIt->chunk->head, pChunkIt->chunk->tokRange))
  {
    pChunkIt->chunk->head = getHeadOfNominalGroup(pChunkIt->chunk->tokRange, pLanguage);
    if (pChunkIt->chunk->type != ChunkType::PREPOSITIONAL_CHUNK &&
        pChunkIt->chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::PREPOSITION)
      pChunkIt->chunk->type = ChunkType::PREPOSITIONAL_CHUNK;
  }
  firstNewChunk->chunk->head = pHeadFirstChunk;
}


void separateEndOfAChunk
(ChunkLinkIter& pChunkIt,
 TokIt pBeginNewChunk,
 ChunkType pChunkType,
 SemanticLanguageEnum pLanguage)
{
  separateEndOfAChunk(*pChunkIt.getList(), pChunkIt.getIt(),
                      pBeginNewChunk, pChunkType, pLanguage);
}


void separateEndOfAChunk
(std::list<ChunkLink>& pSyntTree,
 std::list<ChunkLink>::iterator pChunkIt,
 TokIt pBeginNewChunk,
 ChunkType pChunkType,
 SemanticLanguageEnum pLanguage)
{
  assert(pChunkIt->chunk->tokRange.getItBegin() != pBeginNewChunk);
  assert(pChunkIt->chunk->tokRange.getItEnd() != pBeginNewChunk);
  assert(pChunkIt != pSyntTree.end());
  std::list<ChunkLink>::iterator insertPos = pChunkIt;
  pSyntTree.insert(++insertPos,
                   ChunkLink(ChunkLinkType::SIMPLE,
                             Chunk
                             (TokenRange(pChunkIt->chunk->tokRange.getTokList(),
                                         pBeginNewChunk, pChunkIt->chunk->tokRange.getItEnd()),
                              pChunkType)));

  auto lastNewChunk = pChunkIt;
  ++lastNewChunk;
  for (auto itChild = pChunkIt->chunk->children.begin();
       itChild != pChunkIt->chunk->children.end(); )
  {
    auto nextIt = itChild;
    ++nextIt;
    if (checkOrder(*lastNewChunk->chunk, *itChild->chunk))
      lastNewChunk->chunk->children.splice(lastNewChunk->chunk->children.end(),
                                           pChunkIt->chunk->children, itChild);
    itChild = nextIt;
  }
  pChunkIt->chunk->tokRange.setItEnd(pBeginNewChunk);
  if (!ifContainToken(pChunkIt->chunk->head, pChunkIt->chunk->tokRange))
  {
    pChunkIt->chunk->head = getHeadOfNominalGroup(pChunkIt->chunk->tokRange, pLanguage);
  }
  lastNewChunk->chunk->head = getHeadOfNominalGroup(lastNewChunk->chunk->tokRange, pLanguage);
}


void moveEndOfAChunk
(Chunk& pChunk,
 TokIt pBeginNewChunk,
 ChunkLinkType pChunkLinkType,
 ChunkType pChunkType,
 std::list<ChunkLink>& pDestinationList,
 SemanticLanguageEnum pLanguage)
{
  pDestinationList.emplace_back(
                   ChunkLink(pChunkLinkType,
                             Chunk
                             (TokenRange(pChunk.tokRange.getTokList(),
                                         pBeginNewChunk, pChunk.tokRange.getItEnd()),
                              pChunkType)));

  pChunk.tokRange.setItEnd(pBeginNewChunk);
  if (!ifContainToken(pChunk.head, pChunk.tokRange))
    pChunk.head = getHeadOfNominalGroup(pChunk.tokRange, pLanguage);

  auto& newChunk = *(--pDestinationList.end())->chunk;
  newChunk.head = getHeadOfNominalGroup(newChunk.tokRange, pLanguage);
}


void putBeginOfAChunkInTheChunkLink
(std::list<ChunkLink>& pSyntTree,
 std::list<ChunkLink>::iterator pChunkIt,
 TokIt pBeginNewChunk,
 SemanticLanguageEnum pLanguage)
{
  assert(pChunkIt->chunk->tokRange.getItBegin() != pBeginNewChunk);
  assert(pChunkIt->chunk->tokRange.getItEnd() != pBeginNewChunk);
  assert(pChunkIt != pSyntTree.end());
  pChunkIt->tokRange = TokenRange(pChunkIt->chunk->tokRange.getTokList(),
                                  pChunkIt->chunk->tokRange.getItBegin(), pBeginNewChunk);
  pChunkIt->chunk->tokRange.setItBegin(pBeginNewChunk);
  if (!ifContainToken(pChunkIt->chunk->head, pChunkIt->chunk->tokRange))
    pChunkIt->chunk->head = getHeadOfNominalGroup(pChunkIt->chunk->tokRange, pLanguage);
}


void putEndOfAChunkToHisChildren
(Chunk& pChunk,
 TokIt pBeginNewChunk,
 SemanticLanguageEnum pLanguage)
{
  assert(pChunk.tokRange.getItBegin() != pBeginNewChunk);
  assert(pChunk.tokRange.getItEnd() != pBeginNewChunk);

  pChunk.children.emplace_back(ChunkLinkType::SIMPLE,
                             Chunk
                             (TokenRange(pChunk.tokRange.getTokList(),
                                               pBeginNewChunk, pChunk.tokRange.getItEnd()),
                              ChunkType::NOMINAL_CHUNK));

  pChunk.tokRange.setItEnd(pBeginNewChunk);
  if (!ifContainToken(pChunk.head, pChunk.tokRange))
  {
    pChunk.head = getHeadOfNominalGroup(pChunk.tokRange, pLanguage);
  }
  Chunk& subChunk = *pChunk.children.back().chunk;
  subChunk.head = getHeadOfNominalGroup(subChunk.tokRange, pLanguage);
}


void putPrepositionInCunkLkTokenRange(
    ChunkLink& pChunkLink,
    SemanticLanguageEnum pLanguage)
{
  TokenRange& chunkTokRange = pChunkLink.chunk->tokRange;
  assert(!chunkTokRange.isEmpty());
  TokIt itFirstWord = chunkTokRange.getItBegin();
  if (itFirstWord != pChunkLink.chunk->head)
  {
    for (TokIt itNewBeginOfChunk = itFirstWord;
         itNewBeginOfChunk != chunkTokRange.getItEnd();
         itNewBeginOfChunk = getNextToken(itNewBeginOfChunk, chunkTokRange.getItEnd()))
    {
      if (itNewBeginOfChunk->getTokenLinkage() != TokenLinkage::PART_OF_WORD_GROUP ||
          itFirstWord->getTokenLinkage() == TokenLinkage::HEAD_OF_WORD_GROUP)
      {
        if (itNewBeginOfChunk->inflWords.front().word.partOfSpeech == PartOfSpeech::PREPOSITION)
        {
          continue;
        }
      }
      else if (itFirstWord->getTokenLinkage() == TokenLinkage::PART_OF_WORD_GROUP &&
               itFirstWord->linkedTokens.front()->inflWords.front().word.partOfSpeech == PartOfSpeech::PREPOSITION)
      {
        continue;
      }

      if (itNewBeginOfChunk != itFirstWord &&
          itNewBeginOfChunk != chunkTokRange.getItEnd())
      {
        chunkTokRange.setItBegin(itNewBeginOfChunk);
        pChunkLink.chunk->head = getHeadOfNominalGroup(pChunkLink.chunk->tokRange, pLanguage);
        pChunkLink.tokRange.setItBegin(itFirstWord);
        pChunkLink.tokRange.setItEnd(chunkTokRange.getItBegin());
      }
      break;
    }
  }
}


TokIt getHeadOfNominalGroup
(const TokenRange& pTokRange,
 SemanticLanguageEnum pLanguage)
{
  assert(!pTokRange.isEmpty());
  TokIt begIt = pTokRange.getItBegin();
  TokIt enIt = pTokRange.getItEnd();
  TokIt nextextTokenFromBegin = getTheNextestToken(begIt, enIt, SkipPartOfWord::YES);

  switch (pLanguage)
  {
  case SemanticLanguageEnum::FRENCH:
  {
    static const std::vector<PartOfSpeech> french_possiblesHead1 =
    {PartOfSpeech::NOUN, PartOfSpeech::PROPER_NOUN, PartOfSpeech::VERB, PartOfSpeech::UNKNOWN};
    for (TokIt it = nextextTokenFromBegin; it != enIt; it = getNextToken(it, enIt, SkipPartOfWord::YES))
      if (tokenIsMoreProbablyAType(*it, french_possiblesHead1))
        return it;

    // TODO: remove not really needed
    static const std::vector<PartOfSpeech> french_possiblesHead2 = {PartOfSpeech::ADJECTIVE};
    for (TokIt it = nextextTokenFromBegin; it != enIt; it = getNextToken(it, enIt, SkipPartOfWord::YES))
      if (tokenIsMoreProbablyAType(*it, french_possiblesHead2))
        return it;
    break;
  }
  case SemanticLanguageEnum::ENGLISH:
  {
    static const std::vector<PartOfSpeech> english_possiblesHead1 =
    {PartOfSpeech::ADJECTIVE, PartOfSpeech::NOUN, PartOfSpeech::VERB, PartOfSpeech::UNKNOWN};
    for (TokIt it = getPrevToken(enIt, begIt, enIt, SkipPartOfWord::YES); it != enIt;
         it = getPrevToken(it, begIt, enIt, SkipPartOfWord::YES))
    {
      const auto& inflWord = *it->inflWords.begin();
      auto partOfSpeech = inflWord.word.partOfSpeech;
      if (std::find(english_possiblesHead1.begin(), english_possiblesHead1.end(), partOfSpeech) != english_possiblesHead1.end() &&
          !(partOfSpeech == PartOfSpeech::ADJECTIVE &&
            ConceptSet::haveAConceptThatBeginWithAnyOf(inflWord.infos.concepts, {"number_", "rank_"})))
        return it;
    }

    static const std::vector<PartOfSpeech> english_possiblesHead2 = {PartOfSpeech::PROPER_NOUN, PartOfSpeech::ADJECTIVE};
    for (TokIt it = getPrevToken(enIt, begIt, enIt, SkipPartOfWord::YES); it != enIt;
         it = getPrevToken(it, begIt, enIt, SkipPartOfWord::YES))
      if (tokenIsMoreProbablyAType(*it, english_possiblesHead2))
        return it;

    break;
  }
  default:
  {
    for (TokIt it = nextextTokenFromBegin; it != enIt; it = getNextToken(it, enIt, SkipPartOfWord::YES))
      return it;
    break;
  }
  }

  TokIt lastTokenIt = getPrevToken(enIt, begIt, enIt);
  auto it = lastTokenIt;
  while (it != enIt)
  {
    if (it->getTokenLinkage() == TokenLinkage::HEAD_OF_WORD_GROUP)
        return it;
     if (it->getTokenLinkage() == TokenLinkage::STANDALONE)
     {
       if (partOfSpeech_isPronominal(it->inflWords.front().word.partOfSpeech))
         return it;
     }
    it = getPrevToken(it, begIt, enIt, SkipPartOfWord::YES);
  }
  return lastTokenIt;
}


TokIt getEndOfNominalGroup(const TokenRange& pTokRange,
                           TokIt pBeginOfNominalGroup)
{
  TokIt res = getNextToken(pBeginOfNominalGroup, pTokRange.getItEnd());
  while (res != pTokRange.getItEnd())
  {
    if (pBeginOfNominalGroup->inflWords.front().word.partOfSpeech == PartOfSpeech::PROPER_NOUN &&
        res->inflWords.front().word.partOfSpeech == PartOfSpeech::PROPER_NOUN)
    {
      res = getNextToken(res, pTokRange.getItEnd());
      continue;
    }
    break;
  }
  return res;
}

void setChunkType(Chunk& pChunk)
{
  if (pChunk.head->inflWords.begin()->word.partOfSpeech == PartOfSpeech::PREPOSITION)
    pChunk.type = ChunkType::PREPOSITIONAL_CHUNK;
}


bool ifContainToken
(TokIt pTokenToSearch,
 const TokenRange& pTokRange)
{
  for (TokIt it = pTokRange.getItBegin(); it != pTokRange.getItEnd();
       it = getNextToken(it, pTokRange.getItEnd()))
  {
    if (it == pTokenToSearch)
    {
      return true;
    }
  }
  return false;
}


bool hasAChild
(const Chunk& pChunk,
 const Chunk& pPossibleChild)
{
  for (const auto& currChild : pChunk.children)
    if (&*currChild.chunk == &pPossibleChild ||
        hasAChild(*currChild.chunk, pPossibleChild))
      return true;
  return false;
}


bool haveAnAuxiliary
(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::AUXILIARY)
    {
      return true;
    }
  }
  return false;
}


const Chunk* getAuxiliaryChunk(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::AUXILIARY)
      return &*currChild.chunk;
  return nullptr;
}

Chunk* getAuxiliaryChunk(Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::AUXILIARY)
      return &*currChild.chunk;
  return nullptr;
}


Chunk* getChunkWithAQuestionWordChild(Chunk& pVerbChunk)
{
  for (auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::AUXILIARY)
    {
      auto* res = getChunkWithAQuestionWordChild(*currChild.chunk);
      if (res != nullptr)
        return res;
    }
    if (currChild.type == ChunkLinkType::QUESTIONWORD)
      return &pVerbChunk;
  }
  return nullptr;
}


bool haveAQuestionWordChildAfter(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::QUESTIONWORD &&
        checkOrder(pVerbChunk, *currChild.chunk))
      return true;
  }
  return false;
}


void forEachAuxiliaryChunks(const Chunk& pVerbChunk,
                            const std::function<void(const Chunk&)>& pCallback)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::AUXILIARY)
    {
      pCallback(*currChild.chunk);
      forEachAuxiliaryChunks(*currChild.chunk, pCallback);
    }
  }
}

std::list<ChunkLink>::iterator getChunkLink
(Chunk& pVerbChunk,
 ChunkLinkType pLinkType)
{
  for (auto it = pVerbChunk.children.begin(); it != pVerbChunk.children.end(); ++it)
    if (it->type == pLinkType)
      return it;
  return pVerbChunk.children.end();
}


std::list<ChunkLink>::const_iterator getChunkLink
(const Chunk& pVerbChunk,
 ChunkLinkType pLinkType)
{
  for (auto it = pVerbChunk.children.begin(); it != pVerbChunk.children.end(); ++it)
  {
    if (it->type == pLinkType)
    {
      return it;
    }
  }
  return pVerbChunk.children.end();
}

bool haveChild(const Chunk& pChunk,
               ChunkLinkType pLinkType)
{
  for (const auto& currChild : pChunk.children)
    if (currChild.type == pLinkType)
      return true;
  return false;
}

bool haveChildWithAuxSkip(const Chunk& pVerbChunk,
                          const std::set<ChunkLinkType>& pChildChunkLinks)
{
  for (auto& currChild : pVerbChunk.children)
  {
    if (pChildChunkLinks.count(currChild.type) > 0)
      return true;
    if (currChild.type == ChunkLinkType::AUXILIARY &&
        haveChildWithAuxSkip(*currChild.chunk, pChildChunkLinks))
      return true;
  }
  return false;
}


const Chunk* getChildChunkPtr(const Chunk& pChunk,
                              ChunkLinkType pLinkType)
{
  for (const auto& currChild : pChunk.children)
    if (currChild.type == pLinkType)
      return &*currChild.chunk;
  return nullptr;
}

Chunk* getChildChunkPtr(Chunk& pChunk,
                        ChunkLinkType pLinkType)
{
  for (auto& currChild : pChunk.children)
    if (currChild.type == pLinkType)
      return &*currChild.chunk;
  return nullptr;
}


bool haveAQuestionInDirectObject(const Chunk& pChunk)
{
  auto* objectChunkPtr = getChildChunkPtr(pChunk, ChunkLinkType::DIRECTOBJECT);
  return objectChunkPtr != nullptr && !objectChunkPtr->requests.empty();
}


const Chunk& getFirstListEltChunk
(const Chunk& pChunk)
{
  if (chunkTypeIsAList(pChunk.type) &&
      !pChunk.children.empty())
    return getFirstListEltChunk(*pChunk.children.front().chunk);
  return pChunk;
}


namespace
{
bool _recHaveASubject(const Chunk& pVerbChunk,
                      const SemanticRequests& pVerbRequests)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    switch (currChild.type)
    {
    case ChunkLinkType::SUBJECT:
    {
      return true;
    }
    case ChunkLinkType::AUXILIARY:
    {
      if (_recHaveASubject(*currChild.chunk, pVerbRequests))
        return true;
      break;
    }
    case ChunkLinkType::QUESTIONWORD:
    {
      if (pVerbRequests.has(SemanticRequestType::OBJECT) &&
          !currChild.chunk->head->inflWords.front().infos.hasContextualInfo(WordContextualInfos::CANNOTBEASUBJECT))
        return true;
      break;
    }
    default:
      break;
    }
  }
  return false;
}
}


bool haveASubject(const Chunk& pVerbChunk)
{
  return _recHaveASubject(pVerbChunk, pVerbChunk.requests);
}

bool haveASubjectExceptQuestionWord(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::SUBJECT)
      return true;
    if (currChild.type == ChunkLinkType::AUXILIARY)
      return haveASubject(*currChild.chunk);
  }
  return false;
}


bool haveAChildBefore(const Chunk& pChunk)
{
  for (const auto& currChild : pChunk.children)
    if (checkOrder(*currChild.chunk, pChunk))
      return true;
  return false;
}

bool conditionIsTheFirstChildAndThereIsManyChildren
(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::IF)
      return pVerbChunk.children.size() > 1;
    return false;
  }
  return false;
}


bool doesChunkHaveDeterminerBeforeHead(const Chunk& pChunk)
{
  const auto& itEnd = pChunk.tokRange.getItEnd();
  for (TokIt it = pChunk.tokRange.getItBegin(); it != itEnd;
       it = getNextToken(it, itEnd))
  {
    if (it->getPartOfSpeech() == PartOfSpeech::DETERMINER)
      return true;
  }
  return false;
}


bool haveADO
(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::DIRECTOBJECT)
      return true;
  return false;
}

bool haveASubordonateClause
(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::SUBORDINATE_CLAUSE)
      return true;
  return false;
}

bool haveAChildWithChunkLink
(const Chunk& pVerbChunk,
 ChunkLinkType pChildChunkLink)
{
  for (const auto& currChild : pVerbChunk.children)
    if (currChild.type == pChildChunkLink)
      return true;
  return false;
}


Chunk& whereToLinkSubject
(Chunk& pVerbChunk)
{
  for (std::list<ChunkLink>::iterator it = pVerbChunk.children.begin();
       it != pVerbChunk.children.end(); ++it)
  {
    if (it->type == ChunkLinkType::AUXILIARY)
    {
      return *it->chunk;
    }
  }
  return pVerbChunk;
}


bool haveOtherEltsBetterToLinkInAList(ChunkLinkIter& pBeforeLastElt,
                                      ChunkLinkIter& pLastElt)
{
  auto& lastWord = pLastElt->chunk->getHeadWord();
  if (pBeforeLastElt->chunk->getHeadWord() == lastWord)
    return false;

  auto beforeElt = pBeforeLastElt;
  --beforeElt;
  while (!beforeElt.atEnd())
  {
    auto& beforeEltChunkType = beforeElt->chunk->type;
    if (chunkTypeIsAList(beforeEltChunkType) ||
        (beforeEltChunkType == ChunkType::SEPARATOR_CHUNK && beforeElt->chunk->getHeadPartOfSpeech() != PartOfSpeech::SUBORDINATING_CONJONCTION))
      return false;
    if (beforeElt->chunk->getHeadWord() == lastWord)
      return true;
    --beforeElt;
  }
  return false;
}


const ChunkLink* getChunkLinkWithAuxSkip(const Chunk& pVerbChunk,
                                         ChunkLinkType pChildChunkLink)
{
  for (auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == pChildChunkLink)
      return &currChild;
    if (currChild.type == ChunkLinkType::AUXILIARY)
    {
      auto* subRes = getChunkLinkWithAuxSkip(*currChild.chunk, pChildChunkLink);
      if (subRes != nullptr)
        return subRes;
    }
  }
  return nullptr;
}

ChunkLink* getSubjectChunkLink(Chunk& pVerbChunk)
{
  for (auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::SUBJECT)
      return &currChild;
    if (currChild.type == ChunkLinkType::AUXILIARY)
    {
      auto* subRes = getSubjectChunkLink(*currChild.chunk);
      if (subRes != nullptr)
        return subRes;
    }
  }
  return nullptr;
}


ChunkLinkIter getSubjectChunkLkIterator(Chunk& pVerbChunk)
{
  for (auto it = pVerbChunk.children.begin(); it != pVerbChunk.children.end(); ++it)
  {
    if (it->type == ChunkLinkType::SUBJECT)
      return ChunkLinkIter(pVerbChunk.children, it);
    if (it->type == ChunkLinkType::AUXILIARY)
      return getSubjectChunkLkIterator(*it->chunk);
  }
  return ChunkLinkIter(pVerbChunk.children, pVerbChunk.children.end());
}


Chunk* getSubjectChunk(const Chunk& pVerbChunk)
{
  for (auto it = pVerbChunk.children.begin(); it != pVerbChunk.children.end(); ++it)
  {
    if (it->type == ChunkLinkType::SUBJECT)
    {
      return &*it->chunk;
    }
    if (it->type == ChunkLinkType::AUXILIARY)
    {
      return getSubjectChunk(*it->chunk);
    }
  }
  return nullptr;
}

Chunk* whereToLinkTheSubjectPtr(Chunk* pVerbChunk,
                                const InflectionsChecker& pFls)
{
  for (std::list<ChunkLink>::iterator it = pVerbChunk->children.begin();
       it != pVerbChunk->children.end(); ++it)
  {
    if (it->type == ChunkLinkType::SUBJECT)
      break;
    if (it->type == ChunkLinkType::AUXILIARY)
    {
      it->chunk->hasOnlyOneReference = false;
      return whereToLinkTheSubjectPtr(&*it->chunk, pFls);
    }
  }
  if (pFls.verbCantHaveSubject(pVerbChunk->head->inflWords.front()))
    return pVerbChunk;
  return nullptr;
}


Chunk& whereToLinkTheSubject
(Chunk& pVerbChunk)
{
  for (auto& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::AUXILIARY)
      return whereToLinkTheSubject(*currChild.chunk);
  return pVerbChunk;
}

const Chunk& whereToLinkTheSubject
(const Chunk& pVerbChunk)
{
  for (auto& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::AUXILIARY)
      return whereToLinkTheSubject(*currChild.chunk);
  return pVerbChunk;
}

ChunkLink* getChunkLinkWhereWeCanLinkASubodinate(Chunk& pVerbChunk)
{
  for (auto it = pVerbChunk.children.rbegin(); it != pVerbChunk.children.rend(); ++it)
    if (it->type == ChunkLinkType::DIRECTOBJECT ||
        it->type == ChunkLinkType::SUBORDINATE ||
        it->type == ChunkLinkType::SUBORDINATE_CLAUSE ||
        it->type == ChunkLinkType::WAY)
      return _getChunkLinkWhereWeCanLinkASubodinateFromChildChkLk(*it);
  return nullptr;
}

ChunkLink* getSubjectChunkLinkWhereWeCanLinkASubodinate(Chunk& pVerbChunk)
{
  for (auto& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::SUBJECT)
      return _getChunkLinkWhereWeCanLinkASubodinateFromChildChkLk(currChild);
  return nullptr;
}


ChunkLink* getDOChunkLink(Chunk& pVerbChunk)
{
  for (ChunkLink& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::DIRECTOBJECT)
      return &currChild;
  return nullptr;
}


const ChunkLink* getDOChunkLink(const Chunk& pVerbChunk)
{
  for (const ChunkLink& currChild : pVerbChunk.children)
    if (currChild.type == ChunkLinkType::DIRECTOBJECT)
      return &currChild;
  return nullptr;
}


Chunk* getlatestVerbChunk(Chunk& pChunk)
{
  for (auto it = pChunk.children.rbegin(); it != pChunk.children.rend(); ++it)
  {
    auto* subRes = getlatestVerbChunk(*it->chunk);
    if (subRes != nullptr)
      return subRes;
  }
  if (pChunk.type == ChunkType::VERB_CHUNK)
    return &pChunk;
  return nullptr;
}


Chunk* getDOComplOrSubjClauseChunk(const Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::DIRECTOBJECT ||
        currChild.type == ChunkLinkType::SUBORDINATE_CLAUSE)
    {
      return &*currChild.chunk;
    }
    else if (currChild.type == ChunkLinkType::COMPLEMENT)
    {
      return getDOComplOrSubjClauseChunk(*currChild.chunk);
    }
  }
  return nullptr;
}

Chunk* getNoneVerbalDOComplOrSubjClauseChunk(Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == ChunkLinkType::DIRECTOBJECT ||
        currChild.type == ChunkLinkType::SUBORDINATE_CLAUSE)
    {
      auto* subDoPtr = getNoneVerbalDOComplOrSubjClauseChunk(*currChild.chunk);
      if (subDoPtr != nullptr || chunkTypeIsVerbal(currChild.chunk->type))
        return subDoPtr;
      return &*currChild.chunk;
    }
    else if (currChild.type == ChunkLinkType::COMPLEMENT)
    {
      return getNoneVerbalDOComplOrSubjClauseChunk(*currChild.chunk);
    }
  }
  return nullptr;
}


Chunk* getReflexiveChunk
(const Chunk& pVerbChunk)
{
  for (std::list<ChunkLink>::const_iterator it = pVerbChunk.children.begin();
       it != pVerbChunk.children.end(); ++it)
  {
    if (it->type == ChunkLinkType::REFLEXIVE)
    {
      return &*it->chunk;
    }
    if (it->type == ChunkLinkType::AUXILIARY)
    {
      Chunk* subReflexive = getReflexiveChunk(*it->chunk);
      if (subReflexive != nullptr)
      {
        return subReflexive;
      }
    }
  }
  return nullptr;
}


bool isAPrepositionnalChunkLink
(const ChunkLink& pChunkLink)
{
  // TODO: do a more accurate algo
  return pChunkLink.chunk->type == ChunkType::PREPOSITIONAL_CHUNK;
}



void setSubjectPronounInflectionsAccordingToTheVerb(InflectedWord& pSubjectPronounInfls,
                                                    const InflectedWord& pVerbInfls,
                                                    const InflectionsChecker& pFls)
{
  if (!pFls.verbCanBePlural(pVerbInfls))
    pFls.pronounRemovePluralPossibility(pSubjectPronounInfls);
  if (!pFls.verbCanBeSingular(pVerbInfls))
    pFls.pronounRemoveSingularPossibility(pSubjectPronounInfls);
}


bool canLinkVerbToASubject
(const Chunk& pVerbChunk,
 const Chunk& pSubjectChunk,
 const InflectionsChecker& pFls,
 bool pCanBeAtPastOrPresentParticiple)
{
  std::list<InflectedWord>::const_iterator
      infGramVerb = pVerbChunk.head->inflWords.begin();
  if ((!pCanBeAtPastOrPresentParticiple && pFls.verbIsOnlyAtPastParticiple(*infGramVerb)) ||
      (!pCanBeAtPastOrPresentParticiple && pFls.verbIsAtPresentParticiple(*infGramVerb)) ||
      pFls.verbIsAtInfinitive(*infGramVerb))
  {
    return false;
  }
  if (pSubjectChunk.type == ChunkType::AND_CHUNK ||
      pSubjectChunk.type == ChunkType::THEN_CHUNK ||
      pSubjectChunk.type == ChunkType::THEN_REVERSED_CHUNK)
  {
    return pFls.verbCanBePlural(*infGramVerb);
  }

  const InflectedWord& firstWordIGram = pSubjectChunk.tokRange.getItBegin()->inflWords.front();
  if (firstWordIGram.word.partOfSpeech == PartOfSpeech::PREPOSITION)
    return false;
  InflectedWord& iGramSubject = pSubjectChunk.head->inflWords.front();
  if (ConceptSet::haveAConceptThatBeginWith(iGramSubject.infos.concepts, "time_") &&
      infGramVerb->word.partOfSpeech != PartOfSpeech::AUX &&
      infGramVerb->infos.concepts.count("verb_equal_be") == 0)
    return false;

  if ((iGramSubject.word.partOfSpeech == PartOfSpeech::DETERMINER ||
          partOfSpeech_isNominal(iGramSubject.word.partOfSpeech) ||
          iGramSubject.word.partOfSpeech == PartOfSpeech::PRONOUN ||
          iGramSubject.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT ||
          (iGramSubject.word.partOfSpeech == PartOfSpeech::VERB &&
           InflectionsChecker::verbIsAtInfinitive(iGramSubject))) &&
      !iGramSubject.infos.hasContextualInfo(WordContextualInfos::CANNOTBEASUBJECT) &&
      pFls.areCompatibles(iGramSubject, *infGramVerb))
  {
    setSubjectPronounInflectionsAccordingToTheVerb(iGramSubject, *infGramVerb, pFls);
    return true;
  }
  return false;
}

int nbOfGNs
(const ChunkLinkWorkingZone& pWorkingZone)
{
  int res = 0;
  for (auto it = pWorkingZone.begin(); it != pWorkingZone.end(); ++it)
  {
    if (it->chunk->type != ChunkType::SEPARATOR_CHUNK)
    {
      ++res;
    }
  }
  return res;
}

bool chunkIsVerbal(const Chunk& pChunk)
{
  return chunkTypeIsVerbal(pChunk.type);
}

bool chunkIsVerbalAndNotAYesOrNo(const Chunk& pChunk)
{
  return chunkTypeIsVerbal(pChunk.type) &&
      !pChunk.requests.has(SemanticRequestType::YESORNO);
}

bool chunkIsNominal(const Chunk& pChunk)
{
  return pChunk.type == ChunkType::NOMINAL_CHUNK;
}

bool chunkIsAtInfinitive(const Chunk& pChunk)
{
  return InflectionsChecker::verbIsAtInfinitive(pChunk.head->inflWords.front());
}

bool chunkDontHaveASubjectAndCannotHaveNoSubject(const Chunk& pChunk)
{
  return !haveASubject(pChunk) &&
      !InflectionsChecker::verbCanHaveNoSubject(pChunk.head->inflWords.front());
}

bool chunkCanBeAtImperative(const Chunk& pChunk)
{
  return InflectionsChecker::verbCanBeAtImperative(pChunk.head->inflWords.front()) && !haveASubject(pChunk);
}

bool chunkIsAtPresentParticiple(const Chunk& pChunk)
{
  return InflectionsChecker::verbIsAtPresentParticiple(pChunk.head->inflWords.front());
}

bool chunkIsAtPastParticiple(const Chunk& pChunk)
{
  return InflectionsChecker::verbIsAtPastParticiple(pChunk.head->inflWords.front());
}


bool recInListConst(std::function<bool(const Chunk& pChunk)> pPredicate,
                    const Chunk& pChunk)
{
  if (chunkTypeIsAList(pChunk.type))
  {
    bool res = false;
    for (const auto& currChild : pChunk.children)
    {
      if (currChild.type == ChunkLinkType::SIMPLE)
      {
        res = recInListConst(pPredicate, *currChild.chunk);
        if (!res)
          return false;
      }
    }
    return res;
  }
  return pPredicate(pChunk);
}


void recInList(std::function<void(Chunk&)> pPredicate,
               Chunk& pChunk)
{
  if (chunkTypeIsAList(pChunk.type))
  {
    for (const auto& currChild : pChunk.children)
      if (currChild.type == ChunkLinkType::SIMPLE)
        recInList(pPredicate, *currChild.chunk);
  }
  else
  {
    pPredicate(pChunk);
  }
}


bool hasAChunkTypeOrAListOfChunkTypes
(const Chunk& pChunk,
 ChunkType pChunkType)
{
  return recInListConst([pChunkType](const Chunk& pChunk)
  {
    return pChunk.type == pChunkType;
  }, pChunk);
}


bool isACommandOrAListOfCommands(const Chunk& pChunk)
{
  return recInListConst([](const Chunk& pChunk)
  {
    return pChunk.requests.has(SemanticRequestType::ACTION);
  }, pChunk);
}

bool doesChunkIsOrCanBeAtImperative(const Chunk& pChunk)
{
  return pChunk.requests.has(SemanticRequestType::ACTION) ||
      (pChunk.requests.empty() &&
       !haveASubject(pChunk) &&
       InflectionsChecker::verbCanBeAtImperative(pChunk.head->inflWords.front()));
}

bool chunkCanHaveSubordinates(const Chunk& pChunk)
{
  return !pChunk.isPassive &&
      chunkTypeIsVerbal(pChunk.type);
}


bool chunkCanBeAnObject(const Chunk& pChunk)
{
  switch (pChunk.type)
  {
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
    PartOfSpeech headPartOfSpeech = pChunk.getHeadPartOfSpeech();
    return headPartOfSpeech != PartOfSpeech::PREPOSITION &&
        headPartOfSpeech != PartOfSpeech::PRONOUN_SUBJECT;
  }
  case ChunkType::NOMINAL_CHUNK:
  {
    const auto& headWord = pChunk.head->inflWords.front().word;
    return headWord.partOfSpeech != PartOfSpeech::PARTITIVE &&
        (headWord.partOfSpeech != PartOfSpeech::PRONOUN_SUBJECT ||
        headWord.language == SemanticLanguageEnum::ENGLISH);
  }
  case ChunkType::SEPARATOR_CHUNK:
  case ChunkType::VERB_CHUNK:
    return false;
  default:
    return true;
  }
}

bool canBeTheHeadOfASubordinate(const InflectedWord& pInflWord)
{
  return pInflWord.word.partOfSpeech != PartOfSpeech::PRONOUN ||
      !pInflWord.infos.hasContextualInfo(WordContextualInfos::CANNOTBEACOREFRENCE);
}

PartOfSpeech getFirstPartOfSpeechOfAChunk(const Chunk& pChunk)
{
  return pChunk.tokRange.getItBegin()->inflWords.front().word.partOfSpeech;
}

bool doesBeginWithAnIndefiniteDeterminer(const TokenRange& pTokRange)
{
  if (!pTokRange.isEmpty())
  {
    const auto& inflWord = pTokRange.getItBegin()->inflWords.front();
    return inflWord.word.partOfSpeech == PartOfSpeech::DETERMINER &&
        ConceptSet::haveAConcept(inflWord.infos.concepts, "reference_indefinite");
  }
  return false;
}

bool doesTokRangeBeginsWithARequestSubordinate(
    const TokenRange& pTokRange,
    const StaticLinguisticDictionary& pBinDico)
{
  return !pTokRange.isEmpty() &&
      pBinDico.wordToSubordinateRequest(pTokRange.getItBegin()->inflWords.front().word) != SemanticRequestType::NOTHING;
}



bool isChunkAtPassiveForm(const Chunk& pVerbChunk,
                          const LinguisticDictionary& pLingDico,
                          const InflectionsChecker& pInflChecker)
{
  const Chunk* auxChunk = getAuxiliaryChunk(pVerbChunk);
  const Chunk* verbChunk = &pVerbChunk;
  if (auxChunk != nullptr && getAuxiliaryChunk(*auxChunk) != nullptr)
    verbChunk = auxChunk;
  const InflectedWord& vbIGram = verbChunk->head->inflWords.front();
  if (vbIGram.infos.hasContextualInfo(WordContextualInfos::PASSIVE))
    return true;
  if (vbIGram.infos.hasContextualInfo(WordContextualInfos::CANNOTBEPASSIVE))
    return false;
  if (pLingDico.getLanguage() == SemanticLanguageEnum::FRENCH)
  {
    SemanticVerbTense verbTense = SemanticVerbTense::UNKNOWN;
    VerbGoalEnum verbGoal = VerbGoalEnum::NOTIFICATION;
    pInflChecker.verbTenseAndGoalFromInflections(verbTense, verbGoal, vbIGram.inflections());

    if (!vbIGram.infos.linkedMeanings)
    {
      if (auxChunk != nullptr)
      {
        const SemanticWord& auxWord = auxChunk->head->inflWords.front().word;
        return auxWord == pLingDico.getBeAux().word;
      }
      else if (verbTense == SemanticVerbTense::PUNCTUALPAST &&
               pInflChecker.verbIsAtPastParticiple(vbIGram))
      {
        return true;
      }
    }
    return false;
  }

  if (auxChunk != nullptr)
  {
    const SemanticWord& auxWord = auxChunk->head->inflWords.front().word;
    return auxWord == pLingDico.getBeAux().word &&
        !pInflChecker.verbIsAtPresentParticiple(vbIGram);
  }
  return false;
}


bool checkOrder(const Chunk& pChunkBefore, const Chunk& pChunkAfter)
{
  return pChunkBefore.head->tokenPos < pChunkAfter.head->tokenPos;
}


void setChunkAtInterrogativeForm(Chunk& pChunk)
{
  if (chunkTypeIsAList(pChunk.type))
  {
    if (pChunk.children.empty())
      pChunk.requests.set(SemanticRequestType::YESORNO);
    return;
  }
  const ChunkLink* doChkLinkPtr = getDOChunkLink(pChunk);
  if (doChkLinkPtr != nullptr &&
      doChkLinkPtr->chunk->type == ChunkType::OR_CHUNK)
    pChunk.requests.set(SemanticRequestType::OBJECT);
  else
    pChunk.requests.set(SemanticRequestType::YESORNO);
  pChunk.form = LingVerbForm::INTERROGATIVE;
}


bool isTokenAtEndOfASentence(TokIt pTokenIt,
                             const TokenRange& pTokRangeContext)
{
  const auto itEnd = pTokRangeContext.getItEnd();
  auto nextItToken = getNextToken(pTokenIt, itEnd);
  return nextItToken == itEnd ||
      nextItToken->inflWords.front().word.partOfSpeech == PartOfSpeech::PUNCTUATION;
}


void addChildInGoodOrder(std::list<ChunkLink>& pChildren,
                         std::list<ChunkLink>& pListOfTheChildToMove,
                         std::list<ChunkLink>::iterator pItChildToMove)
{
  for (auto it = pChildren.begin(); it != pChildren.end(); ++it)
  {
    if (checkOrder(*pItChildToMove->chunk, *it->chunk))
    {
      pChildren.splice(it, pListOfTheChildToMove, pItChildToMove);
      return;
    }
  }
  pChildren.splice(pChildren.end(), pListOfTheChildToMove, pItChildToMove);
}



void linkPartitives(Chunk& rootChunk,
                    ChunkLinkIter& pItToPotentialPartitiveChunk,
                    const EntityRecognizer& pEntityRecognizer)
{
  const Chunk& nextChunk = *pItToPotentialPartitiveChunk->chunk;
  if ((rootChunk.type == ChunkType::NOMINAL_CHUNK || rootChunk.type == ChunkType::PREPOSITIONAL_CHUNK) &&
      canBeParentOfANominalGroup(rootChunk.head->inflWords.front()) &&
      EntityRecognizer::isTheBeginOfAPartitive(nextChunk))
  {
    pItToPotentialPartitiveChunk->type = pEntityRecognizer.findNatureOfAChunkLink(*pItToPotentialPartitiveChunk, nullptr);
    rootChunk.children.splice(rootChunk.children.end(),
                              *pItToPotentialPartitiveChunk.getList(), pItToPotentialPartitiveChunk.getIt());
  }
}


bool filterIncompatibleInflectionsInTokenList(std::vector<Token>& pTokens,
                                              const InflectionsChecker& pFls)
{
  bool res = false;
  const Token* prevPrevToken = nullptr;
  auto prevIt = getTheNextestToken(pTokens.begin(), pTokens.end());
  for (auto it = getNextToken(prevIt, pTokens.end());
       it != pTokens.end(); it = getNextToken(it, pTokens.end()))
  {
    if (pFls.filterIncompatibleInflections(prevPrevToken, *prevIt, *it))
      res = true;
    prevPrevToken = &*prevIt;
    prevIt = it;
  }
  return res;
}




void moveChildren(std::list<ChunkLink>& pRootChildren,
                  ChunkLinkIter& pIteratorToMove,
                  ChunkLinkType pNewChkLkType)
{
  auto* listContainingTheItToMove = pIteratorToMove.getList();
  if (listContainingTheItToMove == nullptr)
  {
    assert(false);
    return;
  }
  auto itToMove = pIteratorToMove.getIt();
  itToMove->type = pNewChkLkType;
  if (itToMove->chunk->children.size() == 1 &&
      itToMove->chunk->children.front().type == ChunkLinkType::SIMPLE &&
      itToMove->chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::PREPOSITION)
  {
    itToMove->tokRange = itToMove->chunk->tokRange;
    itToMove->chunk = itToMove->chunk->children.front().chunk;
  }
  pRootChildren.splice(pRootChildren.end(), *listContainingTheItToMove, itToMove);
}


bool getNumberBeforeHead(SemanticFloat& pNumber,
                         const Chunk& pChunk)
{
  for (TokIt itToken = getPrevToken(pChunk.head, pChunk.tokRange.getItBegin(), pChunk.head);
       itToken != pChunk.head;
       itToken = getPrevToken(itToken, pChunk.tokRange.getItBegin(), pChunk.head))
  {
    return getNumberHoldByTheInflWord(pNumber, itToken, pChunk.head, "number_");
  }
  return false;
}



} // End of namespace linguistics
} // End of namespace onsem
