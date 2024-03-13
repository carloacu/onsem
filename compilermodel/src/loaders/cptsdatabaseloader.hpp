#ifndef ONSEM_COMPILERMODEL_SRC_LOADERS_CPTSDATABASELOADER_HPP
#define ONSEM_COMPILERMODEL_SRC_LOADERS_CPTSDATABASELOADER_HPP

#include <string>

namespace onsem {
class LinguisticIntermediaryDatabase;
class LingdbConcept;

class CptsDatabaseLoader {
public:
    void merge(const std::string& pFilename, LinguisticIntermediaryDatabase& pLingdb);

private:
    static void _fillConcept(LinguisticIntermediaryDatabase& pLingdb,
                             const std::string& pWord,
                             const LingdbConcept* pNewConcept,
                             const std::string& pNewConceptStr,
                             char pRelatedToConcept,
                             const std::string& pFilename,
                             const std::string& pLine);
};

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_SRC_LOADERS_CPTSDATABASELOADER_HPP
