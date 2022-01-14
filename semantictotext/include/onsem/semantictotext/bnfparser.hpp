#ifndef ONSEM_SEMANTICTOTEXT_BNFPARSER_HPP
#define ONSEM_SEMANTICTOTEXT_BNFPARSER_HPP

#include <vector>
#include <string>
#include <onsem/semantictotext/api.hpp>

namespace onsem
{

ONSEMSEMANTICTOTEXT_API
std::vector<std::string> flattenBnfRegex(const std::string &pText);

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_BNFPARSER_HPP


