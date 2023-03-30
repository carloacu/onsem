#ifndef ONSEM_SEMANTCDEBUGGER_SEMANTICDEBUG_HPP
#define ONSEM_SEMANTCDEBUGGER_SEMANTICDEBUG_HPP

#include <string>
#include <list>
#include <map>
#include <memory>
#include <vector>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>
#include <onsem/texttosemantic/type/debug/synthanalendingstepfordebug.hpp>
#include <onsem/semanticdebugger/printer/semexplinestostr.hpp>
#include <onsem/semanticdebugger/timechecker.hpp>
#include "api.hpp"



namespace onsem
{
struct TextProcessingContext;
namespace linguistics
{
struct TokensTree;
class StaticLinguisticDictionary;
struct InflectedWord;
struct LinguisticDatabase;
}
struct SemanticMemory;
struct SyntacticGraphResult;


#define CONV_OUTPUT_TABLE                                             \
  ADD_CONV_OUTPUT(CONV_OUTPUT_MIND, "mind")                           \
  ADD_CONV_OUTPUT(CONV_OUTPUT_CURRLANG_TO_MIND, "currLang_to_mind")   \
  ADD_CONV_OUTPUT(CONV_OUTPUT_MIND_TO_FRENCH, "mind_to_french")       \
  ADD_CONV_OUTPUT(CONV_OUTPUT_MIND_TO_ENGLISH, "mind_to_english")     \
  ADD_CONV_OUTPUT(CONV_OUTPUT_UNKNOWN, "")


#define ADD_CONV_OUTPUT(a, b) a,
enum ConvertionOutputEnum
{
  CONV_OUTPUT_TABLE
  CONV_OUTPUT_TABLE_ENDFORNOCOMPILWARNING
};
#undef ADD_CONV_OUTPUT


#define ADD_CONV_OUTPUT(a, b) b,
static std::string ConvertionOutputEnum_toStr[] = {
  CONV_OUTPUT_TABLE
};
#undef ADD_CONV_OUTPUT


inline static void convertionOutputEnum_getAll
(std::list<ConvertionOutputEnum>& pValues)
{
  for (std::size_t i = 0; i < CONV_OUTPUT_UNKNOWN; ++i)
  {
    pValues.emplace_back(ConvertionOutputEnum(i));
  }
}


inline static ConvertionOutputEnum convertionOutputEnum_fromStr
(const std::string& pStr)
{
  std::size_t end = CONV_OUTPUT_UNKNOWN;
  for (std::size_t i = 0; i < end; ++i)
  {
    if (pStr == ConvertionOutputEnum_toStr[i])
    {
      return ConvertionOutputEnum(i);
    }
  }
  return CONV_OUTPUT_UNKNOWN;
}

struct ONSEMSEMANTICDEBUGGER_API SemanticAnalysisHighLevelResults
{
  std::list<std::list<std::string>> initialGramPossibilities{};
  std::string syntGraphStr{};
  std::string parsingConfidenceStr{};
  std::string semExpStr{};
  std::string allFormsStr{};
  std::string sentimentsInfos{};
  bool completeness{};
  std::string reformulations{};
  std::string reformulationInputLanguage{};
};


struct ONSEMSEMANTICDEBUGGER_API SyntacticAnalysisResultToDisplay
{
  SyntacticAnalysisResultToDisplay()
    : tokens(), finalGramPossibilities(), finalConcepts(), taggedTokens(),
      taggedTokensTagsPossibilities(), highLevelResults(), performances(),
      isReformulationOk(true)
  {
  }

  void saveConcepts(const linguistics::TokensTree& pTokensTree);
  void saveContextInfos(const linguistics::TokensTree& pTokensTree);

  std::list<std::pair<std::size_t, std::string> > tokens;
  std::list<std::list<std::string> > finalGramPossibilities;
  std::list<std::list<std::string> > finalConcepts;
  std::list<std::list<std::string> > contextualInfos;
  std::list<std::string> taggedTokens;
  std::list<std::list<std::string> > taggedTokensTagsPossibilities;
  SemanticAnalysisHighLevelResults highLevelResults;
  std::string performances;
  bool isReformulationOk;
};


struct SemanticAnalysisDebugOptions
{
  SemanticAnalysisDebugOptions()
    : endingStep(),
      outputFormat(PrintSemExpDiffsOutPutFormat::HTML),
      convOutput(ConvertionOutputEnum::CONV_OUTPUT_MIND),
      timeChecker(),
      setUsAsEverybody(false)
  {
  }

  linguistics::SynthAnalEndingStepForDebug endingStep;
  PrintSemExpDiffsOutPutFormat outputFormat;
  ConvertionOutputEnum convOutput;
  std::unique_ptr<TimeChecker> timeChecker;
  bool setUsAsEverybody;
};


namespace SemanticDebug
{



// debug
ONSEMSEMANTICDEBUGGER_API
void debugTextAnalyze(SyntacticAnalysisResultToDisplay& pAutoAnnotToDisplay,
                      const std::string& pSentence,
                      const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                      const SemanticAnalysisDebugOptions& pSemanticAnalysisDebugOptions,
                      SemanticLanguageEnum pLanguageType,
                      const linguistics::LinguisticDatabase& pLingDb,
                      const std::map<std::string, std::string>* pEquivalencesPtr = nullptr);



ONSEMSEMANTICDEBUGGER_API
void semAnalResultToStructToDisplay(SyntacticAnalysisResultToDisplay& pAutoAnnotToDisplay,
                                    const SyntacticGraphResult& pSemAnalResult);



} // End of namespace SemanticDebug
} // End of namespace onsem

#endif // ONSEM_SEMANTCDEBUGGER_SEMANTICDEBUG_HPP
