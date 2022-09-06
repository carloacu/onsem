#ifndef ONSEM_TESTER_API_HPP
#define ONSEM_TESTER_API_HPP

#include <onsem/common/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(onsemtester_EXPORTS)
# define ONSEMTESTER_API SEMANTIC_LIB_API_EXPORTS(onsemtester)
#elif !defined(SWIG)
# define ONSEMTESTER_API SEMANTIC_LIB_API(onsemtester)
#else
# define ONSEMTESTER_API
#endif

#endif // ONSEM_TESTER_API_HPP
