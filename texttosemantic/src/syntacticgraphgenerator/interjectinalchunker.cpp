#include "interjectinalchunker.hpp"
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{

InterjectionalChunker::InterjectionalChunker(const AlgorithmSetForALanguage& pConfiguration)
  : fConf(pConfiguration)
{
}


void InterjectionalChunker::process
(std::list<ChunkLink>& pFirstChildren) const
{
  ChunkLinkWorkingZone workingZone(pFirstChildren, pFirstChildren.begin(), pFirstChildren.end());
  for (auto it = pFirstChildren.begin(); it != pFirstChildren.end(); ++it)
  {
    TokIt beginNextChunk = it->chunk->tokRange.getItBegin();
    TokIt itEnd = it->chunk->tokRange.getItEnd();
    TokIt tokenIt = beginNextChunk;
    while (tokenIt != itEnd)
    {
      PartOfSpeech currPartOfSpeech = tokenIt->inflWords.begin()->word.partOfSpeech;

      // not split after an adverb
      if (currPartOfSpeech == PartOfSpeech::ADVERB)
      {
        tokenIt = getNextToken(tokenIt, itEnd);
        if ((tokenIt == itEnd))
          return;
        tokenIt = getNextToken(tokenIt, itEnd);
        continue;
      }

      if (currPartOfSpeech == PartOfSpeech::INTERJECTION)
      {
        // flush between last separator added (or the begin of the chunk)
        // and the new separator
        if (tokenIt != beginNextChunk && tokenIt != itEnd)
          separateBeginOfAChunk(workingZone.syntTree(), it, tokenIt, it->chunk->type,
                                fConf.getLanguageType());


        // find the begin of next chunk
        beginNextChunk = getNextToken(tokenIt, itEnd);
        auto prevIt = tokenIt;
        // take also the word linked to the separator
        while (beginNextChunk != itEnd)
        {
          bool pursue = false;
          if (!beginNextChunk->linkedTokens.empty() &&
              beginNextChunk->linkedTokens.front() == prevIt)
            pursue = true;
          else if (beginNextChunk->inflWords.front().word.partOfSpeech == PartOfSpeech::INTERJECTION)
            pursue = true;
          if (pursue)
          {
            prevIt = beginNextChunk;
            beginNextChunk = getNextToken(beginNextChunk, itEnd);
            continue;
          }
          break;
        }

        if (beginNextChunk != itEnd)
        {
          separateBeginOfAChunk(workingZone.syntTree(), it, beginNextChunk,
                                ChunkType::INTERJECTION_CHUNK, tokenIt,
                                fConf.getLanguageType());
        }
        else
        {
          it->chunk->type = ChunkType::INTERJECTION_CHUNK;
          it->chunk->head = tokenIt;
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




} // End of namespace linguistics
} // End of namespace onsem
