#ifndef ONSEM_SEMANTICDEBUGGER_LOADDBPEDIATXTMEMORY_HPP
#define ONSEM_SEMANTICDEBUGGER_LOADDBPEDIATXTMEMORY_HPP

#include <set>
#include <string>
#include <memory>
#include <fstream>
#include "api.hpp"

namespace onsem {
struct SemanticMemory;
namespace linguistics {
struct LinguisticDatabase;
}

ONSEMSEMANTICDEBUGGER_API
void loadDbPediaMemory(std::size_t& pNbOfInforms,
                       std::set<std::string>& pProperNouns,
                       SemanticMemory& pSemanticMemory,
                       const linguistics::LinguisticDatabase& pLingDb,
                       const std::string& pFilename,
                       std::unique_ptr<std::ofstream>& pTextReplacedFilePtr,
                       bool pAddReferences);

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICDEBUGGER_LOADDBPEDIATXTMEMORY_HPP
