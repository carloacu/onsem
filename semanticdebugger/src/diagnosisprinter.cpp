#include <onsem/semanticdebugger/diagnosisprinter.hpp>
#include <list>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/binarydatabasessizeprinter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticanalysisconfig.hpp>
#include <onsem/texttosemantic/printer/semlinetoprint.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

namespace onsem {
namespace diagnosisPrinter {
namespace {
const std::size_t _maxNbOfSemanticExpressionsToPrint = 50;

void _printPkgMemory(std::list<SemLineToPrint>& pLines, const linguistics::LinguisticDatabase& pLingDb) {
    std::string text = "";
    binaryDbSizePrinter::printSize(text, pLingDb);
    pLines.emplace_back(text);
}

bool _prettyPrintMemory(std::list<SemLineToPrint>& pLines,
                        const std::string& pMemoryZone,
                        const SemanticMemory& pSemanticMemory) {
    if (pMemoryZone == "memoryInformations") {
        mystd::optional<std::size_t> optMaxNbOfSemanticExpressionsToPrint;
        optMaxNbOfSemanticExpressionsToPrint.emplace(_maxNbOfSemanticExpressionsToPrint);
        pSemanticMemory.memBloc.print(pLines, optMaxNbOfSemanticExpressionsToPrint);
        return true;
    }
    return false;
}

void _print_text_lang_convOutput(std::list<SemLineToPrint>& pLines,
                                 const linguistics::LinguisticDatabase& pLingDb,
                                 SemanticLanguageEnum pLanguageType,
                                 ConvertionOutputEnum pConvOutput,
                                 std::vector<std::string>::const_iterator pItMode,
                                 const std::vector<std::string>& pModes) {
    if (pItMode != pModes.end()) {
        SyntacticAnalysisResultToDisplay autoAnnotToDisplay;
        SemanticAnalysisDebugOptions debugOptions;
        debugOptions.outputFormat = PrintSemExpDiffsOutPutFormat::CONSOLE;
        debugOptions.convOutput = pConvOutput;
        LinguisticAnalysisConfig linguisticAnalysisConfig;
        SemanticDebug::debugTextAnalyze(
            autoAnnotToDisplay, *pItMode, linguisticAnalysisConfig, debugOptions, pLanguageType, pLingDb);
        pLines.emplace_back(autoAnnotToDisplay.highLevelResults.semExpStr);
        return;
    }

    pLines.emplace_back("text => OK");
    pLines.emplace_back("language => OK");
    pLines.emplace_back("thing_to_check => OK");
    pLines.emplace_back("<text> => KO");
}

void _print_text_lang_tokens(std::list<SemLineToPrint>& pLines,
                             const linguistics::LinguisticDatabase& pLingDb,
                             SemanticLanguageEnum pLanguageType,
                             std::vector<std::string>::const_iterator pItMode,
                             const std::vector<std::string>& pModes) {
    if (pItMode != pModes.end()) {
        SyntacticAnalysisResultToDisplay autoAnnotToDisplay;
        SemanticAnalysisDebugOptions debugOptions;
        debugOptions.outputFormat = PrintSemExpDiffsOutPutFormat::CONSOLE;
        debugOptions.convOutput = CONV_OUTPUT_MIND;
        LinguisticAnalysisConfig linguisticAnalysisConfig;
        SemanticDebug::debugTextAnalyze(
            autoAnnotToDisplay, *pItMode, linguisticAnalysisConfig, debugOptions, pLanguageType, pLingDb);

        for (const auto& currTok : autoAnnotToDisplay.tokens) {
            pLines.emplace_back(currTok.second);
        }
        return;
    }

    pLines.emplace_back("text => OK");
    pLines.emplace_back("language => OK");
    pLines.emplace_back("thing_to_check => OK");
    pLines.emplace_back("<text> => KO");
}

void _print_text_lang(std::list<SemLineToPrint>& pLines,
                      const linguistics::LinguisticDatabase& pLingDb,
                      SemanticLanguageEnum pLanguageType,
                      std::vector<std::string>::const_iterator pItMode,
                      const std::vector<std::string>& pModes) {
    if (pItMode != pModes.end()) {
        ConvertionOutputEnum convOutput = convertionOutputEnum_fromStr(*pItMode);
        if (convOutput != CONV_OUTPUT_UNKNOWN) {
            _print_text_lang_convOutput(pLines, pLingDb, pLanguageType, convOutput, ++pItMode, pModes);
            return;
        }

        if (*pItMode == "tokens") {
            _print_text_lang_tokens(pLines, pLingDb, pLanguageType, ++pItMode, pModes);
            return;
        }
    }

    pLines.emplace_back("text => OK");
    pLines.emplace_back("language => OK");
    pLines.emplace_back("thing_to_check => KO");
}

void _print_text(std::list<SemLineToPrint>& pLines,
                 std::vector<std::string>::const_iterator pItMode,
                 const std::vector<std::string>& pModes,
                 const linguistics::LinguisticDatabase& pLingDb) {
    if (pItMode != pModes.end()) {
        SemanticLanguageEnum langType = semanticLanguageTypeGroundingEnumFromStr(*pItMode);
        if (langType == SemanticLanguageEnum::UNKNOWN) {
            std::vector<std::string>::const_iterator nextItMode = pItMode;
            ++nextItMode;
            if (nextItMode != pModes.end()) {
                ++nextItMode;
                if (nextItMode != pModes.end()) {
                    langType = linguistics::getLanguage(*nextItMode, pLingDb);
                }
            }
        }
        _print_text_lang(pLines, pLingDb, langType, ++pItMode, pModes);
        return;
    }

    pLines.emplace_back("text => OK");
    pLines.emplace_back("language => KO");
}

void _printRootLevel(std::list<SemLineToPrint>& pLines,
                     std::vector<std::string>::const_iterator pItMode,
                     const std::vector<std::string>& pModes,
                     const SemanticMemory& pSemanticMemory,
                     const linguistics::LinguisticDatabase& pLingDb) {
    if (pItMode != pModes.end()) {
        if (*pItMode == "loadedDatabases") {
            _printPkgMemory(pLines, pLingDb);
            return;
        }
        if (*pItMode == "memory") {
            ++pItMode;
            if (pItMode == pModes.end() || !_prettyPrintMemory(pLines, *pItMode, pSemanticMemory)) {
                pLines.emplace_back("memory => OK");
                pLines.emplace_back("memory_zone => KO");
            }
            return;
        }

        if (*pItMode == "text") {
            _print_text(pLines, ++pItMode, pModes, pLingDb);
            return;
        }
    }

    pLines.emplace_back("Diagnosis help");
    pLines.emplace_back("==============");
    pLines.emplace_back();
    pLines.emplace_back("loadedDatabases: show the loaded databases");
    pLines.emplace_back("memory: show the memory");
    pLines.emplace_back("text: debug a syntactic analysis");
}

void _diagnosis(std::list<SemLineToPrint>& pLines,
                const std::vector<std::string>& pModes,
                const SemanticMemory& pSemanticMemory,
                const linguistics::LinguisticDatabase& pLingDb) {
    diagnosisPrinter::print(pLines, pModes, pSemanticMemory, pLingDb);
}

}

void getNextParameterPossibilities(std::list<std::string>& pNextParameterPossibilities,
                                   const std::vector<std::string>& pModes) {
    if (pModes.empty()) {
        pNextParameterPossibilities.emplace_back("loadedDatabases");
        pNextParameterPossibilities.emplace_back("memory");
        pNextParameterPossibilities.emplace_back("text");
        return;
    }
    if (pModes.size() == 1) {
        if (pModes[0] == "text") {
            std::list<SemanticLanguageEnum> languageTypes;
            getAllLanguageTypes(languageTypes);
            pNextParameterPossibilities.emplace_back("");
            for (std::list<SemanticLanguageEnum>::const_iterator itLang = languageTypes.begin();
                 itLang != languageTypes.end();
                 ++itLang) {
                pNextParameterPossibilities.push_back(semanticLanguageEnum_toStr(*itLang));
            }
            return;
        }
        if (pModes[0] == "memory") {
            pNextParameterPossibilities.emplace_back("memoryInformations");
            pNextParameterPossibilities.emplace_back("systemInformations");
            return;
        }
        return;
    }
    if (pModes.size() == 2) {
        if (pModes[0] == "text") {
            std::list<ConvertionOutputEnum> convValues;
            convertionOutputEnum_getAll(convValues);
            for (std::list<ConvertionOutputEnum>::const_iterator itConv = convValues.begin();
                 itConv != convValues.end();
                 ++itConv) {
                pNextParameterPossibilities.push_back(ConvertionOutputEnum_toStr[*itConv]);
            }
            pNextParameterPossibilities.emplace_back("rdf");
            pNextParameterPossibilities.emplace_back("rdf_debug");
            pNextParameterPossibilities.emplace_back("tokens");
        }
        return;
    }
    if (pModes.size() == 3) {
        if (pModes[0] == "text") {
            pNextParameterPossibilities.emplace_back("");
        }
        return;
    }
}

void print(std::list<SemLineToPrint>& pLines,
           const std::vector<std::string>& pModes,
           const SemanticMemory& pSemanticMemory,
           const linguistics::LinguisticDatabase& pLingDb) {
    _printRootLevel(pLines, pModes.begin(), pModes, pSemanticMemory, pLingDb);
}

std::string diagnosis(const std::vector<std::string>& pModes,
                      const SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb) {
    std::list<SemLineToPrint> lines;
    _diagnosis(lines, pModes, pSemanticMemory, pLingDb);
    std::string result;
    SemExpLinesToStr::getInstance(PrintSemExpDiffsOutPutFormat::CONSOLE).printLines(result, lines);
    return result;
}

}    // End of namespace diagnosisPrinter
}    // End of namespace onsem
