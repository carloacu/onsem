#ifndef ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERTYPES_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERTYPES_HPP

#include <string>
#include <list>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/common/enum/linguisticverbtense.hpp>
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/common/enum/relativeperson.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/wordtosynthesize.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemoryBlock;
struct SemanticExpression;
struct TextProcessingContext;
struct OutSentence;


enum SemExpTypeEnumFormSynt
{
  SEMEXPTYPEENUMFORSYNT_SENTENCE,
  SEMEXPTYPEENUMFORSYNT_WORD,
  SEMEXPTYPEENUMFORSYNT_NOTHING
};



enum SynthesizerCurrentContextType
{
  SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTBEFOREVERB,
  SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT,
  SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB,
  SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB,
  SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTAFTERVERB,
  SYNTHESIZERCURRENTCONTEXTTYPE_SUBVERB,
  SYNTHESIZERCURRENTCONTEXTTYPE_TIME,
  SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC,
  SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN,
  SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERADJECTIVE,
  SYNTHESIZERCURRENTCONTEXTTYPE_CONDITION
};


struct SynthesizerWordContext
{
  RelativePerson relativePerson = RelativePerson::UNKNOWN;
  SemanticGenderType gender = SemanticGenderType::UNKNOWN;
  SemanticNumberType number = SemanticNumberType::UNKNOWN;
  SemanticReferenceType referenceType = SemanticReferenceType::UNDEFINED;
};


struct SynthesizerVerbContext
{
  SynthesizerVerbContext(const linguistics::InflectedWord& pVerbInflWord,
                         const SemanticStatementGrounding& pStatGrd)
      : verbInflWord(pVerbInflWord),
        statGrd(pStatGrd)
  {
  }
  SynthesizerVerbContext(const SynthesizerVerbContext& pOther)
      : verbInflWord(pOther.verbInflWord),
        statGrd(pOther.statGrd)
  {
  }
  const linguistics::InflectedWord& verbInflWord;
  const SemanticStatementGrounding& statGrd;
};


struct SynthesizerCurrentContext
{
  SynthesizerCurrentContext();
  SynthesizerCurrentContext(const SynthesizerCurrentContext& pOther);

  GrammaticalType grammaticalTypeFromParent = GrammaticalType::UNKNOWN;
  SynthesizerCurrentContextType contextType = SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC;
  LinguisticVerbTense verbTense = LinguisticVerbTense::PRESENT_INDICATIVE;
  VerbGoalEnum verbGoal = VerbGoalEnum::NOTIFICATION;
  SemanticRequests requests;
  SynthesizerWordContext wordContext{};
  ComparisonOperator compPolarity = ComparisonOperator::EQUAL;
  mystd::optional<SynthesizerVerbContext> verbContextOpt{};
  OutSentence* currSentence = nullptr;
  SemanticExpression const* parentSubject = nullptr;
  SemanticExpression const* rootSubject = nullptr;
  bool isParentARelativeGrounding = false;
  bool isAtRoot = false;
  bool isPositive = true;
  bool isPassive = false;
  bool hasASubject = false;
  bool ownerWrittenBefore = false;
  bool isPartOfANameAssignement = false;
  bool isASubordinateWithoutPreposition = false;
};


struct SynthesizerConfiguration
{
  SynthesizerConfiguration(const SemanticMemoryBlock& pMemBlock,
                           bool pOneLinePerSentence,
                           const std::string& pCurrentUserId,
                           const TextProcessingContext& pTextProcContext,
                           const linguistics::LinguisticDatabase& pLingDb);

  bool canDoAnotherRecurssiveCall();

  const SemanticMemoryBlock& memBlock;
  const bool oneLinePerSentence;
  const linguistics::LinguisticDatabase& lingDb;
  const SemanticExpression* semExp;
  const std::string authorId;
  const std::string receiverId;
  const bool vouvoiement;

private:
  int _nbOfRemainingPossibleRecursiveCalls;

  SynthesizerConfiguration(const SynthesizerConfiguration& pOther);
  SynthesizerConfiguration& operator=(const SynthesizerConfiguration& pOther);

  static std::string _mergeUserIdWithContext(const std::string& pUserId,
                                             const std::string& pCurrentUserId);
};


struct OutSemExp
{
  OutSemExp() = default;
  OutSemExp(const OutSemExp&) = delete;
  OutSemExp& operator=(const OutSemExp&) = delete;

  PartOfSpeech getFirstPartOfSpeech() const;
  PartOfSpeech getLastPartOfSpeech() const;
  bool containsOnly(PartOfSpeech pPartOfSpeech) const;
  bool empty() const { return out.empty(); }
  std::size_t getPriority();
  void moveContent(OutSemExp& pOut);

  PartOfSpeech partOfSpeech{PartOfSpeech::UNKNOWN};
  SemanticGenderType gender{SemanticGenderType::UNKNOWN};
  RelativePerson relativePerson{RelativePerson::UNKNOWN};
  std::list<WordToSynthesize> out{};
};


struct OutSentence
{
  OutSemExp subject{};
  OutSemExp negation1{};
  OutSemExp negation2{};
  OutSemExp verbGoal{};
  OutSemExp aux{};
  OutSemExp verb{};
  OutSemExp verb2{};
  OutSemExp equalityBeforeVerb{};
  OutSemExp equalityAfterVerb{};
  OutSemExp receiverBeforeVerb{};
  OutSemExp receiverAfterVerb{};
  OutSemExp objectBeforeSubject{};
  OutSemExp objectBeforeVerb{};
  OutSemExp objectAfterVerb{};
  OutSemExp inCaseOf{};
  OutSemExp mannerBetweenAuxAndVerb{};
  OutSemExp manner{};
  OutSemExp other{};
  OutSemExp time{};
  OutSemExp duration{};
  OutSemExp location{};
  OutSemExp occurrenceRank{};
  OutSemExp startingPoint{};
  SemanticRequests requests;
  SynthesizerCurrentContextType contextType = SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC;
  SemanticVerbTense verbTense = SemanticVerbTense::PRESENT;
  bool equVerb = false;
  bool isPassive = false;
};


struct OutNominalGroup
{
  OutSemExp determiner{};
  OutSemExp noun{};
  OutSemExp subConceptsBeforeNoun{};
  OutSemExp subConceptsAfterNoun{};
  OutSemExp modifiersBeforeDeterminer{};
  OutSemExp modifiersBeforeNoun{};
  OutSemExp modifiersAfterNoun{};
  OutSemExp ownerBeforeMainWord{};
  OutSemExp ownerAfterMainWord{};
  OutSemExp time{};
  OutSemExp location{};
  OutSemExp purpose{};
  OutSemExp subordinate{};
};


struct SyntSentWorkStruct
{
  SyntSentWorkStruct(const GroundedExpression& pGrdExp,
                     const SemanticStatementGrounding& pStatementGrd,
                     SemanticExpression const** pLastSubject)
    : outs(),
      grdExp(pGrdExp),
      statementGrd(pStatementGrd),
      subjectPtr(_itToPtr(pGrdExp.children.find(GrammaticalType::SUBJECT))),
      objectPtr(_itToPtr(pGrdExp.children.find(GrammaticalType::OBJECT))),
      objectIsAnNoElement(false),
      lastSubject(pLastSubject)
  {
  }

  SyntSentWorkStruct(const SyntSentWorkStruct&) = delete;
  SyntSentWorkStruct& operator=(const SyntSentWorkStruct&) = delete;

  bool beginOfAVerbalForm() const
  {
    return statementGrd.word.isEmpty() &&
        statementGrd.concepts.empty() &&
        statementGrd.verbGoal != VerbGoalEnum::NOTIFICATION;
  }

  OutSentence outs;
  const GroundedExpression& grdExp;
  const SemanticStatementGrounding& statementGrd;
  const UniqueSemanticExpression* subjectPtr;
  const UniqueSemanticExpression* objectPtr;
  bool objectIsAnNoElement;
  SemanticExpression const** lastSubject;

private:
  const UniqueSemanticExpression* _itToPtr(std::map<GrammaticalType, UniqueSemanticExpression>::const_iterator pIt)
  {
    if (pIt != grdExp.children.end())
    {
      return &pIt->second;
    }
    return nullptr;
  }
};


struct GroundingDurationPrettyPrintStruct
{
  GroundingDurationPrettyPrintStruct(const SemanticDuration& pDuration)
    : hour(-1),
      minute(-1),
      second(-1),
      millisecond(-1)
  {
    for (const auto& currTimeInfo : pDuration.timeInfos)
    {
      switch (currTimeInfo.first)
      {
      case SemanticTimeUnity::HOUR:
        hour = currTimeInfo.second;
        break;
      case SemanticTimeUnity::MINUTE:
        minute = currTimeInfo.second;
        break;
      case SemanticTimeUnity::SECOND:
        second = currTimeInfo.second;
        break;
      case SemanticTimeUnity::MILLISECOND:
        millisecond = currTimeInfo.second;
        break;
      default:
        break;
      }
    }
  }

  int hour;
  int minute;
  int second;
  int millisecond;
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERTYPES_HPP
