#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlengthgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticunitygrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
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
      for (const auto& currLength : semanticLengthUnities)
      {
        if (ConceptSet::haveAConcept(iGram.infos.concepts, semanticLengthUnity_toConcept(currLength)))
        {
          TokIt itNextToken = getNextToken(pContext.chunk.head, pContext.chunk.tokRange.getItEnd());
          if (itNextToken != pContext.chunk.tokRange.getItEnd())
            break; // the length should be the last token

          std::unique_ptr<SemanticLengthGrounding> newLength;
          for (TokIt itToken = getPrevToken(pContext.chunk.head, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head);
               itToken != pContext.chunk.head;
               itToken = getPrevToken(itToken, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head))
          {
            SemanticFloat number;
            if (getNumberHoldByTheInflWord(number, itToken, pContext.chunk.head, "number_"))
            {
              if (!newLength)
                newLength = std::make_unique<SemanticLengthGrounding>();
              newLength->length.lengthInfos[currLength] = number;
            }
            else if (itToken->getPartOfSpeech() == PartOfSpeech::DETERMINER)
            {
              if (newLength)
                return mystd::unique_propagate_const<UniqueSemanticExpression>
                    (std::make_unique<GroundedExpression>(std::move(newLength)));
              return mystd::unique_propagate_const<UniqueSemanticExpression>();
            }
          }

          if (newLength)
            return mystd::unique_propagate_const<UniqueSemanticExpression>
                (std::make_unique<GroundedExpression>(std::move(newLength)));
          return mystd::unique_propagate_const<UniqueSemanticExpression>
              (std::make_unique<GroundedExpression>(std::make_unique<SemanticUnityGrounding>(currLength)));
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
