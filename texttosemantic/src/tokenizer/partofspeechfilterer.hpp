#ifndef LINGUISTICANALYZER_TAGGER_RULES_SYNTACTICTAGGER_HPP
#define LINGUISTICANALYZER_TAGGER_RULES_SYNTACTICTAGGER_HPP

#include <vector>
#include <string>

namespace onsem {
namespace linguistics {
struct Token;
struct SpecificLinguisticDatabase;

namespace partOfSpeechFilterer {

void process(std::vector<Token>& pTokens,
             const SpecificLinguisticDatabase& pSpecLingDb,
             const std::string& pEndingStep,
             std::size_t pNbOfDebugRounds,
             bool pIsRootLevel);

}    // End of namespace partOfSpeechFilterer
}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // !LINGUISTICANALYZER_TAGGER_RULES_SYNTACTICTAGGER_HPP
