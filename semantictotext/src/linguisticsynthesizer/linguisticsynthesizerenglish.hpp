#ifndef ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZERENGLISH_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZERENGLISH_HPP

#include <memory>
#include "linguisticsynthesizerprivate.hpp"
#include "grounding/linguisticsynthesizergroundingenglish.hpp"
#include "merger/synthesizerchunksmergerenglish.hpp"
#include "conversion/tokenstostringconverter.hpp"


namespace onsem
{


class LinguisticSynthesizerEnglish : public LinguisticSynthesizerPrivate
{
public:
  LinguisticSynthesizerEnglish();

  virtual const Linguisticsynthesizergrounding& getSyntGrounding() const
  { return _syntGrounding; }

  virtual const TokensToStringConverter& getTokenToStrConverter() const
  { return _tokenToStrConverter; }

protected:
  virtual const SynthesizerChunksMerger& _getChunksMerger() const
  { return _syntGrounding.getChunksMerger(); }

  virtual LinguisticVerbTense _semanticVerbTenseToLinguisticVerbTense(SemanticVerbTense pSemVerbTense,
                                                                      const linguistics::WordAssociatedInfos* pWordInfosPtr) const;

  virtual bool _subConceptBeforeOrAfter(const linguistics::WordAssociatedInfos& pInfo) const
  { return !pInfo.hasContextualInfo(WordContextualInfos::CANBEBEFORENOUN); }

  virtual void _getNegationsBeforeVerb(std::list<WordToSynthesize>&) const {}
  virtual void _getNegationsAfterVerb(std::list<WordToSynthesize>&) const {}

  virtual ObjectPosition _getObjectPosition(SyntSentWorkStruct& pSentWorkStruct,
                                            const SemanticStatementGrounding&,
                                            const SemanticRequests& pRequests,
                                            const SynthesizerConfiguration&,
                                            LinguisticVerbTense) const;

  virtual bool _putReceiverBeforeVerb(const SemanticExpression&,
                                      bool,
                                      const SemanticRequests& pRequests,
                                      const SynthesizerConfiguration&) const { return false; }

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
   bool pNeedToWriteTheVerb) const;

  virtual void _getPossessiveDeterminer
  (std::list<WordToSynthesize>& pOut,
   RelativePerson pRelPerson,
   SemanticGenderType pGender,
   SemanticNumberType pNumber) const;

  virtual void _getOfWord(std::list<WordToSynthesize>& pOut,
                          const SynthesizerWordContext& pWordContext) const;
  virtual void _getRelTimeFollowingPrep(std::list<WordToSynthesize>& pOut,
                                        const SemanticRelativeTimeGrounding& pRelTimeGrd) const {}

  virtual void _getBeginOfSpecification
  (std::list<WordToSynthesize>& pOut,
   const GroundedExpression& pChildGrdExp,
   const SemanticGrounding& pParentGrounding,
   const SynthesizerWordContext& pWordContext) const;

  virtual void _getBeginOfWithChild
  (std::list<WordToSynthesize>& pOut) const;

  virtual void _getBeginOfWithoutChild
  (std::list<WordToSynthesize>& pOut) const;

  virtual void _getBeginOfForChild
  (std::list<WordToSynthesize>& pOut,
   const SemanticExpression& pSemExp) const;

  virtual void _getThanWord
  (std::list<WordToSynthesize>& pOut,
   const mystd::unique_propagate_const<UniqueSemanticExpression>& pWhatIsComparedExp) const;

  virtual void _getBeginOfMitigationChild
  (std::list<WordToSynthesize>& pOut) const;

  virtual void _getBeginOfYesOrNoSubordonate
  (std::list<WordToSynthesize>& pOut) const;

  virtual bool _beginOfSubordonateIfNeeded
  (bool&,
   OutSemExp&,
   const SyntSentWorkStruct&,
   GrammaticalType,
   const GroundedExpression&,
   const mystd::optional<SynthesizerVerbContext>&,
   const SynthesizerConfiguration&) const { return false; }

  virtual void _getWheneverCondition
  (std::list<WordToSynthesize>& pOut) const;

  virtual void _writeSubjectOfGeneralitySentence(OutSentence& pOutSentence) const;
  virtual void _writeGenericSubject(OutSentence& pOutSentence) const;
  virtual void _getBeginOfBetweenSubordonate(std::list<WordToSynthesize>& pOut) const;
  virtual void _getBeginOfCauseSubordonate(std::list<WordToSynthesize>& pOut) const;

  virtual void _getThenWord
  (std::list<WordToSynthesize>& pOut) const;

  virtual void _getElseWord
  (std::list<WordToSynthesize>& pOut) const;

  virtual void _getPossessiveWord
  (std::list<WordToSynthesize>& pOut,
   const SynthesizerWordContext& pCoreferenceWordContext,
   RelativePerson pOwnerRelativePerson,
   const SemanticGrounding& pGrounding,
   const SynthesizerConfiguration& pConf,
   const SynthesizerCurrentContext& pContext) const;

  virtual SpecifierPosition _getSpecifierPosition
  (const linguistics::InflectedWord& pInflWord,
   const SemanticGrounding& pGrounding,
   bool,
   const linguistics::InflectedWord* pMotherInflWordPtr) const;

  virtual bool _doWeHaveToWriteBeginOfSpecifier
  (SynthesizerCurrentContext& pGrdContext,
   const linguistics::InflectedWord& pInflWord,
   const GroundedExpression& pGrdExpOfTheWord,
   const SemanticGrounding& pMotherGrounding) const;

  virtual void _getComparisonWord(std::list<WordToSynthesize>& pOut,
                                  ComparisonOperator pCompPolarity,
                                  SemanticGenderType pGender,
                                  SemanticNumberType pNumber) const;

private:
  LinguisticsynthesizergroundingEnglish _syntGrounding;
  TokensToStringConverterEnglish _tokenToStrConverter;
};


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZERENGLISH_HPP
