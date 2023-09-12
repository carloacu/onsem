#include "syntacticgraphtosemantic.hpp"
#include <onsem/common/utility/number.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticunitygrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{


std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::xFillDurationStruct
(const ToGenRepContext& pContext) const
{
  switch (pContext.chunk.type)
  {
  case ChunkType::NOMINAL_CHUNK:
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
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
          if (pContext.posFromParent != PartOfSpeech::DETERMINER && pContext.chunk.children.empty())
          {
            bool haveDeterminerBeforeHead = false;
            for (TokIt itToken = getPrevToken(pContext.chunk.head, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head);
                 itToken != pContext.chunk.head;
                 itToken = getPrevToken(itToken, pContext.chunk.tokRange.getItBegin(), pContext.chunk.head))
            {
              const InflectedWord& inflWord = itToken->inflWords.front();
              if (inflWord.word.partOfSpeech == PartOfSpeech::DETERMINER)
              {
                haveDeterminerBeforeHead = true;
                break;
              }
              if (inflWord.word.partOfSpeech == PartOfSpeech::PARTITIVE &&
                  ConceptSet::haveAConcept(inflWord.infos.concepts, "reference_definite"))
              {
                haveDeterminerBeforeHead = true;
                break;
              }
            }
            if (!haveDeterminerBeforeHead)
              return std::make_unique<GroundedExpression>(std::make_unique<SemanticUnityGrounding>(currTimeUnity));
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
  return {};
}


std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::xFillInterval
(const ToGenRepContext& pContext) const
{
  switch (pContext.chunk.type)
  {
  case ChunkType::NOMINAL_CHUNK:
  case ChunkType::PREPOSITIONAL_CHUNK:
  {
    const InflectedWord& iGram = pContext.chunk.head->inflWords.front();
    if (ConceptSet::haveAConceptThatBeginWith(iGram.infos.concepts, "duration_"))
    {
      mystd::unique_propagate_const<UniqueSemanticExpression> res;
      for (auto& currTimeUnity : semanticTimeUnities)
      {
        if (ConceptSet::haveAConcept(iGram.infos.concepts, semanticTimeUnity_toConcept(currTimeUnity)))
        {
          SemanticFloat number;
          if (!getNumberBeforeHead(number, pContext.chunk))
            number.set(1);
          auto newDuration = std::make_unique<SemanticDurationGrounding>();
          newDuration->duration.sign = Sign::POSITIVE;
          newDuration->duration.timeInfos[currTimeUnity] = number;
          return std::make_unique<GroundedExpression>(std::move(newDuration));
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

    const InflectedWord& headInflWord = pContext.chunk.head->inflWords.front();
    if (pContext.grammTypeFromParent == GrammaticalType::TIME)
    {
      if (ConceptSet::haveAConceptThatBeginWith(headInflWord.infos.concepts, "number_"))
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
      else if (InflectionsChecker::nounCanBePlural(headInflWord))
      {
        std::map<std::string, char> timeConcepts;
        ConceptSet::extractConceptsThatBeginWith(timeConcepts, headInflWord.infos.concepts, "time_weekday_");
        if (!timeConcepts.empty())
        {
          auto newTime = std::make_unique<SemanticTimeGrounding>();
          newTime->fromConcepts = std::move(timeConcepts);
          auto timeGrdExp = std::make_unique<GroundedExpression>(std::move(newTime));

          auto newInterval = std::make_unique<SemanticDurationGrounding>();
          newInterval->duration.sign = Sign::POSITIVE;
          newInterval->duration.timeInfos[SemanticTimeUnity::DAY] = 7;
          timeGrdExp->children.emplace(GrammaticalType::INTERVAL, std::make_unique<GroundedExpression>(std::move(newInterval)));
          return timeGrdExp;
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
