#include "interjectionsadder.hpp"
#include <onsem/texttosemantic/type/syntacticgraph.hpp>

namespace onsem
{
namespace linguistics
{


void _addAnInterjectionToAChunk(
    std::list<ChunkLink>& pChunkList,
    Chunk& pChunkVerb,
    std::list<std::list<ChunkLink>::iterator>& pItInterjections,
    std::list<std::list<ChunkLink>::iterator>& pItSeparators,
    ChunkLinkType pIntjChLk = ChunkLinkType::INTERJECTION)
{
  for (const auto& currInt : pItInterjections)
  {
    currInt->type = pIntjChLk;
    if (pItSeparators.size() >= 1)
      currInt->tokRange = pItSeparators.front()->chunk->tokRange;
    pChunkVerb.children.push_front(*currInt);
    pChunkList.erase(currInt);
  }
  pItInterjections.clear();
  if (pItSeparators.size() >= 1)
    pChunkList.erase(pItSeparators.front());
}


void addInterjections(std::list<ChunkLink>& pChunkList)
{
  std::list<std::list<ChunkLink>::iterator> itInterjections;
  std::list<std::list<ChunkLink>::iterator> itSeparators;
  Chunk* lastVerb = nullptr;
  for (auto it = pChunkList.begin(); it != pChunkList.end(); ++it)
  {
    Chunk& chunk = *it->chunk;
    PartOfSpeech sepHeadGram = it->chunk->head->inflWords.front().word.partOfSpeech;
    if (chunk.type == ChunkType::INTERJECTION_CHUNK)
    {
      itInterjections.emplace_back(it);
    }
    else if (chunk.type == ChunkType::SEPARATOR_CHUNK)
    {
      if (sepHeadGram == PartOfSpeech::PUNCTUATION ||
          sepHeadGram == PartOfSpeech::SUBORDINATING_CONJONCTION ||
          sepHeadGram == PartOfSpeech::CONJUNCTIVE)
      {
        if (lastVerb != nullptr && !itInterjections.empty())
          _addAnInterjectionToAChunk(pChunkList, *lastVerb,
                                     itInterjections, itSeparators);
        lastVerb = nullptr;
        itInterjections.clear();
        itSeparators.clear();
      }
      else
      {
        itSeparators.emplace_back(it);
      }
    }
    else if (chunk.type == ChunkType::NOMINAL_CHUNK &&
             sepHeadGram == PartOfSpeech::ADVERB)
    {
      itSeparators.emplace_back(it);
    }
    else if (chunk.type == ChunkType::VERB_CHUNK ||
             chunk.type == ChunkType::NOMINAL_CHUNK ||
             chunkTypeIsAList(chunk.type))
    {
      if (!itInterjections.empty())
      {
        if (chunk.type == ChunkType::VERB_CHUNK &&
            chunk.requests.has(SemanticRequestType::YESORNO))
        {
          const auto& intjWord = itInterjections.back()->chunk->head->inflWords.front().word;
          if (intjWord.lemma == "que" && intjWord.language == SemanticLanguageEnum::FRENCH)
          {
            chunk.requests.set(SemanticRequestType::OBJECT);
            _addAnInterjectionToAChunk(pChunkList, chunk,
                                      itInterjections, itSeparators,
                                       ChunkLinkType::QUESTIONWORD);
          }
        }
        if (!itInterjections.empty())
          _addAnInterjectionToAChunk(pChunkList, chunk,
                                     itInterjections, itSeparators);
      }
      lastVerb = &*it->chunk;
      itInterjections.clear();
      itSeparators.clear();
    }
  }

  if (lastVerb != nullptr && !itInterjections.empty())
    _addAnInterjectionToAChunk(pChunkList, *lastVerb,
                               itInterjections, itSeparators);
}


} // End of namespace linguistics
} // End of namespace onsem
