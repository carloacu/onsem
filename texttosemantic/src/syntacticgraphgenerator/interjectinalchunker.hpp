#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INTERJECTIONALCHUNKER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INTERJECTIONALCHUNKER_HPP

#include <list>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>

namespace onsem {
namespace linguistics {

class InterjectionalChunker {
public:
    explicit InterjectionalChunker(const AlgorithmSetForALanguage& pConfiguration);

    void process(std::list<ChunkLink>& pFirstChildren) const;

private:
    const AlgorithmSetForALanguage& fConf;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_INTERJECTIONALCHUNKER_HPP
