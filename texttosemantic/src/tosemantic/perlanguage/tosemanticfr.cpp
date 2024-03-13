#include "tosemanticfr.hpp"
#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../../tool/semexpgenerator.hpp"
#include "../syntacticgraphtosemantic.hpp"

namespace onsem {
namespace linguistics {

mystd::unique_propagate_const<UniqueSemanticExpression> ToSemanticFr::_processQuestionWithoutVerb(
    std::list<ChunkLink>::const_iterator& pItChild,
    ToGenRepGeneral& pGeneral,
    const Chunk& pChunk,
    const SyntacticGraph& pSyntGraph) const {
    mystd::unique_propagate_const<UniqueSemanticExpression> semExp;
    if (pChunk.type == ChunkType::NOMINAL_CHUNK) {
        const auto& lemmaStr = pChunk.head->inflWords.front().word.lemma;
        if (lemmaStr == "qu'est-ce que") {
            auto itNext = pItChild;
            ++itNext;
            if (itNext != pSyntGraph.firstChildren.end() && itNext->chunk->type == ChunkType::NOMINAL_CHUNK) {
                ToGenRepContext nextContext(*itNext);
                auto nextSemExp = xFillSemExp(pGeneral, nextContext);
                if (nextSemExp) {
                    semExp = mystd::unique_propagate_const<UniqueSemanticExpression>(
                        SemExpGenerator::whatIs(std::move(*nextSemExp), SemanticLanguageEnum::FRENCH));
                    pItChild = itNext;
                }
            }
        } else if (lemmaStr == "qu'en est-il") {
            auto itFirstChild = pChunk.children.begin();
            if (itFirstChild != pChunk.children.end() && itFirstChild->chunk->type == ChunkType::NOMINAL_CHUNK) {
                ToGenRepContext nextContext(*itFirstChild);
                auto nextSemExp = xFillSemExp(pGeneral, nextContext);
                if (nextSemExp) {
                    semExp = mystd::unique_propagate_const<UniqueSemanticExpression>(
                        SemExpGenerator::whatAbout(std::move(*nextSemExp)));
                }
            }
        }
    }
    return semExp;
}

}    // End of namespace linguistics
}    // End of namespace onsem
