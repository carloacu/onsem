#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticdistancegrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{
bool _createDistanceGrd(mystd::unique_propagate_const<UniqueSemanticExpression>& pRes,
                        const SemanticDistanceUnity& pDistanceUnity,
                        const std::string& pConceptName,
                        const InflectedWord& pIGram,
                        const Chunk& pChunk)
{
  if (ConceptSet::haveAConcept(pIGram.infos.concepts, pConceptName))
  {
    int number = 0;
    if (getNumberBeforeHead(number, pChunk))
    {
      auto newDistance = mystd::make_unique<SemanticDistanceGrounding>();
      newDistance->distance.distanceInfos[pDistanceUnity] = number;
      pRes = mystd::unique_propagate_const<UniqueSemanticExpression>
          (mystd::make_unique<GroundedExpression>(std::move(newDistance)));
      return true;
    }
  }
  return false;
}
}


mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xFillDistanceStruct
(const ToGenRepContext& pContext) const
{
  switch (pContext.chunk.type)
  {
  case ChunkType::NOMINAL_CHUNK:
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
    const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
    if (ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "distance_"))
    {
      mystd::unique_propagate_const<UniqueSemanticExpression> res;
      if (_createDistanceGrd(res, SemanticDistanceUnity::MILLIMETER, "distance_millimeter", iGram, pContext.chunk) ||
          _createDistanceGrd(res, SemanticDistanceUnity::CENTIMETER, "distance_centimeter", iGram, pContext.chunk) ||
          _createDistanceGrd(res, SemanticDistanceUnity::DECIMETER, "distance_decimeter", iGram, pContext.chunk) ||
          _createDistanceGrd(res, SemanticDistanceUnity::METER, "distance_meter", iGram, pContext.chunk) ||
          _createDistanceGrd(res, SemanticDistanceUnity::DECAMETER, "distance_decameter", iGram, pContext.chunk) ||
          _createDistanceGrd(res, SemanticDistanceUnity::HECTOMETER, "distance_hectometer", iGram, pContext.chunk) ||
          _createDistanceGrd(res, SemanticDistanceUnity::KILOMETER, "distance_kilometer", iGram, pContext.chunk))
        return res;
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
