#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>

namespace onsem
{
namespace linguistics
{
enum class BeforeOrAfter
{
  BEFORE,
  AFTER,
  UNKNOWN
};
enum class AmOrPm
{
  AM,
  PM,
  UNKNOWN
};

mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xFillHourTimeStruct
(const ToGenRepContext& pContext) const
{
  if (fConfiguration.getLanguageType() == SemanticLanguageEnum::ENGLISH)
  {
    auto itToken = pContext.chunk.tokRange.getItBegin();
    auto endIt = pContext.chunk.tokRange.getItEnd();

    mystd::optional<int> number;
    itToken = eatNumber(number, itToken, endIt, "number_");

    if (!number)
    {
      const auto& tokenInfos = itToken->inflWords.front().infos;
      if (tokenInfos.hasContextualInfo(WordContextualInfos::EN_TIMEWORD))
      {
        if (tokenInfos.hasContextualInfo(WordContextualInfos::EN_TIMEWORDHALF))
          number.emplace(30);
        else if (tokenInfos.hasContextualInfo(WordContextualInfos::EN_TIMEWORDQUARTER))
          number.emplace(15);
      }
      if (!number)
        return {};
    }

    if (itToken != endIt)
      itToken = getNextToken(itToken, endIt);

    if (itToken != endIt)
    {
      // skip minute word
      const auto& tokenInfos = itToken->inflWords.front().infos;
      if (ConceptSet::haveAConcept(tokenInfos.concepts, "duration_minute"))
        itToken = getNextToken(itToken, endIt);
    }

    if (itToken != endIt)
    {
      BeforeOrAfter beforeOrAfter = BeforeOrAfter::UNKNOWN;
      {
        const auto& tokenInfos = itToken->inflWords.front().infos;
        if (tokenInfos.hasContextualInfo(WordContextualInfos::EN_TIMEWORD))
        {
          if (tokenInfos.hasContextualInfo(WordContextualInfos::EN_TIMEWORDPAST))
            beforeOrAfter = BeforeOrAfter::AFTER;
          else if (tokenInfos.hasContextualInfo(WordContextualInfos::EN_TIMEWORDTO))
            beforeOrAfter = BeforeOrAfter::BEFORE;
        }
      }
      if (beforeOrAfter != BeforeOrAfter::UNKNOWN)
      {
        itToken = getNextToken(itToken, endIt);
        if (itToken != endIt)
        {
          mystd::optional<int> hourNumber;
          itToken = eatNumber(hourNumber, itToken, endIt, "number_");
          if (hourNumber && *hourNumber < 24 && *number < 60)
          {
            if (*hourNumber < 12)
            {
              AmOrPm amOrPm = AmOrPm::UNKNOWN;
              if (itToken != endIt)
                itToken = getNextToken(itToken, endIt);
              if (itToken != endIt)
              {
                const auto& tokenInfos = itToken->inflWords.front().infos;
                if (ConceptSet::haveAConceptThatBeginWith(tokenInfos.concepts, "time_hour_"))
                {
                  if (ConceptSet::haveAConcept(tokenInfos.concepts, "time_hour_am"))
                    amOrPm = AmOrPm::AM;
                  else if (ConceptSet::haveAConcept(tokenInfos.concepts, "time_hour_pm"))
                    amOrPm = AmOrPm::PM;
                }
              }
              if (amOrPm == AmOrPm::UNKNOWN)
              {
                auto nowTimeGrd = SemanticTimeGrounding::now();
                auto itHour = nowTimeGrd.timeOfDay.timeInfos.find(SemanticTimeUnity::HOUR);
                if (itHour != nowTimeGrd.timeOfDay.timeInfos.end())
                {
                  if (itHour->second < *hourNumber)
                    amOrPm = AmOrPm::AM;
                  else
                    amOrPm = AmOrPm::PM;
                }
              }
              if (amOrPm == AmOrPm::PM)
                *hourNumber += 12;
            }

            if (beforeOrAfter == BeforeOrAfter::BEFORE && *hourNumber > 0)
            {
              --*hourNumber;
              *number = 60 - *number;
            }
            auto time = std::make_unique<SemanticTimeGrounding>();
            time->timeOfDay.sign = SemanticDurationSign::POSITIVE;
            time->timeOfDay.timeInfos[SemanticTimeUnity::HOUR] = *hourNumber;
            time->timeOfDay.timeInfos[SemanticTimeUnity::MINUTE] = *number;
            return mystd::unique_propagate_const<UniqueSemanticExpression>
                (std::make_unique<GroundedExpression>(std::move(time)));
          }
        }
      }
    }
  }
  return {};
}




} // End of namespace linguistics
} // End of namespace onsem
