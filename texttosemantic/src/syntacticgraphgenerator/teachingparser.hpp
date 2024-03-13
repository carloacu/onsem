#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_TEACHINGPARSER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_TEACHINGPARSER_HPP

#include <list>

namespace onsem {
namespace linguistics {
struct ChunkLink;

void teachingParserFr(std::list<ChunkLink>& pChunkList);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_TEACHINGPARSER_HPP
