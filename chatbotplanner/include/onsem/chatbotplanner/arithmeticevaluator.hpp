#ifndef ONSEM_CHATBOTPLANNER_ARITHMETICEVALUATOR_HPP
#define ONSEM_CHATBOTPLANNER_ARITHMETICEVALUATOR_HPP

#include <string>
#include "api.hpp"

namespace onsem
{

ONSEMCHATBOTPLANNER_API
int evalute(const std::string& pText,
            std::size_t pBeginPos = 0);

ONSEMCHATBOTPLANNER_API
std::string evaluteToStr(const std::string& pText,
                         std::size_t pBeginPos = 0);

}

#endif // ONSEM_CHATBOTPLANNER_ARITHMETICEVALUATOR_HPP
