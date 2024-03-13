#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_EXTRACTSUBCHUNKINSIDELINKEDTOKENS_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_EXTRACTSUBCHUNKINSIDELINKEDTOKENS_HPP

#include <list>
#include <onsem/texttosemantic/type/chunklink.hpp>

namespace onsem {
namespace linguistics {

void extractSubChunkInsideLinkedTokens(std::list<ChunkLink>& pFirstChildren, SemanticLanguageEnum pLanguage);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_EXTRACTSUBCHUNKINSIDELINKEDTOKENS_HPP
