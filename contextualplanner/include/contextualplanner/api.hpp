#ifndef INCLUDE_CONTEXTUALPLANNER_API_HPP
#define INCLUDE_CONTEXTUALPLANNER_API_HPP

#include <contextualplanner/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(contextualplanner_EXPORTS)
# define CONTEXTUALPLANNER_API CONTEXTUALPLANNER_LIB_API_EXPORTS(contextualplanner)
#elif !defined(SWIG)
# define CONTEXTUALPLANNER_API CONTEXTUALPLANNER_LIB_API(contextualplanner)
#else
# define CONTEXTUALPLANNER_API
#endif

#endif // INCLUDE_CONTEXTUALPLANNER_API_HPP
