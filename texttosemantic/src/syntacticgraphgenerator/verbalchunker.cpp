#include "verbalchunker.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../tool/chunkshandler.hpp"
#include "questionwords.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{
void _addAuxiliary(Chunk& pRootChildChk,
                   std::list<ChunkLink>& pPrevAuxs,
                   std::list<ChunkLink>::iterator pPrevAuxIt)
{
  // add some auxiliary infos to the verbal group
  {
    Chunk& prevAuxChunk = *pPrevAuxIt->chunk;
    if (!prevAuxChunk.positive)
      pRootChildChk.positive = !pRootChildChk.positive;
    pRootChildChk.form = prevAuxChunk.form;
    pRootChildChk.requests = prevAuxChunk.requests;
  }
  pRootChildChk.children.splice(pRootChildChk.children.begin(), pPrevAuxs, pPrevAuxIt);
}
}


VerbalChunker::VerbalChunker
(const AlgorithmSetForALanguage& pConfiguration)
  : fConf(pConfiguration),
    fSpecLingDb(fConf.getSpecifcLingDb())
{
}


void VerbalChunker::process
(TokensTree& pTokensTree,
 std::list<ChunkLink>& pFirstChildren) const
{
  SemanticLanguageEnum language = fConf.getLanguageType();
  auto& tokens = pTokensTree.tokens;
  pFirstChildren.clear();

  std::list<TokIt> verbsAndAuxs;
  xFindVerbsAndAuxiliaries(verbsAndAuxs, tokens);

  TokIt firstToken = getTheNextestToken(tokens.begin(), tokens.end());
  TokIt minLimitVerbGroup = firstToken;

  std::list<ChunkLink> prevAux;

  for (std::list<TokIt>::const_iterator it = verbsAndAuxs.begin(); it != verbsAndAuxs.end(); ++it)
  {
    TokenRange verbGroupTkenRange(tokens, minLimitVerbGroup, xUntilNextConjVerb(it, verbsAndAuxs, tokens.end()));

    TokIt itVerbBeginGroup;
    TokIt itVerbEndGroup;
    xDelimitVerbChunk(itVerbBeginGroup, itVerbEndGroup, *it,
                      verbGroupTkenRange);

    // if there are words before
    if (minLimitVerbGroup != itVerbBeginGroup)
    {
      bool wordsBeforeAreSaved = false;
      // save the words before
      if (!prevAux.empty())
      {
        Chunk& lastAuxChunk = *prevAux.front().chunk;
        Chunk subjectChunk(TokenRange(tokens, minLimitVerbGroup, itVerbBeginGroup),
                           ChunkType::NOMINAL_CHUNK);
        subjectChunk.head = getHeadOfNominalGroup(subjectChunk.tokRange, language);

        if (canLinkVerbToASubject(lastAuxChunk, subjectChunk, fConf.getFlsChecker(), false))
        {
          // save the words before has the subject of the auxiliary
          lastAuxChunk.children.push_front(ChunkLink(ChunkLinkType::SUBJECT, std::move(subjectChunk)));
          lastAuxChunk.form = LingVerbForm::INTERROGATIVE;
          if (lastAuxChunk.requests.empty())
            lastAuxChunk.requests.set(SemanticRequestType::YESORNO);
          wordsBeforeAreSaved = true;
        }
      }
      if (!wordsBeforeAreSaved)
      {
        addNominalGroup(pFirstChildren,
                        TokenRange(tokens, minLimitVerbGroup, itVerbBeginGroup),
                        language);
      }
    }

    // if it's a verbal group
    const InflectedWord& headIGram = (*it)->inflWords.front();
    if (headIGram.word.partOfSpeech == PartOfSpeech::VERB)
    {
      // add the verbal group
      auto verbChunkType = [this, &headIGram, &prevAux]
      {
        if (!fConf.getFlsChecker().verbIsConjugated(headIGram))
          return ChunkType::INFINITVE_VERB_CHUNK;
        if (!prevAux.empty())
        {
          Chunk& prevAuxChunk = *prevAux.front().chunk;
          if (!fConf.getFlsChecker().verbIsConjugated(prevAuxChunk.head->inflWords.front()))
            return ChunkType::INFINITVE_VERB_CHUNK;
        }
        return ChunkType::VERB_CHUNK;
      }();
      pFirstChildren.emplace_back(ChunkLinkType::SIMPLE,
                                  Chunk(TokenRange(tokens, itVerbBeginGroup, itVerbEndGroup),
                                        verbChunkType));
      Chunk& verbGroup = *pFirstChildren.back().chunk;
      verbGroup.head = *it;
      verbGroup.positive = xIsPositif(verbGroup);
      if (!prevAux.empty())
        _addAuxiliary(verbGroup, prevAux, prevAux.begin());
      questionWords::extractQuestionWordsInsideAVerbGroup(pFirstChildren, --pFirstChildren.end(), fSpecLingDb);
    }
    else // if we are in an auxilairy chunk
    {
      prevAux.push_front(ChunkLink
                         (ChunkLinkType::AUXILIARY,
                          Chunk(TokenRange(tokens, itVerbBeginGroup, itVerbEndGroup),
                                ChunkType::AUX_CHUNK)));
      Chunk& auxGroup = *prevAux.front().chunk;
      auxGroup.head = *it;
      auxGroup.positive = xIsPositif(auxGroup);
      if (prevAux.size() > 1)
        _addAuxiliary(auxGroup, prevAux, --prevAux.end());
      questionWords::extractQuestionWordsInsideAVerbGroup(prevAux, prevAux.begin(), fSpecLingDb);
    }

    minLimitVerbGroup = itVerbEndGroup;
  }

  // add trailing auxiliary
  while (!prevAux.empty())
  {
    prevAux.front().type = ChunkLinkType::SIMPLE;
    pFirstChildren.splice(pFirstChildren.end(), prevAux, prevAux.begin());
  }

  // Add last nominal group
  if (minLimitVerbGroup != tokens.end())
  {
    addNominalGroup(pFirstChildren,
                    TokenRange(tokens, minLimitVerbGroup, tokens.end()),
                    language);
  }

  {
    ChunkLinkWorkingZone workingZone(pFirstChildren, pFirstChildren.begin(), pFirstChildren.end());
    xSplitChunks(workingZone, {PartOfSpeech::CONJUNCTIVE, PartOfSpeech::SUBORDINATING_CONJONCTION,
                               PartOfSpeech::LINKBETWEENWORDS, PartOfSpeech::PUNCTUATION},
                 ChunkType::SEPARATOR_CHUNK);
  }
}




void VerbalChunker::xFindVerbsAndAuxiliaries
(std::list<TokIt>& pConjVerbs,
 std::vector<Token>& pTokens) const
{
  for (TokIt it = pTokens.begin(); it != pTokens.end();
       it = getNextToken(it, pTokens.end()))
  {
    const Token& currToken = *it;
    const InflectedWord& currIGram = *currToken.inflWords.begin();
    if ((currIGram.word.partOfSpeech == PartOfSpeech::AUX ||
        currIGram.word.partOfSpeech == PartOfSpeech::VERB) &&
        currToken.getTokenLinkage() != TokenLinkage::PART_OF_WORD_GROUP)
      pConjVerbs.push_back(it);
  }
}



void VerbalChunker::xDelimitVerbChunk
(TokIt& pItVerbBeginGroup, TokIt& pItVerbEndGroup, TokIt pItVerb,
 const TokenRange& pTokRangeContext) const
{
  const TokIt beginOfPhrase = pTokRangeContext.getItBegin();
  const TokIt endOfPhrase = pTokRangeContext.getItEnd();
  TokenRange berbGroupTokenRange = tokenToTokenRange(pItVerb, pTokRangeContext);
  pItVerbBeginGroup = berbGroupTokenRange.getItBegin();
  pItVerbEndGroup = berbGroupTokenRange.getItEnd();

  std::vector<PartOfSpeech> gramAfterVerb = {PartOfSpeech::ADVERB};
  if (fConf.getLanguageType() == SemanticLanguageEnum::ENGLISH)
  {
    gramAfterVerb.emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  }
  for (TokIt it = getTheNextestToken(pItVerbEndGroup, endOfPhrase, SkipPartOfWord::YES);
       it != endOfPhrase; )
  {
    if (ConceptSet::haveAConceptThatBeginWith(it->inflWords.front().infos.concepts,
                                              "time_") &&
        it->getHeadToken() != &*pItVerb)
      break;
    if (it->inflWords.front().infos.hasContextualInfo(WordContextualInfos::CANNOTBEAFTERVERB))
      break;

    // if we can have this grammatical type after the verb OR
    // the word is linked to the verb
    const auto& word = it->inflWords.front().word;
    if (hasAPartOfSpeech(gramAfterVerb, word.partOfSpeech) &&
        it->getTokenLinkage() != TokenLinkage::PART_OF_WORD_GROUP &&
        fSpecLingDb.lingDico.statDb.wordToQuestionWord(word, false, false) == nullptr)
    {
      it = goAtEndOfWordGroup(it, pTokRangeContext);
      it = getNextToken(it, endOfPhrase);
      pItVerbEndGroup = it;
      continue;
    }
    break;
  }

  std::vector<PartOfSpeech> gramBeforeVerb =
  {PartOfSpeech::AUX, PartOfSpeech::ADVERB};
  if (fConf.getLanguageType() == SemanticLanguageEnum::FRENCH)
  {
    gramBeforeVerb.emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  }
  for (TokIt it = getPrevToken(pItVerbBeginGroup, beginOfPhrase, endOfPhrase); it != endOfPhrase;
       it = getPrevToken(it, beginOfPhrase, endOfPhrase, SkipPartOfWord::YES))
  {
    // if we can have this grammatical type after the verb
    const auto& word = it->inflWords.front().word;
    if (hasAPartOfSpeech(gramBeforeVerb, word.partOfSpeech))
    {
      it = goAtBeginOfWordGroup(it, beginOfPhrase);
      pItVerbBeginGroup = it;
    }
    else
    {
      break;
    }
  }
}


void VerbalChunker::xSplitChunks
(ChunkLinkWorkingZone& pWorkingZone,
 const std::vector<PartOfSpeech>& pChunckSeps,
 ChunkType pChunkType) const
{
  for (auto it = pWorkingZone.begin(); it != pWorkingZone.end(); ++it)
  {
    TokIt beginNextChunk = it->chunk->tokRange.getItBegin();
    TokIt itEnd = it->chunk->tokRange.getItEnd();
    TokIt tokenIt = beginNextChunk;
    while (tokenIt != itEnd)
    {
      PartOfSpeech currPartOfSpeech = tokenIt->inflWords.begin()->word.partOfSpeech;

      if (tokenIsMoreProbablyAType(*tokenIt, pChunckSeps) &&
          tokenIt->getTokenLinkage() == TokenLinkage::STANDALONE)
      {
        // flush between last separator added (or the begin of the chunk)
        // and the new separator
        if (tokenIt != beginNextChunk && tokenIt != itEnd)
        {
          separateBeginOfAChunk(pWorkingZone.syntTree(), it, tokenIt, it->chunk->type,
                                fConf.getLanguageType());
        }
        // get the head of the new separator
        TokIt head = tokenIt;
        if (currPartOfSpeech == PartOfSpeech::LINKBETWEENWORDS)
        {
          TokIt nextWord = getNextWord(tokenIt, itEnd);
          if (nextWord != itEnd &&
              tokenIsMoreProbablyAType(*nextWord, pChunckSeps))
            head = nextWord;
        }

        // add the new separator
        beginNextChunk = getNextToken(head, itEnd);
        // take also the word linked to the separator
        while (beginNextChunk != itEnd &&
               !beginNextChunk->linkedTokens.empty() &&
               beginNextChunk->linkedTokens.front() == tokenIt)
        {
          beginNextChunk = getNextToken(beginNextChunk, itEnd);
        }

        if (beginNextChunk != itEnd)
        {
          separateBeginOfAChunk(pWorkingZone.syntTree(), it, beginNextChunk, pChunkType, head,
                                fConf.getLanguageType());
        }
        else
        {
          it->chunk->type = pChunkType;
          it->chunk->head = head;
        }
        // update the current iterator
        tokenIt = beginNextChunk;
      }
      else
      {
        tokenIt = getNextToken(tokenIt, itEnd);
      }
    }
  }
}

TokIt VerbalChunker::xUntilNextConjVerb
(std::list<TokIt>::const_iterator pCurrVerb,
 const std::list<TokIt>& pVerbList,
 TokIt pLastToken)
{
  ++pCurrVerb;
  if (pCurrVerb == pVerbList.end())
  {
    return pLastToken;
  }
  return *pCurrVerb;
}


bool VerbalChunker::xIsPositif
(const Chunk& pChunk) const
{
  auto itBegin = pChunk.tokRange.getItBegin();
  auto itEnd = pChunk.tokRange.getItEnd();
  if (fConf.getLanguageType() == SemanticLanguageEnum::FRENCH)
  {
    itBegin = getNextToken(pChunk.head, itEnd);
    for (TokIt it = itBegin; it != itEnd; ++it)
      if (it->inflWords.front().infos.hasContextualInfo(WordContextualInfos::NEGATION))
        return false;
  }
  else
  {
    for (TokIt it = itBegin; it != itEnd; ++it)
      if (it->inflWords.front().infos.hasContextualInfo(WordContextualInfos::NEGATION) ||
          // TODO: Put this on the database.
          (it->str.size() > 4 && it->str.compare(it->str.size() - 3, 3, "n't") == 0) ||
          it->str == "cannot" || it->str == "Cannot")
        return false;
  }

  return true;
}


} // End of namespace linguistics
} // End of namespace onsem
