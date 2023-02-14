#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_ISCOMPLETE_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_ISCOMPLETE_HPP

#include <string>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include "../api.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
struct SyntacticGraph;


ONSEM_TEXTTOSEMANTIC_API
bool isComplete_fromSyntGraph(const SyntacticGraph& pSyntGraph);


ONSEM_TEXTTOSEMANTIC_API
bool isComplete(const std::string& pText,
                const LinguisticDatabase& pLingDb,
                SemanticLanguageEnum pLanguageType);



} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TOOL_ISCOMPLETE_HPP
