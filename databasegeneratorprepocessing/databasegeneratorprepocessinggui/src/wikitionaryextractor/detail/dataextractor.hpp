#ifndef WIKITIONARYDATAEXTRACTOR_H
#define WIKITIONARYDATAEXTRACTOR_H

#include <onsem/compilermodel/savers/traductionwriter.hpp>

class PatternRecognizer;
namespace onsem {
class WikiKeyWords;

class WikiDataExtractor {
public:
    WikiDataExtractor(const WikiKeyWords& pWikikeyWords, const PatternRecognizer& pPatternRecognizer);

    void extractDatasFromFile(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsInToOut,
                              std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsOutToIn,
                              std::ifstream& pWikionaryFile,
                              const LinguisticIntermediaryDatabase& pInLingDatabase,
                              const LinguisticIntermediaryDatabase& pOutLingDatabase);

private:
    const WikiKeyWords& fWikikeyWords;
    const PatternRecognizer& fPatternReco;

    static void xFillTrads(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsInToOut,
                           std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsOutToIn,
                           const LingdbDynamicTrieNode* pInNode,
                           const std::set<PartOfSpeech>& pInGramTypes,
                           const std::set<LingdbMeaning*>& pOutMeanings);

    static void xAddNodesToNode(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads,
                                const std::set<LingdbMeaning*>& pInMeanings,
                                const LingdbDynamicTrieNode* pOutNode);

    static void xAddNodeToNodes(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads,
                                const LingdbDynamicTrieNode* pInNode,
                                const std::set<LingdbMeaning*>& pOutMeanings);

    void xExtractMeaningsOfOutWords(std::set<LingdbMeaning*>& pOutMeanings,
                                    const std::string& pLine,
                                    const LinguisticIntermediaryDatabase& pOutLingDatabase) const;
};

}    // End of namespace onsem

#endif    // WIKITIONARYDATAEXTRACTOR_H
