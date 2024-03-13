#include <onsem/semanticdebugger/semanticdebug.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/semanticdebugger/textananlyzedebugger.hpp>
#include <onsem/semanticdebugger/timechecker.hpp>
#include <onsem/semanticdebugger/aretextsequivalent.hpp>

namespace onsem {
using namespace linguistics;

void SyntacticAnalysisResultToDisplay::saveConcepts(const linguistics::TokensTree& pTokensTree) {
    for (linguistics::ConstTokenIterator itTokens = pTokensTree.beginToken(); !itTokens.atEnd(); ++itTokens) {
        const linguistics::InflectedWord& tokIGram = *itTokens.getToken().inflWords.begin();
        if (partOfSpeech_isAWord(tokIGram.word.partOfSpeech)) {
            finalConcepts.emplace_back();
            ConceptSet::sortAndPrintConcepts(finalConcepts.back(), tokIGram.infos.concepts);
        }
    }
}

void SyntacticAnalysisResultToDisplay::saveContextInfos(const linguistics::TokensTree& pTokensTree) {
    for (linguistics::ConstTokenIterator itTokens = pTokensTree.beginToken(); !itTokens.atEnd(); ++itTokens) {
        const linguistics::InflectedWord& tokIGram = *itTokens.getToken().inflWords.begin();
        if (partOfSpeech_isAWord(tokIGram.word.partOfSpeech)) {
            std::list<std::string> contextInfosStrs;
            for (const auto& currContInfo : tokIGram.infos.contextualInfos)
                contextInfosStrs.emplace_back(wordContextualInfos_toStr(currContInfo));
            contextualInfos.emplace_back(std::move(contextInfosStrs));
        }
    }
}

namespace SemanticDebug {

void debugTextAnalyze(SyntacticAnalysisResultToDisplay& pAutoAnnotToDisplay,
                      const std::string& pSentence,
                      const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                      const SemanticAnalysisDebugOptions& pSemanticAnalysisDebugOptions,
                      SemanticLanguageEnum pLanguageType,
                      const linguistics::LinguisticDatabase& pLingDb,
                      const std::map<std::string, std::string>* pEquivalencesPtr) {
    SyntacticGraphResult results(pLingDb, pLanguageType);
    results.inputText = pSentence;
    auto timeChecker = std::make_unique<TimeChecker>();
    linguistics::TextAnalyzeDebugger::fillSemAnalResult(
        results, pAutoAnnotToDisplay.highLevelResults, pSpellingMistakeTypesPossible, pSemanticAnalysisDebugOptions);
    semAnalResultToStructToDisplay(pAutoAnnotToDisplay, results);
    pAutoAnnotToDisplay.isReformulationOk =
        areTextEquivalent(pSentence, pAutoAnnotToDisplay.highLevelResults.reformulationInputLanguage, pEquivalencesPtr);

    timeChecker->printBilanOfTimeSlots(pAutoAnnotToDisplay.performances);
}

void semAnalResultToStructToDisplay(SyntacticAnalysisResultToDisplay& pAutoAnnotToDisplay,
                                    const SyntacticGraphResult& pSemAnalResult) {
    // Print the list of tokens
    for (linguistics::ConstTokenIterator itTokens = pSemAnalResult.syntGraph.tokensTree.beginToken(); !itTokens.atEnd();
         ++itTokens)
        if (partOfSpeech_isAWord(itTokens.getToken().inflWords.begin()->word.partOfSpeech))
            pAutoAnnotToDisplay.tokens.emplace_back(itTokens.getOffset(), itTokens.getToken().str);
    // Print the grammatical possibilities of the final list of tokens
    linguistics::TextAnalyzeDebugger::saveGramPossibilities(pAutoAnnotToDisplay.finalGramPossibilities,
                                                            pSemAnalResult.syntGraph.tokensTree);
    // Print the concepts of the tokens
    pAutoAnnotToDisplay.saveConcepts(pSemAnalResult.syntGraph.tokensTree);
    // Print the contextual information of the tokens
    pAutoAnnotToDisplay.saveContextInfos(pSemAnalResult.syntGraph.tokensTree);
}

}    // End of namespace SemanticDebug
}    // End of namespace onsem
