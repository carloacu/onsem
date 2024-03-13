#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INCOMPLETELISTSRESOLVER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INCOMPLETELISTSRESOLVER_HPP

#include <list>

namespace onsem {
namespace linguistics {
class LinguisticDictionary;
struct ChunkLink;

void resolveIncompleteLists(std::list<ChunkLink>& pChunkList, const LinguisticDictionary& pLingDico);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INCOMPLETELISTSRESOLVER_HPP
