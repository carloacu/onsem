#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOOL_INFLECTION_INFLECTIONSCHECKERVIRTUAL_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOOL_INFLECTION_INFLECTIONSCHECKERVIRTUAL_HPP

#include <list>
#include <onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp>

namespace onsem
{
struct Inflections;
struct NominalInflections;
struct PronominalInflection;
namespace linguistics
{
struct InflectedWord;
struct SpecificLinguisticDatabase;
class LinguisticDictionary;
struct Chunk;
struct WordAssociatedInfos;


class InflectionsCheckerVirtual
{
public:
  InflectionsCheckerVirtual(const LinguisticDictionary& pLingDic);
  virtual ~InflectionsCheckerVirtual() {}

  virtual bool isAuxVerbCompatibles(const InflectedWord& pIGramAux,
                                    const InflectedWord& pIGramVerb) const = 0;

  virtual bool _isVerbVerbCompatibles(const Inflections& pVerbInfl1,
                                      const Inflections& pVerbInfl2) const  = 0;

  virtual bool isDetAdjCompatibles(const InflectedWord& pIGramDet,
                                   const InflectedWord& pIGramAdj) const = 0;

  virtual bool verbCanHaveAnAuxiliary(const VerbalInflection& pVerbInfl) const = 0;

  virtual bool isVerbSubConjonction(const InflectedWord& pInflVerb,
                                    const InflectedWord& pInflSubConj) const = 0;

  virtual bool isAdjNounCompatibles(const InflectedWord& pIGramAdj,
                                    const InflectedWord& pIGramNoun) const = 0;

  virtual bool isVerbAdjCompatibles(const InflectedWord& pIGramVerb,
                                    const InflectedWord& pIGramAdj) const = 0;

  virtual bool areDetCompatibles(const InflectedWord& pInflDet1,
                                 const InflectedWord& pInflDet2) const = 0;

  virtual bool areDetNounCompatibles(const InflectedWord& pInfWord1,
                                    const InflectedWord& pInfWord2) const = 0;

  virtual bool isDetProperNounCompatibles(const InflectedWord& pInflDet,
                                          const InflectedWord& pInflProperNoun) const  = 0;

  bool isNounAdjCompatibles(const InflectedWord& pNounInflWord,
                            const Inflections& pAdjInflections) const;

  virtual bool areNounDetCompatibles(const InflectedWord& pNounInflWord,
                                     const InflectedWord& pDetInflWord) const = 0;
  virtual bool areNounNounCompatibles(const InflectedWord& pNounInflWord1,
                                      const InflectedWord& pNounInflWord2) const = 0;

  virtual bool isPronounPronounComplementCompatibles(const InflectedWord& pInflPronoun) const = 0;

  virtual bool isIntjInflCompatibles(const InflectedWord& pIntj,
                                     const InflectedWord& pInfl) const = 0;

  virtual bool isAdvIntjCompatibles(const InflectedWord& pAdv,
                                    const InflectedWord& pIntj) const = 0;


protected:
  const LinguisticDictionary& _lingDic;

  static bool _areNounNounInflectionsWeaklyEqual(
      const NominalInflections& pNounInfl1,
      const NominalInflections& pNounInfl2,
      bool pCheckGenders);
};



} // End of namespace linguistics
} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOOL_INFLECTION_INFLECTIONSCHECKERVIRTUAL_HPP
