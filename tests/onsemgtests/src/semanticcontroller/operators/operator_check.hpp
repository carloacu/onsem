#ifndef ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_CHECK_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_CHECK_HPP

#include <string>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;


TruenessValue operator_check(const std::string& pText,
                             const SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb,
                             const TextProcessingContext& pTextProcContext = TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                                                                   SemanticAgentGrounding::me,
                                                                                                   SemanticLanguageEnum::UNKNOWN));

} // End of namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLLER_OPERATORS_OPERATOR_CHECK_HPP
