#ifndef ONSEM_STREAMDATABASEACCESSOR_STREAMDATABASEACCESSOR_HPP
#define ONSEM_STREAMDATABASEACCESSOR_STREAMDATABASEACCESSOR_HPP

#include <string>
#include <onsem/common/keytostreams.hpp>
#include "api.hpp"

namespace onsem {
namespace linguistics {

ONSEMSTREADATABASEGENERATOR_DATABASEPROVIDER_API
KeyToFStreams generateIStreams(const std::string& pLingDbPath, const std::string& pRelationsPath);

ONSEMSTREADATABASEGENERATOR_DATABASEPROVIDER_API
void addDynamicContentFromFolder(KeyToFStreams& pStreams, const std::string& pFolderPath);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_STREAMDATABASEACCESSOR_STREAMDATABASEACCESSOR_HPP
