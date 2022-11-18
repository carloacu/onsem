#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_SYNTHESIZERDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_SYNTHESIZERDICTIONARY_HPP

#include <mutex>
#include <map>
#include <unordered_map>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include "../../api.hpp"

namespace onsem
{
namespace linguistics
{
struct WordAssociatedInfos;


class ONSEM_TEXTTOSEMANTIC_API SynthesizerDictionary
{
public:
  SynthesizerDictionary(std::istream& pIStream,
                        const StaticConceptSet& pConceptsDb,
                        const StaticLinguisticDictionary& pStatLingDic,
                        SemanticLanguageEnum pLangEnum);

  void addInflectedWord(const std::string& pInflectedFrom,
                        const SemanticWord& pWord,
                        const Inflections& pInflections,
                        char pFrequency);
  void addInfosToAWord(const SemanticWord& pWord,
                       const WordAssociatedInfos* pWordAssociatedInfos);
  void reset();

  LinguisticMeaning conceptToMeaning(const std::string& pConcept) const;

  bool hasContextualInfo(WordContextualInfos pContextualInfo,
                         const LinguisticMeaning& pMeaning) const;

  std::string getLemma(const LinguisticMeaning& pMeaning,
                       bool pWithLinkMeanings) const;

  void getNounGendersFromWord(std::set<SemanticGenderType>& pGenders,
                              const SemanticWord& pWord,
                              SemanticNumberType pNumber) const;
  void getNounGenders(std::set<SemanticGenderType>& pGenders,
                      const LinguisticMeaning& pMeaning,
                      SemanticNumberType pNumber) const;

  void getNounForm(std::string& pRes,
                   const LinguisticMeaning& pLinguisticMeaning,
                   SemanticGenderType& pGender,
                   SemanticNumberType& pNumber) const;


  const StaticSynthesizerDictionary& statDb;

private:
  struct InflFormAndFrequency
  {
    InflFormAndFrequency(const std::string& pInflectedFrom,
                         char pFrequency)
      : inflectedFrom(pInflectedFrom),
        frequency(pFrequency)
    {
    }
    bool operator<(const InflFormAndFrequency& pOther)
    {
      if (frequency != pOther.frequency)
        return frequency < pOther.frequency;
      return inflectedFrom < pOther.inflectedFrom;
    }
    std::string inflectedFrom;
    char frequency;
  };
  const StaticLinguisticDictionary& _lingDict;
  std::map<SemanticWord, std::map<NominalInflection, InflFormAndFrequency>> _wordToInflections;
  std::unordered_map<std::string, std::map<SemanticWord, const WordAssociatedInfos*>> _conceptToInfoGrams;

  static std::mutex _pathToStatDbsMutex;
  static std::map<SemanticLanguageEnum, std::unique_ptr<StaticSynthesizerDictionary>> _statDbs;
  static const StaticSynthesizerDictionary& _getStatDbInstance(std::istream& pIStream,
                                                               const StaticConceptSet& pConceptsDb,
                                                               const StaticLinguisticDictionary& pStatLingDic,
                                                               SemanticLanguageEnum pLangEnum);

  void _addInflectionsIfMoreFrequent(std::map<NominalInflection, InflFormAndFrequency>& pMap,
                                     const NominalInflection& pNominalInflection,
                                     const InflFormAndFrequency& pInflFormAndFrequency);
  bool _tryGetInflectedFrom(std::string& pRes,
                            SemanticGenderType pGender,
                            SemanticNumberType pNumber,
                            const std::map<NominalInflection, InflFormAndFrequency>& pInflections) const;
};



} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_SYNTHESIZERDICTIONARY_HPP
