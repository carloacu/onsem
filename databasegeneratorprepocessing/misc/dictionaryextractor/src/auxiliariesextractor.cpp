#include <onsem/dictionaryextractor/auxiliariesextractor.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <thread>
#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/type/chunklink.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticanalysisconfig.hpp>
#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>

namespace onsem {

AuxiliariesExtractor::AuxiliariesExtractor(const linguistics::LinguisticDatabase& pLingDb)
    : _lingDb(pLingDb)
    , _nbOfThreads(8)
    , _mutex()
    , _vertoToAuxMeaningRes()
    , _verbsWithBeAux()
    , _verbsWithHaveAux() {}

void AuxiliariesExtractor::processFile(const std::string& pInFilename) {
    std::ifstream inFile(pInFilename, std::ifstream::in);
    if (!inFile.is_open()) {
        std::cerr << "Error: Can't open " << pInFilename << " file !" << std::endl;
        return;
    }

    SemanticLanguageEnum language = SemanticLanguageEnum::ENGLISH;
    std::vector<std::list<std::string>> textsToProccess(_nbOfThreads);

    std::size_t textsToProccess_size = textsToProccess.size();
    std::size_t currSentenceSet = 0;
    std::string line;
    while (getline(inFile, line)) {
        textsToProccess[currSentenceSet++].push_back(line);
        if (currSentenceSet >= textsToProccess_size)
            currSentenceSet = 0;
    }
    inFile.close();

    std::vector<std::thread> workers;
    for (std::size_t i = 0; i < _nbOfThreads; ++i) {
        workers.push_back(
            std::thread(&AuxiliariesExtractor::_processTexts, this, textsToProccess[i], language, i == 0));
    }
    for (auto& th : workers) {
        th.join();
    }

    {
        std::ofstream outFile(pInFilename + ".debug.txt");
        for (const auto& currVerbRes : _vertoToAuxMeaningRes) {
            const std::string& lemmaStr = currVerbRes.first.lemma;
            const AuxiliariesLinksResult& currRes = currVerbRes.second;
            if (currRes.nbFoundWithBeAux > 10 && currRes.nbFoundWithBeAux * 2 > currRes.nbFound) {
                _verbsWithBeAux.emplace(lemmaStr);
            }
            if (currRes.nbFoundWithHaveAux * 150 > currRes.nbFound || currRes.nbFoundWithHaveAux > 50) {
                _verbsWithHaveAux.emplace(lemmaStr);
            }
            outFile << "lemma: " << lemmaStr;
            outFile << " nbOccurs(" << currRes.nbFound << ")";
            outFile << " nbFoundWithBeAux(" << currRes.nbFoundWithBeAux << ")";
            outFile << " nbFoundWithHaveAux(" << currRes.nbFoundWithHaveAux << ")\n";
        }
        outFile.close();
    }

    {
        // Write the be auxiliaries results
        std::ofstream outFile(pInFilename + ".be.xml");
        outFile << "<linguisticdatabase>\n";
        for (const auto& currVerbsWithBeAux : _verbsWithBeAux) {
            outFile << "<existingMeaning lemme=\"" << currVerbsWithBeAux << "\" gram=\"verb\">\n";
            outFile << "  <contextInfo val=\"beIsTheAuxiliary\"/>\n";
            outFile << "</existingMeaning>\n";
        }
        outFile << "</linguisticdatabase>\n";
        outFile.close();
    }
    {
        // Write the have auxiliaries results
        std::ofstream outFile(pInFilename + ".have.xml");
        outFile << "<linguisticdatabase>\n";
        for (const auto& currVerbsWithHaveAux : _verbsWithHaveAux) {
            outFile << "<existingMeaning lemme=\"" << currVerbsWithHaveAux << "\" gram=\"verb\">\n";
            outFile << "  <contextInfo val=\"haveIsTheAuxiliary\"/>\n";
            outFile << "</existingMeaning>\n";
        }
        outFile << "</linguisticdatabase>\n";
        outFile.close();
    }

    std::cout << std::endl;
    std::cout << "nbHaveBe: " << _verbsWithBeAux.size() << std::endl;
    std::cout << "nbHaveAux: " << _verbsWithHaveAux.size() << std::endl;
}

const linguistics::Chunk* _getAuxiliaryChunk(const linguistics::Chunk& pVerbChunk) {
    for (const auto& currChild : pVerbChunk.children)
        if (currChild.type == linguistics::ChunkLinkType::AUXILIARY)
            return &*currChild.chunk;
    return nullptr;
}

void AuxiliariesExtractor::_processTexts(const std::list<std::string>& pTexts,
                                         SemanticLanguageEnum pLanguage,
                                         bool pPrintLineNumber) {
    const linguistics::SpecificLinguisticDatabase& specLingDb = _lingDb.langToSpec[pLanguage];
    LinguisticAnalysisConfig linguisticAnalysisConfig;

    std::size_t i = 0;
    for (const auto& currText : pTexts) {
        linguistics::SyntacticGraph syntGraph(_lingDb, pLanguage);
        linguistics::tokenizationAndSyntacticalAnalysis(syntGraph, currText, linguisticAnalysisConfig);

        for (const linguistics::ChunkLink& currChkLk : syntGraph.firstChildren) {
            const linguistics::Chunk& chunk = *currChkLk.chunk;
            if (chunk.type == linguistics::ChunkType::VERB_CHUNK) {
                const linguistics::InflectedWord& verbIGram = chunk.head->inflWords.front();
                if (!linguistics::InflectionsChecker::verbIsAtPresentParticiple(verbIGram)) {
                    if (verbIGram.word.language == pLanguage) {
                        std::lock_guard<std::mutex> lock(_mutex);
                        AuxiliariesLinksResult& auxRes = _vertoToAuxMeaningRes[verbIGram.word];
                        ++auxRes.nbFound;
                        const linguistics::Chunk* auxChk = _getAuxiliaryChunk(chunk);
                        if (auxChk != nullptr) {
                            const SemanticWord& auxWord = auxChk->head->inflWords.front().word;
                            if (auxWord == specLingDb.lingDico.getBeAux().word)
                                ++auxRes.nbFoundWithBeAux;
                            else if (auxWord == specLingDb.lingDico.getHaveAux().word)
                                ++auxRes.nbFoundWithHaveAux;
                        }
                    }
                }
            }
        }
        ++i;
        if (pPrintLineNumber) {
            if (i % 10000 == 0)
                std::cout << i * _nbOfThreads << std::endl;
        }
    }
}

}
