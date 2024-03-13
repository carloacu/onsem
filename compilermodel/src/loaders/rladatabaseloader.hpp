#ifndef ONSEM_COMPILERMODEL_SRC_LOADERS_RLADATABASELOADER_HPP
#define ONSEM_COMPILERMODEL_SRC_LOADERS_RLADATABASELOADER_HPP

#include <string>

namespace onsem {
class LinguisticIntermediaryDatabase;
class LingdbTree;
class AnyDatabaseLoader;

class RlaDatabaseLoader {
public:
    static void merge(const std::string& pFilename,
                      const std::string& pInputResourcesDir,
                      LinguisticIntermediaryDatabase& pCurrLingdb,
                      const LingdbTree& pLingdbTree,
                      const AnyDatabaseLoader& pAnyLoader,
                      std::size_t pImbricationLevel);

private:
    static std::string _getPath(const std::string& pLine,
                                const std::string& pInstruction,
                                const std::string& pHoldingFolder,
                                const std::string& pInputResourcesDir);
};

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_SRC_LOADERS_RLADATABASELOADER_HPP
