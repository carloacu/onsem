#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_BINARYDATABASESIZEPRINTER_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_BINARYDATABASESIZEPRINTER_HPP

#include <string>
#include "../../api.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
namespace binaryDbSizePrinter
{

ONSEM_TEXTTOSEMANTIC_API
void printSize(std::string& pText,
               const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace binaryDbSizePrinter
} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_BINARYDATABASESIZEPRINTER_HPP
