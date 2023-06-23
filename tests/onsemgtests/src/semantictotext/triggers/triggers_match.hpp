#ifndef ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_MATCH_HPP
#define ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_MATCH_HPP

#include <string>
#include <onsem/common/enum/semanticlanguageenum.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;
struct DetailedReactionAnswer;
struct ReactionOptions;


DetailedReactionAnswer triggers_match(const std::string& pText,
                                      SemanticMemory& pSemanticMemory,
                                      const linguistics::LinguisticDatabase& pLingDb,
                                      SemanticLanguageEnum pTextLanguage = SemanticLanguageEnum::UNKNOWN,
                                      const ReactionOptions* pReactionOptions = nullptr,
                                      bool pSetUsAsEverybody = false);

} // End of namespace onsem


#endif // ONSEMGTESTS_SEMANTICTOTEXT_TRIGGERS_TRIGGERS_MATCH_HPP
