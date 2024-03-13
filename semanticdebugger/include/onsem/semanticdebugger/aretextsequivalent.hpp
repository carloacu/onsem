#ifndef ONSEM_SEMANTICDEBUGGER_ARETEXTEQUIVALENT_HPP
#define ONSEM_SEMANTICDEBUGGER_ARETEXTEQUIVALENT_HPP

#include <map>
#include <string>
#include "api.hpp"

namespace onsem {

ONSEMSEMANTICDEBUGGER_API
bool areTextEquivalent(const std::string& pText1,
                       const std::string& pText2,
                       const std::map<std::string, std::string>* pEquivalencesPtr);

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICDEBUGGER_ARETEXTEQUIVALENT_HPP
