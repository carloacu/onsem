#ifndef ONSEM_COMMON_API_HPP
#define ONSEM_COMMON_API_HPP

#include <onsem/common/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(onsemcommon_EXPORTS)
# define ONSEM_COMMON_API SEMANTIC_LIB_API_EXPORTS(onsemcommon)
#elif !defined(SWIG)
# define ONSEM_COMMON_API SEMANTIC_LIB_API(onsemcommon)
#else
# define ONSEM_COMMON_API
#endif

#endif // ONSEM_COMMON_API_HPP
