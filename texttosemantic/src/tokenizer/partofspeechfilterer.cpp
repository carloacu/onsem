#include "partofspeechfilterer.hpp"
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcustomfilter.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include "../partofspeechpriorities/englishthendisambiguator.hpp"
#include "../partofspeechpriorities/nounadjpriorities.hpp"
#include "../tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{
namespace partOfSpeechFilterer
{

void process(std::vector<Token>& pTokens,
             const SpecificLinguisticDatabase& pSpecLingDb,
             const std::string& pEndingStep,
             std::size_t pNbOfDebugRounds,
             bool pIsRootLevel)
{
  const auto& contextFilters = pSpecLingDb.getContextFilters();
  if (contextFilters.empty())
    return;
  std::size_t idLastModif = contextFilters.size() - 1;

  bool keepInLoop = true;
  while (keepInLoop)
  {
    std::size_t currId = 0;
    for (const auto& currRulePtr : contextFilters)
    {
      const PartOfSpeechContextFilter& currRule = *currRulePtr;
      if (currRule.process(pTokens))
      {
        idLastModif = currId;
      }
      else if (idLastModif == currId)
      {
        keepInLoop = false;
        break;
      }
      if (pNbOfDebugRounds <= 1 &&
          currRule.getName() == pEndingStep)
      {
        keepInLoop = false;
        break;
      }
      currId++;
    }
    if (pNbOfDebugRounds > 1)
      --pNbOfDebugRounds;
  }

  const auto& inflsCheker =  pSpecLingDb.inflectionsChecker();
  // reoder after the filter
  switch (pSpecLingDb.language)
  {
  case SemanticLanguageEnum::ENGLISH:
  {
    PartOfSpeechCustomFilter genTagger
        ("noun at bottom if two times in a row exept for verb",
         pSpecLingDb.inflectionsChecker(), pSpecLingDb,
         "nounAtBottomIfTwoTimesInARowExeptForVerb");
    genTagger.process(pTokens);

    nounNounPrioritiesEn(pTokens, inflsCheker);
    nounDetPriorities(pTokens, inflsCheker);
    detNounPriorities(pTokens, inflsCheker);
    pronNounPriorities(pTokens, inflsCheker);
    nounAdjPriorities(pTokens);
    verbPriorities(pTokens, inflsCheker);

    // add concept "list_then" for "then" words that doesn't correspond to a conditional sentence
    englishThenDisambiguator(pTokens);
    putApproximatelyConceptInTopPrioritiesIfNecessary(pTokens);
    break;
  }
  case SemanticLanguageEnum::FRENCH:
  {
    // don't put auxiliaries at top possibility if other ones exist
    for (auto& currToken : pTokens)
    {
      if (currToken.inflWords.size() > 1 &&
          currToken.inflWords.front().word.partOfSpeech == PartOfSpeech::AUX)
        putOnBottom(currToken.inflWords,
                    currToken.inflWords.begin());
    }
    partitivePrioritiesFr(pTokens, inflsCheker);
    NounPrioritiesFr(pTokens, inflsCheker, pIsRootLevel);
    detNounPriorities(pTokens, inflsCheker);
    pronounPriorities(pTokens, inflsCheker);
    filterIncompatibleInflectionsInTokenList(pTokens, inflsCheker);
    verbPrioritiesFr(pTokens, inflsCheker);
    break;
  }
  default:
    break;
  }
}


} // End of namespace partOfSpeechFilterer
} // End of namespace linguistics
} // End of namespace onsem

