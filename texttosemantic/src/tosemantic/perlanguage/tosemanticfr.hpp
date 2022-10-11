#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOSEMANTIC_PERLANGUAGE_TOSEMANTICFR_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOSEMANTIC_PERLANGUAGE_TOSEMANTICFR_HPP

#include "../syntacticgraphtosemantic.hpp"

namespace onsem
{
namespace linguistics
{


class ToSemanticFr : public SyntacticGraphToSemantic
{
public:
  ToSemanticFr(const AlgorithmSetForALanguage& pConfiguration)
    : SyntacticGraphToSemantic(pConfiguration)
  {}

protected:
mystd::unique_propagate_const<UniqueSemanticExpression> _processQuestionWithoutVerb(
      std::list<ChunkLink>::const_iterator& pItChild,
      ToGenRepGeneral& pGeneral,
      const Chunk& pChunk,
      const SyntacticGraph& pSyntGraph) const override;
};


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOSEMANTIC_PERLANGUAGE_TOSEMANTICFR_HPP
