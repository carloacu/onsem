#include "../textextractor.hpp"
#include <thread>
#include <filesystem>
#include <fstream>
#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>
#include <onsem/texttosemantic/dbtype/linguisticanalysisconfig.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include "adjbeforenounextractor.hpp"
#include "verbcanbefollowedbydeinfr.hpp"
#include "verbprepextractor.hpp"

namespace onsem {
const int textExtractorNbOfThreads = 8;

void _processTexts(AdjBeforeNounExtractor& pAdjBefNoun,
                   VerbCanBeFollowedByDeInFr& pVerbCanBeFollowedByDeInFr,
                   VerbPrepExtractor& pVerbprepExt,
                   const std::list<std::string>& pTexts,
                   const linguistics::LinguisticDatabase& pLingDb,
                   SemanticLanguageEnum pLanguageType,
                   bool pPrintLineNumber) {
    static const std::size_t stepLinesToPrint = 10000;
    LinguisticAnalysisConfig linguisticAnalysisConfig;

    std::size_t currLine = 0;
    for (const auto& currText : pTexts) {
        linguistics::SyntacticGraph syntGraph(pLingDb, pLanguageType);
        linguistics::tokenizeText(
            syntGraph.tokensTree, syntGraph.langConfig, currText, std::shared_ptr<ResourceGroundingExtractor>());
        pAdjBefNoun.processText(syntGraph.tokensTree);
        linguistics::SynthAnalEndingStepForDebug structForDebug;
        linguistics::syntacticAnalysis(syntGraph, linguisticAnalysisConfig, structForDebug);
        pVerbCanBeFollowedByDeInFr.processText(syntGraph, currText);
        pVerbprepExt.processText(syntGraph, currText);

        ++currLine;
        if (pPrintLineNumber && currLine % stepLinesToPrint == 0)
            std::cout << "currLine: " << currLine * textExtractorNbOfThreads << std::endl;
    }
}

void textsToResults(const LingdbTree& pLingDbTree,
                    const std::string& pResultVerbFollByDeFilename,
                    const std::string& pResultAdjFilename,
                    const std::string& pResultVerbPrepFilename,
                    const linguistics::LinguisticDatabase& pLingDb,
                    const std::string& pShareSemanticPath) {
    const std::string textFilename =
        pShareSemanticPath + "/semantic/corpus/input/french/big/french_sentences_from_srts.txt";
    SemanticLanguageEnum langEnum = SemanticLanguageEnum::FRENCH;

    LinguisticIntermediaryDatabase lingIntDb;
    lingIntDb.setLanguage(semanticLanguageEnum_toLegacyStr(langEnum));
    lingIntDb.load(pLingDbTree.getDynamicDatabasesFolder() + "/" + lingIntDb.getLanguage()->toStr() + "."
                   + pLingDbTree.getExtDynDatabase());

    std::cout << "Extrtact from life: " << textFilename << std::endl;
    std::ifstream textFile(textFilename.c_str(), std::ifstream::in);
    if (!textFile.is_open()) {
        std::cerr << "Error: Can't open " << textFilename << " file !" << std::endl;
        return;
    }

    std::vector<std::list<std::string>> textsToProccess(textExtractorNbOfThreads);

    std::size_t textsToProccess_size = textsToProccess.size();
    std::size_t currSentenceSet = 0;
    std::string line;
    while (getline(textFile, line)) {
        textsToProccess[currSentenceSet++].push_back(line);
        if (currSentenceSet >= textsToProccess_size)
            currSentenceSet = 0;
    }
    textFile.close();

    std::vector<std::thread> workers;
    std::vector<AdjBeforeNounExtractor> adjBefNouns(textExtractorNbOfThreads, lingIntDb.getFPAlloc());
    std::vector<VerbCanBeFollowedByDeInFr> verbCanBeFollowedByDeInFrs(textExtractorNbOfThreads, lingIntDb.getFPAlloc());
    std::vector<VerbPrepExtractor> verbprepExts(textExtractorNbOfThreads);
    for (std::size_t i = 0; i < textExtractorNbOfThreads; ++i)
        workers.push_back(std::thread(_processTexts,
                                      std::ref(adjBefNouns[i]),
                                      std::ref(verbCanBeFollowedByDeInFrs[i]),
                                      std::ref(verbprepExts[i]),
                                      std::ref(textsToProccess[i]),
                                      std::ref(pLingDb),
                                      langEnum,
                                      i == 0));
    for (auto& th : workers)
        th.join();

    VerbCanBeFollowedByDeInFr verbCanBeFollowedByDeInFr(verbCanBeFollowedByDeInFrs);
    verbCanBeFollowedByDeInFr.writeResults(pResultVerbFollByDeFilename);
    AdjBeforeNounExtractor adjBefNoun(adjBefNouns);
    adjBefNoun.writeResults(pResultAdjFilename);
    VerbPrepExtractor verbprepExt(verbprepExts);
    verbprepExt.writeResults(pResultVerbPrepFilename);
}

void resultsToXmls(const std::string& pResultVerbFollByDeFilename,
                   const std::string& pResultAdjFilename,
                   const std::string& pInputResourcesFolder) {
    const std::string resultVerbFollByDeFilenameXml =
        pInputResourcesFolder + "/french/readonly/french_contextinfos_verbFollowedByDe.xml";
    const std::string resultAdjFilenameXml =
        pInputResourcesFolder + "/french/readonly/french_contextinfos_canbebeforenoun.xml";

    VerbCanBeFollowedByDeInFr::writeXml(pResultVerbFollByDeFilename, resultVerbFollByDeFilenameXml);
    AdjBeforeNounExtractor::writeXml(pResultAdjFilename, resultAdjFilenameXml);
}

namespace textextractor {

void run(const LingdbTree& pLingDbTree,
         const std::string& pTmpFolder,
         const linguistics::LinguisticDatabase& pLingDb,
         const std::string& pShareSemanticPath,
         const std::string& pInputResourcesFolder) {
    const auto resultFolderPath = pTmpFolder + "/resultOf_textextractor";
    std::filesystem::create_directory(resultFolderPath);
    const std::string resultFolder = resultFolderPath + "/";
    const std::string resultVerbFollByDeFilename = resultFolder + "result_de.txt";
    const std::string resultAdjFilename = resultFolder + "result_adj.txt";
    const std::string resultVerbPrepFilename = resultFolder + "result_verbprep.txt";

    onsem::textsToResults(pLingDbTree,
                          resultVerbFollByDeFilename,
                          resultAdjFilename,
                          resultVerbPrepFilename,
                          pLingDb,
                          pShareSemanticPath);
    onsem::resultsToXmls(resultVerbFollByDeFilename, resultAdjFilename, pInputResourcesFolder);
}

}    // End of namespace textextractor

}    // End of namespace onsem
