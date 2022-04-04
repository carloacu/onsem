#ifndef INCLUDE_CONTEXTUALPLANNER_ARITHMETICEVALUATOR_HPP
#define INCLUDE_CONTEXTUALPLANNER_ARITHMETICEVALUATOR_HPP

#include <string>
#include "api.hpp"

namespace cp
{

CONTEXTUALPLANNER_API
int evalute(const std::string& pText,
            std::size_t pBeginPos = 0);

CONTEXTUALPLANNER_API
std::string evaluteToStr(const std::string& pText,
                         std::size_t pBeginPos = 0);

}

#endif // INCLUDE_CONTEXTUALPLANNER_ARITHMETICEVALUATOR_HPP
