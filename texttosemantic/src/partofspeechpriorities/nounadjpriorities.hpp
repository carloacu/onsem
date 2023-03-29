#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOKENIZER_NOUNADJPRIORITIES_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOKENIZER_NOUNADJPRIORITIES_HPP

#include <vector>

namespace onsem
{
class InflectionsChecker;
namespace linguistics
{
struct Token;

void detNounPriorities(std::vector<Token>& pTokens,
                       const InflectionsChecker& pInflsCheker);
void pronNounPriorities(std::vector<Token>& pTokens,
                        const InflectionsChecker& pInflsCheker);
void nounDetPriorities(std::vector<Token>& pTokens,
                       const InflectionsChecker& pInflsCheker);
void nounAdjPriorities(std::vector<Token>& pTokens);
void NounPrioritiesFr(std::vector<Token>& pTokens,
                      const InflectionsChecker& pInflsCheker,
                      bool pIsRootLevel);
void nounNounPrioritiesEn(std::vector<Token>& pTokens,
                          const InflectionsChecker& pInflsCheker);
void verbPriorities(std::vector<Token>& pTokens,
                    const InflectionsChecker& pInflsCheker);
void verbPrioritiesFr(std::vector<Token>& pTokens,
                      const InflectionsChecker& pInflsCheker);
void pronounPriorities(std::vector<Token>& pTokens,
                       const InflectionsChecker& pInflsCheker);
void adjPriorities(std::vector<Token>& pTokens);
void putApproximatelyConceptInTopPrioritiesIfNecessary(std::vector<Token>& pTokens);

void partitivePrioritiesFr(std::vector<Token>& pTokens,
                           const InflectionsChecker& pInflsCheker);

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOKENIZER_NOUNADJPRIORITIES_HPP
