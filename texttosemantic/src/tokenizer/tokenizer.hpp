#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SETPS_TOKENIZER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SETPS_TOKENIZER_HPP

#include <vector>
#include <string>
#include <memory>

namespace onsem
{
struct ResourceGroundingExtractor;
namespace linguistics
{
struct Token;
class LinguisticDictionary;


namespace tokenizer
{
/**
 * @brief Split the sentence into tokens
 * (token is more or less a word).
 * @param pTokens List of tokens in result.
 * @param pInputSentence Input sentence.
 */
void tokenize(std::vector<Token>& pTokens,
              const std::string& pInputSentence,
              const LinguisticDictionary& pLingDico,
              const LinguisticDictionary& pCommonLingDico,
              const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr);

}  // End of namespace tokenizer
} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SRC_SETPS_TOKENIZER_HPP
