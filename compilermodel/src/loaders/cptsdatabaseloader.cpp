#include "cptsdatabaseloader.hpp"
#include <assert.h>
#include <fstream>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>

namespace onsem {

void CptsDatabaseLoader::merge(const std::string& pFilename, LinguisticIntermediaryDatabase& pLingdb) {
    std::ifstream infile(pFilename, std::ifstream::in);
    if (!infile.is_open()) {
        std::cerr << "Error: Can't open " << pFilename << " file !" << std::endl;
        return;
    }

    std::string line;
    LingdbConcept const* currConcept = nullptr;
    std::string currConceptStr;
    LingdbConcept const* contraryConcept = nullptr;
    std::string contraryConceptStr;
    while (getline(infile, line)) {
        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            std::size_t tabPos = line.find_first_of("\t", 1);
            if (tabPos == std::string::npos) {
                std::cerr << "Error: \"\\t\" not found in \"" << line << "\"" << std::endl;
                continue;
            }

            std::size_t beginNb = tabPos + 1;
            assert(line.size() > beginNb);
            std::string relToConceptStr = line.substr(beginNb, line.size() - beginNb);
            char relToConcept = 0;
            try {
                relToConcept = static_cast<char>(mystd::lexical_cast<int>(relToConceptStr));
                if (relToConcept < -5 || relToConcept > 5) {
                    throw std::runtime_error("Error: in line \"" + line + "\" too big relation with the concept");
                }
            } catch (const std::exception& e) {
                throw std::runtime_error("Error: in line \"" + line + "\" with the error: " + e.what());
            }

            std::size_t gramSepPos = line.find_first_of(",", 1);
            if (gramSepPos != std::string::npos) {
                std::string lemma = line.substr(1, gramSepPos - 1);
                std::size_t beginGramPos = gramSepPos + 1;
                if (tabPos <= beginGramPos) {
                    throw std::runtime_error("Error: ',' shoud come before '\t' for line: \"" + line
                                             + "\" and for file: \"" + pFilename + "\"");
                }
                std::string gramStr = line.substr(beginGramPos, tabPos - beginGramPos);
                PartOfSpeech gram = partOfSpeech_fromStr(gramStr);
                if (gram == PartOfSpeech::UNKNOWN) {
                    throw std::runtime_error("Error: unknown grammatical type: \"" + gramStr + "\" for line: \"" + line
                                             + "\" and for file: \"" + pFilename + "\"");
                }

                LingdbMeaning* meaningToHandle = pLingdb.getMeaning(lemma, gram);
                if (meaningToHandle != nullptr) {
                    meaningToHandle->addLinkToConcept(pLingdb, currConcept, currConceptStr, relToConcept, true);
                }
            } else {
                std::string word = line.substr(1, tabPos - 1);
                if (relToConcept == 0) {
                    _fillConcept(pLingdb, word, currConcept, currConceptStr, 0, pFilename, line);
                    _fillConcept(pLingdb, word, contraryConcept, contraryConceptStr, 0, pFilename, line);
                } else if (relToConcept > 0) {
                    _fillConcept(pLingdb, word, currConcept, currConceptStr, relToConcept, pFilename, line);
                } else if (relToConcept < 0) {
                    _fillConcept(pLingdb,
                                 word,
                                 contraryConcept,
                                 contraryConceptStr,
                                 static_cast<char>(-relToConcept),
                                 pFilename,
                                 line);
                }
            }
        } else if (line[0] == '[') {
            const std::string conceptLabel = "[concept: ";
            const std::string contraryConceptLabel = "[contraryconcept: ";
            if (line.size() > conceptLabel.size() && line.compare(0, conceptLabel.size(), conceptLabel) == 0) {
                currConceptStr = line.substr(conceptLabel.size(), line.size() - conceptLabel.size());
                currConcept = pLingdb.getConcept(currConceptStr);
                if (currConcept == nullptr) {
                    currConceptStr.clear();
                    throw std::runtime_error("Error: in \"" + pFilename + "\" in the line: \"" + line
                                             + "\" the concept doesn't exists");
                }
            } else if (line.size() > contraryConceptLabel.size()
                       && line.compare(0, contraryConceptLabel.size(), contraryConceptLabel) == 0) {
                contraryConceptStr = line.substr(contraryConceptLabel.size(), line.size() - conceptLabel.size());
                contraryConcept = pLingdb.getConcept(contraryConceptStr);
                if (contraryConcept == nullptr) {
                    contraryConceptStr.clear();
                    throw std::runtime_error("Error: in \"" + pFilename + "\" in the line: \"" + line
                                             + "\" the concept doesn't exists");
                }
            } else {
                std::cerr << "the line \"" << line
                          << "\" has to begin with \"[concept : \" and be followed by a concept name" << std::endl;
            }
        } else {
            std::cerr << "Error: in " << pFilename << " a line don't begins with \"#\"" << std::endl;
        }
    }

    infile.close();
}

void CptsDatabaseLoader::_fillConcept(LinguisticIntermediaryDatabase& pLingdb,
                                      const std::string& pWord,
                                      const LingdbConcept* pNewConcept,
                                      const std::string& pNewConceptStr,
                                      char pRelatedToConcept,
                                      const std::string& pFilename,
                                      const std::string& pLine) {
    if (pNewConcept == nullptr) {
        std::cerr << "Error: in " << pFilename << " the line: " << pLine << " has no associated concept" << std::endl;
        return;
    }
    LingdbDynamicTrieNode* ptrEndOfWord = pLingdb.getPointerToEndOfWord(pWord);
    if (ptrEndOfWord != nullptr) {
        const ForwardPtrList<LingdbWordForms>* itWordForms = ptrEndOfWord->getWordForms();
        while (itWordForms != nullptr) {
            itWordForms->elt->getMeaning()->addLinkToConcept(
                pLingdb, pNewConcept, pNewConceptStr, pRelatedToConcept, true);
            itWordForms = itWordForms->next;
        }
    }
}

}    // End of namespace onsem
