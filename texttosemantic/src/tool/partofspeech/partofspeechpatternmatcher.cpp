#include "partofspeechpatternmatcher.hpp"
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>

namespace onsem {
namespace linguistics {
namespace {
std::list<TaggerListOfTokenChecks>::const_iterator _getNextRuleThatCannotBeEmptyAfter(
    std::list<TaggerListOfTokenChecks>::const_iterator pItCurrRule,
    const std::list<TaggerListOfTokenChecks>& pRules) {
    ++pItCurrRule;
    while (pItCurrRule != pRules.end()) {
        if (pItCurrRule->canBeEmpty == CanBeEmpty::NO)
            return pItCurrRule;
        ++pItCurrRule;
    }
    return pItCurrRule;
}

}

PartOfSpeechPatternMatcher::PartOfSpeechPatternMatcher(const std::string& pName,
                                                       const InflectionsChecker& pFls,
                                                       TaggerTokenCheck&& pRootGram)
    : PartOfSpeechContextFilter(pName)
    , _fls(pFls)
    , _pattern(std::move(pRootGram)) {}

bool PartOfSpeechPatternMatcher::process(std::vector<Token>& pTokens) const {
    bool ifDel = false;
    // iterate over all the tokens of the sentence
    for (auto itCurrToken = pTokens.begin(); itCurrToken != pTokens.end();
         itCurrToken = getNextToken(itCurrToken, pTokens.end())) {
        // the token has to have "rootGram" in his gram possibilities
        auto itRootGram = [this, &itCurrToken] {
            switch (_pattern.rootGram.gramCheckerStrategy) {
                case FinderConstraint::FIRST_ELT: {
                    auto itBeginInfosGram = itCurrToken->inflWords.begin();
                    assert(itBeginInfosGram != itCurrToken->inflWords.end());
                    if (_pattern.rootGram.iGramMatcher(*itBeginInfosGram)) {
                        return itBeginInfosGram;
                    }
                    return itCurrToken->inflWords.end();
                }
                case FinderConstraint::HAS: {
                    for (auto itIGram = itCurrToken->inflWords.begin(); itIGram != itCurrToken->inflWords.end();
                         ++itIGram)
                        if (_pattern.rootGram.iGramMatcher(*itIGram))
                            return itIGram;
                    return itCurrToken->inflWords.end();
                }
                case FinderConstraint::ONLY_ONE_ELT: {
                    if (itCurrToken->inflWords.size() == 1) {
                        auto itBeginInfosGram = itCurrToken->inflWords.begin();
                        if (_pattern.rootGram.iGramMatcher(*itBeginInfosGram))
                            return itBeginInfosGram;
                    }
                    return itCurrToken->inflWords.end();
                }
            }
            return itCurrToken->inflWords.end();
        }();
        if (itRootGram != itCurrToken->inflWords.end()) {
            const auto& currRootGram = *itRootGram;
            LinkedValue isLinked = LinkedValue::UNKNOWN;
            std::map<const TaggerTokenCheck*, std::list<InfoGramIts>> res;
            for (const auto& currContextPoss : _pattern.possibilities) {
                res.clear();
                isLinked = _checkAContextPossibility(res, currContextPoss, currRootGram, itCurrToken, pTokens);
                if (isLinked == LinkedValue::LINKED) {
                    res[&_pattern.rootGram].emplace_back(InfoGramIts(itCurrToken, itRootGram));
                    ifDel = _applyPatternIfLinked(res) || ifDel;
                    break;
                }
            }

            if (isLinked == LinkedValue::NOT_LINKED) {
                res[&_pattern.rootGram].emplace_back(InfoGramIts(itCurrToken, itRootGram));
                ifDel = _applyPatternIfNotLinked(res) || ifDel;
            }
        }
    }
    return ifDel;
}

LinkedValue PartOfSpeechPatternMatcher::_checkAContextPossibility(
    std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pRes,
    const AIGramContext& pContextPoss,
    const InflectedWord& pRootIGram,
    std::vector<Token>::iterator pItCurrToken,
    std::vector<Token>& pTokens) const {
    WorkStructForADirection workStructBefore(
        pRes, pContextPoss.before, true, pRootIGram, pItCurrToken, pTokens.begin(), pTokens.end());
    workStructBefore.goToNextToken();
    _processCurrRule(workStructBefore);
    if (workStructBefore.isLinked != LinkedValue::NOT_LINKED) {
        WorkStructForADirection workStructAfter(
            pRes, pContextPoss.after, false, pRootIGram, pItCurrToken, pTokens.begin(), pTokens.end());
        workStructAfter.goToNextToken();
        _processCurrRule(workStructAfter);
        if (workStructAfter.isLinked == LinkedValue::NOT_LINKED)
            return LinkedValue::NOT_LINKED;
        if (workStructBefore.isLinked == LinkedValue::UNKNOWN || workStructAfter.isLinked == LinkedValue::UNKNOWN)
            return LinkedValue::UNKNOWN;
        return LinkedValue::LINKED;
    }
    return workStructBefore.isLinked;
}

void PartOfSpeechPatternMatcher::_processCurrRule(WorkStructForADirection& pWorkStruct,
                                                  bool pIsTheFirstLoopInThisRule) const {
    if (pWorkStruct.itCurrRule == pWorkStruct.rules.end())
        return;
    if (pWorkStruct.itCurrToken == pWorkStruct.tokEndIt) {
        while (pWorkStruct.itCurrRule != pWorkStruct.rules.end()) {
            if (pWorkStruct.itCurrRule->canBeEmpty == CanBeEmpty::NO && !pWorkStruct.itCurrRule->canBeAPunctuation()) {
                pWorkStruct.notifyNewRuleLinkedValue(pWorkStruct.itCurrRule->defaultLinkValue);
                if (pWorkStruct.isLinked == LinkedValue::NOT_LINKED)
                    return;
            }
            ++pWorkStruct.itCurrRule;
        }
        return;
    }

    if (pWorkStruct.itCurrRule->canBeEmpty == CanBeEmpty::YES || !pIsTheFirstLoopInThisRule) {
        auto itThatCannotBeEmpty = _getNextRuleThatCannotBeEmptyAfter(pWorkStruct.itCurrRule, pWorkStruct.rules);
        if (itThatCannotBeEmpty != pWorkStruct.rules.end()
            && itThatCannotBeEmpty->priority > pWorkStruct.itCurrRule->priority) {
            std::map<const TaggerTokenCheck*, std::list<InfoGramIts>> subRes;
            WorkStructForADirection subWorkStruct(subRes, pWorkStruct, itThatCannotBeEmpty);
            _processRuleWithoutSkip(subWorkStruct);
            if (subWorkStruct.isLinked == LinkedValue::NOT_LINKED)
                _processRuleWithoutSkip(pWorkStruct);
            else
                pWorkStruct.mergeOtherRes(subRes, subWorkStruct.isLinked);
            return;
        }

        std::map<const TaggerTokenCheck*, std::list<InfoGramIts>> subRes;
        WorkStructForADirection subWorkStruct(subRes, pWorkStruct, pWorkStruct.itCurrRule);
        _processRuleWithoutSkip(subWorkStruct);
        if (subWorkStruct.isLinked == LinkedValue::NOT_LINKED) {
            ++pWorkStruct.itCurrRule;
            _processCurrRule(pWorkStruct);
        } else
            pWorkStruct.mergeOtherRes(subRes, subWorkStruct.isLinked);
        return;
    }

    _processRuleWithoutSkip(pWorkStruct);
}

void PartOfSpeechPatternMatcher::_processRuleWithoutSkip(WorkStructForADirection& pWorkStruct) const {
    const auto& currRule = *pWorkStruct.itCurrRule;
    pWorkStruct.isLinked = _doesRuleMatchWithCurrToken(pWorkStruct, currRule);
    if (pWorkStruct.isLinked == LinkedValue::LINKED) {
        pWorkStruct.goToNextToken();
        if (pWorkStruct.itCurrRule->canHaveMany == CanHaveMany::YES) {
            std::map<const TaggerTokenCheck*, std::list<InfoGramIts>> subRes;
            WorkStructForADirection subWorkStruct(subRes, pWorkStruct, pWorkStruct.itCurrRule);
            _processCurrRule(subWorkStruct, false);
            if (subWorkStruct.isLinked == LinkedValue::NOT_LINKED) {
                ++pWorkStruct.itCurrRule;
                _processCurrRule(pWorkStruct);
            } else
                pWorkStruct.mergeOtherRes(subRes, subWorkStruct.isLinked);
            return;
        }

        ++pWorkStruct.itCurrRule;
        _processCurrRule(pWorkStruct);
    }
}

LinkedValue PartOfSpeechPatternMatcher::_doesRuleMatchWithCurrToken(WorkStructForADirection& pWorkStruct,
                                                                    const TaggerListOfTokenChecks& pRule) const {
    auto tryToHadAMatchedToken = [this, &pWorkStruct](const TaggerTokenCheck& pCurrIGramPoss,
                                                      const std::list<InflectedWord>::iterator& pItSubIGram) {
        if (pCurrIGramPoss.iGramMatcher(*pItSubIGram)) {
            auto isCompatibleFn = [this, &pWorkStruct, &pItSubIGram] {
                return (pWorkStruct.beforeOrAfter && _fls.areCompatibles(*pItSubIGram, pWorkStruct.rootIGram))
                    || (!pWorkStruct.beforeOrAfter && _fls.areCompatibles(pWorkStruct.rootIGram, *pItSubIGram));
            };

            bool compatibilityIsOk = false;
            switch (pCurrIGramPoss.checkCompatibilityWithNeighborhood) {
                case CompatibilityCheck::IS_COMPATIBLE: compatibilityIsOk = isCompatibleFn(); break;
                case CompatibilityCheck::ISNT_COMPATIBLE: compatibilityIsOk = !isCompatibleFn(); break;
                case CompatibilityCheck::DONT_CARE: compatibilityIsOk = true; break;
            }
            if (compatibilityIsOk) {
                pWorkStruct.res[&pCurrIGramPoss].emplace_back(InfoGramIts(pWorkStruct.itCurrToken, pItSubIGram));
                return true;
            }
        }
        return false;
    };

    Token& currToken = *pWorkStruct.itCurrToken;
    LinkedValue res = LinkedValue::NOT_LINKED;
    // iterate over all the grammatical possibilities of the pattern node
    for (const auto& currIGramPoss : pRule.elts) {
        bool stopForLoop = false;
        switch (currIGramPoss.gramCheckerStrategy) {
            case FinderConstraint::HAS: {
                // iterate over all the grammatical possibilities of the token
                for (auto itSubIGram = currToken.inflWords.begin(); itSubIGram != currToken.inflWords.end();
                     ++itSubIGram) {
                    if (tryToHadAMatchedToken(currIGramPoss, itSubIGram)) {
                        res = currIGramPoss.linkedValue;
                        stopForLoop = true;
                        break;
                    }
                }
                break;
            }
            case FinderConstraint::FIRST_ELT: {
                assert(!pWorkStruct.itCurrToken->inflWords.empty());
                if (tryToHadAMatchedToken(currIGramPoss, currToken.inflWords.begin())) {
                    res = currIGramPoss.linkedValue;
                    stopForLoop = true;
                }
                break;
            }
            case FinderConstraint::ONLY_ONE_ELT: {
                if (currToken.inflWords.size() == 1
                    && tryToHadAMatchedToken(currIGramPoss, currToken.inflWords.begin())) {
                    res = currIGramPoss.linkedValue;
                    stopForLoop = true;
                }
                break;
            }
        }
        if (stopForLoop)
            return res;
    }

    return pRule.defaultLinkValue;
}

bool PartOfSpeechPatternMatcher::_applyPatternIfLinked(
    std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pLinkedGramTypes) {
    bool ifDel = false;
    for (auto& pGramPattern : pLinkedGramTypes) {
        switch (pGramPattern.first->actionIfLinked) {
            case ActionIfLinked::DEL_ALL_EXPECT_AUX: {
                for (auto& currIGram : pGramPattern.second)
                    ifDel = delAllExept(currIGram.itToken->inflWords, currIGram.itInfoGram, PartOfSpeech::AUX) || ifDel;
                break;
            }
            case ActionIfLinked::DEL_ALL_OTHERS: {
                for (auto& currIGram : pGramPattern.second)
                    ifDel = delAllExept(currIGram.itToken->inflWords, currIGram.itInfoGram) || ifDel;
                break;
            }
            case ActionIfLinked::DEL_LESSER_PROBA: {
                for (auto& currIGram : pGramPattern.second)
                    ifDel = delAllAfter(currIGram.itToken->inflWords, currIGram.itInfoGram) || ifDel;
                break;
            }
            case ActionIfLinked::DEL_THIS_POSSIBILITY: {
                for (auto& currIGram : pGramPattern.second)
                    ifDel = delAPartOfSpeechfPossible(currIGram.itToken->inflWords, currIGram.itInfoGram) || ifDel;
                break;
            }
            case ActionIfLinked::PUT_ON_TOP: {
                for (auto& currIGram : pGramPattern.second)
                    putOnTop(currIGram.itToken->inflWords, currIGram.itInfoGram);
                break;
            }
            case ActionIfLinked::PUT_ON_BOTTOM: {
                for (auto& currIGram : pGramPattern.second)
                    putOnBottom(currIGram.itToken->inflWords, currIGram.itInfoGram);
                break;
            }
            case ActionIfLinked::NOTHING: break;
        }
    }
    return ifDel;
}

bool PartOfSpeechPatternMatcher::_applyPatternIfNotLinked(
    const std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pNotLinkedGramTypes) {
    bool ifDel = false;
    for (auto& pGramPattern : pNotLinkedGramTypes) {
        if (pGramPattern.first->actionIfNotLinked == ActionIfNotLinked::REMOVE) {
            for (auto& currIGram : pGramPattern.second) {
                auto itInfGram = currIGram.itInfoGram;
                ifDel = delAPartOfSpeechfPossible(currIGram.itToken->inflWords, itInfGram) || ifDel;
            }
        }
    }
    return ifDel;
}

void PartOfSpeechPatternMatcher::WorkStructForADirection::goToNextToken() {
    if (beforeOrAfter)
        itCurrToken = getPrevToken(itCurrToken, tokBeginIt, tokEndIt);
    else
        itCurrToken = getNextToken(itCurrToken, tokEndIt);
}

void PartOfSpeechPatternMatcher::WorkStructForADirection::mergeOtherRes(
    std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pOtherRes,
    LinkedValue pIsLinked) {
    auto itOtherPoss = pOtherRes.begin();
    while (itOtherPoss != pOtherRes.end()) {
        auto& listToFill = res[itOtherPoss->first];
        listToFill.splice(listToFill.end(), itOtherPoss->second);
        itOtherPoss = pOtherRes.erase(itOtherPoss);
    }
    isLinked = pIsLinked;
}

void PartOfSpeechPatternMatcher::WorkStructForADirection::notifyNewRuleLinkedValue(LinkedValue pIsLinked) {
    if (isLinked == LinkedValue::NOT_LINKED || (isLinked == LinkedValue::UNKNOWN && pIsLinked == LinkedValue::LINKED))
        return;
    isLinked = pIsLinked;
}

}    // End of namespace linguistics
}    // End of namespace onsem
