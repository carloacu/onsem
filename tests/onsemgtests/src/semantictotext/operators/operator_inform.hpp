#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_INFORM_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_INFORM_HPP

#include <string>
#include <list>
#include <memory>
#include <map>
#include <set>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;
struct ExpressionWithLinks;
struct SentenceWithLinks;
struct TextProcessingContext;
struct SemanticAgentGrounding;


std::shared_ptr<ExpressionWithLinks> operator_inform_withTextProc(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::list<std::string>& pReferences,
    std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
    const TextProcessingContext& pTextProcContext,
    std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>());

std::shared_ptr<ExpressionWithLinks> operator_inform(const std::string& pText,
                                                     SemanticMemory& pSemanticMemory,
                                                     const linguistics::LinguisticDatabase& pLingDb,
                                                     const std::list<std::string>& pReferences = std::list<std::string>(),
                                                     std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);

std::shared_ptr<ExpressionWithLinks> operator_inform_fromRobot(const std::string& pText,
                                                                      SemanticMemory& pSemanticMemory,
                                                                      const linguistics::LinguisticDatabase& pLingDb,
                                                                      const std::list<std::string>& pReferences = std::list<std::string>(),
                                                                      std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);

void operator_mergeAndInform(const std::string& pText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb,
                             const std::list<std::string>& pReferences = std::list<std::string>());

std::shared_ptr<ExpressionWithLinks> operator_informAxiom(const std::string& pText,
                                                            SemanticMemory& pSemanticMemory,
                                                            const linguistics::LinguisticDatabase& pLingDb,
                                                            const std::list<std::string>& pReferences = std::list<std::string>());

std::shared_ptr<ExpressionWithLinks>operator_informAxiom_fromRobot(const std::string& pText,
                                                                     SemanticMemory& pSemanticMemory,
                                                                     const linguistics::LinguisticDatabase& pLingDb,
                                                                     const std::list<std::string>& pReferences = std::list<std::string>(),
                                                                     std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);

std::shared_ptr<ExpressionWithLinks> operator_addFallback(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb);

std::shared_ptr<ExpressionWithLinks> operator_inform_withAgentNameFilter(
    const std::string& pAgentName,
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    SemanticLanguageEnum pLanguage,
    const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_INFORM_HPP
