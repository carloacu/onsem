#ifndef ONSEM_TEXTTOSEMANTIC_LANGUAGEDETECTOR_HPP
#define ONSEM_TEXTTOSEMANTIC_LANGUAGEDETECTOR_HPP

#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include "api.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;

ONSEM_TEXTTOSEMANTIC_API
SemanticLanguageEnum getLanguage(const std::string& pText,
                                 const LinguisticDatabase& pLingDb);

} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_LANGUAGEDETECTOR_HPP
