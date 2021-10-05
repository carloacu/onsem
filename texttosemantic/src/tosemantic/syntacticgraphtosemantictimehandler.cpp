#include "syntacticgraphtosemantic.hpp"
#include <onsem/common/utility/number.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>


namespace onsem
{
namespace linguistics
{

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
      if (ConceptSet::haveAConcept(iGram.infos.concepts, "duration_millisecond"))
      {
        int number = 0;
        if (xGetNumberBeforeHead(number, pContext.chunk))
        {
          auto newDuration = mystd::make_unique<SemanticDurationGrounding>();
          newDuration->duration.sign = SemanticDurationSign::POSITIVE;
          newDuration->duration.timeInfos[SemanticTimeUnity::MILLISECOND] = number;
          return mystd::unique_propagate_const<UniqueSemanticExpression>
              (mystd::make_unique<GroundedExpression>(std::move(newDuration)));
        }
      }
      if (ConceptSet::haveAConcept(iGram.infos.concepts, "duration_second"))
      {
        int number = 0;
        if (xGetNumberBeforeHead(number, pContext.chunk))
        {
          auto newDuration = mystd::make_unique<SemanticDurationGrounding>();
          newDuration->duration.sign = SemanticDurationSign::POSITIVE;
          newDuration->duration.timeInfos[SemanticTimeUnity::SECOND] = number;
          return mystd::unique_propagate_const<UniqueSemanticExpression>
              (mystd::make_unique<GroundedExpression>(std::move(newDuration)));
        }
      }
      else if (ConceptSet::haveAConcept(iGram.infos.concepts, "duration_minute"))
      {
        int number = 0;
        if (xGetNumberBeforeHead(number, pContext.chunk))
        {
          auto newDuration = mystd::make_unique<SemanticDurationGrounding>();
          newDuration->duration.sign = SemanticDurationSign::POSITIVE;
          newDuration->duration.timeInfos[SemanticTimeUnity::MINUTE] = number;
          return mystd::unique_propagate_const<UniqueSemanticExpression>
              (mystd::make_unique<GroundedExpression>(std::move(newDuration)));
        }
      }
      else if (ConceptSet::haveAConcept(iGram.infos.concepts, "duration_hour"))
      {
        int number = 0;
        if (xGetNumberBeforeHead(number, pContext.chunk))
        {
          auto newDuration = mystd::make_unique<SemanticDurationGrounding>();
          newDuration->duration.sign = SemanticDurationSign::POSITIVE;
          newDuration->duration.timeInfos[SemanticTimeUnity::HOUR] = number;
          return mystd::unique_propagate_const<UniqueSemanticExpression>
              (mystd::make_unique<GroundedExpression>(std::move(newDuration)));
        }
      }
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


bool SyntacticGraphToSemantic::xGetNumberBeforeHead
(int& pNumber,
 const Chunk& pChunk) const
{
  for (TokIt itToken = getPrevToken(pChunk.head, pChunk.tokRange.getItBegin(), pChunk.head);
       itToken != pChunk.head;
       itToken = getPrevToken(itToken, pChunk.tokRange.getItBegin(), pChunk.head))
    if (getNumberHoldByTheInflWord(pNumber, itToken, pChunk.head, "number_"))
      return true;
  return false;
}



} // End of namespace linguistics
} // End of namespace onsem
