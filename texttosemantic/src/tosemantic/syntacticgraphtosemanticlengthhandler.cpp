#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlengthgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{


mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xFillLengthStruct
(const ToGenRepContext& pContext) const
{
  switch (pContext.chunk.type)
  {
  case ChunkType::NOMINAL_CHUNK:
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
    const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
    if (ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "length_"))
    {
      mystd::unique_propagate_const<UniqueSemanticExpression> res;
      for (const auto& currLength : semanticLengthUnities)
      {
        if (ConceptSet::haveAConcept(iGram.infos.concepts, semanticLengthUnity_toConcept(currLength)))
        {
          int number = 0;
          if (getNumberBeforeHead(number, pContext.chunk))
          {
            auto newLength = mystd::make_unique<SemanticLengthGrounding>();
            newLength->length.lengthInfos[currLength] = number;
            res = mystd::unique_propagate_const<UniqueSemanticExpression>
                (mystd::make_unique<GroundedExpression>(std::move(newLength)));
            return res;
          }
        }
      }
    }
    break;
  }
  default:
  {
    break;
  }
  }
  return mystd::unique_propagate_const<UniqueSemanticExpression>();
}


} // End of namespace linguistics
} // End of namespace onsem
