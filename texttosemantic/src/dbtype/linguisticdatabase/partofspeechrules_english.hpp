#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_PARTOFSPEECHRULES_ENGLISH_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_PARTOFSPEECHRULES_ENGLISH_HPP

#include <list>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcontextfilter.hpp>

namespace onsem {
namespace linguistics {
struct SpecificLinguisticDatabase;
class InflectionsChecker;
namespace partofspeechrules {
namespace english {

std::list<std::unique_ptr<PartOfSpeechContextFilter>> getPartOfSpeechRules(
    const InflectionsChecker& pInfls,
    const SpecificLinguisticDatabase& pSpecLingDb);

}    // End of namespace english
}    // End of namespace partofspeechrules
}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_PARTOFSPEECHRULES_ENGLISH_HPP
