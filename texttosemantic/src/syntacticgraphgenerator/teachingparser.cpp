#include "teachingparser.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../tool/chunkshandler.hpp"


namespace onsem
{
namespace linguistics
{

namespace
{
bool _isATaughtElt(const Chunk& pChunk)
{
  return pChunk.type == ChunkType::VERB_CHUNK &&
      ConceptSet::haveAConcept(pChunk.head->inflWords.begin()->infos.concepts, "verb_haveto");
}

bool _isATaughtList(const Chunk& pChunk)
{
  if (pChunk.type == ChunkType::AND_CHUNK ||
      pChunk.type == ChunkType::THEN_CHUNK)
  {
    bool res = false;
    for (const auto& currChild : pChunk.children)
    {
      if (!_isATaughtElt(*currChild.chunk))
        return false;
      res = true;
    }
    return res;
  }
  return false;
}
}


void teachingParserFr(std::list<ChunkLink>& pChunkList)
{
  ChunkLinkWorkingZone workingZone(pChunkList, pChunkList.begin(), pChunkList.end());
  for (auto it = pChunkList.begin(); it != pChunkList.end(); )
  {
    auto& chunk = *it->chunk;
    if (chunk.type == ChunkType::PREPOSITIONAL_CHUNK)
    {
      auto& headWord = chunk.head->inflWords.begin()->word;
      if (headWord.lemma == "pour")
      {
        auto purposeIt = it;
        ++purposeIt;
        if (purposeIt == pChunkList.end())
          break;
        auto& purposeChunk = *purposeIt->chunk;
        if (purposeChunk.type != ChunkType::INFINITVE_VERB_CHUNK)
        {
          ++it;
          continue;
        }

        auto definitionIt = purposeIt;
        ++definitionIt;
        if (definitionIt == pChunkList.end())
          break;
        auto& definitionChunk = *definitionIt->chunk;

        if (_isATaughtElt(definitionChunk) || _isATaughtList(definitionChunk))
        {
          chunk.type = ChunkType::TEACH_CHUNK;
          it = definitionIt;
          ++it;
          purposeIt->type = ChunkLinkType::PURPOSE;
          chunk.children.splice(chunk.children.end(), pChunkList, purposeIt);
          definitionIt->type = ChunkLinkType::DIRECTOBJECT;
          chunk.children.splice(chunk.children.end(), pChunkList, definitionIt);
          continue;
        }
      }
    }
    ++it;
  }
}


} // End of namespace linguistics
} // End of namespace onsem
