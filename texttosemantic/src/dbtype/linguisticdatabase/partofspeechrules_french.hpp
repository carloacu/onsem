#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_PARTOFSPEECHRULES_FRENCH_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_PARTOFSPEECHRULES_FRENCH_HPP

#include <list>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcontextfilter.hpp>

namespace onsem
{
namespace linguistics
{
struct SpecificLinguisticDatabase;
class InflectionsChecker;
namespace partofspeechrules
{
namespace french
{


std::list<std::unique_ptr<PartOfSpeechContextFilter>> getPartOfSpeechRules
(const InflectionsChecker& pInfls,
 const linguistics::SpecificLinguisticDatabase& pSpecLingDb);



} // End of namespace french
} // End of namespace partofspeechrules
} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_PARTOFSPEECHRULES_FRENCH_HPP
