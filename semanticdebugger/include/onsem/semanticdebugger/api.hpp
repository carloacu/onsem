#ifndef ONSEM_SEMANTICDEBUGGER_SEMANTICDEBUGGER_API_HPP
#define ONSEM_SEMANTICDEBUGGER_SEMANTICDEBUGGER_API_HPP

#include <onsem/common/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(onsemsemanticdebugger_EXPORTS)
    #define ONSEMSEMANTICDEBUGGER_API SEMANTIC_LIB_API_EXPORTS(onsemsemanticdebugger)
#elif !defined(SWIG)
    #define ONSEMSEMANTICDEBUGGER_API SEMANTIC_LIB_API(onsemsemanticdebugger)
#else
    #define ONSEMSEMANTICDEBUGGER_API
#endif

#endif    // ONSEM_SEMANTICDEBUGGER_SEMANTICDEBUGGER_API_HPP
