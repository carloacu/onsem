#include "linguisticsynthesizerenglish.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include "tool/synthesizeradder.hpp"
#include "tool/synthesizergetter.hpp"


namespace onsem
{


LinguisticSynthesizerEnglish::LinguisticSynthesizerEnglish()
  : LinguisticSynthesizerPrivate(SemanticLanguageEnum::ENGLISH),
    _syntGrounding(*this),
    _tokenToStrConverter()
{
}


LinguisticSynthesizerPrivate::ObjectPosition LinguisticSynthesizerEnglish::_getObjectPosition
(SyntSentWorkStruct& pSentWorkStruct,
 const SemanticStatementGrounding&,
 const SemanticRequests& pRequests,
 const SynthesizerConfiguration& pConf,
 LinguisticVerbTense) const
{
  if (pSentWorkStruct.objectPtr == nullptr)
    return ObjectPosition::AFTERVERB;
  const GroundedExpression* objectGrdExpPtr = pSentWorkStruct.objectPtr->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
  if (objectGrdExpPtr != nullptr)
  {
    const GroundedExpression& objectGrdExp = *objectGrdExpPtr;
    const SemanticGrounding& objectGrd = objectGrdExp.grounding();
    auto getObjPositionOfGenericGrd = [&](const SemanticGenericGrounding& pGenGrd) {
      if ((pRequests.has(SemanticRequestType::OBJECT) ||
           pRequests.has(SemanticRequestType::QUANTITY)) &&
          !pGenGrd.coreference &&
          pGenGrd.referenceType != SemanticReferenceType::DEFINITE)
        return LinguisticSynthesizerPrivate::ObjectPosition::BEFOREVERB;
      if (pGenGrd.coreference && pGenGrd.word.isEmpty())
        return LinguisticSynthesizerPrivate::ObjectPosition::JUSTAFTERVERB;
      return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
    };
    if (objectGrd.type == SemanticGroundingType::GENERIC)
    {
      const SemanticGenericGrounding& objectGenGrd = objectGrdExp->getGenericGrounding();
      return getObjPositionOfGenericGrd(objectGenGrd);
    }
    if (objectGrd.type == SemanticGroundingType::STATEMENT)
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
            if (objObjGenExpObjPos == ObjectPosition::BEFOREVERB)
            {
              SynthesizerCurrentContext context;
              context.verbTense = LinguisticVerbTense::INFINITIVE;
              _syntGrounding.writeVerbalSemWord(pSentWorkStruct.outs.verb2.out, pSentWorkStruct.outs.verb2,
                                          pSentWorkStruct.outs.verb2.out, objectStatGrd.word, pConf.lingDb, context);
              pSentWorkStruct.objectPtr = &objObjUSemExp;
              return objObjGenExpObjPos;
            }
          }
        }
      }
    }
    if (objectGrd.type == SemanticGroundingType::UNITY)
    {
      return LinguisticSynthesizerPrivate::ObjectPosition::BEFORESUBJECT;
    }
  }
  return LinguisticSynthesizerPrivate::ObjectPosition::AFTERVERB;
}


LinguisticVerbTense LinguisticSynthesizerEnglish::_semanticVerbTenseToLinguisticVerbTense(SemanticVerbTense pSemVerbTense,
                                                                                          SynthesizerCurrentContextType,
                                                                                          const SemanticStatementGrounding*,
                                                                                          const SemanticRequests&,
                                                                                          const linguistics::WordAssociatedInfos* pWordInfosPtr) const
{
  switch (pSemVerbTense)
  {
  case SemanticVerbTense::FUTURE:
    return LinguisticVerbTense::FUTURE_INDICATIVE;
  case SemanticVerbTense::PAST:
  {
    if (pWordInfosPtr != nullptr)
    {
      if (ConceptSet::haveAConceptThatBeginWith(pWordInfosPtr->concepts, "verb_action_"))
        return LinguisticVerbTense::PRETERIT_CONTINUOUS;
      if (pWordInfosPtr->hasContextualInfo(WordContextualInfos::CANNOTBECONTINUOUS))
        return LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
    }
    return LinguisticVerbTense::IMPERFECT_INDICATIVE;
  }
  case SemanticVerbTense::PRESENT:
    return LinguisticVerbTense::PRESENT_INDICATIVE;
  case SemanticVerbTense::PUNCTUALPRESENT:
  {
    if (pWordInfosPtr != nullptr &&
        pWordInfosPtr->hasContextualInfo(WordContextualInfos::CANNOTBECONTINUOUS))
      return LinguisticVerbTense::PRESENT_INDICATIVE;
    return LinguisticVerbTense::PRESENT_CONTINUOUS;
  }
  case SemanticVerbTense::PUNCTUALPAST:
    return LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
  case SemanticVerbTense::UNKNOWN:
    return LinguisticVerbTense::INFINITIVE;
  }
  return LinguisticVerbTense::INFINITIVE;
}


void LinguisticSynthesizerEnglish::_getQuestionWord
(std::list<WordToSynthesize>& pOut,
 OutSemExp&,
 const SemanticRequests& pRequests,
 bool,
 SemanticVerbTense,
 const SemanticStatementGrounding& pStatGrd,
 const UniqueSemanticExpression* pSubjectPtr,
 const UniqueSemanticExpression*& pObjectPtr,
 bool pIsPassive,
 ObjectPosition,
 const UniqueSemanticExpression*& pChildToPutBeforeSubject,
 const SynthesizerConfiguration& pConf,
 SynthesizerCurrentContextType pHoldingContextType,
 const SynthesizerCurrentContext&,
 bool) const
{
  bool firstIteration = true;
  for (const auto& currRequest : pRequests.types)
  {
    if (firstIteration)
      firstIteration = false;
    else
      _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "and");
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
            _strToOut(pOut, PartOfSpeech::PRONOUN, "which");
            break;
          }
        }
      }
      _strToOut(pOut, PartOfSpeech::PRONOUN, "where");
      break;
    }
    case SemanticRequestType::QUANTITY:
    {
      if (pObjectPtr != nullptr &&
          SemExpGetter::isUncountableSemExp(**pObjectPtr, pConf.lingDb) == true)
        _strToOut(pOut, PartOfSpeech::PRONOUN, "how much");
      else
        _strToOut(pOut, PartOfSpeech::PRONOUN, "how many");
      break;
    }
    case SemanticRequestType::TIME:
    {
      if (pChildToPutBeforeSubject != nullptr)
        _strToOut(pOut, PartOfSpeech::PRONOUN, "what");
      else
        _strToOut(pOut, PartOfSpeech::PRONOUN, "when");
      break;
    }
    case SemanticRequestType::OBJECT:
    {
      auto* objectPtr = pIsPassive ? pSubjectPtr : pObjectPtr;
      if (objectPtr != nullptr)
      {
        if (SemExpGetter::isWhoSemExp(**objectPtr))
        {
          _strToOut(pOut, PartOfSpeech::PRONOUN, "who");
          break;
        }
        if (currRequest == SemanticRequestType::OBJECT &&
            objectPtr->getSemExp().getListExpPtr_SkipWrapperPtrs() != nullptr)
          break;
      }
      bool introWordWritten = false;
      if (pObjectPtr != nullptr)
      {
        auto* objGrdPtr = pObjectPtr->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
        if (objGrdPtr != nullptr)
        {
          auto* objGenGrdPtr = objGrdPtr->grounding().getGenericGroundingPtr();
          if (objGenGrdPtr != nullptr &&
              objGenGrdPtr->word.isEmpty() && objGenGrdPtr->concepts.empty() && !objGenGrdPtr->coreference)
          {
            _strToOut(pOut, PartOfSpeech::PRONOUN, "which");
            pObjectPtr = nullptr;
            introWordWritten = true;
          }
        }
      }
      if (!introWordWritten)
        _strToOut(pOut, PartOfSpeech::PRONOUN, pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN ||
                  pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERADJECTIVE ?
                    "that" : "what");
      break;
    }
    case SemanticRequestType::SUBJECT:
    {
      if (pSubjectPtr != nullptr &&
          SemExpGetter::isWhoSemExp(**pSubjectPtr))
      {
        _strToOut(pOut, PartOfSpeech::PRONOUN, "who");
        break;
      }
      _strToOut(pOut, PartOfSpeech::PRONOUN, pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN ||
                pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERADJECTIVE?
                  "that" : "what");
      break;
    }
    case SemanticRequestType::MANNER:
    case SemanticRequestType::WAY:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "how");
      break;
    case SemanticRequestType::DISTANCE:
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN, "how far");
      break;
    }
    case SemanticRequestType::DURATION:
    {
      if (pChildToPutBeforeSubject != nullptr)
        _strToOut(pOut, PartOfSpeech::PRONOUN, "how many");
      else if (!ConceptSet::haveAConcept(pStatGrd.concepts, "verb_have"))
        _strToOut(pOut, PartOfSpeech::PRONOUN, "how long");
      else
        _strToOut(pOut, PartOfSpeech::PRONOUN, "how much time");
      break;
    }
    case SemanticRequestType::CAUSE:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "why");
      break;
    case SemanticRequestType::PURPOSE:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "for what purpose");
      break;
    case SemanticRequestType::TIMES:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "how many times");
      break;
    case SemanticRequestType::ABOUT:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "what about");
      break;
    case SemanticRequestType::TOPIC:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "what");
      break;
    case SemanticRequestType::ACTION:
    case SemanticRequestType::YESORNO:
    case SemanticRequestType::CHOICE:
    case SemanticRequestType::NOTHING:
    case SemanticRequestType::VERB:
      break;
    }
  }
}

void LinguisticSynthesizerEnglish::_getPossessiveDeterminer
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelPerson,
 SemanticGenderType pGender,
 SemanticNumberType) const
{
  _strToOut(pOut, PartOfSpeech::DETERMINER, [&]()
  {
    switch (pRelPerson)
    {
    case RelativePerson::FIRST_SING:
      return "my";
    case RelativePerson::SECOND_SING:
    case RelativePerson::SECOND_PLUR:
      return "your";
    case RelativePerson::THIRD_SING:
    case RelativePerson::UNKNOWN:
      return pGender == SemanticGenderType::FEMININE ?
            "her" : "his";
    case RelativePerson::FIRST_PLUR:
      return "our";
    case RelativePerson::THIRD_PLUR:
      return "their";
    }
    assert(false);
    return "";
  }());
}


void LinguisticSynthesizerEnglish::_getOfWord(std::list<WordToSynthesize>& pOut,
                                              const SynthesizerWordContext&) const
{
  static const std::string specificationWordStr = "of";
  pOut.emplace_back(SemanticWord(_language, specificationWordStr,
                                 PartOfSpeech::PREPOSITION),
                    InflectionToSynthesize(specificationWordStr, true, true,
                                           ifNextWordIsNotAPrepostion));
}

void LinguisticSynthesizerEnglish::_getBeginOfSpecification
(std::list<WordToSynthesize>& pOut,
 const GroundedExpression& pChildGrdExp,
 const SemanticGrounding& pParentGrounding,
 const SynthesizerWordContext& pWordContext) const
{
  const SemanticStatementGrounding* statGrdPtr = pChildGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
  {
    auto itSubject = pChildGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject != pChildGrdExp.children.end() &&
        SemExpGetter::isACoreference(*itSubject->second, CoreferenceDirectionEnum::PARENT))
    {
      if (SemExpGetter::isAHumanFromGrd(pParentGrounding))
        _strToOut(pOut, PartOfSpeech::SUBORDINATING_CONJONCTION, "who");
      else
        _strToOut(pOut, PartOfSpeech::SUBORDINATING_CONJONCTION, "that");
    }
    return;
  }
  _getOfWord(pOut, pWordContext);
}

void LinguisticSynthesizerEnglish::_getBeginOfIntervalChild
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::DETERMINER, "every");
}

void LinguisticSynthesizerEnglish::_getBeginOfWithChild
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "with");
}

void LinguisticSynthesizerEnglish::_getBeginOfWithoutChild
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "without");
}

void LinguisticSynthesizerEnglish::_getBeginOfForChild
(std::list<WordToSynthesize>& pOut,
 const SemanticExpression& pSemExp) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION,
           SemExpGetter::semExphasAStatementGrd(pSemExp) ? "in order to" : "for");
}

void LinguisticSynthesizerEnglish::_getBeginOfReceiverOfNounChild
(std::list<WordToSynthesize>& pOut,
 const SemanticExpression&) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "to");
}

void LinguisticSynthesizerEnglish::_getThanWord
(std::list<WordToSynthesize>& pOut,
 const mystd::unique_propagate_const<UniqueSemanticExpression>& pWhatIsComparedExp) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, pWhatIsComparedExp ? "than" : "to");
}

void LinguisticSynthesizerEnglish::_getBeginOfMitigationChild
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "but");
}

void LinguisticSynthesizerEnglish::_getBeginOfYesOrNoSubordonate
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "if");
}

void LinguisticSynthesizerEnglish::_getWheneverCondition
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "whenever");
}

void LinguisticSynthesizerEnglish::_writeSubjectOfGeneralitySentence(OutSentence& pOutSentence) const
{
  _strToOut(pOutSentence.subject.out, PartOfSpeech::PRONOUN, "there");
}

void LinguisticSynthesizerEnglish::_writeGenericSubject(OutSentence& pOutSentence) const
{
  _strToOut(pOutSentence.subject.out, PartOfSpeech::PRONOUN_SUBJECT, "we");
}

void LinguisticSynthesizerEnglish::_writeAnythingHumanQuantity(OutSentence& pOutSentence) const
{
  pOutSentence.subject.partOfSpeech = PartOfSpeech::PRONOUN_SUBJECT;
  pOutSentence.subject.relativePerson = RelativePerson::THIRD_PLUR;
  _strToOut(pOutSentence.subject.out, pOutSentence.subject.partOfSpeech, "we");
}

void LinguisticSynthesizerEnglish::_getBeginOfBetweenSubordonate
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, "between");
}

void LinguisticSynthesizerEnglish::_getBeginOfCauseSubordonate
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::CONJUNCTIVE, "because");
}

void LinguisticSynthesizerEnglish::_getThenWord
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::ADVERB, "then");
}

void LinguisticSynthesizerEnglish::_getElseWord
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::ADVERB, "else");
}

void LinguisticSynthesizerEnglish::_getPossessiveWord
(std::list<WordToSynthesize>& pOut,
 const SynthesizerWordContext&,
 RelativePerson pOwnerRelativePerson,
 const SemanticGrounding& pGrounding,
  const SynthesizerConfiguration& pConf,
  const SynthesizerCurrentContext& pContext) const
{
  _strToOut(pOut, PartOfSpeech::DETERMINER, [&]()
  {
    switch (pOwnerRelativePerson)
    {
    case RelativePerson::FIRST_SING:
      return "mine";
    case RelativePerson::SECOND_SING:
      return "yours";
    case RelativePerson::THIRD_SING:
    {
      auto gender = synthGetter::getGenderFromGrounding(pGrounding, pConf, pContext, getSyntGrounding());
      if (gender == SemanticGenderType::FEMININE)
        return "hers";
      if (gender == SemanticGenderType::FEMININE)
        return "his";
      return "its";
    }
    case RelativePerson::FIRST_PLUR:
      return "ours";
    case RelativePerson::SECOND_PLUR:
      return "yours";
    case RelativePerson::THIRD_PLUR:
      return "theirs";
    case RelativePerson::UNKNOWN:
      return "its";
    }
    return "its";
  }());
}

LinguisticSynthesizerPrivate::SpecifierPosition LinguisticSynthesizerEnglish::_getSpecifierPosition
(const linguistics::InflectedWord&,
 const SemanticGrounding& pGrounding,
 bool,
 const linguistics::InflectedWord* pMotherInflWordPtr) const
{
  if (pMotherInflWordPtr != nullptr &&
      pMotherInflWordPtr->infos.hasContextualInfo(WordContextualInfos::CANBEBEFORENOUN))
    return SpecifierPosition::AFTER;
  if (ConceptSet::haveAConceptThatBeginWith(pGrounding.concepts, "rank_"))
    return SpecifierPosition::BEFORE;
  const SemanticGenericGrounding* genGrdPtr = pGrounding.getGenericGroundingPtr();
  if (genGrdPtr != nullptr)
  {
    if (genGrdPtr->entityType == SemanticEntityType::MODIFIER)
      return SpecifierPosition::BEFORE;
    if (genGrdPtr->referenceType != SemanticReferenceType::INDEFINITE &&
        genGrdPtr->referenceType != SemanticReferenceType::DEFINITE)
      return SpecifierPosition::BEFORE;
    return SpecifierPosition::AFTER;
  }
  if (pGrounding.getStatementGroundingPtr() != nullptr)
    return SpecifierPosition::AFTER;
  if (pGrounding.getAgentGroundingPtr() == nullptr)
    return SpecifierPosition::BEFORE;
  return SpecifierPosition::AFTER;
}


bool LinguisticSynthesizerEnglish::_doWeHaveToWriteBeginOfSpecifier
(SynthesizerCurrentContext& pGrdContext,
 const linguistics::InflectedWord& pInflWord,
 const GroundedExpression& pGrdExpOfTheWord,
 const SemanticGrounding& pMotherGrounding) const
{
  if (pMotherGrounding.type == SemanticGroundingType::RELATIVEDURATION ||
      pMotherGrounding.type == SemanticGroundingType::RELATIVELOCATION ||
      pMotherGrounding.type == SemanticGroundingType::RELATIVETIME)
    return false;
  if (pGrdExpOfTheWord.children.count(GrammaticalType::INTRODUCTING_WORD) > 0)
    return false;
  if (partOfSpeech_isNominal(pInflWord.word.partOfSpeech))
    return true;
  const auto& grdOfTheWord = pGrdExpOfTheWord.grounding();
  if (grdOfTheWord.type == SemanticGroundingType::STATEMENT)
  {
    if (grdOfTheWord.getStatementGrounding().verbTense != SemanticVerbTense::PUNCTUALPAST)
      return true;
    pGrdContext.isASubordinateWithoutPreposition = true;
  }
  return false;
}



void LinguisticSynthesizerEnglish::_getComparisonWord
(std::list<WordToSynthesize>& pOut,
 ComparisonOperator pCompPolarity,
 SemanticGenderType,
 SemanticNumberType) const
{
  static const PartOfSpeech adj = PartOfSpeech::ADJECTIVE;
  switch (pCompPolarity)
  {
  case ComparisonOperator::DIFFERENT:
    _strToOut(pOut, adj, "different");
    break;
  case ComparisonOperator::EQUAL:
    _strToOut(pOut, adj, "equal");
    break;
  case ComparisonOperator::LESS:
    _strToOut(pOut, adj, "inferior");
    break;
  case ComparisonOperator::MORE:
    _strToOut(pOut, adj, "superior");
    break;
  }
}



} // End of namespace onsem
