#ifndef ONSEM_COMPILERMODEL_SRC_LOADERS_OMLDLOADER_HPP
#define ONSEM_COMPILERMODEL_SRC_LOADERS_OMLDLOADER_HPP

#include <string>

namespace onsem {
class LinguisticIntermediaryDatabase;

/**
 * @brief Merge the current database with a omld file.
 * @param pFilename Filename of the omld file.
 * @param pWords The database currently in memory.
 */
void mergeOmld(const std::string& pFilename, LinguisticIntermediaryDatabase& pWords);

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_SRC_LOADERS_OMLDLOADER_HPP
