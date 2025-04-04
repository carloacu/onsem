#include <onsem/semanticdebugger/textananlyzedebugger.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/printer/expressionprinter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpsimplifer.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/tool/iscomplete.hpp>
#include <onsem/semanticdebugger/printer/semanticprinter.hpp>
#include <onsem/semanticdebugger/aretextsequivalent.hpp>
#include <onsem/semanticdebugger/dotsaver.hpp>
#include <onsem/semanticdebugger/timechecker.hpp>

namespace onsem {
namespace linguistics {
namespace TextAnalyzeDebugger {

namespace {

std::string _printParsingConfidence(const ParsingConfidence& pParsingConfidence) {
    std::stringstream ss;
    if (pParsingConfidence.nbOfNotUnderstood > 0)
        ss << "nbOfNotUnderstood: " << pParsingConfidence.nbOfNotUnderstood << "\n";
    if (pParsingConfidence.nbOfProblematicRetries > 0)
        ss << "nbOfProblematicRetries: " << pParsingConfidence.nbOfProblematicRetries << "\n";
    if (pParsingConfidence.nbOfSuspiciousChunks > 0)
        ss << "nbOfSuspiciousChunks: " << pParsingConfidence.nbOfSuspiciousChunks << "\n";
    if (pParsingConfidence.nbOfTransitveVerbsWithoutDirectObject > 0)
        ss << "nbOfTransitveVerbsWithoutDirectObject: " << pParsingConfidence.nbOfTransitveVerbsWithoutDirectObject
           << "\n";
    ss << "\nscore: " << static_cast<int>(pParsingConfidence.toPercentage()) << "\n";
    return ss.str();
}

void _syntSemExpWithDebugInfosIfAsked(std::string& pRes,
                                      std::string& pSemExpPrettyPrint,
                                      UniqueSemanticExpression pSemExp,
                                      const SemExpLinesToStr& pPrintSemExpDiffs,
                                      SemanticLanguageEnum pLanguage,
                                      const SemanticAnalysisDebugOptions& pSemanticAnalysisDebugOptions,
                                      const SemanticMemoryBlock& pMemBlock,
                                      const LinguisticDatabase& pLingDb) {
    TextProcessingContext textPorcContext(SemanticAgentGrounding::getCurrentUser(), SemanticAgentGrounding::getMe(), pLanguage);
    textPorcContext.writeParametersToFill = true;
    if (pSemanticAnalysisDebugOptions.setUsAsEverybody)
        textPorcContext.setUsAsEverybody();

    if ((pLanguage == SemanticLanguageEnum::FRENCH
         && pSemanticAnalysisDebugOptions.convOutput == CONV_OUTPUT_MIND_TO_FRENCH)
        || (pLanguage == SemanticLanguageEnum::ENGLISH
            && pSemanticAnalysisDebugOptions.convOutput == CONV_OUTPUT_MIND_TO_ENGLISH)) {
        std::list<std::list<SemLineToPrint>> convOutputs;
        converter::semExpToText(pRes,
                                std::move(pSemExp),
                                textPorcContext,
                                true,
                                pMemBlock,
                                textPorcContext.author.userId,
                                pLingDb,
                                &convOutputs);
        pPrintSemExpDiffs.printAlternativelySemExpAndDiffOfSemExps(pSemExpPrettyPrint, convOutputs);
    } else {
        converter::semExpToText(pRes,
                                std::move(pSemExp),
                                textPorcContext,
                                true,
                                pMemBlock,
                                textPorcContext.author.userId,
                                pLingDb,
                                nullptr);
    }
}

}

void saveGramPossibilities(std::list<std::list<std::string>>& pGramPossibilitiesForEachToken,
                           const TokensTree& pTokensTree) {
    for (ConstTokenIterator itTokens = pTokensTree.beginToken(); !itTokens.atEnd(); ++itTokens) {
        if (partOfSpeech_isAWord(itTokens.getToken().inflWords.begin()->word.partOfSpeech)) {
            pGramPossibilitiesForEachToken.emplace_back();
            std::list<std::string>& grams = pGramPossibilitiesForEachToken.back();
            for (const auto& currGram : itTokens.getToken().inflWords) {
                std::stringstream ss;
                if (!currGram.word.lemma.empty()) {
                    ss << partOfSpeech_toStr(currGram.word.partOfSpeech) << ", " << currGram.word.lemma;
                } else {
                    ss << partOfSpeech_toStr(currGram.word.partOfSpeech);
                }

                ss << " (";
                currGram.inflections().concisePrint(ss);
                ss << ")";
                grams.emplace_back(ss.str());
            }
        }
    }
}

void fillSemAnalResult(SyntacticGraphResult& pResults,
                       SemanticAnalysisHighLevelResults& pHighLevelResults,
                       LinguisticAnalysisConfig& pLinguisticAnalysisConfig,
                       const SemanticAnalysisDebugOptions& pSemanticAnalysisDebugOptions) {
    const auto& lingDb = pResults.syntGraph.langConfig.lingDb;
    SemanticLanguageEnum language = pResults.syntGraph.langConfig.getLanguageType();
    const SemExpLinesToStr& semExpLinesToStr =
        SemExpLinesToStr::getInstance(pSemanticAnalysisDebugOptions.outputFormat);

    pLinguisticAnalysisConfig.cmdGrdExtractorPtr =
        std::make_shared<ResourceGroundingExtractor>(std::vector<std::string>{"any_resource", "any_url"});

    // tokenization
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->beginOfTimeSlot();
    linguistics::tokenizeText(
        pResults.syntGraph.tokensTree, pResults.syntGraph.langConfig, pResults.inputText, pLinguisticAnalysisConfig.cmdGrdExtractorPtr);
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->endOfTimeSlot("tokenisation");

    saveGramPossibilities(pHighLevelResults.initialGramPossibilities, pResults.syntGraph.tokensTree);

    // syntactic analysis
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->beginOfTimeSlot();
    linguistics::syntacticAnalysis(
        pResults.syntGraph, pLinguisticAnalysisConfig, pSemanticAnalysisDebugOptions.endingStep);
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->endOfTimeSlot("Syntactic Analysis");

    {
        std::stringstream ss;
        linguistics::DotSaver::writeChunkLinks(ss, pResults.syntGraph.firstChildren);
        pHighLevelResults.syntGraphStr = ss.str();
    }

    pHighLevelResults.parsingConfidenceStr = _printParsingConfidence(pResults.syntGraph.parsingConfidence);

    // semantic expressions pretty printer
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->beginOfTimeSlot();
    UniqueSemanticExpression semExp([&]() {
        TextProcessingContext textProcCont(SemanticAgentGrounding::getCurrentUser(), SemanticAgentGrounding::getMe(), language);
        if (pSemanticAnalysisDebugOptions.setUsAsEverybody)
            textProcCont.setUsAsEverybody();
        textProcCont.linguisticAnalysisConfig = pLinguisticAnalysisConfig;
        if (pSemanticAnalysisDebugOptions.convOutput == CONV_OUTPUT_CURRLANG_TO_MIND) {
            std::list<std::list<SemLineToPrint>> convOutputs;
            auto res = converter::syntGraphToSemExp(pResults.syntGraph, textProcCont, &convOutputs);
            semExpLinesToStr.printAlternativelySemExpAndDiffOfSemExps(pHighLevelResults.semExpStr, convOutputs);
            return res;
        }
        return converter::syntGraphToSemExp(pResults.syntGraph, textProcCont, nullptr);
    }());
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->endOfTimeSlot("Semantic Expressions Converter");

    if (pSemanticAnalysisDebugOptions.convOutput == CONV_OUTPUT_MIND) {
        std::list<SemLineToPrint> semExpStrs;
        printer::prettyPrintSemExp(semExpStrs, *semExp);
        semExpLinesToStr.printLines(pHighLevelResults.semExpStr, semExpStrs);
    }

    // all forms
    {
        UniqueSemanticExpression semWithOtherForms = semExp->clone();
        converter::addDifferentForms(semWithOtherForms, lingDb);
        std::list<SemLineToPrint> semExpStrs;
        printer::prettyPrintSemExp(semExpStrs, *semWithOtherForms);
        semExpLinesToStr.printLines(pHighLevelResults.allFormsStr, semExpStrs);
    }

    // sentiments inforations
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->beginOfTimeSlot();
    SemanticPrinter::printSentiments(pHighLevelResults.sentimentsInfos,
                                     *semExp,
                                     SemanticAgentGrounding::getCurrentUser(),
                                     pResults.syntGraph.langConfig.lingDb.conceptSet,
                                     std::make_unique<SemanticAgentGrounding>(SemanticAgentGrounding::getCurrentUser()),
                                     SemanticSourceEnum::WRITTENTEXT);
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->endOfTimeSlot("Sentiments extractor");

    // completeness
    pHighLevelResults.completeness = linguistics::isComplete_fromSyntGraph(pResults.syntGraph);

    // reformulation
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->beginOfTimeSlot();
    SemanticMemoryBlock memBock;
    semExp = semExp->clone(nullptr, true);    // remove the interpretations for the reformulations
    simplifier::processFromMemBlock(semExp, memBock, lingDb);
    {
        pHighLevelResults.reformulations = "French:\n";
        std::string frReformulation;
        _syntSemExpWithDebugInfosIfAsked(frReformulation,
                                         pHighLevelResults.semExpStr,
                                         semExp->clone(),
                                         semExpLinesToStr,
                                         SemanticLanguageEnum::FRENCH,
                                         pSemanticAnalysisDebugOptions,
                                         memBock,
                                         lingDb);
        if (language == SemanticLanguageEnum::FRENCH)
            pHighLevelResults.reformulationInputLanguage = frReformulation;
        pHighLevelResults.reformulations += std::move(frReformulation);
    }
    {
        pHighLevelResults.reformulations += "\n\nEnglish:\n";
        std::string enReformulation;
        _syntSemExpWithDebugInfosIfAsked(enReformulation,
                                         pHighLevelResults.semExpStr,
                                         std::move(semExp),
                                         semExpLinesToStr,
                                         SemanticLanguageEnum::ENGLISH,
                                         pSemanticAnalysisDebugOptions,
                                         memBock,
                                         lingDb);
        if (language == SemanticLanguageEnum::ENGLISH || language == SemanticLanguageEnum::UNKNOWN)
            pHighLevelResults.reformulationInputLanguage = enReformulation;
        pHighLevelResults.reformulations += std::move(enReformulation);
    }
    if (pSemanticAnalysisDebugOptions.timeChecker)
        pSemanticAnalysisDebugOptions.timeChecker->endOfTimeSlot("Reformulator");
}

}    // End of namespace TextAnalyzeDebugger
}    // End of namespace linguistics
}    // End of namespace onsem
