#include "../frgrammarbookextractor.hpp"
#include <iostream>
#include "helpers.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>

namespace onsem {

void _printCurrVerb(std::ofstream& pInfosFile,
                    const std::string& pCurrVerb,
                    const std::set<FrGrammarBookExtHelpers::PrepToObjectType>& pPrepsToObjectType) {
    pInfosFile << "#" << pCurrVerb << std::endl;
    for (std::set<FrGrammarBookExtHelpers::PrepToObjectType>::const_iterator itPO = pPrepsToObjectType.begin();
         itPO != pPrepsToObjectType.end();
         ++itPO) {
        pInfosFile << "___" << itPO->prep << ":" << FrGrammarBookObjectType_toStr[itPO->objType] << std::endl;
    }
}

void grammarBookToInfosFile(const std::string& pGrammarFilename,
                            const std::string& pInfosFilename,
                            const std::string& pVerbsprepsXmlFilename,
                            const LingdbTree& pLingDbTree,
                            const linguistics::LinguisticDatabase& pLingDb) {
    const linguistics::StaticLinguisticDictionary& staticBinDico =
        pLingDb.langToSpec[SemanticLanguageEnum::FRENCH].lingDico.statDb;
    FrGrammarBookExtHelpers helpers(staticBinDico, pLingDbTree);

    std::ifstream grammarFile(pGrammarFilename.c_str(), std::ifstream::in);
    if (!grammarFile.is_open()) {
        std::cerr << "Error: Can't open " << pGrammarFilename << " file !" << std::endl;
        return;
    }

    std::list<FrGrammarBookExtHelpers::VerbToPreps> vbToPrepsList;
    FrGrammarBookExtHelpers::VerbToPreps vbToPreps;
    std::string line;
    while (getline(grammarFile, line)) {
        if (line.empty() || line == "\r" || line.compare(0, pageSeparatorStr.size(), pageSeparatorStr) == 0
            || !helpers.removeBlanksAtBeginning(line, 50)) {
            continue;
        }

        helpers.removeBackSlashRAtEnd(line);
        helpers.replaceSomeCharsToBlanck(line);

        std::list<std::string> lines;
        helpers.removeAllSpacesAndSplit(lines, line);
        for (std::list<std::string>::iterator itLine = lines.begin(); itLine != lines.end(); ++itLine) {
            std::string& subLine = *itLine;

            bool beginWithArrow = subLine.compare(0, arrowStr.size(), arrowStr) == 0;
            if (!beginWithArrow) {
                helpers.toLowerCase(subLine);

                std::list<std::string> verbs;
                bool findVerb = helpers.findVerb(verbs, subLine);
                if (findVerb) {
                    if (!vbToPreps.currVerbs.empty()) {
                        vbToPrepsList.emplace_back(vbToPreps);
                    }
                    if (!verbs.empty()) {
                        vbToPreps.currVerbs = verbs;
                    } else {
                        std::cout << "not a pronominal verb: " << subLine << std::endl;
                        vbToPreps.currVerbs.clear();
                    }
                    vbToPreps.prepsToObjectType.clear();
                }
            } else if (!vbToPreps.currVerbs.empty()) {
                helpers.removeArrowAtBeginning(subLine);
                helpers.removeAfterDoublePoint(subLine);
                helpers.extractPrepsToObjectType(vbToPreps.prepsToObjectType, subLine);
            }
        }
    }

    if (!vbToPreps.currVerbs.empty()) {
        vbToPrepsList.emplace_back(vbToPreps);
    }
    grammarFile.close();

    helpers.removeVerbsThatOccur2Times(vbToPrepsList);
    std::ofstream infosFile(pInfosFilename.c_str());
    helpers.writeMostFrequentAfterPrep(infosFile, vbToPrepsList);
    for (std::list<FrGrammarBookExtHelpers::VerbToPreps>::const_iterator it = vbToPrepsList.begin();
         it != vbToPrepsList.end();
         ++it) {
        for (std::list<std::string>::const_iterator itVb = it->currVerbs.begin(); itVb != it->currVerbs.end(); ++itVb) {
            _printCurrVerb(infosFile, *itVb, it->prepsToObjectType);
        }
    }
    infosFile.close();

    helpers.writeVerbsPrepsXmlFile(pVerbsprepsXmlFilename, vbToPrepsList);
}

namespace frgrammarbookextractor {
void run(const LingdbTree& pLingDbTree,
         const std::string& pShareDbFolder,
         const linguistics::LinguisticDatabase& pLingDb) {
    const auto verbsprepsXmlFilename = pShareDbFolder + "/semantic/relations/semantic_frames/readonly/french.xml";
    const auto grammarFilename =
        pShareDbFolder + "/databasegeneratorpreprocessing/frgrammarbookextractor/in/verbtoprep.txt";
    const auto infosFilename =
        pShareDbFolder + "/databasegeneratorpreprocessing/frgrammarbookextractor/in/infosFile.txt";

    grammarBookToInfosFile(grammarFilename, infosFilename, verbsprepsXmlFilename, pLingDbTree, pLingDb);
}

}    // End of namespace frgrammarbookextractor

}    // End of namespace onsem
