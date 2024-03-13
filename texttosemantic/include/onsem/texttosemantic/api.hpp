#ifndef ONSEM_TEXTTOSEMANTIC_API_H
#define ONSEM_TEXTTOSEMANTIC_API_H

#include <onsem/common/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(onsemtexttosemantic_EXPORTS)
    #define ONSEM_TEXTTOSEMANTIC_API SEMANTIC_LIB_API_EXPORTS(onsemtexttosemantic)
#elif !defined(SWIG)
    #define ONSEM_TEXTTOSEMANTIC_API SEMANTIC_LIB_API(onsemtexttosemantic)
#else
    #define ONSEM_TEXTTOSEMANTIC_API
#endif

#endif    // ONSEM_TEXTTOSEMANTIC_API_H
