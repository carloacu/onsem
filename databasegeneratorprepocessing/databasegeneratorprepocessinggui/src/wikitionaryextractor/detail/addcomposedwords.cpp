#include "addcomposedwords.hpp"
#include <QFile>
#include <fstream>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include "frwiki/frpatternrecognizer.hpp"
#include "frwiki/frwikikeywords.hpp"
#include "wikiutils.hpp"

namespace onsem {

AddComposedWords::AddComposedWords(const PatternRecognizer& pPatternRecognizer)
    : fPatternReco(pPatternRecognizer) {}

void AddComposedWords::extractDatasFromFile(std::set<Wikitionary_ComposedWord>& pNewComposedWords,
                                            std::ifstream& pWikionaryFile,
                                            const LinguisticIntermediaryDatabase& pInLingDatabase,
                                            SemanticLanguageEnum pLangEnum) const {
    std::string currWord;
    std::string line;
    long numLine = 0;
    while (getline(pWikionaryFile, line)) {
        if (!line.empty()) {
            if (fPatternReco.isTitle(line)) {
                currWord.clear();
                wikiutil::extractWordOfTitleLine(currWord, line);
                if (pLangEnum == SemanticLanguageEnum::FRENCH && !currWord.empty() && currWord[0] == 's') {
                    if (currWord.compare(0, 3, "se ") == 0)
                        xTryToAddPronominalFrenchVerb(
                            pNewComposedWords, pInLingDatabase, currWord.substr(3, currWord.size() - 3));
                    else if (currWord.compare(0, 2, "s'") == 0)
                        xTryToAddPronominalFrenchVerb(
                            pNewComposedWords, pInLingDatabase, currWord.substr(2, currWord.size() - 2));
                } else if (pLangEnum == SemanticLanguageEnum::ENGLISH) {
                    std::size_t endFirstWord = currWord.find_first_of(' ');
                    if (endFirstWord != std::string::npos && endFirstWord + 1 < currWord.size()
                        && currWord.find_first_of(' ', endFirstWord + 1) == std::string::npos) {
                        LingdbDynamicTrieNode* endOfFirstWord =
                            pInLingDatabase.getPointerToEndOfWord(currWord.substr(0, endFirstWord));
                        if (endOfFirstWord != nullptr) {
                            LingdbMeaning* firstWordVerbMeaning = endOfFirstWord->getMeaning(PartOfSpeech::VERB);
                            if (firstWordVerbMeaning != nullptr) {
                                LingdbDynamicTrieNode* endOfSecondWord = pInLingDatabase.getPointerToEndOfWord(
                                    currWord.substr(endFirstWord + 1, currWord.size() - (endFirstWord + 1)));
                                if (endOfSecondWord != nullptr) {
                                    LingdbMeaning* subMeaning = endOfSecondWord->getMeaning(PartOfSpeech::PREPOSITION);
                                    if (subMeaning == nullptr) {
                                        subMeaning = endOfSecondWord->getMeaning(PartOfSpeech::ADJECTIVE);
                                    } else if (subMeaning->getLemma()->getWord() == "for"
                                               && firstWordVerbMeaning->getLemma()->getWord() != "look") {
                                        subMeaning = nullptr;
                                    }
                                    if (subMeaning != nullptr) {
                                        Wikitionary_ComposedWord newComposedWord(PartOfSpeech::VERB);
                                        newComposedWord.rootSubMeaning = firstWordVerbMeaning;
                                        newComposedWord.subMeanings.emplace_back(subMeaning,
                                                                                 LinkedMeaningDirection::FORWARD);
                                        pNewComposedWords.insert(newComposedWord);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        ++numLine;
        if (numLine % 10000000 == 0)
            std::cout << "numLine: " << numLine / 10000000 << "0 000 000" << std::endl;
    }
    pWikionaryFile.close();
}

void AddComposedWords::writeNewComposedWords(const std::set<Wikitionary_ComposedWord>& pNewComposedWords,
                                             const std::string& poutFile) {
    QDomDocument document;
    QDomElement rootXml = document.createElement("linguisticdatabase");
    document.appendChild(rootXml);

    for (std::set<Wikitionary_ComposedWord>::const_iterator it = pNewComposedWords.begin();
         it != pNewComposedWords.end();
         ++it) {
        QDomElement newWord = document.createElement("words");
        xFillMeaningAttriabutes(newWord, it->rootSubMeaning);

        for (auto itSMeaning = it->subMeanings.begin(); itSMeaning != it->subMeanings.end(); ++itSMeaning) {
            QDomElement newSubMeaning;
            switch (itSMeaning->second) {
                case LinkedMeaningDirection::FORWARD: newSubMeaning = document.createElement("nextMeaning"); break;
                case LinkedMeaningDirection::BACKWARD: newSubMeaning = document.createElement("prevMeaning"); break;
                case LinkedMeaningDirection::BOTH:
                default: newSubMeaning = document.createElement("nextOrPrevMeaning"); break;
            }
            xFillMeaningAttriabutes(newSubMeaning, itSMeaning->first);
            newWord.appendChild(newSubMeaning);
        }

        QDomElement newWordForm = document.createElement("wordForm");
        newWordForm.setAttribute("gram", QString::fromUtf8(partOfSpeech_toStr(it->newGram).c_str()));
        newWord.appendChild(newWordForm);

        rootXml.appendChild(newWord);
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

void AddComposedWords::xFillMeaningAttriabutes(QDomElement& pMeaningElt, LingdbMeaning* pMeaning) {
    assert(pMeaning != nullptr);
    pMeaningElt.setAttribute("gram", QString::fromUtf8(partOfSpeech_toStr(pMeaning->getPartOfSpeech()).c_str()));
    pMeaningElt.setAttribute("lemme", QString::fromUtf8(pMeaning->getLemma()->getWord().c_str()));
}

void AddComposedWords::xTryToAddPronominalFrenchVerb(std::set<Wikitionary_ComposedWord>& pNewComposedWords,
                                                     const LinguisticIntermediaryDatabase& pInLingDatabase,
                                                     const std::string& pRootVerb) {
    LingdbDynamicTrieNode* endOfWord = pInLingDatabase.getPointerToEndOfWord(pRootVerb);
    if (endOfWord != nullptr) {
        const ForwardPtrList<LingdbWordForms>* wfs = endOfWord->getWordForms();
        while (wfs != nullptr) {
            if (wfs->elt->getMeaning()->getPartOfSpeech() == PartOfSpeech::VERB) {
                LingdbMeaning* seMeaning = pInLingDatabase.getMeaning("se", PartOfSpeech::PRONOUN_COMPLEMENT);
                assert(seMeaning != nullptr);

                Wikitionary_ComposedWord newComposedWord(PartOfSpeech::VERB);
                newComposedWord.rootSubMeaning = wfs->elt->getMeaning();
                newComposedWord.subMeanings.emplace_back(seMeaning, LinkedMeaningDirection::BOTH);
                pNewComposedWords.insert(newComposedWord);
            }
            wfs = wfs->next;
        }
    }
}

}    // End of namespace onsem
