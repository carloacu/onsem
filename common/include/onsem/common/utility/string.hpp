#ifndef ONSEM_COMMON_UTILITY_STRING_HPP
#define ONSEM_COMMON_UTILITY_STRING_HPP

#include <string>
#include <vector>
#include "../api.hpp"

namespace onsem
{
namespace mystd
{

/// Takes a string, sanitizes it for usage in URLs, and converts spaces to hyphens.
ONSEM_COMMON_API
std::string urlizeText(const std::string& pText);


/**
 * @brief findFirstOf Find the first position of some characters in a part of a string.
 * @param pStr The string where we look for the characters.
 * @param pChars The characters to look for.
 * @param pBegin The begin of the string.
 * @param pEnd The end of the string.
 * @return The first position of any pChars in pStr between pBegin and pEnd.\n
 * If no character found the function return pEnd.
 */
ONSEM_COMMON_API
std::size_t findFirstOf(const std::string& pStr,
                        const std::string& pChars,
                        std::size_t pBegin,
                        std::size_t pEnd);


ONSEM_COMMON_API
void replace_all(std::string& str,
                 const std::string& oldStr,
                 const std::string& newStr);


template <typename STRING_CONTAINER>
std::string join(const std::vector<std::string>& pStrs,
                 const std::string& pSeparator);


ONSEM_COMMON_API
void split(std::vector<std::string>& pStrs,
           const std::string& pStr,
           const std::string& pSeparator);


ONSEM_COMMON_API
std::string filenameToSuffix(const std::string& pFileName);




template <typename STRING_CONTAINER>
std::string join(const STRING_CONTAINER& pStrs,
                 const std::string& pSeparator)
{
  std::string res;
  bool firstIteration = true;
  for (const auto& currStr : pStrs)
  {
    if (firstIteration)
      firstIteration = false;
    else
      res += pSeparator;
    res += currStr;
  }
  return res;
}

} // End of namespace mystd
} // End of namespace onsem

#endif // ONSEM_COMMON_UTILITY_STRING_HPP
