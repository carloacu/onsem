#include <onsem/texttosemantic/tool/partofspeech/partofspeechcustomfilter.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include "../chunkshandler.hpp"

namespace onsem {
namespace linguistics {

PartOfSpeechCustomFilter::PartOfSpeechCustomFilter(const std::string& pName,
                                                   const InflectionsChecker& pFls,
                                                   const SpecificLinguisticDatabase& pSpecLingDb,
                                                   const std::string& pLabel)
    : PartOfSpeechContextFilter(pName)
    , fFls(pFls)
    , fSpecLingDb(pSpecLingDb)
    , fLabel(pLabel)
    , fGramTypesWeCanSkipBefore() {
    fGramTypesWeCanSkipBefore.emplace_back(PartOfSpeech::AUX);
    fGramTypesWeCanSkipBefore.emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
    fGramTypesWeCanSkipBefore.emplace_back(PartOfSpeech::PRONOUN_SUBJECT);
}

bool PartOfSpeechCustomFilter::process(std::vector<Token>& pTokens) const {
    if (fLabel == "removeMeaningsThatCannotBeAtTheBeginning") {
        bool isAtTheBeginOfASentence = true;
        bool ifDel = false;
        TokIt itEnd = pTokens.end();
        // iterate over all the tokens of the text
        for (TokIt itTok = pTokens.begin(); itTok != itEnd;) {
            if (itTok->getPartOfSpeech() == PartOfSpeech::PUNCTUATION) {
                isAtTheBeginOfASentence = true;
            } else if (isAtTheBeginOfASentence) {
                isAtTheBeginOfASentence = false;
                for (auto itIGram = itTok->inflWords.begin(); itIGram != itTok->inflWords.end(); ++itIGram) {
                    const WordAssociatedInfos& wordInfos = itIGram->infos;
                    if (wordInfos.contextualInfos.count(WordContextualInfos::SENTENCECANNOTBEGINWITH) != 0)
                        ifDel |= delAPartOfSpeechfPossible(itTok->inflWords, itIGram);
                }
            } else {
                auto nextItTok = getNextToken(itTok, itEnd, SkipPartOfWord::YES);
                if (nextItTok == itEnd || nextItTok->getPartOfSpeech() == PartOfSpeech::PUNCTUATION) {
                    for (auto itIGram = itTok->inflWords.begin(); itIGram != itTok->inflWords.end(); ++itIGram) {
                        const WordAssociatedInfos& wordInfos = itIGram->infos;
                        if (wordInfos.contextualInfos.count(WordContextualInfos::SENTENCECANNOTENDWITH) != 0)
                            ifDel |= delAPartOfSpeechfPossible(itTok->inflWords, itIGram);
                    }
                }
                itTok = nextItTok;
                continue;
            }
            itTok = getNextToken(itTok, itEnd, SkipPartOfWord::YES);
        }
        return ifDel;
    }
    if (fLabel == "gatherLinkedMeanings") {
        bool ifDel = false;
        // iterate over all the tokens of the text
        for (auto itTok = getPrevToken(pTokens.end(), pTokens.begin(), pTokens.end(), SkipPartOfWord::YES);
             itTok != pTokens.end();
             itTok = getPrevToken(itTok, pTokens.begin(), pTokens.end(), SkipPartOfWord::YES)) {
            for (auto itIGram = itTok->inflWords.begin(); itIGram != itTok->inflWords.end(); ++itIGram) {
                for (const LingWordsGroup& meanGroup : itIGram->infos.metaMeanings) {
                    std::list<std::pair<TokIt, std::list<InflectedWord>::iterator>> linkedTokens;
                    if (xTryToLinkAToken(linkedTokens, pTokens, itTok, itIGram, meanGroup)) {
                        LinguisticMeaning rootMeaning;
                        SemExpGetter::wordToAMeaning(
                            rootMeaning, *meanGroup.rootWord, fSpecLingDb.language, fSpecLingDb.lingDb);
                        if (!(fSpecLingDb.lingDico.hasContextualInfo(WordContextualInfos::SENTENCEENDSWITH, rootMeaning)
                              && !isTokenAtEndOfASentence(itTok, pTokens))) {
                            // if we are here it means that we have found a need links of words
                            ifDel |= delAllExept(itTok->inflWords, itIGram);
                            itTok->linkedTokens.push_back(itTok);
                            for (const auto& currLinkedToken : linkedTokens) {
                                ifDel |= delAllExept(currLinkedToken.first->inflWords, currLinkedToken.second);
                                currLinkedToken.first->linkedTokens.push_back(itTok);
                                itTok->linkedTokens.push_back(currLinkedToken.first);
                            }

                            if (itTok->getTokenLinkage() == TokenLinkage::HEAD_OF_WORD_GROUP) {
                                fSpecLingDb.lingDico.getInfoGram(*itIGram, rootMeaning);
                            }
                        }

                        break;
                    }
                }
            }
        }
        return ifDel;
    }
    if (fLabel == "nounAtBottomIfTwoTimesInARow") {
        xTestOfPuttingNounAtBottomOfListIfOccurTwoTimesInARow(pTokens, false);
        return false;
    }
    if (fLabel == "nounAtBottomIfTwoTimesInARowExeptForVerb") {
        xTestOfPuttingNounAtBottomOfListIfOccurTwoTimesInARow(pTokens, true);
        return false;
    }
    return false;
}

bool PartOfSpeechCustomFilter::xTryToLinkAToken(
    std::list<std::pair<TokIt, std::list<InflectedWord>::iterator>>& pLinkedTokens,
    std::vector<Token>& pTokens,
    std::vector<Token>::iterator pRootTok,
    std::list<InflectedWord>::iterator pRootItIGram,
    const LingWordsGroup& pMeanGroup) const {
    std::list<InflectedWord>::iterator onlyItIGramToKeep;
    TokIt downTok = pRootTok;
    TokIt upTok = pRootTok;
    for (const auto& currLkMean : pMeanGroup.linkedMeanings) {
        const SemanticWord& currSemWord = *currLkMean.first;
        switch (currLkMean.second) {
            case LinkedMeaningDirection::FORWARD: {
                upTok = xSearchWordAfter(pTokens, onlyItIGramToKeep, upTok, currSemWord, *pRootItIGram);
                if (upTok == pTokens.end()) {
                    return false;
                }
                pLinkedTokens.emplace_back(upTok, onlyItIGramToKeep);
                break;
            }
            case LinkedMeaningDirection::BACKWARD: {
                downTok = xSearchWordBefore(pTokens, onlyItIGramToKeep, downTok, currSemWord, *pRootItIGram);
                if (downTok == pTokens.end()) {
                    return false;
                }
                pLinkedTokens.emplace_back(downTok, onlyItIGramToKeep);
                break;
            }
            case LinkedMeaningDirection::BOTH: {
                TokIt resTok = xSearchWordBefore(pTokens, onlyItIGramToKeep, downTok, currSemWord, *pRootItIGram);
                if (resTok == pTokens.end()) {
                    upTok = xSearchWordAfter(pTokens, onlyItIGramToKeep, upTok, currSemWord, *pRootItIGram);
                    if (upTok == pTokens.end()) {
                        return false;
                    }
                    pLinkedTokens.emplace_back(upTok, onlyItIGramToKeep);
                } else {
                    downTok = resTok;
                    pLinkedTokens.emplace_back(downTok, onlyItIGramToKeep);
                }
                break;
            }
        }
    }
    if (pMeanGroup.rootWord
        && fSpecLingDb.lingDico.hasContextualInfo(WordContextualInfos::CANNOTBEBEFORENOUN, *pMeanGroup.rootWord)) {
        auto upTokNext = getNextToken(upTok, pTokens.end(), SkipPartOfWord::YES);
        if (upTokNext != pTokens.end() && upTokNext->getPartOfSpeech() == PartOfSpeech::NOUN)
            return false;
    }
    return true;
}

std::vector<Token>::iterator PartOfSpeechCustomFilter::xSearchWordAfter(
    std::vector<Token>& pTokens,
    std::list<InflectedWord>::iterator& pOnlyItIGramToKeep,
    std::vector<Token>::iterator pCurrTok,
    const SemanticWord& pWord,
    const InflectedWord& pMainIGram) const {
    bool hasDeterminerInBetween = false;
    bool hasNounInBetween = false;
    for (TokIt itTok = getNextToken(pCurrTok, pTokens.end(), SkipPartOfWord::YES); itTok != pTokens.end();
         itTok = getNextToken(itTok, pTokens.end(), SkipPartOfWord::YES)) {
        if (hasDeterminerInBetween == hasNounInBetween) {
            for (auto itIGram = itTok->inflWords.begin(); itIGram != itTok->inflWords.end(); ++itIGram) {
                if (itIGram->word == pWord && xCanBeLinked(pMainIGram, *itIGram)) {
                    pOnlyItIGramToKeep = itIGram;
                    return itTok;
                }
            }
        }

        if (pMainIGram.word.partOfSpeech == PartOfSpeech::VERB) {
            if (itTok->inflWords.front().word.partOfSpeech == PartOfSpeech::ADVERB)
                continue;
            if (itTok->inflWords.front().word.partOfSpeech == PartOfSpeech::DETERMINER) {
                hasDeterminerInBetween = true;
                continue;
            }
            if (itTok->inflWords.front().word.partOfSpeech == PartOfSpeech::NOUN) {
                hasNounInBetween = true;
                continue;
            }
        }
        break;
    }
    return pTokens.end();
}

std::vector<Token>::iterator PartOfSpeechCustomFilter::xSearchWordBefore(
    std::vector<Token>& pTokens,
    std::list<InflectedWord>::iterator& pOnlyItIGramToKeep,
    std::vector<Token>::iterator pCurrTok,
    const SemanticWord& pWord,
    const InflectedWord& pMainIGram) const {
    for (TokIt itTok = getPrevToken(pCurrTok, pTokens.begin(), pTokens.end(), SkipPartOfWord::YES);
         itTok != pTokens.end();
         itTok = getPrevToken(itTok, pTokens.begin(), pTokens.end(), SkipPartOfWord::YES)) {
        for (auto itIGram = itTok->inflWords.begin(); itIGram != itTok->inflWords.end(); ++itIGram) {
            if (itIGram->word == pWord && xCanBeLinked(pMainIGram, *itIGram)) {
                pOnlyItIGramToKeep = itIGram;
                return itTok;
            }
        }

        if (!tokenIsMoreProbablyAType(*itTok, fGramTypesWeCanSkipBefore))
            break;
    }
    return pTokens.end();
}

bool PartOfSpeechCustomFilter::xCanBeLinked(const InflectedWord& pMainIGram, const InflectedWord& pSubIGram) const {
    if (pSubIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT
        && pMainIGram.word.partOfSpeech == PartOfSpeech::VERB) {
        return fFls.areVerbAndPronComplCanBeLinked(pSubIGram.inflections(), pMainIGram.inflections());
    }
    return true;
}

void PartOfSpeechCustomFilter::xTestOfPuttingNounAtBottomOfListIfOccurTwoTimesInARow(std::vector<Token>& pTokens,
                                                                                     bool pExceptForVerb) const {
    TokIt prevIt = getTheNextestToken(pTokens.begin(), pTokens.end());
    if (prevIt == pTokens.end())
        return;
    for (TokIt it = getNextToken(prevIt, pTokens.end()); it != pTokens.end(); it = getNextToken(it, pTokens.end())) {
        auto& inflWords1 = prevIt->inflWords;
        auto& inflWords2 = it->inflWords;
        if ((inflWords1.size() > 1 || inflWords2.size() > 1) && (inflWords1.size() == 1 || inflWords2.size() == 1)) {
            auto& inflWord1 = inflWords1.front();
            auto& inflWord2 = inflWords2.front();
            if (inflWord1.word.partOfSpeech == PartOfSpeech::NOUN && inflWord2.word.partOfSpeech == PartOfSpeech::NOUN
                && !fFls.areCompatibles(inflWord1, inflWord2)) {
                xPutNounAtBottomIfNecessary(inflWords2, pExceptForVerb);
                xPutNounAtBottomIfNecessary(inflWords1, pExceptForVerb);
            }
        }
        prevIt = it;
    }
}

void PartOfSpeechCustomFilter::xPutNounAtBottomIfNecessary(std::list<InflectedWord>& pInflWord,
                                                           bool pExceptForVerb) const {
    auto itSecondIGram2 = ++pInflWord.begin();
    if ((!pExceptForVerb || itSecondIGram2->word.partOfSpeech != PartOfSpeech::VERB)
        && itSecondIGram2->word.partOfSpeech != PartOfSpeech::NOUN)
        pInflWord.splice(pInflWord.begin(), pInflWord, itSecondIGram2);
}

}    // End of namespace linguistics
}    // End of namespace onsem
