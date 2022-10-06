#include "syntacticgraphtosemantic.hpp"
#include <onsem/common/utility/number.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{
bool _createDurationGrd(mystd::unique_propagate_const<UniqueSemanticExpression>& pRes,
                        const SemanticTimeUnity& pTimeUnity,
                        const std::string& pConceptName,
                        const InflectedWord& pIGram,
                        const Chunk& pChunk)
{
  if (ConceptSet::haveAConcept(pIGram.infos.concepts, pConceptName))
  {
    int number = 0;
    if (getNumberBeforeHead(number, pChunk))
    {
      auto newDuration = mystd::make_unique<SemanticDurationGrounding>();
      newDuration->duration.sign = SemanticDurationSign::POSITIVE;
      newDuration->duration.timeInfos[pTimeUnity] = number;
      pRes = mystd::unique_propagate_const<UniqueSemanticExpression>
          (mystd::make_unique<GroundedExpression>(std::move(newDuration)));
      return true;
    }
  }
  return false;
}
}


mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xFillTimeStruct
(const ToGenRepContext& pContext) const
{
  switch (pContext.chunk.type)
  {
  case ChunkType::NOMINAL_CHUNK:
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
    auto dateOpt = extractDate(pContext.chunk.head, pContext.chunk.tokRange);
    if (dateOpt)
    {
      auto newTime = mystd::make_unique<SemanticTimeGrounding>();
      newTime->date = std::move(*dateOpt);
      return mystd::unique_propagate_const<UniqueSemanticExpression>
          (mystd::make_unique<GroundedExpression>(std::move(newTime)));
    }

    const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
    if (ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "duration_"))
    {
      mystd::unique_propagate_const<UniqueSemanticExpression> res;
      if (_createDurationGrd(res, SemanticTimeUnity::MILLISECOND, "duration_millisecond", iGram, pContext.chunk) ||
          _createDurationGrd(res, SemanticTimeUnity::SECOND, "duration_second", iGram, pContext.chunk) ||
          _createDurationGrd(res, SemanticTimeUnity::MINUTE, "duration_minute", iGram, pContext.chunk) ||
          _createDurationGrd(res, SemanticTimeUnity::HOUR, "duration_hour", iGram, pContext.chunk))
        return res;
    }
    else if (pContext.grammTypeFromParent == GrammaticalType::TIME &&
             ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "number_"))
    {
      int year = 0;
      if (getNumberHoldByTheInflWord(year, pContext.chunk.tokRange.getItBegin(), pContext.chunk.tokRange.getItEnd(), "number_") &&
          hasNotMoreThanANumberOfDigits(year, 4))
      {
        auto newTime = mystd::make_unique<SemanticTimeGrounding>();
        newTime->date.year.emplace(year);
        return mystd::unique_propagate_const<UniqueSemanticExpression>
            (mystd::make_unique<GroundedExpression>(std::move(newTime)));
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
