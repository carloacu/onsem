#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKERENGLISH_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKERENGLISH_HPP

#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include "inflectionscheckervirtual.hpp"


namespace onsem
{
namespace linguistics
{

class InflectionsCheckerEnglish : public InflectionsCheckerVirtual
{
public:
  InflectionsCheckerEnglish(const LinguisticDictionary& pLingDic);

  bool isAuxVerbCompatibles(const InflectedWord& pIGramAux,
                            const InflectedWord& pIGramVerb) const override;

  bool _isVerbVerbCompatibles(const Inflections&,
                              const Inflections&) const override { return true; }

  bool isDetAdjCompatibles(const InflectedWord& pIGramDet,
                           const InflectedWord& pIGramAdj) const override;

  bool verbCanHaveAnAuxiliary(const VerbalInflection& pVerbInfl) const override;

  bool isVerbSubConjonction(const InflectedWord& pInflVerb,
                            const InflectedWord& pInflSubConj) const override { return true; }

  bool isAdjNounCompatibles(const InflectedWord& pIGramAdj,
                            const InflectedWord& pIGramNoun) const override;

  bool isVerbAdjCompatibles(const InflectedWord& pIGramVerb,
                            const InflectedWord& pIGramAdj) const override;

  bool areDetCompatibles(const InflectedWord& pInflDet1,
                         const InflectedWord& pInflDet2) const override;

  bool areDetNounCompatibles(const InflectedWord& pInfWord1,
                            const InflectedWord& pInfWord2) const override;

  bool isDetProperNounCompatibles(const InflectedWord& pInflDet,
                                  const InflectedWord& pInflProperNoun) const override;

  bool areNounDetCompatibles(const InflectedWord& pNounInflWord,
                             const InflectedWord& pDetInflWord) const override;

  bool areNounNounCompatibles(const InflectedWord& pNounInflWord1,
                              const InflectedWord& pNounInflWord2) const override;

  bool isPronounPronounComplementCompatibles(const InflectedWord& pInflPronoun) const override { return true; }
};




inline bool InflectionsCheckerEnglish::isDetAdjCompatibles
(const InflectedWord& pIGramDet,
 const InflectedWord& pIGramAdj) const
{
  return isNounAdjCompatibles(pIGramDet, pIGramAdj.inflections());
}

inline bool InflectionsCheckerEnglish::verbCanHaveAnAuxiliary
(const VerbalInflection& pVerbInfl) const
{
  return pVerbInfl.tense == LinguisticVerbTense::PRESENT_IMPERATIVE ||
      pVerbInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE ||
      pVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE;
}

inline bool InflectionsCheckerEnglish::isAdjNounCompatibles
(const InflectedWord& pIGramAdj,
 const InflectedWord& pIGramNoun) const
{
  return isNounAdjCompatibles(pIGramNoun, pIGramAdj.inflections());
}

} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKERENGLISH_HPP
