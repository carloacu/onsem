#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOOL_PARTOFSPEECH_PARTOFSPEECHPATTERNMATCHER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOOL_PARTOFSPEECH_PARTOFSPEECHPATTERNMATCHER_HPP

#include <map>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcontextfilter.hpp>
#include "type/taggertypes.hpp"

namespace onsem {
namespace linguistics {
struct InflectedWord;
class InflectionsChecker;

/// A visitor responding to predefined rules.
class PartOfSpeechPatternMatcher : public PartOfSpeechContextFilter {
public:
    PartOfSpeechPatternMatcher(const std::string& pName, const InflectionsChecker& pFls, TaggerTokenCheck&& pRootGram);

    bool process(std::vector<linguistics::Token>& pTokens) const override;

    TaggerPattern& getPattern() { return _pattern; }

private:
    struct InfoGramIts {
        InfoGramIts(const std::vector<Token>::iterator& pItToken, const std::list<InflectedWord>::iterator& pItInfoGram)
            : itToken(pItToken)
            , itInfoGram(pItInfoGram) {}

        std::vector<Token>::iterator itToken;
        std::list<InflectedWord>::iterator itInfoGram;
    };
    struct WorkStructForADirection {
        WorkStructForADirection(std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pRes,
                                const WorkStructForADirection& pOtherWorkStruct,
                                std::list<TaggerListOfTokenChecks>::const_iterator pItCurrRule)
            : res(pRes)
            , isLinked(LinkedValue::LINKED)
            , itCurrRule(pItCurrRule)
            , rules(pOtherWorkStruct.rules)
            , beforeOrAfter(pOtherWorkStruct.beforeOrAfter)
            , rootIGram(pOtherWorkStruct.rootIGram)
            , itCurrToken(pOtherWorkStruct.itCurrToken)
            , tokBeginIt(pOtherWorkStruct.tokBeginIt)
            , tokEndIt(pOtherWorkStruct.tokEndIt) {}

        WorkStructForADirection(std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pRes,
                                const std::list<TaggerListOfTokenChecks>& pRules,
                                const bool pBeforeOrAfter,
                                const InflectedWord& pRootIGram,
                                std::vector<Token>::iterator pItCurrToken,
                                const std::vector<Token>::iterator pTokBeginIt,
                                const std::vector<Token>::iterator pTokEndIt)
            : res(pRes)
            , isLinked(LinkedValue::LINKED)
            , itCurrRule(pRules.begin())
            , rules(pRules)
            , beforeOrAfter(pBeforeOrAfter)
            , rootIGram(pRootIGram)
            , itCurrToken(pItCurrToken)
            , tokBeginIt(pTokBeginIt)
            , tokEndIt(pTokEndIt) {}

        void goToNextToken();
        void mergeOtherRes(std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pOtherRes, LinkedValue pIsLinked);
        void notifyNewRuleLinkedValue(LinkedValue pIsLinked);

        std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& res;
        LinkedValue isLinked;
        std::list<TaggerListOfTokenChecks>::const_iterator itCurrRule;
        const std::list<TaggerListOfTokenChecks>& rules;
        const bool beforeOrAfter;
        const InflectedWord& rootIGram;
        std::vector<Token>::iterator itCurrToken;
        const std::vector<Token>::iterator tokBeginIt;
        const std::vector<Token>::iterator tokEndIt;
    };
    const InflectionsChecker& _fls;
    TaggerPattern _pattern;

    LinkedValue _checkAContextPossibility(std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pRes,
                                          const AIGramContext& pContextPoss,
                                          const InflectedWord& pRootIGram,
                                          std::vector<linguistics::Token>::iterator pItCurrToken,
                                          std::vector<Token>& pTokens) const;

    void _processCurrRule(WorkStructForADirection& pWorkStruct, bool pIsTheFirstLoopInThisRule = true) const;

    void _processRuleWithoutSkip(WorkStructForADirection& pWorkStruct) const;

    LinkedValue _doesRuleMatchWithCurrToken(WorkStructForADirection& pWorkStruct,
                                            const TaggerListOfTokenChecks& pRule) const;

    static bool _applyPatternIfLinked(std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pLinkedGramTypes);

    static bool _applyPatternIfNotLinked(
        const std::map<const TaggerTokenCheck*, std::list<InfoGramIts>>& pNotLinkedGramTypes);
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_TOOL_PARTOFSPEECH_PARTOFSPEECHPATTERNMATCHER_HPP
