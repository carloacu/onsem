#ifndef TEXTEXTRACTOR_TEXTEXTRACTOR_HPP
#define TEXTEXTRACTOR_TEXTEXTRACTOR_HPP

#include <string>

namespace onsem {
class LingdbTree;
namespace linguistics {
struct LinguisticDatabase;
}

namespace textextractor {

void run(const LingdbTree& pLingDbTree,
         const std::string& pTmpFolder,
         const onsem::linguistics::LinguisticDatabase& pLingDb,
         const std::string& pShareSemanticPath,
         const std::string& pInputResourcesFolder);

}    // End of namespace textextractor

}    // End of namespace onsem

#endif    // TEXTEXTRACTOR_TEXTEXTRACTOR_HPP
