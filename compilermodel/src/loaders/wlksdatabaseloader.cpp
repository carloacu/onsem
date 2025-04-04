#include <onsem/compilermodel/loaders/wlksdatabaseloader.hpp>
#include <sstream>
#include <fstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include "../concept/lingdbconcept.hpp"
#include "../concept/lingdblinktoaconcept.hpp"

namespace onsem {

void WlksDatabaseLoader::loadAndSave(const std::filesystem::path& pFilename, const LingdbTree& pLingbTree) const {
    WlksDatabaseLoader_WorkState workState(pLingbTree);
    load(workState, pFilename);

    for (auto it = workState.strToLangSpecs.begin(); it != workState.strToLangSpecs.end(); ++it) {
        it->second.lingDatabase->save(workState.lingbTree.getDynDbFilenameForLanguage(it->first));
    }
}

void _fillConceptsWithTranslations(WlksDatabaseLoader::WlksDatabaseLoader_TradSpec& pTradSpec,
                                   const std::string& pConceptStr,
                                   const LingdbConcept* pOutConcept,
                                   char pConceptConfidence,
                                   LingdbMeaning* pMeaning) {
    // in meanings to out meanings
    auto itOutMeaningsList = pTradSpec.traductions.find(pMeaning);
    if (itOutMeaningsList != pTradSpec.traductions.end())
        // out meanings
        for (auto& currOutMeaning : itOutMeaningsList->second)
            // if we have a confidence of 10 we add translated word in the same concept
            if (currOutMeaning.confidence >= 10
                && pMeaning->getPartOfSpeech() == currOutMeaning.meaning->getPartOfSpeech())
                currOutMeaning.meaning->addLinkToConcept(
                    *pTradSpec.outLingDb.lingDatabase, pOutConcept, pConceptStr, pConceptConfidence, false);
}

void WlksDatabaseLoader::load(WlksDatabaseLoader_WorkState& pWorkState, const std::filesystem::path& pFilename) const {
    std::ifstream infile(pFilename, std::ifstream::in);
    if (!infile.is_open()) {
        throw std::runtime_error("Can't open: \"" + pFilename.string() + "\" file !");
    }

    std::string line;
    while (getline(infile, line)) {
        if (line.empty()) {
            continue;
        }

        if (line[0] == '<') {
            switch (pWorkState.nextLinesSpec) {
                case NextLineSpec::TRADUCTION: {
                    WlksDatabaseLoader_TradSpec& tradSpec = pWorkState.tradSpecs.back();
                    LingdbMeaning* inMeaning = nullptr;
                    char confidence = 0;
                    std::size_t currPos = 0;
                    xGetNextMeaningInLine(&inMeaning, confidence, currPos, *tradSpec.inLingDb.lingDatabase, line);
                    if (inMeaning == nullptr) {
                        continue;
                    }
                    LingdbMeaning* outMeaning = nullptr;
                    xGetNextMeaningInLine(&outMeaning, confidence, currPos, *tradSpec.outLingDb.lingDatabase, line);
                    while (outMeaning != nullptr) {
                        MeaningAndConfidence newMeaningAndConfidence(outMeaning, confidence);
                        std::set<MeaningAndConfidence>& trads = tradSpec.traductions[inMeaning];
                        for (std::set<MeaningAndConfidence>::iterator itExistingMeaning = trads.begin();
                             itExistingMeaning != trads.end();
                             ++itExistingMeaning) {
                            if (itExistingMeaning->meaning == outMeaning) {
                                newMeaningAndConfidence.confidence = static_cast<char>(
                                    newMeaningAndConfidence.confidence + itExistingMeaning->confidence);
                                trads.erase(itExistingMeaning);
                                break;
                            }
                        }
                        trads.insert(newMeaningAndConfidence);
                        xGetNextMeaningInLine(&outMeaning, confidence, currPos, *tradSpec.outLingDb.lingDatabase, line);
                    }
                    break;
                }
                default:
                    throw std::runtime_error("No line specification as been befined for line: \"" + line
                                             + "\" in the file: \"" + pFilename.string() + "\".");
            };
        } else if (line[0] == '>') {
            std::istringstream iss(line);
            std::string instruction;
            if (iss) {
                iss >> instruction;
                iss >> instruction;
            }

            if (instruction == "nextswillbe") {
                if (!iss) {
                    throw std::runtime_error("Line too short: \"" + line + "\" of the file \"" + pFilename.string()
                                             + "\"");
                }
                std::string label;
                iss >> label;
                if (!iss) {
                    throw std::runtime_error("Line too short: \"" + line + "\" of the file \"" + pFilename.string()
                                             + "\"");
                }
                if (label == "traduction") {
                    std::string inputLanguage;
                    iss >> inputLanguage;
                    if (!iss) {
                        throw std::runtime_error("Line too short: \"" + line + "\" of the file \"" + pFilename.string()
                                                 + "\"");
                    }
                    std::string outputLanguage;
                    iss >> outputLanguage;
                    if (!iss) {
                        throw std::runtime_error("Line too short: \"" + line + "\" of the file \"" + pFilename.string()
                                                 + "\"");
                    }
                    iss >> outputLanguage;

                    xLoadDynNewDb(pWorkState, inputLanguage);
                    xLoadDynNewDb(pWorkState, outputLanguage);
                    pWorkState.tradSpecs.emplace_back(pWorkState.strToLangSpecs[inputLanguage],
                                                      pWorkState.strToLangSpecs[outputLanguage]);
                    pWorkState.nextLinesSpec = NextLineSpec::TRADUCTION;
                }
            } else if (instruction == "include") {
                if (!iss) {
                    throw std::runtime_error("Line too short: \"" + line + "\" of the file \"" + pFilename.string()
                                             + "\"");
                }
                std::string fileStr;
                iss >> fileStr;
                std::string subFolder;
                pWorkState.lingbTree.getHoldingFolder(subFolder, pFilename.string());
                load(pWorkState, subFolder + "/" + fileStr);
            } else if (instruction == "fillconceptsofotherlanguages") {
                // for every traductions
                for (auto& currTradSpec : pWorkState.tradSpecs) {
                    // in concepts to in meanings
                    for (const auto& currInConcToMeaning : currTradSpec.inLingDb.conceptToMeanings) {
                        // get in concept
                        const LingdbConcept* inConcept = currInConcToMeaning.first;
                        if (!inConcept->isAutoFill())
                            continue;
                        std::string currConceptStr = inConcept->getName()->toStr();

                        // get out concept
                        LingdbConcept* outConcept = currTradSpec.outLingDb.lingDatabase->getConcept(currConceptStr);
                        if (outConcept == nullptr)
                            throw std::runtime_error(
                                "The concept: \""
                                + currTradSpec.outLingDb.lingDatabase->getConcept(inConcept->getName()->toStr())
                                      ->getName()
                                      ->toStr()
                                + "\" exist in \"" + currTradSpec.inLingDb.lingDatabase->getLanguage()->toStr()
                                + "\" but dont exist in \""
                                + currTradSpec.outLingDb.lingDatabase->getLanguage()->toStr() + "\".");

                        // in meanings
                        for (const auto& currInMeaning : currInConcToMeaning.second) {
                            char relatedToConcept = currInMeaning.confidence;
                            if (relatedToConcept > -2 && relatedToConcept < 2)
                                continue;
                            if (relatedToConcept > 0)
                                --relatedToConcept;
                            else
                                ++relatedToConcept;
                            _fillConceptsWithTranslations(
                                currTradSpec, currConceptStr, outConcept, relatedToConcept, currInMeaning.meaning);
                        }
                    }
                }

                std::string englishLanguageStr = "english";
                auto tradSpecsOfEnglish = pWorkState.getTraductionsOfALanguage(englishLanguageStr);
                auto* enLangSpecPtr = pWorkState.getLangSpec(englishLanguageStr);
                if (enLangSpecPtr != nullptr) {
                    auto& enLingDatabase = *enLangSpecPtr->lingDatabase;
                    auto& fpAlloc = enLingDatabase.getFPAlloc();
                    LingdbMeaning* meaningPtr = fpAlloc.first<LingdbMeaning>();
                    while (meaningPtr != nullptr) {
                        PartOfSpeech meaningPartOfSpeech = meaningPtr->getPartOfSpeech();
                        if (partOfSpeech_isAWord(meaningPartOfSpeech) && meaningPartOfSpeech != PartOfSpeech::AUX
                            && meaningPartOfSpeech != PartOfSpeech::CONJUNCTIVE
                            && meaningPartOfSpeech != PartOfSpeech::DETERMINER
                            && meaningPartOfSpeech != PartOfSpeech::SUBORDINATING_CONJONCTION
                            && meaningPartOfSpeech != PartOfSpeech::PARTITIVE
                            && meaningPartOfSpeech != PartOfSpeech::PREPOSITION
                            && !partOfSpeech_isPronominal(meaningPartOfSpeech)) {
                            bool isLinkToASpecificConcept = false;
                            const auto* itConcepts = meaningPtr->getLinkToConcepts();
                            while (itConcepts != nullptr) {
                                if (!LingdbConcept::conceptNameFinishWithAStar(
                                        itConcepts->elt->getConcept()->getName()->toStr())) {
                                    isLinkToASpecificConcept = true;
                                    break;
                                }
                                itConcepts = itConcepts->next;
                            }
                            if (!isLinkToASpecificConcept) {
                                std::string meaningConceptStr = "meaning_";
                                meaningConceptStr += partOfSpeech_toCptName(meaningPartOfSpeech) + ",";
                                meaningConceptStr += meaningPtr->getLemma()->getWord();
                                bool cptInserted = false;

                                LingdbConcept* meaningConcept =
                                    enLingDatabase.addConcept(cptInserted, meaningConceptStr, true);
                                meaningPtr->addLinkToConcept(
                                    enLingDatabase, meaningConcept, meaningConceptStr, 3, false);
                                for (auto& currTrad : tradSpecsOfEnglish) {
                                    LingdbConcept* outConcept = currTrad->outLingDb.lingDatabase->addConcept(
                                        cptInserted, meaningConceptStr, true);
                                    _fillConceptsWithTranslations(
                                        *currTrad, meaningConceptStr, outConcept, 3, meaningPtr);
                                }
                            }
                        }
                        meaningPtr = fpAlloc.next<LingdbMeaning>(meaningPtr);
                    }
                }
            } else {
                throw std::runtime_error("Unknown instruction \"" + instruction + "\" in line: \"" + line
                                         + "\" of the file \"" + pFilename.string() + "\"");
            }
        } else {
            throw std::runtime_error("The line has to begin with \">\" or \"<\", line \"" + line + "\" of the file \""
                                     + pFilename.string() + "\"");
        }
    }
    infile.close();
}

void WlksDatabaseLoader::xLoadDynNewDb(WlksDatabaseLoader_WorkState& pWorkState, const std::string& pLang) const {
    std::map<std::string, WlksDatabaseLoader_LangSpec>::iterator it = pWorkState.strToLangSpecs.find(pLang);
    if (it != pWorkState.strToLangSpecs.end()) {
        return;
    }

    WlksDatabaseLoader_LangSpec& langSpec = pWorkState.strToLangSpecs[pLang];
    langSpec.lingDatabase = std::make_shared<LinguisticIntermediaryDatabase>();
    langSpec.lingDatabase->load(pWorkState.lingbTree.getDynDbFilenameForLanguage(pLang));
    langSpec.lingDatabase->getConceptToMeanings(langSpec.conceptToMeanings);
}

void WlksDatabaseLoader::xGetNextMeaningInLine(LingdbMeaning** pMeaning,
                                               char& pConfidence,
                                               std::size_t& pCurrPos,
                                               const LinguisticIntermediaryDatabase& pLingDb,
                                               const std::string& pLine) const {
    std::size_t beginPos = pLine.find_first_of('<', pCurrPos);
    std::size_t endingPos = pLine.find_first_of('>', beginPos);
    if (beginPos == std::string::npos || endingPos == std::string::npos) {
        *pMeaning = nullptr;
        return;
    }
    pCurrPos = endingPos;

    std::size_t endLemmeName = pLine.find_first_of(',', beginPos);
    while (endLemmeName != std::string::npos && endLemmeName > 0 && pLine[endLemmeName - 1] == '\\') {
        endLemmeName = pLine.find_first_of(',', endLemmeName + 1);
    }
    std::size_t endGramName = pLine.find_first_of(",>", endLemmeName + 1);
    if (endLemmeName == std::string::npos || endGramName == std::string::npos) {
        *pMeaning = nullptr;
        std::cerr << "Meaning badly formatted !" << std::endl;
        return;
    }
    if (endGramName != endingPos) {
        std::size_t endNextName = pLine.find_first_of(",>", endGramName + 1);
        std::size_t begConfidence = (endNextName != endingPos) ? endNextName + 1 : endGramName + 1;
        try {
            pConfidence =
                static_cast<char>(mystd::lexical_cast<int>(pLine.substr(begConfidence, endingPos - begConfidence)));
        } catch (...) {
            pConfidence = 0;
            std::cerr << "Meaning badly formatted (confidence is not a valid number) ! "
                      << "confidence: \"" << pLine.substr(begConfidence, endingPos - begConfidence) << "\" line: \""
                      << pLine << "\"." << std::endl;
        }
    }

    std::string lemmeStr = pLine.substr((beginPos + 1), endLemmeName - (beginPos + 1));
    if (xHasSlash(lemmeStr)) {
        xRemoveSlashesBeforeSpecifcChars(lemmeStr);
    }
    *pMeaning = pLingDb.getMeaning(
        lemmeStr, partOfSpeech_fromStr(pLine.substr(endLemmeName + 1, endGramName - (endLemmeName + 1))));
    if (*pMeaning == nullptr) {
        std::cerr << "Meaning not found, lemma: \"" << lemmeStr << "\", gram: \""
                  << pLine.substr(endLemmeName + 1, endGramName - (endLemmeName + 1)) << "\"." << std::endl;
    }
}

bool WlksDatabaseLoader::xHasSlash(const std::string& pStr) const {
    std::size_t i = pStr.find_first_of('\\');
    return i != std::string::npos;
}

void WlksDatabaseLoader::xRemoveSlashesBeforeSpecifcChars(std::string& pStr) const {
    for (std::size_t i = 0; i < pStr.size(); ++i) {
        if (pStr[i] == '\\' && (i + 1 < pStr.size())) {
            pStr.erase(i, 1);
            xRemoveSlashesBeforeSpecifcChars(pStr);
        }
    }
}

std::size_t WlksDatabaseLoader::WlksDatabaseLoader_WorkState::maxOccupatedSize() const {
    std::size_t res = 0;
    for (const auto& currElt : strToLangSpecs) {
        std::size_t occupatedSize = currElt.second.lingDatabase->getFPAlloc().getOccupatedSize();
        if (res < occupatedSize)
            res = occupatedSize;
    }
    return res;
}

WlksDatabaseLoader::WlksDatabaseLoader_LangSpec* WlksDatabaseLoader::WlksDatabaseLoader_WorkState::getLangSpec(
    const std::string& pLanguage) {
    for (auto& currElt : strToLangSpecs) {
        if (currElt.first == pLanguage)
            return &currElt.second;
    }
    return nullptr;
}

std::list<WlksDatabaseLoader::WlksDatabaseLoader_TradSpec*>
    WlksDatabaseLoader::WlksDatabaseLoader_WorkState::getTraductionsOfALanguage(const std::string& pLanguage) {
    std::list<WlksDatabaseLoader::WlksDatabaseLoader_TradSpec*> res;
    for (auto& currElt : tradSpecs)
        if (currElt.inLingDb.lingDatabase->getLanguage()->toStr() == pLanguage)
            res.emplace_back(&currElt);
    return res;
}

}    // End of namespace onsem
