#include "linguisticsynthesizerfrench.hpp"
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/type/enumsconvertions.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "tool/synthesizeradder.hpp"
#include "tool/synthesizergetter.hpp"
#include "grounding/linguisticsynthesizergrounding.hpp"


namespace onsem
{

namespace
{
const std::vector<std::string> _rootVerbsForSubjonctiveInSubordinate{"verb_like", "verb_want", "verb_doubt", "verb_require"};

bool _doWeNeedToPutObjectGenGrdBeforeVerb(const SemanticGenericGrounding& pGenGrd,
                                          const SemanticRequests& pRequests)
{
  return pGenGrd.coreference && pGenGrd.word.lemma.empty() &&
      pGenGrd.quantity.type != SemanticQuantityType::MAXNUMBER &&
      pGenGrd.quantity.type != SemanticQuantityType::EVERYTHING &&
      !pRequests.has(SemanticRequestType::ACTION);
}

}



LinguisticSynthesizerFrench::LinguisticSynthesizerFrench()
  : LinguisticSynthesizerPrivate(SemanticLanguageEnum::FRENCH),
    _syntGrounding(*this),
    _tokenToStrConverter()
{
}


LinguisticVerbTense LinguisticSynthesizerFrench::_semanticVerbTenseToLinguisticVerbTense(SemanticVerbTense pSemVerbTense,
                                                                                         SynthesizerCurrentContextType pContextType,
                                                                                         SemanticStatementGrounding const* pRootStatementPtr,
                                                                                         const SemanticRequests& pRequests,
                                                                                         const linguistics::WordAssociatedInfos*) const
{
  switch (pSemVerbTense)
  {
  case SemanticVerbTense::FUTURE:
    return LinguisticVerbTense::FUTURE_INDICATIVE;
  case SemanticVerbTense::PAST:
    return LinguisticVerbTense::IMPERFECT_INDICATIVE;
  case SemanticVerbTense::PRESENT:
  {
    return LinguisticVerbTense::PRESENT_INDICATIVE;
  }
  case SemanticVerbTense::PUNCTUALPRESENT:
  {
    if (pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB ||
        pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB)
    {
      if (pRequests.empty() &&
          pRootStatementPtr != nullptr &&
          ConceptSet::haveAnyOfConcepts(pRootStatementPtr->concepts, _rootVerbsForSubjonctiveInSubordinate))
        return LinguisticVerbTense::PRESENT_SUBJONCTIVE;
    }
    return LinguisticVerbTense::PRESENT_CONTINUOUS;
  }
  case SemanticVerbTense::PUNCTUALPAST:
    return LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
  case SemanticVerbTense::UNKNOWN:
    return LinguisticVerbTense::INFINITIVE;
  }
  return LinguisticVerbTense::INFINITIVE;
}


void LinguisticSynthesizerFrench::_getPossessiveDeterminer
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelPerson,
 SemanticGenderType pGender,
 SemanticNumberType pNumber) const
{
  static const PartOfSpeech det = PartOfSpeech::DETERMINER;
  switch (pRelPerson)
  {
  case RelativePerson::FIRST_SING:
  {
    if (pNumber == SemanticNumberType::PLURAL)
    {
      _strToOut(pOut, det, "mes");
    }
    else if (pGender == SemanticGenderType::FEMININE)
    {
      pOut.emplace_back(SemanticWord(_language, "mon", det),
                        InflectionToSynthesize("mon", true, true, ifNeedAnApostropheBefore));
      pOut.back().inflections.emplace_back("ma", true, true, alwaysTrue);
    }
    else
    {
      _strToOut(pOut, det, "mon");
    }
    return;
  }
  case RelativePerson::SECOND_SING:
  {
    if (pNumber == SemanticNumberType::PLURAL)
    {
      _strToOut(pOut, det, "tes");
    }
    else if (pGender == SemanticGenderType::FEMININE)
    {
      pOut.emplace_back(SemanticWord(_language, "ton", det),
                        InflectionToSynthesize("ton", true, true, ifNeedAnApostropheBefore));
      pOut.back().inflections.emplace_back("ta", true, true, alwaysTrue);
    }
    else
    {
      _strToOut(pOut, det, "ton");
    }
    return;
  }
  case RelativePerson::THIRD_SING:
  case RelativePerson::UNKNOWN:
  {
    if (pNumber == SemanticNumberType::PLURAL)
    {
      _strToOut(pOut, det, "ses");
    }
    else if (pGender == SemanticGenderType::FEMININE)
    {
      pOut.emplace_back(SemanticWord(_language, "son", det),
                        InflectionToSynthesize("son", true, true, ifNeedAnApostropheBefore));
      pOut.back().inflections.emplace_back("sa", true, true, alwaysTrue);
    }
    else
    {
      _strToOut(pOut, det, "son");
    }
    return;
  }
  case RelativePerson::FIRST_PLUR:
    _strToOut(pOut, det, pNumber == SemanticNumberType::PLURAL ?
                "nos" : "notre");
    return;
  case RelativePerson::SECOND_PLUR:
    _strToOut(pOut, det, pNumber == SemanticNumberType::PLURAL ?
                "vos" : "votre");
    return;
  case RelativePerson::THIRD_PLUR:
    _strToOut(pOut, det, pNumber == SemanticNumberType::PLURAL ?
                "leurs" : "leur");
    return;
  }
  assert(false);
}


void LinguisticSynthesizerFrench::_getOfWord(std::list<WordToSynthesize>& pOut,
                                             const SynthesizerWordContext& pWordContext) const
{
  if (pWordContext.number != SemanticNumberType::PLURAL ||
      pWordContext.referenceType != SemanticReferenceType::INDEFINITE)
  {
    pOut.emplace_back([&]
    {
      WordToSynthesize wordToToSynth(SemanticWord(_language, "de", PartOfSpeech::PREPOSITION),
                                     InflectionToSynthesize("d'", true, false,
                                                            [](const WordToSynthesize& pNext)
      {
        return ifNeedAnApostropheBefore(pNext) && ifNextWordIsNotAPrepostion(pNext);
      }));
      wordToToSynth.inflections.emplace_back("de", true, true, ifNextWordIsNotAPrepostion);
      return wordToToSynth;
    }());
  }
}


void LinguisticSynthesizerFrench::_getRelTimeFollowingPrep(std::list<WordToSynthesize>& pOut,
                                                           const SemanticRelativeTimeGrounding& pRelTimeGrd) const
{
  switch (pRelTimeGrd.timeType)
  {
  case SemanticRelativeTimeType::SINCE:
  {
    _strWithApostropheToOut(pOut, PartOfSpeech::PREPOSITION, "qu'", "que");
    break;
  }
  case SemanticRelativeTimeType::DELAYEDSTART:
  {
    _strToOut(pOut, PartOfSpeech::PREPOSITION, "dans");
   break;
  }
  case SemanticRelativeTimeType::AFTER:
  case SemanticRelativeTimeType::BEFORE:
  case SemanticRelativeTimeType::JUSTAFTER:
  case SemanticRelativeTimeType::JUSTBEFORE:
  {
    _strToOut(pOut, PartOfSpeech::PREPOSITION, "de");
   break;
  }
  }
}


void LinguisticSynthesizerFrench::_getBeginOfSpecification
(std::list<WordToSynthesize>& pOut,
 const GroundedExpression& pChildGrdExp,
 const SemanticGrounding& pParentGrounding,
 const SynthesizerWordContext& pWordContext) const
{
  if (pChildGrdExp.children.count(GrammaticalType::INTRODUCTING_WORD) > 0)
    return;
  static const std::vector<std::string> cptsToFollowByA{"rank_", "manner_"};
  const SemanticStatementGrounding* statGrdPtr = pChildGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
  {
    if (ConceptSet::haveAConceptThatBeginWithAnyOf(pParentGrounding.concepts, cptsToFollowByA))
    {
     _strToOut(pOut, PartOfSpeech::PREPOSITION, "à");
     return;
    }

    auto itSubject = pChildGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject != pChildGrdExp.children.end() &&
        SemExpGetter::isACoreference(*itSubject->second, CoreferenceDirectionEnum::PARENT))
    {
      _strToOut(pOut, PartOfSpeech::SUBORDINATING_CONJONCTION, "qui");
      return;
    }
    if (statGrdPtr->verbTense != SemanticVerbTense::UNKNOWN)
    {
      _strToOut(pOut, PartOfSpeech::SUBORDINATING_CONJONCTION, "dont");
      return;
    }
  }
  _getOfWord(pOut, pWordContext);
}


void LinguisticSynthesizerFrench::_getNegationsBeforeVerb
(std::list<WordToSynthesize>& pOut) const
{
  _strWithApostropheToOut(pOut, PartOfSpeech::ADVERB, "n'", "ne");
}

void LinguisticSynthesizerFrench::_getNegationsAfterVerb
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::ADVERB, "pas");
}


LinguisticSynthesizerPrivate::ObjectPosition LinguisticSynthesizerFrench::_getObjectPosition
(SyntSentWorkStruct& pSentWorkStruct,
 const SemanticStatementGrounding& pStatementGrd,
 const SemanticRequests& pRequests,
 const SynthesizerConfiguration& pConf,
 LinguisticVerbTense pVerbTense) const
{
  if (pSentWorkStruct.objectPtr == nullptr)
    return ObjectPosition::AFTERVERB;
  const GroundedExpression* objectGrdExpPtr = pSentWorkStruct.objectPtr->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
  if (objectGrdExpPtr != nullptr)
  {
    const GroundedExpression& objectGrdExp = *objectGrdExpPtr;
    auto getObjPositionOfGenericGrd = [&](const SemanticGenericGrounding& pGenGrd) {
      if (pGenGrd.coreference && objectGrdExp.children.count(GrammaticalType::OWNER) > 0)
        return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
      if (pRequests.has(SemanticRequestType::OBJECT) ||
          pRequests.has(SemanticRequestType::QUANTITY))
      {
        if (pGenGrd.coreference)
          return LinguisticSynthesizerPrivate::ObjectPosition::BEFOREVERB;
        if (pGenGrd.referenceType != SemanticReferenceType::DEFINITE)
          return LinguisticSynthesizerPrivate::ObjectPosition::BEFORESUBJECT;
      }
      if (_doWeNeedToPutObjectGenGrdBeforeVerb(pGenGrd, pRequests))
        return LinguisticSynthesizerPrivate::ObjectPosition::BEFOREVERB;
      if (pSentWorkStruct.objectIsAnNoElement && pVerbTense == LinguisticVerbTense::INFINITIVE)
        return LinguisticSynthesizerPrivate::ObjectPosition::BEFOREVERB;
      if (pGenGrd.coreference && pGenGrd.word.isEmpty())
        return LinguisticSynthesizerPrivate::ObjectPosition::JUSTAFTERVERB;
      return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
    };
    if (ConceptSet::haveAConcept(objectGrdExp->concepts, "tolink_1p"))
      return LinguisticSynthesizerPrivate::ObjectPosition::BEFOREVERB;
    const SemanticGrounding& objectGrd = objectGrdExp.grounding();
    switch (objectGrd.type)
    {
    case SemanticGroundingType::STATEMENT:
    {
      const SemanticStatementGrounding& objectStatGrd = objectGrdExp->getStatementGrounding();
      if (objectStatGrd.isAtInfinitive() &&
          objectGrdExp.children.size() == 1)
      {
        auto& objObjUSemExp = objectGrdExp.children.begin()->second;
        auto* objObjGrdExpPtr = objObjUSemExp->getGrdExpPtr_SkipWrapperPtrs();
        if (objObjGrdExpPtr != nullptr)
        {
          auto* objObjGenExpPtr = objObjGrdExpPtr->grounding().getGenericGroundingPtr();
          if (objObjGenExpPtr != nullptr)
          {
            auto objObjGenExpObjPos = getObjPositionOfGenericGrd(*objObjGenExpPtr);
            if (objObjGenExpObjPos == ObjectPosition::BEFORESUBJECT)
            {
              SynthesizerCurrentContext context;
              context.verbTense = LinguisticVerbTense::INFINITIVE;
              _syntGrounding.writeSemWord(pSentWorkStruct.outs.verb2.out, pSentWorkStruct.outs.verb2,
                                          pSentWorkStruct.outs.verb2.out, objectStatGrd.word, pConf.lingDb, context);
              pSentWorkStruct.objectPtr = &objObjUSemExp;
              return objObjGenExpObjPos;
            }
          }
        }
      }
      if (objectStatGrd.coreference && objectStatGrd.requests.empty())
        return LinguisticSynthesizerPrivate::ObjectPosition::BEFOREVERB;
      return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
    }
    case SemanticGroundingType::GENERIC:
    {
      const SemanticGenericGrounding& genGrd = objectGrd.getGenericGrounding();
      return getObjPositionOfGenericGrd(genGrd);
    }
    case SemanticGroundingType::AGENT:
    {
      const SemanticAgentGrounding& agentGrd = objectGrd.getAgentGrounding();
      if (_syntGrounding.agentTypeToRelativePerson(agentGrd, pConf, true) != RelativePerson::THIRD_SING &&
          !pRequests.has(SemanticRequestType::ACTION) &&
          pStatementGrd.concepts.count("verb_equal_be") == 0)
        return LinguisticSynthesizerPrivate::ObjectPosition::BEFOREVERB;
      return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
    }
    default:
      return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
    }
  }
  return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
}


LinguisticSynthesizerPrivate::ReceiverPosition LinguisticSynthesizerFrench::_getReceiverPosition
(const SemanticExpression& pSemExpObj,
 bool pVerbIsAffirmative,
 const SemanticRequests& pRequests,
 const SynthesizerConfiguration& pConf) const
{
  const GroundedExpression* objectGrdExpPtr = pSemExpObj.getGrdExpPtr_SkipWrapperPtrs();
  if (objectGrdExpPtr != nullptr)
  {
    const GroundedExpression& objectGrdExp = *objectGrdExpPtr;
    const SemanticGrounding& objectGrd = objectGrdExp.grounding();
    if (!objectGrdExp.children.empty())
      return LinguisticSynthesizerPrivate::ReceiverPosition::AFTERVERB;
    switch (objectGrd.type)
    {
    case SemanticGroundingType::GENERIC:
    {
      const SemanticGenericGrounding& genGrd = objectGrd.getGenericGrounding();
      if (_doWeNeedToPutObjectGenGrdBeforeVerb(genGrd, pRequests))
        return LinguisticSynthesizerPrivate::ReceiverPosition::BEFOREVERB;
      return LinguisticSynthesizerPrivate::ReceiverPosition::AFTERVERB;
    }
    case SemanticGroundingType::AGENT:
    {
      const SemanticAgentGrounding& agentGrd = objectGrd.getAgentGrounding();
      if (pRequests.has(SemanticRequestType::ACTION) && !pVerbIsAffirmative)
        return LinguisticSynthesizerPrivate::ReceiverPosition::BEFOREVERB;
      if (_syntGrounding.agentTypeToRelativePerson(agentGrd, pConf, true) != RelativePerson::THIRD_SING &&
          !pRequests.has(SemanticRequestType::ACTION))
        return LinguisticSynthesizerPrivate::ReceiverPosition::BEFOREVERB;
      return LinguisticSynthesizerPrivate::ReceiverPosition::AFTERVERB;
    }
    case SemanticGroundingType::NAME:
      return LinguisticSynthesizerPrivate::ReceiverPosition::AFTERVERB;
    default:
      if (pRequests.has(SemanticRequestType::ACTION))
        return LinguisticSynthesizerPrivate::ReceiverPosition::AFTERVERB;
      return LinguisticSynthesizerPrivate::ReceiverPosition::BEFOREVERB;
    }
  }
  return LinguisticSynthesizerPrivate::ReceiverPosition::AFTERVERB;
}


void LinguisticSynthesizerFrench::_writeQuel(
    std::list<WordToSynthesize>& pOut,
    const SemanticExpression& pSemExp,
    const SynthesizerConfiguration& pConf,
    const SynthesizerCurrentContext& pContext) const
{
  const auto& grdSynth = getSyntGrounding();
  SemanticGenderType gender = synthGetter::getGenderFromSemExp(pSemExp, pConf, pContext, grdSynth);
  if (SemExpGetter::getNumber(pSemExp) == SemanticNumberType::PLURAL)
    _strToOut(pOut, PartOfSpeech::PRONOUN,
              gender == SemanticGenderType::FEMININE ? "quelles" : "quels");
  else
    _strToOut(pOut, PartOfSpeech::PRONOUN,
              gender == SemanticGenderType::FEMININE ? "quelle" : "quel");
}


void LinguisticSynthesizerFrench::_writeCaWord(OutSemExp& pSubjectOut) const
{
  pSubjectOut.relativePerson = RelativePerson::THIRD_SING;
  pSubjectOut.partOfSpeech = PartOfSpeech::PRONOUN;
  _strWithApostropheToOut(pSubjectOut.out, pSubjectOut.partOfSpeech, "c'", "ça");
}


void LinguisticSynthesizerFrench::_getQuestionWord
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
 bool pNeedToWriteTheVerb) const
{
  bool firstIteration = true;
  for (const auto& currRequest : pRequests.types)
  {
    if (firstIteration)
      firstIteration = false;
    else
      _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "et");
    switch (currRequest)
    {
    case SemanticRequestType::LOCATION:
    {
      if (pChildToPutBeforeSubject != nullptr)
      {
        const auto* childGrdExpPtr = pChildToPutBeforeSubject->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
        if (childGrdExpPtr != nullptr)
        {
          const auto* childRelLocationPtr = childGrdExpPtr->grounding().getRelLocationGroundingPtr();
          if (childRelLocationPtr != nullptr)
          {
            getSyntGrounding().writeReLocationType(pOut, childRelLocationPtr->locationType);
            {
              auto itSpecifier = childGrdExpPtr->children.find(GrammaticalType::SPECIFIER);
              if (itSpecifier != childGrdExpPtr->children.end())
                pChildToPutBeforeSubject = &itSpecifier->second;
            }
            _writeQuel(pOut, **pChildToPutBeforeSubject, pConf, pContext);
            break;
          }
        }
      }
      _strToOut(pOut, PartOfSpeech::PRONOUN, "où");
      break;
    }
    case SemanticRequestType::QUANTITY:
    {
      if (pContext.verbContextOpt)
      {
        const auto& verbContext = *pContext.verbContextOpt;
        mystd::optional<SemanticWord> introWord;
        pConf.lingDb.langToSpec[_language].getSemFrameDict().getIntroWord(introWord, linguistics::ChunkLinkType::DIRECTOBJECT,
                                                                          pContext.verbContextOpt->statGrd.concepts,
                                                                          pContext.verbContextOpt->verbInflWord,
                                                                          nullptr, nullptr,
                                                                          pContext.wordContext.number,
                                                                          pContext.wordContext.gender, verbContext.verbInflWord);
        if (introWord && introWord->lemma == "à")
        {
          _strToOut(pOut, PartOfSpeech::ADVERB, "à combien");
          break;
        }
      }
      _strToOut(pOut, PartOfSpeech::ADVERB, "combien");
      break;
    }
    case SemanticRequestType::DISTANCE:
    {
      _strToOut(pOut, PartOfSpeech::ADVERB, "jusqu'où");
      break;
    }
    case SemanticRequestType::DURATION:
    {
      if (pChildToPutBeforeSubject != nullptr)
        _strToOut(pOut, PartOfSpeech::ADVERB, "combien");
      else
        _strToOut(pOut, PartOfSpeech::ADVERB, "combien de temps");
      break;
    }
    case SemanticRequestType::TIME:
    {
      if (pChildToPutBeforeSubject != nullptr)
      {
        _strToOut(pOut, PartOfSpeech::ADVERB, "en");
        _writeQuel(pOut, **pChildToPutBeforeSubject, pConf, pContext);
        break;
      }
      _strToOut(pOut, PartOfSpeech::ADVERB, "quand");
      break;
    }
    case SemanticRequestType::OBJECT:
    {
      auto* objectPtr = pIsPassive ? pSubjectPtr : pObjectPtr;
      if (objectPtr != nullptr && (pIsPassive || pObjectPosition == ObjectPosition::BEFORESUBJECT))
      {
        const auto& objSemExp = objectPtr->getSemExp();
        if (SemExpGetter::isWhoSemExp(objSemExp))
        {
          _strToOut(pOut, PartOfSpeech::PRONOUN, "qui");
        }
        else
        {
          if (pIsEquVerb)
          {
            auto* objGrdExpPtr = objSemExp.getGrdExpPtr_SkipWrapperPtrs();
            if (objGrdExpPtr != nullptr &&
                !ConceptSet::haveAConceptOrAHyponym(objGrdExpPtr->grounding().concepts, "time"))
              _strToOut(pOut, PartOfSpeech::PREPOSITION, "de");
          }
          _writeQuel(pOut, objSemExp, pConf, pContext);
        }
      }
      else if (objectPtr != nullptr &&
               objectPtr->getSemExp().getListExpPtr_SkipWrapperPtrs() != nullptr)
      {
        // If it's a question about the object and the object a list of element,
        // we assume it's a choice.
        // In that case there is no interrogative word at the beginning
        break;
      }
      else if (pIsEquVerb && pNeedToWriteTheVerb && pSubjectPtr != nullptr &&
               !SemExpGetter::semExpHasACoreferenceOrAnAgent(**pSubjectPtr))
      {
        const auto& subjSemExp = pSubjectPtr->getSemExp();
        _writeQuel(pOut, subjSemExp, pConf, pContext);
      }
      else
      {
        if (objectPtr != nullptr && SemExpGetter::semExpIsAnEmptyStatementGrd(**objectPtr))
        {
          _strToOut(pOut, PartOfSpeech::PRONOUN, "à quoi");
          break;
        }
        if (pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB)
        {
          if (pSubjectPtr == nullptr)
          {
            if (pVerbTense == SemanticVerbTense::UNKNOWN)
              _strToOut(pOut, PartOfSpeech::PRONOUN, "quoi");
            else
              _strToOut(pOut, PartOfSpeech::PRONOUN, "ce qui");
            break;
          }
          _strToOut(pOut, PartOfSpeech::PRONOUN, "ce");
        }
        if (pStatGrd.word.isEmpty() && pStatGrd.concepts.empty())
        {
          _strToOut(pOut, PartOfSpeech::PRONOUN, "lequel");
        }
        else
        {
          _strWithApostropheToOut(pOut, PartOfSpeech::SUBORDINATING_CONJONCTION, "qu'", "que");
          if (pIsEquVerb && pNeedToWriteTheVerb && objectPtr == nullptr)
            _writeCaWord(pSubjectOut);
        }
      }
      break;
    }
    case SemanticRequestType::SUBJECT:
    {
      auto* objectPtr = pIsPassive ? pSubjectPtr : pObjectPtr;
      if (pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN ||
          (pSubjectPtr != nullptr &&
           SemExpGetter::isWhoSemExp(**pSubjectPtr)))
      {
        _strToOut(pOut, PartOfSpeech::PRONOUN, "qui");
        break;
      }
      if (objectPtr == nullptr)
      {
        _strToOut(pOut, PartOfSpeech::PRONOUN, "à qui");
        break;
      }
      bool needToWriteSubject = pIsEquVerb && pSubjectPtr == nullptr &&
          (pObjectPtr == nullptr || SemExpGetter::getMainPartOfSpeech(**pObjectPtr) != PartOfSpeech::ADJECTIVE);
      if (pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB)
      {
        _strToOut(pOut, PartOfSpeech::PRONOUN, "ce");
        if (!needToWriteSubject)
        {
          _strToOut(pOut, PartOfSpeech::PRONOUN, "qui");
          break;
        }
      }
      _strWithApostropheToOut(pOut, PartOfSpeech::SUBORDINATING_CONJONCTION, "qu'", "que");
      if (needToWriteSubject)
        _writeCaWord(pSubjectOut);
      break;
    }
    case SemanticRequestType::MANNER:
      _strToOut(pOut, PartOfSpeech::ADVERB, "comment");
      break;
    case SemanticRequestType::CAUSE:
    case SemanticRequestType::PURPOSE:
      _strToOut(pOut, PartOfSpeech::ADVERB, "pourquoi");
      break;
    case SemanticRequestType::TIMES:
      _strToOut(pOut, PartOfSpeech::ADVERB, "combien de fois");
      break;
    case SemanticRequestType::ABOUT:
    {
      if (pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB)
      {
        _strToOut(pOut, PartOfSpeech::SUBORDINATING_CONJONCTION, "ce qu'il en est");
        _strWithApostropheToOut(pOut, PartOfSpeech::DETERMINER, "d'", "de");
      }
      else
      {
        _strToOut(pOut, PartOfSpeech::PRONOUN, "qu'en est-il");
      }
      break;
    }
    case SemanticRequestType::TOPIC:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "que");
      break;
    case SemanticRequestType::WAY:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "en quoi");
      break;
    case SemanticRequestType::YESORNO:
    case SemanticRequestType::ACTION:
    case SemanticRequestType::CHOICE:
    case SemanticRequestType::NOTHING:
    case SemanticRequestType::VERB:
      break;
    }
  }
}


bool LinguisticSynthesizerFrench::_tryToWriteTopicBeforeVerb
(std::list<WordToSynthesize>& pOut,
 const SemanticExpression& pSemExp) const
{
  if (SemExpGetter::isACoreference(pSemExp, CoreferenceDirectionEnum::BEFORE))
  {
    _strToOut(pOut, PartOfSpeech::PRONOUN_COMPLEMENT, "y");
    return true;
  }
  return false;
}

void LinguisticSynthesizerFrench::_getBeginOfWithChild
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "avec");
}

void LinguisticSynthesizerFrench::_getBeginOfWithoutChild
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "sans");
}

void LinguisticSynthesizerFrench::_getBeginOfForChild
(std::list<WordToSynthesize>& pOut,
 const SemanticExpression&) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "pour");
}

void LinguisticSynthesizerFrench::_getBeginOfReceiverOfNounChild
(std::list<WordToSynthesize>& pOut,
 const SemanticExpression&) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "à");
}

void LinguisticSynthesizerFrench::_getThanWord
(std::list<WordToSynthesize>& pOut,
 const mystd::unique_propagate_const<UniqueSemanticExpression>& pWhatIsComparedExp) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, pWhatIsComparedExp ? "que" : "à");
}

void LinguisticSynthesizerFrench::_getBeginOfMitigationChild
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "mais");
}

void LinguisticSynthesizerFrench::_getBeginOfYesOrNoSubordonate
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "si");
}

bool LinguisticSynthesizerFrench::_beginOfSubordonateIfNeeded
(bool& pIsASubordinateWithoutPreposition,
 OutSemExp& pOutSemExp,
 const SyntSentWorkStruct& pSentWorkStruct,
 GrammaticalType pSubordinateGrammaticalType,
 const GroundedExpression& pSubordinateGrdExp,
 const mystd::optional<SynthesizerVerbContext>&,
 const SynthesizerConfiguration&) const
{
  if (pSubordinateGrammaticalType == GrammaticalType::OBJECT)
  {
    const SemanticStatementGrounding* statGrd = pSubordinateGrdExp->getStatementGroundingPtr();
    if (statGrd != nullptr &&
        statGrd->requests.empty() &&
        (!ConceptSet::haveAConcept(pSentWorkStruct.statementGrd.concepts, "verb_action_say") ||
         pSentWorkStruct.grdExp.children.count(GrammaticalType::RECEIVER) > 0))
    {
      auto itObject = pSubordinateGrdExp.children.find(GrammaticalType::OBJECT);
      const bool objectIsAParentCoreference = itObject != pSubordinateGrdExp.children.end() &&
          SemExpGetter::isACoreference(*itObject->second, CoreferenceDirectionEnum::PARENT);
      if (!objectIsAParentCoreference &&
          (statGrd->verbTense != SemanticVerbTense::UNKNOWN ||
           SemExpGetter::hasChild(pSubordinateGrdExp, GrammaticalType::SUBJECT)))
      {
        _strWithApostropheToOut(pOutSemExp.out, PartOfSpeech::SUBORDINATING_CONJONCTION,
                                "qu'", "que");
        return true;
      }
      pIsASubordinateWithoutPreposition = true;
    }
  }
  return false;
}


void LinguisticSynthesizerFrench::_getWheneverCondition
(std::list<WordToSynthesize>& pOut) const
{
  _strWithApostropheToOut(pOut, PartOfSpeech::CONJUNCTIVE,
                         "à chaque fois qu'", "à chaque fois que");
}

void LinguisticSynthesizerFrench::_writeSubjectOfGeneralitySentence(OutSentence& pOutSentence) const
{
  pOutSentence.subject.partOfSpeech = PartOfSpeech::PRONOUN_SUBJECT;
  pOutSentence.subject.relativePerson = RelativePerson::THIRD_SING;
  _strToOut(pOutSentence.subject.out, pOutSentence.subject.partOfSpeech, "il");
  _strToOut(pOutSentence.receiverBeforeVerb.out, PartOfSpeech::PRONOUN_COMPLEMENT, "y");
}

void LinguisticSynthesizerFrench::_writeGenericSubject(OutSentence& pOutSentence) const
{
  pOutSentence.subject.partOfSpeech = PartOfSpeech::PRONOUN_SUBJECT;
  pOutSentence.subject.relativePerson = RelativePerson::THIRD_SING;
  _strToOut(pOutSentence.subject.out, pOutSentence.subject.partOfSpeech, "il");
}

void LinguisticSynthesizerFrench::_writeAnythingHumanQuantity(OutSentence& pOutSentence) const
{
  pOutSentence.subject.partOfSpeech = PartOfSpeech::PRONOUN_SUBJECT;
  pOutSentence.subject.relativePerson = RelativePerson::THIRD_SING;
  _strToOut(pOutSentence.subject.out, pOutSentence.subject.partOfSpeech, "on");
}

void LinguisticSynthesizerFrench::_getBeginOfBetweenSubordonate
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "entre");
}

void LinguisticSynthesizerFrench::_getBeginOfCauseSubordonate
(std::list<WordToSynthesize>& pOut) const
{
  _strWithApostropheToOut(pOut, PartOfSpeech::CONJUNCTIVE,
                         "parce qu'", "parce que");
}

void LinguisticSynthesizerFrench::_getThenWord
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::ADVERB, "alors");
}

void LinguisticSynthesizerFrench::_getElseWord
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "sinon");
}

void LinguisticSynthesizerFrench::_getPossessiveWord
(std::list<WordToSynthesize>& pOut,
 const SynthesizerWordContext&,
 RelativePerson pOwnerRelativePerson,
 const SemanticGrounding&,
 const SynthesizerConfiguration&,
 const SynthesizerCurrentContext&) const
{
  _strToOut(pOut, PartOfSpeech::DETERMINER, [&]()
  {
    switch (pOwnerRelativePerson)
    {
    case RelativePerson::FIRST_SING:
      return "le mien";
    case RelativePerson::SECOND_SING:
      return "le tien";
    case RelativePerson::THIRD_SING:
      return "leur";
    case RelativePerson::FIRST_PLUR:
      return "le notre";
    case RelativePerson::SECOND_PLUR:
      return "le votre";
    case RelativePerson::THIRD_PLUR:
      return "leur";
    case RelativePerson::UNKNOWN:
      return "leur";
    }
    return "leur";
  }());
}

LinguisticSynthesizerPrivate::SpecifierPosition LinguisticSynthesizerFrench::_getSpecifierPosition
(const linguistics::InflectedWord& pInflWord,
 const SemanticGrounding& pGrd,
 bool pADetermantHasBeenWrote,
 const linguistics::InflectedWord*) const
{
  if (pInflWord.word.partOfSpeech == PartOfSpeech::NOUN ||
      pInflWord.word.partOfSpeech == PartOfSpeech::PROPER_NOUN)
    return SpecifierPosition::AFTER;
  if (!pADetermantHasBeenWrote &&
      pInflWord.word.partOfSpeech == PartOfSpeech::ADVERB &&
      !pInflWord.infos.hasContextualInfo(WordContextualInfos::CANNOTBEBEFORENOUN))
  {
    if (ConceptSet::haveAConceptThatBeginWithAnyOf(pInflWord.infos.concepts, {"comparison_", "degree_", "manner_"}))
      return SpecifierPosition::BEFORE;
    return SpecifierPosition::BEFORE_DETERMINER;
  }
  if (ConceptSet::haveAConceptThatBeginWith(pGrd.concepts, "rank_"))
    return SpecifierPosition::BEFORE;
  if (pInflWord.word.partOfSpeech == PartOfSpeech::INTERJECTION)
    return SpecifierPosition::BEFORE;
  if (pInflWord.infos.hasContextualInfo(WordContextualInfos::CANBEBEFORENOUN))
    return SpecifierPosition::BEFORE;
  auto* genGrdPtr = pGrd.getGenericGroundingPtr();
  if (genGrdPtr != nullptr &&
      genGrdPtr->referenceType == SemanticReferenceType::DEFINITE)
    return SpecifierPosition::BEFORE;
  return SpecifierPosition::AFTER;
}


bool LinguisticSynthesizerFrench::_doWeHaveToWriteBeginOfSpecifier
(SynthesizerCurrentContext& pGrdContext,
 const linguistics::InflectedWord& pInflWord,
 const GroundedExpression& pGrdExpOfTheWord,
 const SemanticGrounding& pMotherGrounding) const
{
  if (pGrdContext.grammaticalTypeFromParent == GrammaticalType::SPECIFIER &&
      pInflWord.word.partOfSpeech == PartOfSpeech::PROPER_NOUN &&
      SemExpGetter::getReferenceTypeFromGrd(*pGrdExpOfTheWord) != SemanticReferenceType::DEFINITE)
    return false;
  if (pMotherGrounding.type == SemanticGroundingType::RELATIVETIME &&
      pInflWord.word.partOfSpeech == PartOfSpeech::VERB)
    return true;
  if (pMotherGrounding.type == SemanticGroundingType::GENERIC)
  {
    auto genGrdPtr = pMotherGrounding.getGenericGroundingPtr();
    if (genGrdPtr->entityType == SemanticEntityType::NUMBER)
      return false;
  }
  if (pMotherGrounding.type == SemanticGroundingType::RELATIVEDURATION ||
      pMotherGrounding.type == SemanticGroundingType::RELATIVELOCATION ||
      pMotherGrounding.type == SemanticGroundingType::RELATIVETIME)
    return false;
  if (partOfSpeech_isNominal(pInflWord.word.partOfSpeech) && !pInflWord.word.isEmpty())
    return true;
  const auto& grdOfTheWord = pGrdExpOfTheWord.grounding();
  if (grdOfTheWord.type == SemanticGroundingType::STATEMENT)
  {
    auto itObject = pGrdExpOfTheWord.children.find(GrammaticalType::OBJECT);
    if (itObject != pGrdExpOfTheWord.children.end() &&
        SemExpGetter::isACoreference(*itObject->second, CoreferenceDirectionEnum::PARENT))
    {
      pGrdContext.isASubordinateWithoutPreposition = true;
      return false;
    }
    if (grdOfTheWord.getStatementGrounding().verbTense != SemanticVerbTense::PUNCTUALPAST)
      return true;

    pGrdContext.isASubordinateWithoutPreposition = true;
  }
  return grdOfTheWord.type == SemanticGroundingType::AGENT ||
      grdOfTheWord.type == SemanticGroundingType::META;
}


void LinguisticSynthesizerFrench::_getComparisonWord
(std::list<WordToSynthesize>& pOut,
 ComparisonOperator pCompPolarity,
 SemanticGenderType pGender,
 SemanticNumberType pNumber) const
{
  static const PartOfSpeech adj = PartOfSpeech::ADJECTIVE;
  if (pGender == SemanticGenderType::FEMININE)
  {
    switch (pCompPolarity)
    {
    case ComparisonOperator::DIFFERENT:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "différentes" : "différente");
      break;
    case ComparisonOperator::EQUAL:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "égales" : "égale");
      break;
    case ComparisonOperator::LESS:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "inférieures" : "inférieure");
      break;
    case ComparisonOperator::MORE:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "supérieures" : "supérieure");
      break;
    }
  }
  else
  {
    switch (pCompPolarity)
    {
    case ComparisonOperator::DIFFERENT:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "différents" : "différent");
      break;
    case ComparisonOperator::EQUAL:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "égaux" : "égal");
      break;
    case ComparisonOperator::LESS:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "inférieurs" : "inférieur");
      break;
    case ComparisonOperator::MORE:
      _strToOut(pOut, adj, pNumber == SemanticNumberType::PLURAL ?
               "supérieurs" : "supérieur");
      break;
    }
  }
}

} // End of namespace onsem
