#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICCONVERTER_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICCONVERTER_HPP

#include <list>
#include <memory>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include "api.hpp"


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
struct SyntacticGraph;
}
struct TextProcessingContext;
struct SemanticMemory;
struct SemanticMemoryBlock;
struct SentimentContext;
struct SemLineToPrint;
class ConceptSet;
struct NaturalLanguageExpression;

namespace converter
{

ONSEMSEMANTICTOTEXT_API
void splitPossibilitiesOfQuestions(UniqueSemanticExpression& pSemExp,
                                   const linguistics::LinguisticDatabase& pLingDb,
                                   std::list<std::list<SemLineToPrint>>* pDebugOutput = nullptr);

ONSEMSEMANTICTOTEXT_API
void splitEquivalentQuestions(UniqueSemanticExpression& pSemExp,
                              const linguistics::LinguisticDatabase& pLingDb,
                              std::list<std::list<SemLineToPrint>>* pDebugOutput = nullptr);

ONSEMSEMANTICTOTEXT_API
void unsplitPossibilitiesOfQuestions(UniqueSemanticExpression& pSemExp);


ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression textToSemExp(const std::string& pText,
                                      const TextProcessingContext& pTextProcContext,
                                      const linguistics::LinguisticDatabase& pLingDb,
                                      bool pDoWeSplitQuestions = false,
                                      SemanticLanguageEnum* pExtractedLanguagePtr = nullptr,
                                      std::unique_ptr<SemanticTimeGrounding>* pNowTimePtr = nullptr,
                                      const std::list<std::string>* pReferencesPtr = nullptr,
                                      std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>());

ONSEMSEMANTICTOTEXT_API
std::unique_ptr<MetadataExpression> wrapSemExpWithContextualInfos(UniqueSemanticExpression pSemExp,
                                                                  const std::string& pText,
                                                                  const TextProcessingContext& pLocutionContext,
                                                                  SemanticSourceEnum pFrom,
                                                                  SemanticLanguageEnum pLanguage,
                                                                  std::unique_ptr<SemanticTimeGrounding> pNowTimeGrd,
                                                                  const std::list<std::string>* pReferencesPtr = nullptr);

ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression textToContextualSemExp(const std::string& pText,
                                                const TextProcessingContext& pLocutionContext,
                                                SemanticSourceEnum pFrom,
                                                const linguistics::LinguisticDatabase& pLingDb,
                                                const std::list<std::string>* pReferencesPtr = nullptr,
                                                std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>());

ONSEMSEMANTICTOTEXT_API
void semExpToSentiments(std::list<std::unique_ptr<SentimentContext>>& pSentInfos,
                        const SemanticExpression& pSemExp,
                        const ConceptSet& pConceptSet);

ONSEMSEMANTICTOTEXT_API
void semExpToText(std::string& pResStr,
                  UniqueSemanticExpression pSemExp,
                  const TextProcessingContext& pTextProcContext,
                  bool pOneLinePerSentence,
                  const SemanticMemoryBlock& pMemBlock,
                  const std::string& pCurrentUserId,
                  const linguistics::LinguisticDatabase& pLingDb,
                  std::list<std::list<SemLineToPrint> >* pDebugOutput);


ONSEMSEMANTICTOTEXT_API
void semExpToText(std::string& pRes,
                  UniqueSemanticExpression pSemExp,
                  const TextProcessingContext& pTextProcContext,
                  bool pOneLinePerSentence,
                  const SemanticMemory& pSemanticMemory,
                  const linguistics::LinguisticDatabase& pLingDb,
                  std::list<std::list<SemLineToPrint> >* pDebugOutput);



ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression naturalLanguageExpressionToSemanticExpression(
    const NaturalLanguageExpression& pNaturalLanguageExpression,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::vector<std::string>& pResourceLabels = std::vector<std::string>());



/**
 * @brief This function generates a semantic expression that do the correspondance between an id of an agent and his name.
 * This function takes "toto_id" and "Toto" and returns a semantic expression that corresponds to "@toto_id is Toto."
 * @param pAgentId Id of the agent.
 * @param pNames Names of the agent.
 * @return A semantic expression that associate the agent id to the names. (ex: "@toto_id is Toto.")
 */
ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression agentIdWithNameToSemExp(const std::string& pAgentId,
                                                 const std::vector<std::string>& pNames);



// Very advanced

ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression syntGraphToSemExp(const linguistics::SyntacticGraph& pSyntGraph,
                                           const TextProcessingContext& pLocutionContext,
                                           std::list<std::list<SemLineToPrint> >* pDebugOutput);





ONSEMSEMANTICTOTEXT_API
void getInfinitiveToTwoDifferentPossibleWayToAskForIt(UniqueSemanticExpression& pOut1,
                                                      UniqueSemanticExpression& pOut2,
                                                      UniqueSemanticExpression pUSemExp);

ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression getFutureIndicativeFromInfinitive(UniqueSemanticExpression pUSemExp);

} // End of namespace converter
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SEMANTICCONVERTER_HPP
