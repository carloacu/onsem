#ifndef ONSEM_SEMANTICDEBUGGER_SRC_DEBUG_DIAGNOSISPRINTER_HPP
#define ONSEM_SEMANTICDEBUGGER_SRC_DEBUG_DIAGNOSISPRINTER_HPP

#include <string>
#include <vector>
#include <onsem/semanticdebugger/textananlyzedebugger.hpp>
#include "api.hpp"

namespace onsem {
struct SemanticMemory;
namespace diagnosisPrinter {

ONSEMSEMANTICDEBUGGER_API
void getNextParameterPossibilities(std::list<std::string>& pNextParameterPossibilities,
                                   const std::vector<std::string>& pModes);

ONSEMSEMANTICDEBUGGER_API
void print(std::list<SemLineToPrint>& pLines,
           const std::vector<std::string>& pModes,
           const SemanticMemory& pSemanticMemory,
           const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICDEBUGGER_API
std::string diagnosis(const std::vector<std::string>& pModes,
                      const SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb);

}    // End of namespace diagnosisPrinter
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICDEBUGGER_SRC_DEBUG_DIAGNOSISPRINTER_HPP
