#ifndef ONSEM_COMMON_UTILITY_GETENDOFPARENTHESIS_HPP
#define ONSEM_COMMON_UTILITY_GETENDOFPARENTHESIS_HPP

#include <string>
#include "../api.hpp"

namespace onsem {

/**
 * @brief getEndOfParenthesis Get the position of the end of a parenthesis.\n
 * Both ( ) and [ ] are concidered as parenthesis.
 * @param pStr The string.
 * @param pPos The position of the begin of the parenthesis ( or [.
 * @param pEnd The last position of the string to consider.
 * @return The position of the end of the parenthesis ) or ].
 */
ONSEM_COMMON_API
std::size_t getEndOfParenthesis(const std::string& pStr, std::size_t pPos, const std::size_t pEnd);

}    // End of namespace onsem

#endif    // ONSEM_COMMON_UTILITY_GETENDOFPARENTHESIS_HPP
