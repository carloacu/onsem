#include "syntacticgraphtosemantic.hpp"
#include <onsem/common/utility/number.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
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

std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::xFillTimeStruct
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
      auto newTime = std::make_unique<SemanticTimeGrounding>();
      newTime->date = std::move(*dateOpt);
      return std::make_unique<GroundedExpression>(std::move(newTime));
    }

    const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
    if (ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "duration_"))
    {
      mystd::unique_propagate_const<UniqueSemanticExpression> res;
      for (auto& currTimeUnity : semanticTimeUnities)
      {
        if (ConceptSet::haveAConcept(iGram.infos.concepts, semanticTimeUnity_toConcept(currTimeUnity)))
        {
          SemanticFloat number;
          if (getNumberBeforeHead(number, pContext.chunk))
          {
            auto newDuration = std::make_unique<SemanticDurationGrounding>();
            newDuration->duration.sign = Sign::POSITIVE;
            newDuration->duration.timeInfos[currTimeUnity] = number;
            return std::make_unique<GroundedExpression>(std::move(newDuration));
          }
        }
      }
    }
    else if (pContext.grammTypeFromParent == GrammaticalType::TIME &&
             ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "number_"))
    {
      SemanticFloat year;
      if (getNumberHoldByTheInflWord(year, pContext.chunk.tokRange.getItBegin(), pContext.chunk.tokRange.getItEnd(), "number_") &&
          year.isPositive() && year.isAnInteger() &&
          hasNotMoreThanANumberOfDigits(year.value, 4))
      {
        auto newTime = std::make_unique<SemanticTimeGrounding>();
        newTime->date.year.emplace(year.value);
        return std::make_unique<GroundedExpression>(std::move(newTime));
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
