#ifndef ONSEM_CHATBOTPLANNER_API_HPP
#define ONSEM_CHATBOTPLANNER_API_HPP

#include <onsem/chatbotplanner/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(onsemchatbotplanner_EXPORTS)
# define ONSEMCHATBOTPLANNER_API CHATBOTPLANNER_LIB_API_EXPORTS(onsemchatbotplanner)
#elif !defined(SWIG)
# define ONSEMCHATBOTPLANNER_API CHATBOTPLANNER_LIB_API(onsemchatbotplanner)
#else
# define ONSEMCHATBOTPLANNER_API
#endif

#endif // ONSEM_CHATBOTPLANNER_API_HPP
