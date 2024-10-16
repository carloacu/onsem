#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZEGETTER_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZEGETTER_HPP

#include <list>
#include <set>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>

namespace onsem {
namespace linguistics {
struct InflectedWord;
struct LinguisticDatabase;
class SynthesizerDictionary;
}
struct UniqueSemanticExpression;
struct SemanticGenericGrounding;
struct LinguisticMeaning;
struct SemanticExpression;
struct WordToSynthesize;
class Linguisticsynthesizergrounding;
struct SynthesizerCurrentContext;
struct SynthesizerConfiguration;
struct SemanticGrounding;
struct SemanticWord;

namespace synthGetter {

void fillLingMeaningFromConcepts(LinguisticMeaning& pLingMeaning,
                                 const std::map<std::string, char>& pConcepts,
                                 const linguistics::SynthesizerDictionary& pOutSynth);

SemanticGenderType getGender(SemanticGenderType pContextGender,
                             const std::set<SemanticGenderType>& pMeaningPossGenders,
                             const std::set<SemanticGenderType>& pGenGrdPossGenders);

bool getInflWordFromWordAndConcepts(linguistics::InflectedWord& pIGram,
                                    const SemanticWord& pWord,
                                    const std::map<std::string, char>& pConcepts,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    SemanticLanguageEnum pLanguage);

SemanticGenderType getGenderFromGrounding(const SemanticGrounding& pGrounding,
                                          const SynthesizerConfiguration& pConf,
                                          const SynthesizerCurrentContext& pContext,
                                          const Linguisticsynthesizergrounding& pGrdSynth);

SemanticGenderType getGenderFromSemExp(const SemanticExpression& pSemExp,
                                       const SynthesizerConfiguration& pConf,
                                       const SynthesizerCurrentContext& pContext,
                                       const Linguisticsynthesizergrounding& pGrdSynth);

bool doesOutFinishedWithAS(const std::list<WordToSynthesize>& pOut);

}    // End of namespace synthGetter
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZEGETTER_HPP
