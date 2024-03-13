#ifndef ONSEM_TEXTTOSEMANTIC_LINGUISTICANALYZER_HPP
#define ONSEM_TEXTTOSEMANTIC_LINGUISTICANALYZER_HPP

#include <string>
#include <memory>
#include <vector>
#include <onsem/texttosemantic/type/debug/synthanalendingstepfordebug.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>
#include "api.hpp"

namespace onsem {
struct TextProcessingContext;
struct UniqueSemanticExpression;
struct ResourceGroundingExtractor;
struct SemanticTimeGrounding;
struct SemanticAgentGrounding;
namespace linguistics {
struct SyntacticGraph;
struct LinguisticDatabase;

ONSEM_TEXTTOSEMANTIC_API
void tokenizationAndSyntacticalAnalysis(SyntacticGraph& pSyntGraph,
                                        const std::string& pInputSentence,
                                        const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                                        const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr);

ONSEM_TEXTTOSEMANTIC_API
void tokenizeText(TokensTree& pTokensTree,
                  const AlgorithmSetForALanguage& pLangConfig,
                  const std::string& pInputSentence,
                  const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr);

ONSEM_TEXTTOSEMANTIC_API
void syntacticAnalysis(SyntacticGraph& pSyntGraph,
                       const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                       const SynthAnalEndingStepForDebug& pEndingStep);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression convertToSemExp(const SyntacticGraph& pSyntGraph,
                                         const TextProcessingContext& pLocutionContext,
                                         const SemanticTimeGrounding& pTimeGrd,
                                         std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout);

ONSEM_TEXTTOSEMANTIC_API
void extractProperNounsThatDoesntHaveAnyOtherGrammaticalTypes(std::set<std::string>& pNewProperNouns,
                                                              const std::string& pInputSentence,
                                                              SemanticLanguageEnum pLanguage,
                                                              const LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
void extractProperNouns(std::vector<std::string>& pProperNouns,
                        const std::string& pInputSentence,
                        SemanticLanguageEnum pLanguage,
                        const LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
bool isAProperNoun(const std::string& pStr, const LinguisticDatabase& pLingDb);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_LINGUISTICANALYZER_HPP
