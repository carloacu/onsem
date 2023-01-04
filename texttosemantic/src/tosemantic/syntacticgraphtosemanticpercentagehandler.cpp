#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticpercentagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>

namespace onsem
{
namespace linguistics
{


mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xFillPercentageStruct
(const ToGenRepContext& pContext) const
{
  switch (pContext.chunk.type)
  {
  case ChunkType::NOMINAL_CHUNK:
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
    const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
    if (ConceptSet::haveAConcept(iGram.infos.concepts, "percent"))
    {
      std::unique_ptr<SemanticPercentageGrounding> newPercentage;
      for (TokIt itToken = getPrevToken(pContext.chunk.head, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head);
           itToken != pContext.chunk.head;
           itToken = getPrevToken(itToken, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head))
      {
        SemanticFloat number;
        if (getNumberHoldByTheInflWord(number, itToken, pContext.chunk.head, "number_"))
        {
          if (!newPercentage)
            newPercentage = std::make_unique<SemanticPercentageGrounding>();
          newPercentage->value = number;
        }
        else if (itToken->getPartOfSpeech() == PartOfSpeech::DETERMINER)
        {
          if (newPercentage)
            return mystd::unique_propagate_const<UniqueSemanticExpression>
                (std::make_unique<GroundedExpression>(std::move(newPercentage)));
          return mystd::unique_propagate_const<UniqueSemanticExpression>();
        }
      }

      if (newPercentage)
        return mystd::unique_propagate_const<UniqueSemanticExpression>
            (std::make_unique<GroundedExpression>(std::move(newPercentage)));
      return mystd::unique_propagate_const<UniqueSemanticExpression>();
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
