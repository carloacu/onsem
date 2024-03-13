#ifndef INTRANSITIVEVERBS_INTRANSITIVEVERBSEXTRACTOR_HPP
#define INTRANSITIVEVERBS_INTRANSITIVEVERBSEXTRACTOR_HPP

#include <map>
#include <set>
#include <string>
#include <onsem/common/enum/partofspeech.hpp>

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}
namespace intransitiveVerbsExtractor {

void run(const std::string& pInFilename,
         const std::string& pInputResourcesFolder,
         const linguistics::LinguisticDatabase& pLingDb);

}    // End of namespace intransitiveVerbsExtractor
}    // End of namespace onsem

#endif    // INTRANSITIVEVERBS_INTRANSITIVEVERBSEXTRACTOR_HPP
