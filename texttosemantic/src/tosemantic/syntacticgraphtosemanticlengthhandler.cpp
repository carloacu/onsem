#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlengthgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{
bool _createLengthGrd(mystd::unique_propagate_const<UniqueSemanticExpression>& pRes,
                        const SemanticLengthUnity& pLengthUnity,
                        const std::string& pConceptName,
                        const InflectedWord& pIGram,
                        const Chunk& pChunk)
{
  if (ConceptSet::haveAConcept(pIGram.infos.concepts, pConceptName))
  {
    int number = 0;
    if (getNumberBeforeHead(number, pChunk))
    {
      auto newLength = mystd::make_unique<SemanticLengthGrounding>();
      newLength->length.lengthInfos[pLengthUnity] = number;
      pRes = mystd::unique_propagate_const<UniqueSemanticExpression>
          (mystd::make_unique<GroundedExpression>(std::move(newLength)));
      return true;
    }
  }
  return false;
}
}


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
      if (_createLengthGrd(res, SemanticLengthUnity::MILLIMETER, "length_millimeter", iGram, pContext.chunk) ||
          _createLengthGrd(res, SemanticLengthUnity::CENTIMETER, "length_centimeter", iGram, pContext.chunk) ||
          _createLengthGrd(res, SemanticLengthUnity::DECIMETER, "length_decimeter", iGram, pContext.chunk) ||
          _createLengthGrd(res, SemanticLengthUnity::METER, "length_meter", iGram, pContext.chunk) ||
          _createLengthGrd(res, SemanticLengthUnity::DECAMETER, "length_decameter", iGram, pContext.chunk) ||
          _createLengthGrd(res, SemanticLengthUnity::HECTOMETER, "length_hectometer", iGram, pContext.chunk) ||
          _createLengthGrd(res, SemanticLengthUnity::KILOMETER, "length_kilometer", iGram, pContext.chunk))
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
