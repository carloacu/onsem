#ifndef DICTIONARYEXTRACTOR_API_HPP
#define DICTIONARYEXTRACTOR_API_HPP

#include <onsem/common/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(onsemdictionaryextractor_EXPORTS)
    #define DICTIONARYEXTRACTOR_API SEMANTIC_LIB_API_EXPORTS(onsemdictionaryextractor)
#elif !defined(SWIG)
    #define DICTIONARYEXTRACTOR_API SEMANTIC_LIB_API(onsemdictionaryextractor)
#else
    #define DICTIONARYEXTRACTOR_API
#endif

#endif    // DICTIONARYEXTRACTOR_API_HPP
