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
struct ExpressionHandleInMemory;
struct SemanticContextAxiom;
struct TextProcessingContext;

std::shared_ptr<ExpressionHandleInMemory> operator_inform(const std::string& pText,
                                                            SemanticMemory& pSemanticMemory,
                                                            const linguistics::LinguisticDatabase& pLingDb,
                                                            const std::list<std::string>& pReferences = std::list<std::string>(),
                                                            std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr,
                                                            const TextProcessingContext* pTextProcContextPtr = nullptr);

std::shared_ptr<ExpressionHandleInMemory> operator_inform_fromRobot(const std::string& pText,
                                                                      SemanticMemory& pSemanticMemory,
                                                                      const linguistics::LinguisticDatabase& pLingDb,
                                                                      const std::list<std::string>& pReferences = std::list<std::string>(),
                                                                      std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);

void operator_mergeAndInform(const std::string& pText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb,
                             const std::list<std::string>& pReferences = std::list<std::string>());

std::shared_ptr<ExpressionHandleInMemory> operator_informAxiom(const std::string& pText,
                                                            SemanticMemory& pSemanticMemory,
                                                            const linguistics::LinguisticDatabase& pLingDb,
                                                            const std::list<std::string>& pReferences = std::list<std::string>());

std::shared_ptr<ExpressionHandleInMemory>operator_informAxiom_fromRobot(const std::string& pText,
                                                                     SemanticMemory& pSemanticMemory,
                                                                     const linguistics::LinguisticDatabase& pLingDb,
                                                                     const std::list<std::string>& pReferences = std::list<std::string>(),
                                                                     std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);

std::shared_ptr<ExpressionHandleInMemory> operator_addFallback(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb);

std::shared_ptr<ExpressionHandleInMemory> operator_inform_withAgentNameFilter(
    const std::string& pAgentName,
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    SemanticLanguageEnum pLanguage,
    const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_INFORM_HPP
