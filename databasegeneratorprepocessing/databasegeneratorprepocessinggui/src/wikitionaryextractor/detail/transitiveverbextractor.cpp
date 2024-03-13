#include "transitiveverbextractor.hpp"
#include <fstream>
#include <QDomDocument>
#include <QFile>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include "frwiki/frpatternrecognizer.hpp"
#include "frwiki/frwikikeywords.hpp"
#include "wikiutils.hpp"

namespace onsem {

WikiTransitiveVerbExtractor::WikiTransitiveVerbExtractor(const WikiKeyWords& pWikikeyWords,
                                                         const PatternRecognizer& pPatternRecognizer)
    : fWikikeyWords(pWikikeyWords)
    , fPatternReco(pPatternRecognizer) {}

enum class IsTransitiveEnum { YES, NO, UNKNOWN };

struct CurrentWordInfos {
    CurrentWordInfos()
        : word()
        , isTransitive(IsTransitiveEnum::UNKNOWN)
        , wordNode(nullptr)
        , partOfSpeeches()
        , meanings() {}

    void clear() {
        word.clear();
        isTransitive = IsTransitiveEnum::UNKNOWN;
        wordNode = nullptr;
        partOfSpeeches.clear();
        meanings.clear();
    }

    std::string word;
    IsTransitiveEnum isTransitive;
    LingdbDynamicTrieNode* wordNode;
    std::set<PartOfSpeech> partOfSpeeches;
    std::set<LingdbMeaning*> meanings;
};

void WikiTransitiveVerbExtractor::extract(std::set<LingdbMeaning*>& pTransitiveVerbs,
                                          std::ifstream& pWikionaryFile,
                                          const LinguisticIntermediaryDatabase& pLingDatabase) {
    CurrentWordInfos currWordInfos;
    PatternRecognizerLanguageEnum currLang = PATTERNRECO_LANG_OTHERLANG;
    std::string line;
    long numLine = 0;
    while (getline(pWikionaryFile, line)) {
        if (!line.empty()) {
            if (fPatternReco.isTitle(line)) {
                if (currWordInfos.isTransitive == IsTransitiveEnum::YES)
                    pTransitiveVerbs.insert(currWordInfos.meanings.begin(), currWordInfos.meanings.end());
                currWordInfos.clear();
                currLang = PATTERNRECO_LANG_OTHERLANG;
                wikiutil::extractWordOfTitleLine(currWordInfos.word, line);
            } else if (fPatternReco.isSeparator(line)) {
                if (fPatternReco.isLanguage(line)) {
                    currLang = fPatternReco.getLanguage(line);
                    currWordInfos.partOfSpeeches.clear();
                    if (currLang == PATTERNRECO_LANG_INLANG) {
                        currWordInfos.wordNode = pLingDatabase.getPointerToEndOfWord(currWordInfos.word);
                        if (currWordInfos.wordNode == nullptr)
                            currWordInfos.wordNode = pLingDatabase.findComposedWordFromString(currWordInfos.word);
                    }
                } else if (currLang == PATTERNRECO_LANG_INLANG && currWordInfos.wordNode != nullptr) {
                    if (fPatternReco.isGramInLineType(line)) {
                        std::string gramStr;
                        PatternRecognizerSeparatorType separatorType;
                        fPatternReco.getSperator(gramStr, separatorType, line);
                        fWikikeyWords.getGramEnum(currWordInfos.partOfSpeeches, gramStr, line);
                    }
                }
            } else {
                switch (currLang) {
                    case PATTERNRECO_LANG_INLANG: {
                        if (currWordInfos.wordNode == nullptr || currWordInfos.isTransitive == IsTransitiveEnum::NO
                            || currWordInfos.partOfSpeeches.find(PartOfSpeech::VERB)
                                   == currWordInfos.partOfSpeeches.end()) {
                            break;
                        }

                        static const std::string isTransitiveBegOfLine = "# {{context|transitive|";
                        if (currWordInfos.isTransitive == IsTransitiveEnum::UNKNOWN
                            && line.compare(0, isTransitiveBegOfLine.size(), isTransitiveBegOfLine) == 0) {
                            const ForwardPtrList<LingdbWordForms>* wfs = currWordInfos.wordNode->getWordForms();
                            while (wfs != nullptr) {
                                LingdbMeaning* meaningPtr = wfs->elt->getMeaning();
                                if (meaningPtr->getPartOfSpeech() == PartOfSpeech::VERB) {
                                    currWordInfos.isTransitive = IsTransitiveEnum::YES;
                                    currWordInfos.meanings.emplace(meaningPtr);
                                }
                                wfs = wfs->next;
                            }
                        }

                        static const std::string isIntransitiveBegOfLine = "# {{context|intransitive|";
                        if (line.compare(0, isIntransitiveBegOfLine.size(), isIntransitiveBegOfLine) == 0) {
                            const ForwardPtrList<LingdbWordForms>* wfs = currWordInfos.wordNode->getWordForms();
                            while (wfs != nullptr) {
                                LingdbMeaning* meaningPtr = wfs->elt->getMeaning();
                                if (meaningPtr->getPartOfSpeech() == PartOfSpeech::VERB) {
                                    currWordInfos.isTransitive = IsTransitiveEnum::NO;
                                    currWordInfos.meanings.clear();
                                }
                                wfs = wfs->next;
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

void WikiTransitiveVerbExtractor::writeNewTransitiveVerbs(const std::set<LingdbMeaning*>& pTransitiveVerbs,
                                                          const std::string& poutFile) {
    QDomDocument document;
    QDomElement rootXml = document.createElement("linguisticdatabase");
    document.appendChild(rootXml);

    for (const LingdbMeaning* currTrVerb : pTransitiveVerbs) {
        QDomElement newMeaning = document.createElement("existingMeaning");
        newMeaning.setAttribute("lemme", QString::fromUtf8(currTrVerb->getLemma()->getWord().c_str()));
        newMeaning.setAttribute("gram", QString::fromUtf8(partOfSpeech_toStr(PartOfSpeech::VERB).c_str()));
        QDomElement contextInfo = document.createElement("contextInfo");
        contextInfo.setAttribute("val", "transitiveVerb");
        newMeaning.appendChild(contextInfo);
        rootXml.appendChild(newMeaning);
    }

    QFile file(QString::fromUtf8(poutFile.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Open the file for writing failed" << std::endl;
    } else {
        QString text = document.toString();
        file.write(text.toUtf8());
        file.close();
    }
}

}    // End of namespace onsem
