#ifndef ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZERPRIVATE_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZERPRIVATE_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include <onsem/common/enum/semanticentitytype.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>
#include "synthesizerresulttypes.hpp"
#include "synthesizertypes.hpp"
#include "tool/synthesizeradder.hpp"

namespace onsem
{
namespace linguistics
{
struct InflectedWord;
struct LingWordsGroup;
struct WordAssociatedInfos;
}
struct SemanticDurationGrounding;
struct SemanticStatementGrounding;
struct SemanticGenericGrounding;
struct SemanticTextGrounding;
struct SemanticAgentGrounding;
struct ListExpression;
struct GroundedExpression;
class Linguisticsynthesizergrounding;
class SynthesizerChunksMerger;
class TokensToStringConverter;
struct SemanticMemoryBlock;


class LinguisticSynthesizerPrivate
{
public:
  static const LinguisticSynthesizerPrivate& getInstance(SemanticLanguageEnum pLangueType);

  virtual ~LinguisticSynthesizerPrivate() {}

  void getTranslationToNaturalLanguage
  (std::list<std::unique_ptr<SynthesizerResult> >& pNaturalLanguageResult,
   const SemanticExpression& pSemExp,
   bool pOneLinePerSentence,
   const SemanticMemoryBlock& pMemBlock,
   const std::string& pCurrentUserId,
   const TextProcessingContext& pTextProcContext,
   const linguistics::LinguisticDatabase& pLingDb) const;

  SemExpTypeEnumFormSynt writeGrdExp
  (OutSemExp& pOutSemExp,
   SemanticRequests& pRequests,
   SemanticExpression const** pLastSubject,
   const GroundedExpression& pGrdExp,
   SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const;

  virtual const Linguisticsynthesizergrounding& getSyntGrounding() const = 0;
  virtual const TokensToStringConverter& getTokenToStrConverter() const = 0;


protected:
  const SemanticLanguageEnum _language;

  enum class SpecifierPosition
  {
    BEFORE_DETERMINER,
    BEFORE,
    AFTER
  };

  LinguisticSynthesizerPrivate(SemanticLanguageEnum pLanguage)
    : _language(pLanguage)
  {
  }

  static void _initInstances();

  virtual const SynthesizerChunksMerger& _getChunksMerger() const = 0;
  virtual LinguisticVerbTense _semanticVerbTenseToLinguisticVerbTense(SemanticVerbTense pSemVerbTense,
                                                                      SynthesizerCurrentContextType pContextType,
                                                                      SemanticStatementGrounding const* pRootStatementPtr,
                                                                      const SemanticRequests& pRequests,
                                                                      const linguistics::WordAssociatedInfos* pWordInfosPtr = nullptr) const  = 0;

  void _strToOut
  (std::list<WordToSynthesize>& pOut,
   PartOfSpeech pPartOfSpeech,
   const std::string& pStr) const
  { synthTool::strToOut(pOut, pPartOfSpeech, pStr, _language); }

  bool _strToOutIfNotEmpty
  (std::list<WordToSynthesize>& pOut,
   PartOfSpeech pPartOfSpeech,
   const std::string& pStr) const
  { return synthTool::strToOutIfNotEmpty(pOut, pPartOfSpeech, pStr, _language); }

  void _strWithApostropheToOut
  (std::list<WordToSynthesize>& pOut,
   PartOfSpeech pPartOfSpeech,
   const std::string& pStrApos,
   const std::string& pStr) const
  { synthTool::strWithApostropheToOut(pOut, pPartOfSpeech, pStrApos, pStr, _language); }

  virtual bool _subConceptBeforeOrAfter(const linguistics::WordAssociatedInfos& pInfo) const = 0;
  virtual void _getNegationsBeforeVerb(std::list<WordToSynthesize>& pOut) const = 0;
  virtual void _getNegationsAfterVerb(std::list<WordToSynthesize>& pOut) const = 0;

  enum class ObjectPosition
  {
    BEFORESUBJECT,
    BEFOREVERB,
    JUSTAFTERVERB,
    AFTERVERB
  };

  enum class ReceiverPosition
  {
    BEFOREVERB,
    AFTERVERB
  };

  virtual ObjectPosition _getObjectPosition(SyntSentWorkStruct& pSentWorkStruct,
                                            const SemanticStatementGrounding& pStatementGrd,
                                            const SemanticRequests& pRequests,
                                            const SynthesizerConfiguration& pConf,
                                            LinguisticVerbTense pVerbTense) const = 0;

  virtual ReceiverPosition _getReceiverPosition(const SemanticExpression& pSemExpObj,
                                                bool pVerbIsAffirmative,
                                                const SemanticRequests& pRequests,
                                                const SynthesizerConfiguration& pConf) const = 0;

  virtual void _getQuestionWord
  (std::list<WordToSynthesize>& pOut,
   OutSemExp& pSubjectOut,
   const SemanticRequests& pRequests,
   bool pIsEquVerb,
   SemanticVerbTense pVerbTense,
   const SemanticStatementGrounding& pStatGrd,
   const UniqueSemanticExpression* pSubjectPtr,
   const UniqueSemanticExpression* pObjectPtr,
   bool pIsPassive,
   ObjectPosition pObjectPosition,
   const UniqueSemanticExpression*& pChildToPutBeforeSubject,
   const SynthesizerConfiguration& pConf,
   SynthesizerCurrentContextType pHoldingContextType,
   const SynthesizerCurrentContext& pContext,
   bool pNeedToWriteTheVerb) const = 0;

  virtual bool _tryToWriteTopicBeforeVerb
  (std::list<WordToSynthesize>&,
   const SemanticExpression&) const { return false; }

  virtual void _getBeginOfWithChild(std::list<WordToSynthesize>& pOut) const = 0;
  virtual void _getBeginOfWithoutChild(std::list<WordToSynthesize>& pOut) const = 0;

  virtual void _getBeginOfForChild
  (std::list<WordToSynthesize>& pOut,
   const SemanticExpression& pSemExp) const = 0;

  virtual void _getBeginOfReceiverOfNounChild
  (std::list<WordToSynthesize>& pOut,
   const SemanticExpression& pSemExp) const = 0;

  virtual void _getOfWord(std::list<WordToSynthesize>& pOut,
                          const SynthesizerWordContext& pWordContext) const = 0;
  virtual void _getRelTimeFollowingPrep(std::list<WordToSynthesize>& pOut,
                                        const SemanticRelativeTimeGrounding& pRelTimeGrd) const = 0;

  virtual void _getBeginOfSpecification
  (std::list<WordToSynthesize>& pOut,
   const GroundedExpression& pChildGrdExp,
   const SemanticGrounding& pParentGrounding,
   const SynthesizerWordContext& pWordContext) const = 0;

  virtual void _getThanWord
  (std::list<WordToSynthesize>& pOut,
   const mystd::unique_propagate_const<UniqueSemanticExpression>& pWhatIsComparedExp) const = 0;

  virtual void _getBeginOfMitigationChild
  (std::list<WordToSynthesize>& pOut) const = 0;

  virtual void _getBeginOfYesOrNoSubordonate
  (std::list<WordToSynthesize>& pOut) const = 0;

  virtual bool _beginOfSubordonateIfNeeded(bool& pIsASubordinateWithoutPreposition,
                                           OutSemExp& pOutSemExp,
                                           const SyntSentWorkStruct& pSentWorkStruct,
                                           GrammaticalType pSubordinateGrammaticalType,
                                           const GroundedExpression& pSubordinateGrdExp,
                                           const mystd::optional<SynthesizerVerbContext>& pVerbContextOpt,
                                           const SynthesizerConfiguration& pConf) const = 0;

  virtual void _getWheneverCondition
  (std::list<WordToSynthesize>& pOut) const = 0;

  virtual void _writeSubjectOfGeneralitySentence(OutSentence& pOutSentence) const = 0;
  virtual void _writeGenericSubject(OutSentence& pOutSentence) const = 0;
  virtual void _getBeginOfBetweenSubordonate(std::list<WordToSynthesize>& pOut) const = 0;
  virtual void _getBeginOfCauseSubordonate(std::list<WordToSynthesize>& pOut) const = 0;

  virtual void _getThenWord
  (std::list<WordToSynthesize>& pOut) const = 0;

  virtual void _getElseWord
  (std::list<WordToSynthesize>& pOut) const = 0;

  virtual void _getPossessiveWord
  (std::list<WordToSynthesize>& pOut,
   const SynthesizerWordContext& pCoreferenceWordContext,
   RelativePerson pOwnerRelativePerson,
   const SemanticGrounding& pGrounding,
   const SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const = 0;

  virtual void _getPossessiveDeterminer
  (std::list<WordToSynthesize>& pOut,
   RelativePerson pRelPerson,
   SemanticGenderType pGender,
   SemanticNumberType pNumber) const = 0;

  virtual SpecifierPosition _getSpecifierPosition(
      const linguistics::InflectedWord& pInflWord,
      const SemanticGrounding& pGrounding,
      bool pADetermantHasBeenWrote,
      const linguistics::InflectedWord* pMotherInflWordPtr) const = 0;

  virtual bool _doWeHaveToWriteBeginOfSpecifier
  (SynthesizerCurrentContext& pGrdContext,
   const linguistics::InflectedWord& pInflWord,
   const GroundedExpression& pGrdExpOfTheWord,
   const SemanticGrounding& pMotherGrounding) const = 0;

  virtual void _getComparisonWord(std::list<WordToSynthesize>& pOut,
                                  ComparisonOperator pCompPolarity,
                                  SemanticGenderType pGender,
                                  SemanticNumberType pNumber) const = 0;


private:
  static std::map<SemanticLanguageEnum, std::unique_ptr<LinguisticSynthesizerPrivate>> _instances;

  void _writeSemExp
  (std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
   OutSemExp& pOutSemExp,
   SemanticRequests& pRequests,
   SemanticExpression const** pLastSubject,
   const SemanticExpression& pSemExp,
   SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const;

  bool _handleSpecificLanguageSynthesis(std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
                                        OutSemExp& pOutSemExp,
                                        SemanticRequests& pRequests,
                                        SemanticExpression const** pLastSubject,
                                        const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations,
                                        const SemanticExpression& pSemExpToExecute,
                                        SynthesizerConfiguration& pConf,
                                        const SynthesizerCurrentContext& pContext) const;

  void _addPunctuation(std::list<WordToSynthesize>& pOut,
                       const std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
                       const SemanticRequests& pRequests,
                       bool pOneLinePerSentence) const;

  void _startAQuestion
  (std::list<WordToSynthesize>& pOut,
   OutSemExp& pSubjectOut,
   const SemanticRequests& pRequests,
   bool pEquVerb,
   SemanticVerbTense pVerbTense,
   const SemanticStatementGrounding& pStatGrd,
   const UniqueSemanticExpression* pSubjectPtr,
   const UniqueSemanticExpression* pObjectPtr,
   bool pIsPassive,
   ObjectPosition pObjectPosition,
   const UniqueSemanticExpression*& pChildToPutBeforeSubject,
   const SynthesizerConfiguration& pConf,
   SynthesizerCurrentContextType pHoldingContextType,
   const SynthesizerCurrentContext& pContext,
   bool pNeedToWriteTheVerb) const;

  void _writeSentenceGrdExp
  (std::list<WordToSynthesize>& pOut,
   SemanticRequests& pRequests,
   SemanticExpression const** pLastSubject,
   const GroundedExpression& pGrdExp,
   const SemanticStatementGrounding& pStatementGrd,
   SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const;

  void _writeNominalChild(OutNominalGroup& outNominalGroup,
                          SemanticRequests& pRequests,
                          SemanticExpression const** pLastSubject,
                          SynthesizerConfiguration& pConf,
                          GrammaticalType pChildGrammaticalType,
                          const SemanticExpression& currSemExpChild,
                          SynthesizerCurrentContext& pContext,
                          const linguistics::InflectedWord* pInflWordPtr,
                          const GroundedExpression* pGrdExpPtr) const;

  void _writeNominalGrdExp
  (OutSemExp& pOutSemExp,
   SemanticRequests& pRequests,
   SemanticExpression const** pLastSubject,
   const GroundedExpression& pGrdExp,
   const linguistics::InflectedWord& pOutInfoGram,
   SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const;

  SemExpTypeEnumFormSynt _writeCondExp
  (OutSemExp& pOutSemExp,
   SemanticRequests& pRequests,
   SemanticExpression const** pLastSubject,
   const ConditionExpression& pCondExp,
   SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const;

  void _writeObjects(OutSemExp& pOutSemExp,
                     OutSentence& pCurrSentence,
                     const SyntSentWorkStruct& pSentWorkStruct,
                     const SynthesizerCurrentContext& pVerbContext,
                     const SemanticStatementGrounding& pStatementGrd,
                     ObjectPosition pObjectPosition,
                     const SemanticExpression& pObjectSemExp,
                     SynthesizerConfiguration& pConf,
                     const SemanticRequests& pRequests,
                     bool pIsANameAssignement,
                     SemanticExpression const** pLastSubject) const;

  SemExpTypeEnumFormSynt _writeComparisonExp
  (std::list<WordToSynthesize>& pOut,
   SemanticRequests& pRequests,
   SemanticExpression const** pLastSubject,
   const ComparisonExpression& pCompExp,
   SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const;

  void _writeListExp(std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
                     OutSemExp& pOutSemExp,
                     SynthesizerConfiguration& pConf,
                     const ListExpression& pListExp,
                     const SynthesizerCurrentContext& pContext,
                     SemanticExpression const** pLastSubject) const;


  void _writeReceiver(OutSemExp& pOutSemExpBeforeVerb,
                      OutSemExp &pOutSemExpJustAfterVerb,
                      OutSemExp& pOutSemExpAfterVerb,
                      const SemanticExpression& pSemExp,
                      SynthesizerConfiguration& pConf,
                      bool pVerbIsAffirmative,
                      const SemanticRequests& pRequests,
                      const SynthesizerCurrentContext& pVerbContext,
                      SemanticExpression const** pLastSubject) const;

  bool _writeReflexionIfNeeded(SyntSentWorkStruct& pSentWorkStruct,
                               OutSemExp& pOutSemExp,
                               RelativePerson pSubjectRelativePerson,
                               const SemanticStatementGrounding& pStatementGrd,
                               const linguistics::InflectedWord& pInflectedVerb,
                               const GroundedExpression& pHoldingGrdExp,
                               const SemanticExpression& pSemExp,
                               const SynthesizerConfiguration& pConf,
                               const SynthesizerCurrentContext& pContext,
                               const SemanticExpression* pRootSubject,
                               const mystd::optional<SynthesizerVerbContext>& pVerbContextOpt) const;

  void _writeObjectAfterVerb(SyntSentWorkStruct& pSentWorkStruct,
                             OutSemExp& pOutSemExp,
                             GrammaticalType pGramType,
                             const SemanticStatementGrounding& pStatementGrd,
                             const linguistics::InflectedWord& pInflectedVerb,
                             const GroundedExpression& pHoldingGrdExp,
                             const SemanticExpression& pSemExp,
                             SynthesizerConfiguration& pConf,
                             const SemanticRequests& pRequests,
                             SynthesizerCurrentContextType pChildContextType,
                             const SynthesizerCurrentContext& pVerbContext,
                             SemanticExpression const** pLastSubject) const;

  void _writeLocation(OutSemExp& pOutSemExp,
                      const SemanticExpression& pSemExp,
                      SynthesizerConfiguration& pConf,
                      const SemanticRequests& pRequests,
                      const SynthesizerCurrentContext& pContext,
                      const SyntSentWorkStruct* pSentWorkStructPtr,
                      const SynthesizerCurrentContext* pVerbContextPtr,
                      SemanticExpression const** pLastSubject) const;

  void _writeEquality
  (SyntSentWorkStruct& pSentWorkStruct,
   const SemanticExpression& pSemExp,
   SynthesizerConfiguration& pConf,
   const SemanticRequests& pRequests,
   const SynthesizerCurrentContext& pContext,
   SynthesizerCurrentContextType pParentContextType,
   SemanticExpression const** pLastSubject) const;

  RelativePerson _semExpToRelativePerson
  (const SemanticExpression& pSemExp,
   const SynthesizerConfiguration& pConf,
   bool pIsANameAssignement) const;

  void _writeSubjectContext
  (OutSentence& pOutSentence,
   SemanticExpression const** pLastSubject,
   const UniqueSemanticExpression& pSemExp,
   SynthesizerCurrentContext& pSubjectContext,
   SynthesizerConfiguration& pConf,
   const SemanticRequests& pRequests) const;

  void _writeNounSubordinates
  (OutSemExp& pOutSemExp,
   const SemanticExpression& pSemExp,
   GrammaticalType pSemExpFatherGramType,
   SynthesizerConfiguration& pConf,
   const SemanticRequests& pRequests,
   const GroundedExpression* pGrdExpPtr,
   SemanticExpression const** pLastSubject) const;

  void _writeSomeModifiersOfAWord
  (OutSemExp& pOutBeforeDeterminer,
   OutSemExp& pOutBeforeMainWord,
   OutSemExp& pOutAfterMainWord,
   OutSemExp& pOutSubordinate,
   const SemanticExpression& pModifierSemExp,
   SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext,
   const SemanticRequests& pRequests,
   bool pADetermantHasBeenWrote,
   const GroundedExpression& pMotherGrdExp,
   const linguistics::InflectedWord* pInflWordPtr,
   SemanticExpression const** pLastSubject) const;

  RelativePerson _grdExpToRelativePerson(const GroundedExpression& pGrdExp,
                                         const SynthesizerConfiguration& pConf,
                                         bool pIsANameAssignement) const;

  void _fillOutBlocsList
  (std::list<WordToSynthesize>& pOut,
   std::list<OutSemExp>& pSubOutElts,
   ListExpressionType pListType) const;
};



} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZERPRIVATE_HPP
