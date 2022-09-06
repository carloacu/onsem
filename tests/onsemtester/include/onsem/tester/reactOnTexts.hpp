#ifndef ONSEM_TESTER_REACTONTEXTS_HPP
#define ONSEM_TESTER_REACTONTEXTS_HPP

#include <list>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include "api.hpp"
#include "detailedreactionanswer.hpp"


namespace onsem
{
struct SemanticMemory;
struct UniqueSemanticExpression;
struct TextProcessingContext;
class ScenarioContainer;
struct ReactionOptions;
namespace linguistics
{
struct LinguisticDatabase;
}

ONSEMTESTER_API
DetailedReactionAnswer reactionToAnswer(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                                        SemanticMemory& pSemanticMemory,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        SemanticLanguageEnum pLanguage);

ONSEMTESTER_API
DetailedReactionAnswer operator_react_fromSemExp(UniqueSemanticExpression pSemExp,
                                                 SemanticMemory& pSemanticMemory,
                                                 const linguistics::LinguisticDatabase& pLingDb,
                                                 SemanticLanguageEnum pTextLanguage,
                                                 const ReactionOptions* pReactionOptions = nullptr);

ONSEMTESTER_API
DetailedReactionAnswer operator_react(const std::string& pText,
                                      SemanticMemory& pSemanticMemory,
                                      const linguistics::LinguisticDatabase& pLingDb,
                                      SemanticLanguageEnum pTextLanguage = SemanticLanguageEnum::UNKNOWN,
                                      const ReactionOptions* pReactionOptions = nullptr);

ONSEMTESTER_API
bool test_knowTheNameOf(const std::string& pName,
                        SemanticMemory& pSemanticMemory,
                        const linguistics::LinguisticDatabase& pLingDb);


ONSEMTESTER_API
void getResultOfAScenario(
    std::list<std::string>& pResult,
    const std::string& pFilename,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb);



ONSEMTESTER_API
void loadOneFileInSemanticMemory(std::size_t& pNbOfInforms,
                                 const std::string& pFilename,
                                 SemanticMemory& pSemanticMemory,
                                 linguistics::LinguisticDatabase& pLingDb,
                                 bool pAddReferences,
                                 const std::string* pPathToWriteTextReplaced = nullptr);


} // End of namespace onsem

#endif // ONSEM_TESTER_REACTONTEXTS_HPP
