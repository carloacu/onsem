#ifndef ONSEM_COMPILERMODEL_SRC_LOADERS_GFSDATABASELOADER_HPP
#define ONSEM_COMPILERMODEL_SRC_LOADERS_GFSDATABASELOADER_HPP

#include <filesystem>
#include <string>

namespace onsem {
class LinguisticIntermediaryDatabase;

/// This class load a database from a dela dictionnary file.
class GFSDatabaseLoader {
public:
    static void merge(const std::filesystem::path& pFilename, LinguisticIntermediaryDatabase& pLingdb);
};

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_SRC_LOADERS_GFSDATABASELOADER_HPP
