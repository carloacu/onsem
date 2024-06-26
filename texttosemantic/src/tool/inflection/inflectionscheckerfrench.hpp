#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKERFRENCH_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKERFRENCH_HPP

#include "inflectionscheckervirtual.hpp"

namespace onsem {
namespace linguistics {
struct InflectedWord;

class InflectionsCheckerFrench : public InflectionsCheckerVirtual {
public:
    InflectionsCheckerFrench(const LinguisticDictionary& pLingDic);

    bool isAuxVerbCompatibles(const InflectedWord& pIGramAux, const InflectedWord& pIGramVerb) const override;

    bool _isVerbVerbCompatibles(const Inflections& pVerbInfl1, const Inflections& pVerbInfl2) const override;

    bool isDetAdjCompatibles(const InflectedWord& pIGramDet, const InflectedWord& pIGramAdj) const override;

    bool verbCanHaveAnAuxiliary(const VerbalInflection& pVerbInfl) const override;

    bool isVerbSubConjonction(const InflectedWord& pInflVerb, const InflectedWord& pInflSubConj) const override;

    bool isAdjNounCompatibles(const InflectedWord& pIGramAdj, const InflectedWord& pIGramNoun) const override;

    bool isVerbAdjCompatibles(const InflectedWord& pIGramVerb, const InflectedWord& pIGramAdj) const override;

    bool areDetCompatibles(const InflectedWord& pInflDet1, const InflectedWord& pInflDet2) const override;

    bool areDetNounCompatibles(const InflectedWord& pInfWord1, const InflectedWord& pInfWord2) const override;

    bool isDetProperNounCompatibles(const InflectedWord& pInflDet, const InflectedWord& pInflProperNoun) const override;

    bool areNounDetCompatibles(const InflectedWord&, const InflectedWord&) const override { return true; }

    bool canFinishWithPerp(const InflectedWord& pPrepInflWord) const override;

    bool areNounNounCompatibles(const InflectedWord& pNounInflWord1,
                                const InflectedWord& pNounInflWord2) const override;

    bool isPronounPronounComplementCompatibles(const InflectedWord& pInflPronoun) const override;

    bool isPronounComplAdverbCompatibles(const InflectedWord& pInflPronCompl,
                                         const InflectedWord& pInflAdv) const override;

    bool isPronounComplAdjectiveCompatibles(const InflectedWord&, const InflectedWord&) const { return false; }

    bool isPronounComplVerbCompatibles(const InflectedWord& pInflPronCompl,
                                       const InflectedWord& pInflVerb) const override;

    bool isPronounCompDetCompatibles(const InflectedWord&) const override;

    bool isIntjInflCompatibles(const InflectedWord& pIntj, const InflectedWord& pInfl2) const override;

    bool isAdvIntjCompatibles(const InflectedWord& pAdv, const InflectedWord& pIntj) const override;
};

inline bool InflectionsCheckerFrench::verbCanHaveAnAuxiliary(const VerbalInflection& pVerbInfl) const {
    return pVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE;
}

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKERFRENCH_HPP
