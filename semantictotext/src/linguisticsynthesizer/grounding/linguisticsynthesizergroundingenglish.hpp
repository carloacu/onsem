#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_GROUNDINGENGLISH_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_GROUNDINGENGLISH_HPP

#include "linguisticsynthesizergrounding.hpp"
#include "../merger/synthesizerchunksmergerenglish.hpp"

namespace onsem
{
namespace linguistics
{
class StaticSynthesizerDictionary;
}

class LinguisticsynthesizergroundingEnglish : public Linguisticsynthesizergrounding
{
public:
  LinguisticsynthesizergroundingEnglish(const LinguisticSynthesizerPrivate& pLingSynth);

  virtual const SynthesizerChunksMerger& getChunksMerger() const
  { return _chunksMerger; }

  virtual PartOfSpeech writeRelativePerson
  (std::list<WordToSynthesize>& pOut,
   RelativePerson pRelativePerson,
   SemanticReferenceType pReferenceType,
   bool pHasToBeCompletedFromContext,
   SemanticEntityType pAgentType,
   const SemanticQuantity& pQuantity,
   const SynthesizerCurrentContext& pContext,
   const SemanticRequests& pRequests) const;

  virtual void rankConceptToInlfWord(linguistics::InflectedWord& pInflWord,
                                     const std::map<std::string, char>& pConcepts) const;

  virtual void writeReflexiveObject(OutSentence& pOutSentence,
                                    std::list<WordToSynthesize>& pOut,
                                    RelativePerson pSubjectRelativePerson,
                                    SemanticGenderType pGender,
                                    const mystd::optional<SynthesizerVerbContext>& pVerbContextOpt) const;

  virtual void writeReLocationType(std::list<WordToSynthesize>& pOut,
                                   SemanticRelativeLocationType pLocationType) const;


protected:
  virtual bool _dateTranslation
  (std::list<WordToSynthesize>& pOut,
   const linguistics::SynthesizerDictionary& pDicoSynth,
   const SemanticDate& pDate) const;

  virtual void _writeDurationAgo(std::list<WordToSynthesize>& pOut) const;
  virtual void _writeDurationIn(std::list<WordToSynthesize>& pOut) const;

  virtual bool _dayHourTranslation(std::list<WordToSynthesize>& pOut,
                                   const linguistics::SynthesizerDictionary& pStatSynthDico,
                                   const SemanticDuration& pDuration,
                                   bool pDateWritten) const;

  virtual void _writeReTimeType
  (std::list<WordToSynthesize>& pOut,
   SemanticRelativeTimeType pTimeType,
   LinguisticVerbTense pVerbTense) const;

  virtual void _writeReDurationType
  (std::list<WordToSynthesize>& pOut,
   SemanticRelativeDurationType pDurationType,
   const GroundedExpression& pHoldingGrdExp) const;

  virtual void _getAllWord(std::list<WordToSynthesize>& pOut,
                           const SynthesizerCurrentContext& pContext) const;
  virtual void _getEveryWord(std::list<WordToSynthesize>& pOut,
                             const SynthesizerCurrentContext& pContext) const;

  virtual void _getDeterminerThatReferToRecentContext
  (std::list<WordToSynthesize>& pOut,
   SemanticNumberType pNumber,
   SemanticGenderType pGender) const;

  virtual void _getDeterminer(std::list<WordToSynthesize>& pOut,
                              const SemanticGenericGrounding& pGrounding,
                              const GroundedExpression& pHoldingGrdExp,
                              const SynthesizerCurrentContext& pContext) const;

  virtual void _writeReflexivePronoun
  (std::list<WordToSynthesize>& pOut,
   RelativePerson pRelativePerson,
   SemanticGenderType pGender) const;

  virtual bool _writeVerbGoal(OutSentence& pOutSentence,
                              SynthesizerCurrentContext& pVerbContext,
                              const SemanticStatementGrounding& pStatementGrd,
                              const linguistics::InflectedWord& pOutInfoGram,
                              const SynthesizerConfiguration& pConf,
                              bool pHaveASubject) const;

  virtual std::string _usRelativePersonToStr(SynthesizerCurrentContextType pContextType,
                                             LinguisticVerbTense pVerbTense) const;

  virtual bool _groundingAttributesToWord(linguistics::InflectedWord& pOutInfoGram,
                                          const SemanticGenericGrounding& pGrounding) const;

private:
  const SynthesizerChunksMergerEnglish _chunksMerger;

  PartOfSpeech _writeRelativePerson(std::list<WordToSynthesize>& pOut,
                                    RelativePerson pRelativePerson,
                                    SemanticGenderType pGender,
                                    SemanticReferenceType pReferenceType,
                                    SemanticEntityType pAgentType,
                                    const SemanticQuantity& pQuantity,
                                    SynthesizerCurrentContextType pContextType,
                                    LinguisticVerbTense pVerbTense,
                                    bool pIsPosisitve) const;
};


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_GROUNDINGENGLISH_HPP
