#ifndef ONSEMSEMANTICTOTEXT_API_H
#define ONSEMSEMANTICTOTEXT_API_H

#include <onsem/common/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(onsemsemantictotext_EXPORTS)
# define ONSEMSEMANTICTOTEXT_API SEMANTIC_LIB_API_EXPORTS(onsemsemantictotext)
#elif !defined(SWIG)
# define ONSEMSEMANTICTOTEXT_API SEMANTIC_LIB_API(onsemsemantictotext)
#else
# define ONSEMSEMANTICTOTEXT_API
#endif

#endif // ONSEMSEMANTICTOTEXT_API_H
