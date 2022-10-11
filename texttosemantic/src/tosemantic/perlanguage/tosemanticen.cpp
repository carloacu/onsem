#include "tosemanticen.hpp"
#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../../tool/semexpgenerator.hpp"
#include "../syntacticgraphtosemantic.hpp"


namespace onsem
{
namespace linguistics
{

mystd::unique_propagate_const<UniqueSemanticExpression> ToSemanticEn::_processQuestionWithoutVerb(
    std::list<ChunkLink>::const_iterator& pItChild,
    ToGenRepGeneral& pGeneral,
    const Chunk& pChunk,
    const SyntacticGraph& pSyntGraph) const
{
  mystd::unique_propagate_const<UniqueSemanticExpression> semExp;
  if (pChunk.type == ChunkType::NOMINAL_CHUNK)
  {
    const auto& lemmaStr = pChunk.head->inflWords.front().word.lemma;
    if (lemmaStr == "what about")
    {
      auto itNext = pItChild;
      ++itNext;
      if (itNext != pSyntGraph.firstChildren.end() &&
          itNext->chunk->type == ChunkType::NOMINAL_CHUNK)
      {
        ToGenRepContext nextContext(*itNext);
        auto nextSemExp = xFillSemExp(pGeneral, nextContext);
        if (nextSemExp)
        {
          semExp = mystd::unique_propagate_const<UniqueSemanticExpression>(SemExpGenerator::whatAbout(std::move(*nextSemExp)));
          pItChild = itNext;
        }
      }
    }
  }
  return semExp;
}


} // End of namespace linguistics
} // End of namespace onsem
