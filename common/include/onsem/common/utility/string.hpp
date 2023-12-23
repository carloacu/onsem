#ifndef ONSEM_COMMON_UTILITY_STRING_HPP
#define ONSEM_COMMON_UTILITY_STRING_HPP

#include <set>
#include <string>
#include <vector>
#include "../api.hpp"
#include "radix_map.hpp"
#include "radix_map_forward_declaration.hpp"

namespace onsem
{
namespace mystd
{

/// Takes a string, sanitizes it for usage in URLs, and converts spaces to hyphens.
ONSEM_COMMON_API
std::string urlizeText(const std::string& pText, bool pMergeTokens = false);


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

/**
 * @brief Replace all occurences of a string to another string.
 * @param str Input string to modify inplace.
 * @param oldStr String to search.
 * @param newStr String to put instead of the string ti search.
 */
ONSEM_COMMON_API
void replace_all(std::string& str,
                 const std::string& oldStr,
                 const std::string& newStr);


/// Class to do many replacements in a string.
class ONSEM_COMMON_API Replacer
{
public:
  /**
   * @brief Construct a Replacer.
   * @param pIsCaseSensitive If the matching will be case sensitive.
   * @param pHaveSeparatorBetweenWords If we expect separators between words.
   */
  Replacer(bool pIsCaseSensitive,
           bool pHaveSeparatorBetweenWords);

  /**
   * @brief Notify by what a string should be replaced by.
   * @param pPatternToSearch String a pattern to search.
   * @param pOutput Output to put instead of the corresponding pattern to search.
   */
  void addReplacementPattern(const std::string& pPatternToSearch, const std::string& pOutput);

  /**
   * @brief Take an input string and return the corresponding string after applying the replacement patterns.
   * @param pInput Input string.
   * @return The corresponding string after applying the replacement patterns.
   */
  std::string doReplacements(const std::string& pInput) const;

private:
  /// True the matchings will be case sensitive, false otherwise.
  bool _isCaseSensitive;
  /// True if we expect separators between words, false otherwise.
  bool _haveSeparatorBetweenWords;
  /// Map of string patterns to search to the string output to apply.
  mystd::radix_map_str<std::string> _patternsToSearchToOutput;
};


template <typename STRING_CONTAINER>
std::string join(const std::vector<std::string>& pStrs,
                 const std::string& pSeparator);


ONSEM_COMMON_API
void split(std::vector<std::string>& pStrs,
           const std::string& pStr,
           const std::string& pSeparator);

ONSEM_COMMON_API
void splitAnyOf(std::vector<std::string>& pStrs,
                const std::string& pStr,
                const std::set<char>& pChars);

ONSEM_COMMON_API
void splitNotEmpty(std::vector<std::string>& pStrs,
                   const std::string& pStr,
                   const std::string& pSeparator);

ONSEM_COMMON_API
std::string filenameToSuffix(const std::string& pFileName);


ONSEM_COMMON_API
bool differentThanBoth(const std::string& pStr, const std::string& pPossibility1, const std::string& pPossibility2);


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
