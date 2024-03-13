#ifndef SEMANTICTOOLSTOFILLDATABASE_WIKITIONARYEXTRACTOR_CONCEPTSEXTRACTOR_HPP
#define SEMANTICTOOLSTOFILLDATABASE_WIKITIONARYEXTRACTOR_CONCEPTSEXTRACTOR_HPP

#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include "metawiki/patternrecognizer.hpp"
#include "metawiki/wikikeywords.hpp"

namespace onsem {
class ConceptsExtractor {
public:
    ConceptsExtractor(const WikiKeyWords& pWikikeyWords, const PatternRecognizer& pPatternRecognizer);

    void extractDatasFromFile(std::map<std::string, std::set<LingdbMeaning*>>& pConceptToMeanings,
                              std::ifstream& pWikionaryFile,
                              const LinguisticIntermediaryDatabase& pLingDatabase);

    void writeConcepts(const std::string& pWikionaryFile,
                       const std::map<std::string, std::set<LingdbMeaning*>>& pConceptToMeanings);

private:
    const WikiKeyWords& _wikikeyWords;
    const PatternRecognizer& _patternReco;
};

}    // End of namespace onsem

#endif    // SEMANTICTOOLSTOFILLDATABASE_WIKITIONARYEXTRACTOR_CONCEPTSEXTRACTOR_HPP
