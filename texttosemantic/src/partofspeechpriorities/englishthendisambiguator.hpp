#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TAGGER_ENGLSIHTHENDISAMBIGUATOR_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TAGGER_ENGLSIHTHENDISAMBIGUATOR_HPP

#include <vector>

namespace onsem
{
namespace linguistics
{
struct Token;

void englishThenDisambiguator(std::vector<Token>& pTokens);


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_TAGGER_ENGLSIHTHENDISAMBIGUATOR_HPP
