#ifndef ONSEM_COMMON_MACRO_HPP
#define ONSEM_COMMON_MACRO_HPP

#if defined _WIN32 || defined __CYGWIN__
#  define SEMANTIC_LIB_API_EXPORTS(LIBRARY_NAME) __declspec(dllexport)
#  define SEMANTIC_LIB_API(LIBRARY_NAME)         __declspec(dllimport)
#elif __GNUC__ >= 4
#  define SEMANTIC_LIB_API_EXPORTS(LIBRARY_NAME) __attribute__ ((visibility("default")))
#  define SEMANTIC_LIB_API(LIBRARY_NAME)        __attribute__ ((visibility("default")))
#else
#  define SEMANTIC_LIB_API_EXPORTS(LIBRARY_NAME)
#  define SEMANTIC_LIB_API(LIBRARY_NAME)
#endif

#endif // ONSEM_COMMON_MACRO_HPP
