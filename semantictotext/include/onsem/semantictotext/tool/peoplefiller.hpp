#ifndef ONSEM_SEMANTICTOTEXT_TOOL_PEOPLEFILLER_HPP
#define ONSEM_SEMANTICTOTEXT_TOOL_PEOPLEFILLER_HPP

#include <istream>
#include <onsem/common/enum/semanticlanguageenum.hpp>

namespace onsem
{
struct SemanticMemoryBlock;
namespace linguistics
{
struct LinguisticDatabase;
}

namespace peopleFiller
{

void addPeople(SemanticMemoryBlock& pMemoryBlock,
               std::istream& pInputStream,
               SemanticLanguageEnum pLanguage,
               const linguistics::LinguisticDatabase& pLingDb);

} // End of namespace peopleFiller

} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_TOOL_PEOPLEFILLER_HPP
