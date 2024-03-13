#include "dataextractor.hpp"
#include <fstream>
#include "frwiki/frpatternrecognizer.hpp"
#include "frwiki/frwikikeywords.hpp"
#include "wikiutils.hpp"

namespace onsem {

WikiDataExtractor::WikiDataExtractor(const WikiKeyWords& pWikikeyWords, const PatternRecognizer& pPatternRecognizer)
    : fWikikeyWords(pWikikeyWords)
    , fPatternReco(pPatternRecognizer) {}

void WikiDataExtractor::extractDatasFromFile(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsInToOut,
                                             std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsOutToIn,
                                             std::ifstream& pWikionaryFile,
                                             const LinguisticIntermediaryDatabase& pInLingDatabase,
                                             const LinguisticIntermediaryDatabase& pOutLingDatabase) {
    std::string currWord;
    LingdbDynamicTrieNode* inNode = nullptr;
    LingdbDynamicTrieNode* outNode = nullptr;
    PatternRecognizerLanguageEnum currLang = PATTERNRECO_LANG_OTHERLANG;
    std::set<PartOfSpeech> inGramTypes;
    std::string line;
    long numLine = 0;
    while (getline(pWikionaryFile, line)) {
        if (!line.empty()) {
            if (fPatternReco.isTitle(line)) {
                currWord.clear();
                inNode = nullptr;
                outNode = nullptr;
                inGramTypes.clear();
                currLang = PATTERNRECO_LANG_OTHERLANG;
                wikiutil::extractWordOfTitleLine(currWord, line);
            } else if (fPatternReco.isSeparator(line)) {
                if (fPatternReco.isLanguage(line)) {
                    currLang = fPatternReco.getLanguage(line);
                    inGramTypes.clear();
                    if (currLang == PATTERNRECO_LANG_INLANG) {
                        inNode = pInLingDatabase.getPointerToEndOfWord(currWord);
                        if (inNode == nullptr) {
                            inNode = pInLingDatabase.findComposedWordFromString(currWord);
                        }
                    }
                    if (currLang == PATTERNRECO_LANG_OUTLANG) {
                        outNode = pOutLingDatabase.getPointerToEndOfWord(currWord);
                        if (outNode == nullptr) {
                            outNode = pOutLingDatabase.findComposedWordFromString(currWord);
                        }
                    }
                } else if ((currLang == PATTERNRECO_LANG_INLANG && inNode != nullptr)
                           || (currLang == PATTERNRECO_LANG_OUTLANG && outNode != nullptr)) {
                    if (fPatternReco.isGramInLineType(line)) {
                        std::string gramStr;
                        PatternRecognizerSeparatorType separatorType;
                        fPatternReco.getSperator(gramStr, separatorType, line);
                        fWikikeyWords.getGramEnum(inGramTypes, gramStr, line);
                    }
                }
            } else {
                switch (currLang) {
                    case PATTERNRECO_LANG_INLANG: {
                        if (inNode == nullptr) {
                            break;
                        } else if (fPatternReco.isBeginLineOfTrads(line)) {
                            // new traductions
                            std::set<LingdbMeaning*> outMeanings;
                            xExtractMeaningsOfOutWords(outMeanings, line, pOutLingDatabase);
                            if (!outMeanings.empty()) {
                                if (!inGramTypes.empty()) {
                                    xFillTrads(pTradsInToOut, pTradsOutToIn, inNode, inGramTypes, outMeanings);
                                } else {
                                    xAddNodeToNodes(pTradsInToOut, inNode, outMeanings);
                                    xAddNodesToNode(pTradsOutToIn, outMeanings, inNode);
                                }
                            }
                        }
                        break;
                    }
                    default: break;
                }
            }
        }

        ++numLine;
        if (numLine % 10000000 == 0) {
            std::cout << "numLine: " << numLine / 10000000 << "0 000 000" << std::endl;
        }
    }
    pWikionaryFile.close();
}

void WikiDataExtractor::xExtractMeaningsOfOutWords(std::set<LingdbMeaning*>& pOutMeanings,
                                                   const std::string& pLine,
                                                   const LinguisticIntermediaryDatabase& pOutLingDatabase) const {
    std::list<std::string> tradsStr;
    fPatternReco.getStradsStr(tradsStr, pLine);
    for (std::list<std::string>::const_iterator itTrad = tradsStr.begin(); itTrad != tradsStr.end(); ++itTrad) {
        LingdbDynamicTrieNode* outNode = pOutLingDatabase.getPointerToEndOfWord(*itTrad);
        if (outNode == nullptr) {
            outNode = pOutLingDatabase.findComposedWordFromString(*itTrad);
        }
        if (outNode != nullptr) {
            const ForwardPtrList<LingdbWordForms>* wfs = outNode->getWordForms();
            while (wfs != nullptr) {
                pOutMeanings.insert(wfs->elt->getMeaning());
                wfs = wfs->next;
            }
        }
    }
}

void WikiDataExtractor::xAddNodeToNodes(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads,
                                        const LingdbDynamicTrieNode* pInNode,
                                        const std::set<LingdbMeaning*>& pOutMeanings) {
    const ForwardPtrList<LingdbWordForms>* wfs = pInNode->getWordForms();
    while (wfs != nullptr) {
        pTrads[wfs->elt->getMeaning()].notLinkedWithInMeaningGram.insert(pOutMeanings.begin(), pOutMeanings.end());
        wfs = wfs->next;
    }
}

void WikiDataExtractor::xAddNodesToNode(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads,
                                        const std::set<LingdbMeaning*>& pInMeanings,
                                        const LingdbDynamicTrieNode* pOutNode) {
    for (std::set<LingdbMeaning*>::const_iterator itIM = pInMeanings.begin(); itIM != pInMeanings.end(); ++itIM) {
        const ForwardPtrList<LingdbWordForms>* wfs = pOutNode->getWordForms();
        while (wfs != nullptr) {
            pTrads[*itIM].notLinkedWithInMeaningGram.insert(wfs->elt->getMeaning());
            wfs = wfs->next;
        }
    }
}

void WikiDataExtractor::xFillTrads(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsInToOut,
                                   std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsOutToIn,
                                   const LingdbDynamicTrieNode* pInNode,
                                   const std::set<PartOfSpeech>& pInGramTypes,
                                   const std::set<LingdbMeaning*>& pOutMeanings) {
    bool inOutHaveSameGram = false;
    const ForwardPtrList<LingdbWordForms>* wfs = pInNode->getWordForms();
    while (wfs != nullptr) {
        LingdbMeaning* inMeaning = wfs->elt->getMeaning();

        std::set<PartOfSpeech>::const_iterator itInGram = pInGramTypes.find(inMeaning->getPartOfSpeech());
        if (itInGram != pInGramTypes.end()) {
            inOutHaveSameGram = true;
            for (std::set<LingdbMeaning*>::const_iterator itOutMeaning = pOutMeanings.begin();
                 itOutMeaning != pOutMeanings.end();
                 ++itOutMeaning) {
                pTradsInToOut[inMeaning].linkedWithInMeaningGram.insert(*itOutMeaning);
                pTradsOutToIn[*itOutMeaning].linkedWithInMeaningGram.insert(inMeaning);
            }
        }
        wfs = wfs->next;
    }

    // if pInNode has no in pInGramTypes
    if (!inOutHaveSameGram) {
        xAddNodeToNodes(pTradsInToOut, pInNode, pOutMeanings);
        xAddNodesToNode(pTradsOutToIn, pOutMeanings, pInNode);
    }
}

}    // End of namespace onsem
