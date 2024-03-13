#include <onsem/semanticdebugger/loaddbpediatxtmemory.hpp>
#include <iostream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>

namespace onsem {

void loadDbPediaMemory(std::size_t& pNbOfInforms,
                       std::set<std::string>& pProperNouns,
                       SemanticMemory& pSemanticMemory,
                       const linguistics::LinguisticDatabase& pLingDb,
                       const std::string& pFilename,
                       std::unique_ptr<std::ofstream>& pTextReplacedFilePtr,
                       bool pAddReferences) {
    std::cout << "input filename : " << pFilename << std::endl;
    if (pTextReplacedFilePtr)
        *pTextReplacedFilePtr << "input filename : " << pFilename << std::endl;

    std::ifstream infile(pFilename.c_str(), std::ifstream::in);
    if (!infile.is_open())
        throw std::runtime_error("cannot open file \"" + pFilename + "\"");

    pSemanticMemory.memBloc.maxNbOfExpressionsInAMemoryBlock = SemanticMemoryBlock::infinteMemory;

    mystd::observable::Connection grdExpDisabledByGrdExpConnection;
    if (pTextReplacedFilePtr) {
        grdExpDisabledByGrdExpConnection = pSemanticMemory.memBloc.grdExpReplacedGrdExps.connectUnsafe(
            [&](const GroundedExpression& pNewGrdExp, const std::list<const GroundedExpression*>& pGrdExpsDisabled) {
                if (pGrdExpsDisabled.empty())
                    return;
                std::string newText;
                TextProcessingContext textProcContext(SemanticAgentGrounding::userNotIdentified,
                                                      SemanticAgentGrounding::userNotIdentified,
                                                      SemanticLanguageEnum::FRENCH);
                converter::semExpToText(newText,
                                        pNewGrdExp.clone(),
                                        textProcContext,
                                        false,
                                        pSemanticMemory.memBloc,
                                        pSemanticMemory.getCurrUserId(),
                                        pLingDb,
                                        nullptr);
                *pTextReplacedFilePtr << "Expression \"" << newText << "\" replaced";

                for (const auto& currGrdExp : pGrdExpsDisabled) {
                    std::string textDisabled;
                    converter::semExpToText(textDisabled,
                                            currGrdExp->clone(),
                                            textProcContext,
                                            false,
                                            pSemanticMemory.memBloc,
                                            pSemanticMemory.getCurrUserId(),
                                            pLingDb,
                                            nullptr);
                    *pTextReplacedFilePtr << "\n\t\"" << textDisabled << "\"";
                }
                *pTextReplacedFilePtr << std::endl;
            });
    }
    pSemanticMemory.memBloc.disableOldContrarySentences = false;
    auto cleanState = [&] {
        infile.close();
        if (pTextReplacedFilePtr)
            pSemanticMemory.memBloc.grdExpReplacedGrdExps.disconnectUnsafe(grdExpDisabledByGrdExpConnection);
        pSemanticMemory.memBloc.disableOldContrarySentences = true;
    };

    try {
        std::string line;
        while (getline(infile, line)) {
            if (!line.empty()) {
                if (line[0] == '#') {
                    std::size_t posEndOfLabel = line.find_first_of('#', 1);
                    if (posEndOfLabel != std::string::npos) {
                        const std::string reference = line.substr(1, posEndOfLabel - 1);
                        std::size_t posBeginOfLanguage = posEndOfLabel + 1;
                        if (line.size() > posBeginOfLanguage) {
                            std::size_t posEndOfLanguage = line.find_first_of('#', posBeginOfLanguage);
                            if (posEndOfLanguage != std::string::npos) {
                                std::size_t posBeginOfRevelance = posEndOfLanguage + 1;
                                std::string langaugeStr =
                                    line.substr(posBeginOfLanguage, posEndOfLanguage - posBeginOfLanguage);
                                SemanticLanguageEnum language =
                                    langaugeStr == "fr" ? SemanticLanguageEnum::FRENCH : SemanticLanguageEnum::ENGLISH;

                                std::size_t posEndORevelance = line.find_first_of('#', posBeginOfRevelance);
                                if (posEndORevelance != std::string::npos) {
                                    std::size_t posBeginOfText = posEndORevelance + 1;
                                    std::string text = line.substr(posBeginOfText, line.size() - posBeginOfText);
                                    TextProcessingContext textProcContext(SemanticAgentGrounding::userNotIdentified,
                                                                          SemanticAgentGrounding::userNotIdentified,
                                                                          language);
                                    linguistics::extractProperNounsThatDoesntHaveAnyOtherGrammaticalTypes(
                                        pProperNouns, reference, language, pLingDb);
                                    const std::list<std::string> references{1, reference};
                                    auto agentWeAreTalkingAbout =
                                        SemanticMemoryBlock::generateNewAgentGrd(reference, language, pLingDb);
                                    auto semExp = converter::textToSemExp(
                                        text,
                                        textProcContext,
                                        pLingDb,
                                        false,
                                        nullptr,
                                        nullptr,
                                        pAddReferences ? &references : nullptr,
                                        std::make_unique<SemanticAgentGrounding>(*agentWeAreTalkingAbout));
                                    memoryOperation::addAgentInterpretations(semExp, pSemanticMemory, pLingDb);
                                    SemExpModifier::removeSemExpPartsThatDoesntHaveAnAgent(semExp,
                                                                                           *agentWeAreTalkingAbout);
                                    memoryOperation::inform(std::move(semExp), pSemanticMemory, pLingDb);
                                    ++pNbOfInforms;
                                    if (pNbOfInforms % 1000 == 0)
                                        std::cout << "nbOfInforms: " << pNbOfInforms << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    } catch (...) {
        cleanState();
        throw;
    }
    cleanState();
}

}    // End of namespace onsem
