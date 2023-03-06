#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_GROUNDING_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_GROUNDING_HPP

#include <map>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include "../tool/synthesizeradder.hpp"
#include "../synthesizertypes.hpp"



namespace onsem
{
namespace linguistics
{
struct InflectedWord;
struct LingWordsGroup;
struct LinguisticDatabase;
struct SpecificLinguisticDatabase;
class StaticSynthesizerDictionary;
class SynthesizerDictionary;
}
struct GroundedExpression;
struct SemanticGrounding;
struct SemanticTimeGrounding;
struct SemanticDurationGrounding;
struct SemanticStatementGrounding;
struct SemanticGenericGrounding;
struct SemanticAgentGrounding;
struct SemanticTextGrounding;
struct SemanticMetaGrounding;
struct StaticLinguisticMeaning;
class SynthesizerChunksMerger;
class LinguisticSynthesizerPrivate;


class Linguisticsynthesizergrounding
{
public:
  virtual ~Linguisticsynthesizergrounding() {}


  virtual const SynthesizerChunksMerger& getChunksMerger() const = 0;

  void writeListSeparators
  (std::list<WordToSynthesize>& pOut,
   ListExpressionType pListType,
   bool pBeforeLastElt) const;

  void modifyContextForAGrounding
  (SynthesizerWordContext& pWordContext,
   linguistics::InflectedWord& pOutInfoGram,
   const SynthesizerConfiguration& pConf,
   const SemanticGrounding& pGrounding,
   SynthesizerCurrentContextType pContextType,
   LinguisticVerbTense pVerbTense) const;

  void writeGroundingIntroduction
  (OutSemExp& pBeforeOut,
   const OutSemExp& pOut,
   const linguistics::InflectedWord& pOutInfoGram,
   SynthesizerCurrentContext& pContext,
   SynthesizerConfiguration& pConf,
   const SemanticGrounding& pGrounding,
   const GroundedExpression& pHoldingGrdExp) const;

  void writeGrounding
  (OutSemExp& pOutSemExp,
   const linguistics::InflectedWord& pOutInfoGram,
   SynthesizerCurrentContext& pContext,
   SynthesizerConfiguration& pConf,
   const SemanticGrounding& pGrounding,
   const GroundedExpression& pHoldingGrdExp,
   const SemanticRequests& pRequests) const;

  bool angleTranslation
  (std::list<WordToSynthesize>& pOut,
   const linguistics::SynthesizerDictionary& pStatSynthDico,
   const SemanticAngle& pAngle,
   bool pPrintPrecisely) const;

  bool lengthTranslation
  (std::list<WordToSynthesize>& pOut,
   const linguistics::SynthesizerDictionary& pStatSynthDico,
   const SemanticLength& pLength,
   bool pPrintPrecisely) const;

  bool durationTranslation
  (std::list<WordToSynthesize>& pOut,
   const linguistics::SynthesizerDictionary& pStatSynthDico,
   const SemanticDuration& pDuration,
   bool pPrintPrecisely) const;

  void statGroundingTranslation
  (OutSentence& pOutSentence,
   const SynthesizerConfiguration& pConf,
   const SemanticStatementGrounding& pStatementGrd,
   const linguistics::InflectedWord& pVerbInfoGram,
   const GroundedExpression& pHoldingGrdExp,
   const SynthesizerCurrentContext& pVerbContext,
   const UniqueSemanticExpression* pSubjectPtr) const;

  void langGroundingTranslation
  (std::list<WordToSynthesize>& pOut,
   const SemanticLanguageGrounding& pGrounding) const;

  void textGroundingTranslation
  (std::list<WordToSynthesize>& pOut,
   const SemanticTextGrounding& pGrounding,
   const SynthesizerCurrentContext& pContext) const;

  void nameInfosTranslation
  (std::list<WordToSynthesize>& pOut,
   const NameInfos& pNameInfos) const;

  void resourceGroundingTranslation
  (std::list<WordToSynthesize>& pOut,
   const SemanticResourceGrounding& pGrounding) const;

  void unityGroundingTranslation
  (std::list<WordToSynthesize>& pOut,
   const SemanticUnityGrounding& pGrounding,
   const linguistics::SynthesizerDictionary& pStatSynthDico) const;

  void metaGroundingTranslation
  (std::list<WordToSynthesize>& pOut,
   const SemanticMetaGrounding& pGrounding) const;

  void agentGroundingTranslation
  (OutSemExp& pOutSemExp,
   SynthesizerConfiguration& pConf,
   const SemanticAgentGrounding& pGrounding,
   const SynthesizerCurrentContext& pContext,
   const SemanticRequests& pRequests) const;

  void writePreposition(std::list<WordToSynthesize>& pOut,
                        const OutSemExp* pSubordinateOutPtr,
                        const linguistics::InflectedWord& pInflectedVerb,
                        const SynthesizerCurrentContext& pContext,
                        const SynthesizerConfiguration& pConf,
                        const SemanticGrounding& pGrounding,
                        const GroundedExpression& pHoldingGrdExp) const;

  void writePrepositionWithoutContext(std::list<WordToSynthesize>& pOut,
                                      const SynthesizerCurrentContext& pContext,
                                      const SynthesizerConfiguration& pConf) const;

  void beforeGenGroundingTranslation
  (OutSemExp& pBeforeOut,
   const linguistics::InflectedWord& pOutInfoGram,
   const SynthesizerCurrentContext& pContext,
   const SynthesizerConfiguration& pConf,
   const SemanticGenericGrounding& pGrounding,
   const GroundedExpression& pHoldingGrdExp) const;

  void genGroundingTranslation
  (OutSemExp& pOutSemExp,
   const linguistics::InflectedWord& pOutInfoGram,
   SynthesizerCurrentContext& pContext,
   const linguistics::LinguisticDatabase& pLingDb,
   const SemanticGenericGrounding& pGrounding) const;

  void timeGroundingTranslation
  (std::list<WordToSynthesize>& pOut,
   const linguistics::LinguisticDatabase& pLingDb,
   const SemanticTimeGrounding& pGrounding) const;

  virtual PartOfSpeech writeRelativePerson
  (std::list<WordToSynthesize>& pOut,
   RelativePerson& pRelativePerson,
   SemanticReferenceType pReferenceType,
   bool pHasToBeCompletedFromContext,
   SemanticEntityType pAgentType,
   const SemanticQuantity& pQuantity,
   const SynthesizerCurrentContext& pContext,
   const SemanticRequests& pRequests) const = 0;

  virtual void rankConceptToInlfWord
  (linguistics::InflectedWord& pInflWord,
   const std::map<std::string, char>& pConcepts) const = 0;

  virtual void writeReflexiveObject(OutSentence& pOutSentence,
                                    std::list<WordToSynthesize>& pOut,
                                    RelativePerson pSubjectRelativePerson,
                                    SemanticGenderType pGender,
                                    const mystd::optional<SynthesizerVerbContext>& pVerbContextOpt) const = 0;

  SemanticLanguageEnum getOutLanguage() const { return _language; }

  void writeVerbalSemWord
  (std::list<WordToSynthesize>& pBeforeOut,
   OutSemExp& pOutSemExp,
   std::list<WordToSynthesize>& pAfterOut,
   const SemanticWord& pWord,
   const linguistics::LinguisticDatabase& pLingDb,
   const SynthesizerCurrentContext& pContext) const;

  void writeSemWord
  (std::list<WordToSynthesize>& pBeforeOut,
   OutSemExp& pOutSemExp,
   std::list<WordToSynthesize>& pAfterOut,
   const SemanticWord& pWord,
   const linguistics::LinguisticDatabase& pLingDb,
   SynthesizerCurrentContext& pContext) const;

  RelativePerson agentTypeToRelativePerson
  (const SemanticAgentGrounding& pGrounding,
   const SynthesizerConfiguration& pConf,
   bool pCanReplaceByEquivalent) const;

  void getIGramOfAStatementMeaning
  (linguistics::InflectedWord& pIGram,
   const SemanticStatementGrounding& pStatGrounding,
   const SynthesizerConfiguration& pConf) const;

  void getIGramOfAWord
  (linguistics::InflectedWord& pIGram,
   const SemanticWord& pWord,
   const std::map<std::string, char>& pConcepts,
   const SynthesizerConfiguration& pConf) const;

  virtual void writeReLocationType(std::list<WordToSynthesize>& pOut,
                                   SemanticRelativeLocationType pLocationType) const = 0;


protected:
  const SemanticLanguageEnum _language;
  const LinguisticSynthesizerPrivate& _lingSynth;

  Linguisticsynthesizergrounding(SemanticLanguageEnum pLanguage,
                                 const LinguisticSynthesizerPrivate& pLingSynth)
    : _language(pLanguage),
      _lingSynth(pLingSynth)
  {
  }

  void _strToOut
  (std::list<WordToSynthesize>& pOut,
   PartOfSpeech pPartOfSpeech,
   const std::string& pStr,
   WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY) const
  { synthTool::strToOut(pOut, pPartOfSpeech, pStr, _language, pTag); }

  void _strToOutCptsMove
  (std::list<WordToSynthesize>& pOut,
   PartOfSpeech pPartOfSpeech,
   const std::string& pStr,
   const std::map<std::string, char>&& pConcepts,
   WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY) const
  { synthTool::strToOutCptsMove(pOut, pPartOfSpeech, pStr, _language, std::move(pConcepts), pTag); }

  bool _strToOutIfNotEmpty
  (std::list<WordToSynthesize>& pOut,
   PartOfSpeech pPartOfSpeech,
   const std::string& pStr,
   const std::set<WordContextualInfos>* pContextualInfosPtr = nullptr) const
  { return synthTool::strToOutIfNotEmpty(pOut, pPartOfSpeech, pStr, _language, pContextualInfosPtr); }

  void _strWithApostropheToOut
  (std::list<WordToSynthesize>& pOut,
   PartOfSpeech pPartOfSpeech,
   const std::string& pStrApos,
   const std::string& pStr,
   bool (*pContextCondition)(const OutSentence*) = [](const OutSentence*){ return true; }) const
  { synthTool::strWithApostropheToOut(pOut, pPartOfSpeech, pStrApos, pStr, _language, pContextCondition); }

  virtual bool _dateTranslation
  (std::list<WordToSynthesize>& pOut,
   const linguistics::SynthesizerDictionary& pDicoSynth,
   const SemanticDate& pDate) const = 0;

  virtual bool _dayHourTranslation(std::list<WordToSynthesize>& pOut,
                                   const linguistics::SynthesizerDictionary& pStatSynthDico,
                                   const SemanticDuration& pDuration,
                                   bool pDateWritten) const = 0;

  virtual void _writeDurationAgo(std::list<WordToSynthesize>& pOut) const = 0;
  virtual void _writeDurationIn(std::list<WordToSynthesize>& pOut) const = 0;

  virtual void _writeReTimeType
  (std::list<WordToSynthesize>& pOut,
   SemanticRelativeTimeType pTimeType,
   LinguisticVerbTense pVerbTense) const = 0;

  virtual void _writeReDurationType
  (std::list<WordToSynthesize>& pOut,
   SemanticRelativeDurationType pDurationType,
   const GroundedExpression& pHoldingGrdExp) const = 0;

  virtual bool _doWeHaveToPutSubMeaningBeforeOrAfterTheWord
  (const SynthesizerCurrentContext&,
   LinkedMeaningDirection pDirection) const
  { return pDirection != LinkedMeaningDirection::FORWARD; }

  virtual void _getPronounComplement
  (std::list<WordToSynthesize>& pOut,
   const StaticLinguisticMeaning& pMeaning,
   const linguistics::SpecificLinguisticDatabase& pSpecLingDb,
   const SynthesizerCurrentContext& pContext) const;

  virtual void _getAllWord(std::list<WordToSynthesize>& pOut,
                           const SynthesizerCurrentContext& pContext) const = 0;
  virtual void _getEveryWord(std::list<WordToSynthesize>& pOut,
                             const SynthesizerCurrentContext& pContext) const = 0;

  virtual void _getDeterminerThatReferToRecentContext
  (std::list<WordToSynthesize>& pOut,
   SemanticNumberType pNumber,
   SemanticGenderType pGender) const = 0;

  virtual void _getDeterminer(std::list<WordToSynthesize>& pOut,
                              const SemanticGenericGrounding& pGrounding,
                              const GroundedExpression& pHoldingGrdExp,
                              const SynthesizerCurrentContext& pContext) const = 0;

  virtual void _writeReflexivePronoun
  (std::list<WordToSynthesize>& pOut,
   RelativePerson pRelativePerson,
   SemanticGenderType pGender) const = 0;

  bool _writeVerbalLinguisticMeaning(std::list<WordToSynthesize>& pBeforeOut,
                                     OutSemExp& pOutSemExp,
                                     std::list<WordToSynthesize>& pAfterOut,
                                     const StaticLinguisticMeaning& pLingMeaning,
                                     const linguistics::LinguisticDatabase& pLingDb,
                                     const SynthesizerCurrentContext& pContext,
                                     bool pTryToWriteInSubordianteForm) const;

  virtual bool _writeVerbGoal(OutSentence& pOutSentence,
                              SynthesizerCurrentContext& pVerbContext,
                              const SemanticStatementGrounding& pStatementGrd,
                              const linguistics::InflectedWord& pOutInfoGram,
                              const SynthesizerConfiguration& pConf,
                              const UniqueSemanticExpression* pSubjectPtr) const = 0;

  virtual std::string _usRelativePersonToStr(SynthesizerCurrentContextType pContextType,
                                             LinguisticVerbTense pVerbTense) const = 0;

  virtual bool _groundingAttributesToWord(linguistics::InflectedWord& pOutInfoGram,
                                          const SemanticGenericGrounding& pGrounding) const = 0;

private:
  void _printIntroWord(std::list<WordToSynthesize>& pOut,
                       const SemanticWord& introWord,
                       const SynthesizerCurrentContext& pContext) const;

  bool _needToWriteDeterminer
  (const linguistics::InflectedWord& pOutInfoGram,
   const SynthesizerCurrentContext& pContext,
   const SemanticGenericGrounding& pGrounding,
   const GroundedExpression& pHoldingGrdExp,
   const linguistics::LinguisticDatabase& pLingDb,
   bool pOwnerWrittenBefore) const;

  void _statGroundingTranslation
  (OutSentence& pOutSentence,
   const linguistics::LinguisticDatabase& pLingDb,
   const linguistics::InflectedWord& pOutInfoGram,
   const GroundedExpression& pHoldingGrdExp,
   const SynthesizerCurrentContext& pContext) const;

  void _writeSubMeanings
  (std::list<WordToSynthesize>& pOut,
   const linguistics::LinguisticDatabase& pLingDb,
   const linguistics::LingWordsGroup& pMeaningsGroup,
   const SynthesizerCurrentContext& pContext,
   bool pBeforeVerb) const;

  void _writeGenGrounding
  (OutSemExp& pOutSemExp,
   const linguistics::InflectedWord& pInfoGram,
   const linguistics::LinguisticDatabase& pLingDb,
   const SemanticGenericGrounding& pGrounding,
   SynthesizerCurrentContext& pContext) const;

  void _modifyContextForAGenGrounding
  (SynthesizerWordContext& pWordContext,
   linguistics::InflectedWord& pOutInfoGram,
   const SynthesizerConfiguration& pConf,
   const SemanticGenericGrounding& pGrounding) const;

  static std::string _getRootLemma(const std::string& pLemma);

  bool _getIntroductingWordStoredInTheGrdExp(mystd::optional<SemanticWord>& pIntroWord,
                                             const GroundedExpression& pGrdExp,
                                             const SynthesizerConfiguration& pConf) const;
};



} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_GROUNDING_HPP
