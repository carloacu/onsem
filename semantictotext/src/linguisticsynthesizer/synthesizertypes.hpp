#ifndef ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERTYPES_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERTYPES_HPP

#include <string>
#include <list>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/common/enum/linguisticverbtense.hpp>
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
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
  SemanticStatementGrounding const* rootStatement = nullptr;
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
  const TextProcessingContext textProcessingContext;

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
  OutSemExp receiverJustAfterVerb{};
  OutSemExp receiverAfterVerb{};
  OutSemExp objectBeforeSubject{};
  OutSemExp objectBeforeVerb{};
  OutSemExp objectJustAfterVerb{};
  OutSemExp objectAfterVerb{};
  OutSemExp inCaseOf{};
  OutSemExp mannerBetweenAuxAndVerb{};
  OutSemExp manner{};
  OutSemExp other{};
  OutSemExp time{};
  OutSemExp length{};
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

struct GroundingAnglePrettyPrintStruct
{
  GroundingAnglePrettyPrintStruct(const SemanticAngle& pAngle)
    : degree(),
      radian()
  {
    for (const auto& currAngleInfo : pAngle.angleInfos)
    {
      switch (currAngleInfo.first)
      {
      case SemanticAngleUnity::DEGREE:
        degree.emplace(currAngleInfo.second);
        break;
      case SemanticAngleUnity::RADIAN:
        radian.emplace(currAngleInfo.second);
        break;
      default:
        break;
      }
    }
  }

  mystd::optional<SemanticFloat> degree;
  mystd::optional<SemanticFloat> radian;
};

struct GroundingLengthPrettyPrintStruct
{
  GroundingLengthPrettyPrintStruct(const SemanticLength& pLength)
    : kilometer(),
      hectometer(),
      decameter(),
      meter(),
      decimeter(),
      centimeter(),
      millimeter()
  {
    for (const auto& currLengthInfo : pLength.lengthInfos)
    {
      switch (currLengthInfo.first)
      {
      case SemanticLengthUnity::KILOMETER:
        kilometer.emplace(currLengthInfo.second);
        break;
      case SemanticLengthUnity::HECTOMETER:
        hectometer.emplace(currLengthInfo.second);
        break;
      case SemanticLengthUnity::DECAMETER:
        decameter.emplace(currLengthInfo.second);
        break;
      case SemanticLengthUnity::METER:
        meter.emplace(currLengthInfo.second);
        break;
      case SemanticLengthUnity::DECIMETER:
        decimeter.emplace(currLengthInfo.second);
        break;
      case SemanticLengthUnity::CENTIMETER:
        centimeter.emplace(currLengthInfo.second);
        break;
      case SemanticLengthUnity::MILLIMETER:
        millimeter.emplace(currLengthInfo.second);
        break;
      default:
        break;
      }
    }
  }

  mystd::optional<SemanticFloat> kilometer;
  mystd::optional<SemanticFloat> hectometer;
  mystd::optional<SemanticFloat> decameter;
  mystd::optional<SemanticFloat> meter;
  mystd::optional<SemanticFloat> decimeter;
  mystd::optional<SemanticFloat> centimeter;
  mystd::optional<SemanticFloat> millimeter;
};

struct GroundingDurationPrettyPrintStruct
{
  GroundingDurationPrettyPrintStruct(const SemanticDuration& pDuration)
    : hour(),
      minute(),
      second(),
      millisecond()
  {
    for (const auto& currTimeInfo : pDuration.timeInfos)
    {
      switch (currTimeInfo.first)
      {
      case SemanticTimeUnity::HOUR:
        hour.emplace(currTimeInfo.second);
        break;
      case SemanticTimeUnity::MINUTE:
        minute.emplace(currTimeInfo.second);
        break;
      case SemanticTimeUnity::SECOND:
        second.emplace(currTimeInfo.second);
        break;
      case SemanticTimeUnity::MILLISECOND:
        millisecond.emplace(currTimeInfo.second);
        break;
      default:
        break;
      }
    }
  }

  mystd::optional<SemanticFloat> hour;
  mystd::optional<SemanticFloat> minute;
  mystd::optional<SemanticFloat> second;
  mystd::optional<SemanticFloat> millisecond;
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERTYPES_HPP
