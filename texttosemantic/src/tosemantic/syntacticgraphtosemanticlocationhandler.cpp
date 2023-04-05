#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticanglegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticunitygrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{


std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::xFillLocationStruct
(const ToGenRepContext& pContext) const
{
  switch (pContext.chunk.type)
  {
  case ChunkType::NOMINAL_CHUNK:
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
    const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
    if (ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "location_relative_angle_"))
    {
      for (const auto& currAngle : semanticAngleUnities)
      {
        if (ConceptSet::haveAConcept(iGram.infos.concepts, semanticAngleUnity_toConcept(currAngle)))
        {
          std::unique_ptr<SemanticAngleGrounding> newAngle;
          for (TokIt itToken = getPrevToken(pContext.chunk.head, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head);
               itToken != pContext.chunk.head;
               itToken = getPrevToken(itToken, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head))
          {
            SemanticFloat number;
            if (getNumberHoldByTheInflWord(number, itToken, pContext.chunk.head, "number_"))
            {
              if (!newAngle)
                newAngle = std::make_unique<SemanticAngleGrounding>();
              newAngle->angle.angleInfos[currAngle] = number;
            }
            else if (itToken->getPartOfSpeech() == PartOfSpeech::DETERMINER)
            {
              if (newAngle)
                return std::make_unique<GroundedExpression>(std::move(newAngle));
              return {};
            }
          }
          if (newAngle)
            return std::make_unique<GroundedExpression>(std::move(newAngle));
          return std::make_unique<GroundedExpression>(std::make_unique<SemanticUnityGrounding>(currAngle));
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
  return {};
}


} // End of namespace linguistics
} // End of namespace onsem
