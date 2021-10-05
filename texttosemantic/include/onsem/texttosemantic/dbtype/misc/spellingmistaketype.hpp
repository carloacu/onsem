#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_MISC_SPELLINGMISTAKETYPE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_MISC_SPELLINGMISTAKETYPE_HPP

#include <set>

namespace onsem
{

enum class SpellingMistakeType
{
  CONJUGATION
};


static const std::set<SpellingMistakeType> _spellingMistakeTypesPossibleForDebugOnTextComparisons{SpellingMistakeType::CONJUGATION};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPES_MISC_SPELLINGMISTAKETYPE_HPP
