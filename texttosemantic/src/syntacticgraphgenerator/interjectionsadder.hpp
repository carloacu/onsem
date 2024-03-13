#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INTERJECTIONSADDER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INTERJECTIONSADDER_HPP

#include <list>

namespace onsem {
namespace linguistics {
struct ChunkLink;

void addInterjections(std::list<ChunkLink>& pChunkList);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INTERJECTIONSADDER_HPP
