#include "syntacticgraphtosemantic.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticquantity.hpp>
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


std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::_fillHourTimeStructEn
(const ToGenRepContext& pContext) const
{
  auto itToken = pContext.chunk.tokRange.getItBegin();
  auto endIt = pContext.chunk.tokRange.getItEnd();

  mystd::optional<SemanticFloat> number;
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
        mystd::optional<SemanticFloat> hourNumber;
        itToken = eatNumber(hourNumber, itToken, endIt, "number_");
        if (hourNumber && hourNumber->isAnInteger() && *hourNumber < 24 && *number < 60)
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
                if (itHour->second < hourNumber->signedValueWithoutDecimal())
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
            --hourNumber->value;
            *number = 60 - number->value;
          }
          auto time = std::make_unique<SemanticTimeGrounding>();
          time->timeOfDay.sign = Sign::POSITIVE;
          time->timeOfDay.timeInfos[SemanticTimeUnity::HOUR] = hourNumber->value;
          time->timeOfDay.timeInfos[SemanticTimeUnity::MINUTE] = number->value;
          return std::make_unique<GroundedExpression>(std::move(time));
        }
      }
    }
  }
  return {};
}


std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::_fillHourTimeStructFr
(const ToGenRepContext& pContext) const
{
  auto itToken = pContext.chunk.tokRange.getItBegin();
  auto endIt = pContext.chunk.tokRange.getItEnd();

  if (itToken == endIt)
    return {};

  auto pos = itToken->getPartOfSpeech();
  if (pos == PartOfSpeech::PREPOSITION)
  {
    itToken = getNextToken(itToken, endIt, PartOfSpeech::PREPOSITION);
    if (itToken == endIt)
      return {};
  }

  std::size_t separatorOfHourMinute = getSeparatorOfHourMinute(itToken->str);
  if (separatorOfHourMinute != std::string::npos)
  {
    try {
      auto time = std::make_unique<SemanticTimeGrounding>();
      time->timeOfDay.sign = Sign::POSITIVE;
      time->timeOfDay.timeInfos[SemanticTimeUnity::HOUR] = mystd::lexical_cast<int>(itToken->str.substr(0, separatorOfHourMinute));
      std::size_t beginOfMinutePos = separatorOfHourMinute + 1;
      if (itToken->str.size() > beginOfMinutePos)
      {
        int minuteStrSize = itToken->str.size() - beginOfMinutePos;
        time->timeOfDay.timeInfos[SemanticTimeUnity::MINUTE] = mystd::lexical_cast<int>(itToken->str.substr(beginOfMinutePos, minuteStrSize));
      }
      return std::make_unique<GroundedExpression>(std::move(time));
    }  catch (...) {}
  }
  return {};
}


std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::xFillHourTimeStruct
(const ToGenRepContext& pContext) const
{
  auto language = fConfiguration.getLanguageType();
  std::unique_ptr<GroundedExpression> res;

  if (language == SemanticLanguageEnum::ENGLISH || language == SemanticLanguageEnum::UNKNOWN)
    res = _fillHourTimeStructEn(pContext);

  if (!res && (language == SemanticLanguageEnum::FRENCH || language == SemanticLanguageEnum::UNKNOWN))
    res = _fillHourTimeStructFr(pContext);

  return res;
}


} // End of namespace linguistics
} // End of namespace onsem
