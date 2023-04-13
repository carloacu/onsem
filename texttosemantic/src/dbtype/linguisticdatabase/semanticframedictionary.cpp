#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/common/linguisticsubordinateid.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/wordtosynthesize.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include "childspecification.hpp"


namespace onsem
{
namespace linguistics
{
struct ChildSpecificationsContainer
{
  void addChildSpec(ChildSpecification&& pChildSpec);
  mystd::optional<std::string> templateNameOpt{};
  std::list<ChildSpecification> listOfSpecifications{};
  std::map<std::string, std::list<ChildSpecification*>> lemmaToChildSpecs{};
  std::map<ChunkLinkType, std::list<ChildSpecification*>> chkLinkToChildSpecs{};
};

namespace
{

template<typename TOKENIT>
void _skipDeterminer(TOKENIT& pItToken,
                     const TOKENIT& pItEnd)
{
  while (pItToken != pItEnd &&
         pItToken->inflWords.front().word.partOfSpeech == PartOfSpeech::DETERMINER)
  {
    pItToken = getNextToken(pItToken, pItEnd);
    continue;
  }
}

bool _conceptConditionChecking(const LinguisticConditionTreeValue& pConditionValue,
                               bool pBeginOfConceptOrHyponym,
                               const ConstTokenIterator* pNextToken)
{
  if (pNextToken == nullptr || pNextToken->atEnd())
    return false;
  auto itToken = pNextToken->getTokenIt();
  auto itEnd = pNextToken->getItEnd();
  if (itToken != itEnd)
  {
    if (pConditionValue.parameters.size() >= 2)
    {
      if (pConditionValue.parameters[1] == "skip_determiners")
      {
        _skipDeterminer(itToken, itEnd);
        if (itToken == itEnd)
          return false;
      }
      else if (pConditionValue.parameters[1] == "have_number_before")
      {
        if (!ConceptSet::haveAConceptThatBeginWith(itToken->inflWords.front().infos.concepts, "number_"))
          return false;
        itToken = getNextToken(itToken, itEnd);
        while (itToken != itEnd &&
               ConceptSet::haveAConceptThatBeginWith(itToken->inflWords.front().infos.concepts, "number_"))
          itToken = getNextToken(itToken, itEnd);
        if (itToken == itEnd)
          return false;
      }
    }
    const InflectedWord& inflWord = itToken->inflWords.front();
    if (!pConditionValue.parameters.empty() &&
        ((pBeginOfConceptOrHyponym && ConceptSet::haveAConceptThatBeginWith(inflWord.infos.concepts, pConditionValue.parameters[0])) ||
         (!pBeginOfConceptOrHyponym && ConceptSet::haveAConceptOrAHyponym(inflWord.infos.concepts, pConditionValue.parameters[0]))))
    {
      if (pConditionValue.parameters.size() >= 3 &&
          pConditionValue.parameters[2].size() >= 2)
      {
        SemanticNumberType numberOfText = SemanticNumberType::UNKNOWN;
        SemanticGenderType genderOfText = SemanticGenderType::UNKNOWN;
        InflectionsChecker::getNounNumberAndGender(numberOfText, genderOfText, inflWord);
        SemanticGenderType genderOfCondition = SemanticGenderType::UNKNOWN;
        gender_fromConcisePrint(genderOfCondition, pConditionValue.parameters[2][0]);
        if (genderOfText == genderOfCondition)
        {
          SemanticNumberType numberOfCondition = SemanticNumberType::UNKNOWN;
          number_fromConcisePrint(numberOfCondition, pConditionValue.parameters[2][1]);
          return numberOfText == numberOfCondition;
        }
        return false;
      }
      return true;
    }
  }
  return false;
}


bool _checkDate(const SemanticDate& pDate,
                const std::vector<std::string>& pParameters)
{
  return pParameters.empty() || pDate.day.has_value();
}

bool _isConditionIsVerified(const LinguisticConditionTreeValue& pConditionValue,
                            const ConstTokenIterator* pNextToken)
{
  switch (pConditionValue.condition)
  {
  case LinguisticCondition::FOLLOWEDBYBEGINOFCONCEPT:
    return _conceptConditionChecking(pConditionValue, true, pNextToken);
  case LinguisticCondition::FOLLOWEDBYCONCEPTORHYPONYM:
    return _conceptConditionChecking(pConditionValue, false, pNextToken);
  case LinguisticCondition::FOLLOWEDBYDEFINITENOUN:
  {
    if (pNextToken != nullptr && !pNextToken->atEnd())
    {
      const InflectedWord& inflWord = pNextToken->getToken().inflWords.front();
      return inflWord.word.partOfSpeech == PartOfSpeech::DETERMINER &&
          ConceptSet::haveAConcept(inflWord.infos.concepts, {"reference_definite"}) &&
          !ConceptSet::haveAConceptThatBeginWithAnyOf(inflWord.infos.concepts, {"tolink_", "number_"});
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYHOUR:
  {
    if (pNextToken != nullptr && !pNextToken->atEnd())
      return isAnHour(*pNextToken);
    break;
  }
  case LinguisticCondition::FOLLOWEDBYINFINITIVEVERB:
  {
    if (pNextToken != nullptr && !pNextToken->atEnd())
    {
      const InflectedWord& inflWord = pNextToken->getToken().inflWords.front();
      if (partOfSpeech_isVerbal(inflWord.word.partOfSpeech))
        return true;
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYOCCURRENCERANK:
  {
    if (pNextToken == nullptr || pNextToken->atEnd())
      return false;
    auto itToken = pNextToken->getTokenIt();
    auto itEnd = pNextToken->getItEnd();
    if (itToken != itEnd)
    {
      if (itToken->inflWords.front().word.partOfSpeech != PartOfSpeech::DETERMINER)
        return false;
      itToken = getNextToken(itToken, itEnd);
      if (itToken != itEnd &&
          ConceptSet::haveAConceptThatBeginWith(itToken->inflWords.front().infos.concepts, "rank_"))
      {
        itToken = getNextToken(itToken, itEnd);
        if (itToken != itEnd)
        {
          const auto& currInflWord = itToken->inflWords.front();
          const std::string timeCpt = currInflWord.word.language == SemanticLanguageEnum::ENGLISH ? "time" : "times";
          return currInflWord.infos.concepts.count(timeCpt) > 0;
        }
      }
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYPARTOFSPEECH:
  {
    if (!pConditionValue.parameters.empty())
    {
      return pNextToken != nullptr && !pNextToken->atEnd() &&
          pNextToken->getToken().inflWords.front().word.partOfSpeech == partOfSpeech_fromStr(pConditionValue.parameters[0]);
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYRESOURCE:
  {
    if (!pConditionValue.parameters.empty())
    {
      return pNextToken != nullptr && !pNextToken->atEnd() &&
          pNextToken->getToken().inflWords.front().word.partOfSpeech == PartOfSpeech::BOOKMARK;
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYSUBORDINATE:
  {
    if (pNextToken == nullptr || pNextToken->atEnd())
      return false;
    auto itToken = pNextToken->getTokenIt();
    auto itEnd = pNextToken->getItEnd();
    while (itToken != itEnd)
    {
      const InflectedWord& inflWord = itToken->inflWords.front();
      if (partOfSpeech_isNominal(inflWord.word.partOfSpeech) ||
          partOfSpeech_isPronominal(inflWord.word.partOfSpeech))
      {
        itToken = getNextToken(itToken, itEnd);
        continue;
      }
      return inflWord.word.partOfSpeech == PartOfSpeech::VERB;
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYUNDEFINEDREF:
  {
    if (pNextToken != nullptr && !pNextToken->atEnd())
    {
      const InflectedWord& inflWord = pNextToken->getToken().inflWords.front();
      if (inflWord.word.partOfSpeech == PartOfSpeech::NOUN)
        return true;
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYQUANTITYEVERYTHING:
  {
    return pNextToken != nullptr && !pNextToken->atEnd() &&
        ConceptSet::haveAConcept(pNextToken->getToken().inflWords.front().infos.concepts, "quantity_everything");
  }
  case LinguisticCondition::FOLLOWEDBYVOWEL:
  {
    if (pNextToken == nullptr || pNextToken->atEnd())
      return false;
    auto itToken = pNextToken->getTokenIt();
    auto itEnd = pNextToken->getItEnd();
    while (itToken != itEnd)
    {
      const InflectedWord& inflWord = itToken->inflWords.front();
      if (inflWord.word.partOfSpeech == PartOfSpeech::DETERMINER)
      {
        itToken = getNextToken(itToken, itEnd);
        continue;
      }
      return isFirstLetterAVowel(itToken->str, 0);
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYONEOFPREFIXES:
  {
    if (pNextToken != nullptr && !pNextToken->atEnd())
    {
      const auto& tokenStr = pNextToken->getToken().str;
      for (const auto& param : pConditionValue.parameters)
        if (tokenStr.compare(0, param.size(), param) == 0)
          return true;
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYFEMININE:
  {
    if (pNextToken != nullptr && !pNextToken->atEnd())
    {
      const InflectedWord& inflWord = pNextToken->getToken().inflWords.front();
      SemanticNumberType number = SemanticNumberType::UNKNOWN;
      SemanticGenderType gender = SemanticGenderType::UNKNOWN;
      InflectionsChecker::getNounNumberAndGender(number, gender, inflWord);
      return gender == SemanticGenderType::FEMININE;
    }
    return false;
  }
  case LinguisticCondition::LOCATIONEN_FR:
  {
    if (pNextToken != nullptr && !pNextToken->atEnd())
    {
      const InflectedWord& inflWord = pNextToken->getToken().inflWords.front();
      if (ConceptSet::haveAnyOfConcepts(inflWord.infos.concepts, {"location_relative_high", "location_relative_low"}))
        return true;
      SemanticNumberType number = SemanticNumberType::UNKNOWN;
      SemanticGenderType gender = SemanticGenderType::UNKNOWN;
      InflectionsChecker::getNounNumberAndGender(number, gender, inflWord);
      if (ConceptSet::haveAConceptThatBeginWith(inflWord.infos.concepts, "country_") &&
          number != SemanticNumberType::PLURAL &&
          !(gender == SemanticGenderType::MASCULINE &&
            !inflWord.word.lemma.empty() &&
            !isFirstLetterAVowel(inflWord.word.lemma)))
        return true;
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYDATE:
  {
    if (pNextToken != nullptr)
    {
      auto itToken = pNextToken->getTokenIt();
      auto itEnd = pNextToken->getItEnd();
      _skipDeterminer(itToken, itEnd);
      auto dateOpt = extractDate(itToken, pNextToken->getTokenRange());
      if (dateOpt)
        return _checkDate(*dateOpt, pConditionValue.parameters);
    }
    return false;
  }
  }
  return false;
}

bool _areConditionSatisfied(const std::shared_ptr<LinguisticConditionTree>& pConditionTree,
                            const ConstTokenIterator* pNextToken)
{
  if (!pConditionTree)
    return true;
  switch (pConditionTree->type)
  {
  case LinguisticConditionTreeType::VALUE:
  {
    return _isConditionIsVerified(pConditionTree->getValue(), pNextToken);
  }
  case LinguisticConditionTreeType::OPERAND:
  {
    const auto& operandStruct = pConditionTree->getOperand();
    switch (operandStruct.operand)
    {
    case LinguisticConditionTreeOperandEnum::AND:
    {
      for (const auto& currChild : operandStruct.children)
        if (!_areConditionSatisfied(currChild, pNextToken))
          return false;
      return true;
    }
    case LinguisticConditionTreeOperandEnum::NOT:
    {
      if (!operandStruct.children.empty())
        return !_areConditionSatisfied(operandStruct.children.front(), pNextToken);
      break;
    }
    case LinguisticConditionTreeOperandEnum::OR:
    {
      for (const auto& currChild : operandStruct.children)
        if (_areConditionSatisfied(currChild, pNextToken))
          return true;
      return false;
    }
    }
    break;
  }
  }
  return false;
}


bool _doesConceptConditionMatchWithGrounding(const LinguisticConditionTreeValue& pConditionValue,
                                             bool pBeginOfConceptOrConceptWithHyponyms,
                                             const SemanticGrounding& pGrounding,
                                             const GroundedExpression& pHoldingGrdExp,
                                             const std::list<WordToSynthesize>* pOut,
                                             SemanticNumberType pNumber,
                                             SemanticGenderType pGender)
{
  if (!pConditionValue.parameters.empty())
  {
    if ((pConditionValue.parameters.size() < 2 ||
         pConditionValue.parameters[1] != "skip_determiners") &&
        pHoldingGrdExp.children.count(GrammaticalType::OWNER) > 0)
      return false;

    const auto& concept = pConditionValue.parameters.front();
    switch (pGrounding.type)
    {
    case SemanticGroundingType::TIME:
    {
      if (pBeginOfConceptOrConceptWithHyponyms)
      {
        if (concept == "time_")
          return true;
        if (pOut != nullptr && !pOut->empty() &&
            ((pBeginOfConceptOrConceptWithHyponyms && ConceptSet::haveAConceptThatBeginWith(pOut->front().concepts, concept)) ||
             (!pBeginOfConceptOrConceptWithHyponyms && ConceptSet::haveAConceptOrAHyponym(pOut->front().concepts, concept))))
          return true;
      }
      return false;
    }
    case SemanticGroundingType::DURATION:
      return concept == "duration_";
    case SemanticGroundingType::LANGUAGE:
      return concept == "language_";
    default:
    {
      auto* genGrdPtr = pGrounding.getGenericGroundingPtr();
      if (genGrdPtr != nullptr &&
          genGrdPtr->entityType == SemanticEntityType::NUMBER &&
          concept == "number_")
        return true;

      if ((pBeginOfConceptOrConceptWithHyponyms && ConceptSet::haveAConceptThatBeginWith(pGrounding.concepts, concept)) ||
          (!pBeginOfConceptOrConceptWithHyponyms && ConceptSet::haveAConceptOrAHyponym(pGrounding.concepts, concept)))
      {
        if (pConditionValue.parameters.size() >= 3 &&
            pConditionValue.parameters[2].size() >= 2)
        {
          SemanticGenderType genderOfCondition = SemanticGenderType::UNKNOWN;
          gender_fromConcisePrint(genderOfCondition, pConditionValue.parameters[2][0]);
          if (pGender == genderOfCondition)
          {
            SemanticNumberType numberOfCondition = SemanticNumberType::UNKNOWN;
            number_fromConcisePrint(numberOfCondition, pConditionValue.parameters[2][1]);
            return pNumber == numberOfCondition;
          }
          return false;
        }
        return true;
      }
      return false;
    }
    }
  }
  return false;
}


bool _doesConditionMatchWithGrounding(const LinguisticConditionTreeValue& pConditionValue,
                                      const GroundedExpression& pObjectGrdExp,
                                      const std::list<WordToSynthesize>* pOut,
                                      SemanticNumberType pNumber,
                                      SemanticGenderType pGender,
                                      const linguistics::InflectedWord& pInflWord)
{
  const auto& grounding = pObjectGrdExp.grounding();
  switch (pConditionValue.condition)
  {
  case LinguisticCondition::FOLLOWEDBYBEGINOFCONCEPT:
    return _doesConceptConditionMatchWithGrounding(pConditionValue, true, grounding, pObjectGrdExp, pOut, pNumber, pGender);
  case LinguisticCondition::FOLLOWEDBYCONCEPTORHYPONYM:
    return _doesConceptConditionMatchWithGrounding(pConditionValue, false, grounding, pObjectGrdExp, pOut, pNumber, pGender);
  case LinguisticCondition::FOLLOWEDBYDEFINITENOUN:
  {
    const auto* genGrdPtr = grounding.getGenericGroundingPtr();
    if (genGrdPtr != nullptr)
      return genGrdPtr->referenceType == SemanticReferenceType::DEFINITE;
    const auto* timeGrdPtr = grounding.getTimeGroundingPtr();
    if (timeGrdPtr != nullptr && timeGrdPtr->date.day)
      return true;
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYHOUR:
  {
    return grounding.getTimeGroundingPtr() != nullptr;
  }
  case LinguisticCondition::FOLLOWEDBYINFINITIVEVERB:
  {
    const auto* statGrdPtr = grounding.getStatementGroundingPtr();
    return statGrdPtr != nullptr && statGrdPtr->isAtInfinitive();
  }
  case LinguisticCondition::FOLLOWEDBYOCCURRENCERANK:
  {
    const std::string timeCpt = pInflWord.word.language == SemanticLanguageEnum::ENGLISH ? "time" : "times";
    if (pInflWord.infos.concepts.count(timeCpt) > 0)
    {
      auto itSpecifier = pObjectGrdExp.children.find(GrammaticalType::SPECIFIER);
      if (itSpecifier != pObjectGrdExp.children.end())
      {
        const auto* specGrdExpPtr = itSpecifier->second->getGrdExpPtr_SkipWrapperPtrs();
        return specGrdExpPtr != nullptr &&
            ConceptSet::haveAConceptThatBeginWith(specGrdExpPtr->grounding().concepts, "rank_");
      }
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYPARTOFSPEECH:
  {
    if (!pConditionValue.parameters.empty())
    {
      PartOfSpeech pos = partOfSpeech_fromStr(pConditionValue.parameters[0]);
      return pInflWord.word.partOfSpeech == pos;
    }
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYRESOURCE:
  {
    return grounding.getResourceGroundingPtr() != nullptr;
  }
  case LinguisticCondition::FOLLOWEDBYSUBORDINATE:
  {
    return grounding.getStatementGroundingPtr() != nullptr;
  }
  case LinguisticCondition::FOLLOWEDBYUNDEFINEDREF:
  {
    const auto* genGrdPtr = grounding.getGenericGroundingPtr();
    return genGrdPtr != nullptr && genGrdPtr->word.partOfSpeech == PartOfSpeech::NOUN &&
        genGrdPtr->referenceType == SemanticReferenceType::UNDEFINED &&
        genGrdPtr->entityType == SemanticEntityType::THING;
  }
  case LinguisticCondition::FOLLOWEDBYQUANTITYEVERYTHING:
  {
    const auto* genGrdPtr = grounding.getGenericGroundingPtr();
    return genGrdPtr != nullptr && genGrdPtr->quantity.type == SemanticQuantityType::EVERYTHING;
  }
  case LinguisticCondition::FOLLOWEDBYVOWEL:
  {
    return isFirstLetterAVowel(pInflWord.word.lemma, 0);
  }
  case LinguisticCondition::FOLLOWEDBYONEOFPREFIXES:
  {
    for (const auto& param : pConditionValue.parameters)
      if (pInflWord.word.lemma.compare(0, param.size(), param) == 0)
        return true;
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYFEMININE:
  {
    return pGender == SemanticGenderType::FEMININE;
  }
  case LinguisticCondition::LOCATIONEN_FR:
  {
    if (ConceptSet::haveAnyOfConcepts(grounding.concepts, {"location_relative_high", "location_relative_low"}))
      return true;
    if (ConceptSet::haveAConceptThatBeginWith(grounding.concepts, "country_") &&
        pNumber != SemanticNumberType::PLURAL &&
        !(pGender == SemanticGenderType::MASCULINE &&
          !pInflWord.word.lemma.empty() &&
          !isFirstLetterAVowel(pInflWord.word.lemma)))
      return true;
    return false;
  }
  case LinguisticCondition::FOLLOWEDBYDATE:
  {
    if (!pOut || pOut->empty() || pOut->front().tag != WordToSynthesizeTag::DATE)
      return false;
    auto* timeGrdPtr = grounding.getTimeGroundingPtr();
    if (timeGrdPtr != nullptr)
      return _checkDate(timeGrdPtr->date, pConditionValue.parameters);
    return false;
  }
  }
  return false;
}

bool _doesPrepIsChildSpecIntroWord(const ChildSpecification& pChildSpecification,
                                   const InflectedWord* pPrepInflWordPtr)
{
  if (pPrepInflWordPtr == nullptr)
    return !pChildSpecification.introWord;
  return pChildSpecification.introWord &&
      pChildSpecification.introWord->lemma == pPrepInflWordPtr->word.lemma &&
      (pChildSpecification.introWord->partOfSpeech == PartOfSpeech::UNKNOWN ||
       pChildSpecification.introWord->partOfSpeech == pPrepInflWordPtr->word.partOfSpeech);
}

bool _doesChildSpecMatch(const ChildSpecification& pChildSpecification,
                         const InflectedWord* pPrepInflWordPtr,
                         const ConstTokenIterator* pNextToken)
{
  return _doesPrepIsChildSpecIntroWord(pChildSpecification, pPrepInflWordPtr) &&
      _areConditionSatisfied(pChildSpecification.conditionTree, pNextToken);
}


const ChildSpecification* _getChildSpecThatMatch(const std::list<ChildSpecification*>& pChildSpecifications,
                                                 const InflectedWord* pPrepInflWordPtr,
                                                 const ConstTokenIterator* pNextToken)
{
  for (const auto& currChildSpec : pChildSpecifications)
    if (_doesChildSpecMatch(*currChildSpec, pPrepInflWordPtr, pNextToken))
      return currChildSpec;
  return nullptr;
}


const ChildSpecification* _getChildSpecThatMatchFromTwoLists(const std::list<ChildSpecification*>& pChildSpecifications1,
                                                             const std::list<ChildSpecification*>& pChildSpecifications2,
                                                             const InflectedWord* pPrepInflWordPtr,
                                                             const ConstTokenIterator* pNextToken)
{
  auto it1 = pChildSpecifications1.begin();
  auto it2 = pChildSpecifications2.begin();
  while (it1 != pChildSpecifications1.end() && it2 != pChildSpecifications2.end())
  {
    if ((*it1)->templatePos < (*it2)->templatePos)
    {
      if (_doesChildSpecMatch(**it1, pPrepInflWordPtr, pNextToken))
        return *it1;
      ++it1;
      continue;
    }
    if (_doesChildSpecMatch(**it2, pPrepInflWordPtr, pNextToken))
      return *it2;
    ++it2;
  }

  while (it1 != pChildSpecifications1.end())
  {
    if (_doesChildSpecMatch(**it1, pPrepInflWordPtr, pNextToken))
      return *it1;
    ++it1;
  }
  while (it2 != pChildSpecifications2.end())
  {
    if (_doesChildSpecMatch(**it2, pPrepInflWordPtr, pNextToken))
      return *it2;
    ++it2;
  }
  return nullptr;
}


bool _ifWordWillBeChosenForSynthesisIsItTheSameLemma(
    bool& pRes,
    const ChildSpecification* pChildSpecPtr,
    const ChildSpecification* pSpecChoosenForLingAnalysisPtr,
    const ConstTokenIterator* pNextToken)
{
  if (pChildSpecPtr == pSpecChoosenForLingAnalysisPtr) // This is only for optimisation because if we are on the same child spec we already checked that the condition is true
  {
    pRes = true;
    return true;
  }
  if (_areConditionSatisfied(pChildSpecPtr->conditionTree, pNextToken))
  {
    if (pChildSpecPtr->introWord.has_value() && pSpecChoosenForLingAnalysisPtr->introWord.has_value())
      pRes = pChildSpecPtr->introWord->lemma == pSpecChoosenForLingAnalysisPtr->introWord->lemma;
    else
      pRes = pChildSpecPtr->introWord == pSpecChoosenForLingAnalysisPtr->introWord;
    return true;
  }
  return false;
}


bool _willBeAbleToSynthesizeIt(const std::list<ChildSpecification*>& pChildSpecifications,
                               const ChildSpecification* pSpecChoosenForLingAnalysisPtr,
                               const ConstTokenIterator* pNextToken)
{
  bool res = false;
  for (const auto& currChildSpec : pChildSpecifications)
    if (_ifWordWillBeChosenForSynthesisIsItTheSameLemma(res, currChildSpec, pSpecChoosenForLingAnalysisPtr, pNextToken))
      return res;
  return false;
}


bool _willBeAbleToSynthesizeItFromTwoLists(const std::list<ChildSpecification*>& pChildSpecifications1,
                                           const std::list<ChildSpecification*>& pChildSpecifications2,
                                           const ChildSpecification* pSpecChoosenForLingAnalysisPtr,
                                           const ConstTokenIterator* pNextToken)
{
  bool res = false;
  auto it1 = pChildSpecifications1.begin();
  auto it2 = pChildSpecifications2.begin();
  while (it1 != pChildSpecifications1.end() && it2 != pChildSpecifications2.end())
  {
    if ((*it1)->templatePos < (*it2)->templatePos)
    {
      if (_ifWordWillBeChosenForSynthesisIsItTheSameLemma(res, *it1, pSpecChoosenForLingAnalysisPtr, pNextToken))
        return res;
      ++it1;
      continue;
    }
    if (_ifWordWillBeChosenForSynthesisIsItTheSameLemma(res, *it2, pSpecChoosenForLingAnalysisPtr, pNextToken))
      return res;
    ++it2;
  }

  while (it1 != pChildSpecifications1.end())
  {
    if (_ifWordWillBeChosenForSynthesisIsItTheSameLemma(res, *it1, pSpecChoosenForLingAnalysisPtr, pNextToken))
      return res;
    ++it1;
  }
  while (it2 != pChildSpecifications2.end())
  {
    if (_ifWordWillBeChosenForSynthesisIsItTheSameLemma(res, *it2, pSpecChoosenForLingAnalysisPtr, pNextToken))
      return res;
    ++it2;
  }
  return false;
}


const ChildSpecification* _getChildSpecFromContainers(const ChildSpecificationsContainer& pDefaultChilds,
                                                      const ChildSpecificationsContainer* pVerbChildsPtr,
                                                      const InflectedWord* pPrepInflWordPtr,
                                                      const ConstTokenIterator* pNextToken)
{
  std::string prepLemma;
  if (pPrepInflWordPtr != nullptr)
    prepLemma = pPrepInflWordPtr->word.lemma;
  auto itLemmaToSpecs = pDefaultChilds.lemmaToChildSpecs.find(prepLemma);
  if (itLemmaToSpecs != pDefaultChilds.lemmaToChildSpecs.end())
  {
    if (pVerbChildsPtr != nullptr)
    {
      auto itVerbLemmaToSpecs = pVerbChildsPtr->lemmaToChildSpecs.find(prepLemma);
      if (itVerbLemmaToSpecs != pVerbChildsPtr->lemmaToChildSpecs.end())
        return _getChildSpecThatMatchFromTwoLists(itLemmaToSpecs->second, itVerbLemmaToSpecs->second,
                                                  pPrepInflWordPtr, pNextToken);
    }
    return _getChildSpecThatMatch(itLemmaToSpecs->second, pPrepInflWordPtr, pNextToken);
  }
  else if (pVerbChildsPtr != nullptr)
  {
    auto itVerbLemmaToSpecs = pVerbChildsPtr->lemmaToChildSpecs.find(prepLemma);
    if (itVerbLemmaToSpecs != pVerbChildsPtr->lemmaToChildSpecs.end())
      return _getChildSpecThatMatch(itVerbLemmaToSpecs->second, pPrepInflWordPtr, pNextToken);
  }
  return nullptr;
}



mystd::optional<ChunkLinkType> _getChunkLinkFromSchildsSpecs(bool& pWillBeAbleToSynthesizeIt,
                                                             std::map<std::string, char>& pVerbConcepts,
                                                             const ChildSpecificationsContainer& pDefaultChilds,
                                                             const ChildSpecificationsContainer* pVerbChildsPtr,
                                                             InflectedWord* pPrepInflWordPtr,
                                                             const ConstTokenIterator* pNextToken)
{
  const ChildSpecification* childSpecThatMatchPtr = _getChildSpecFromContainers(pDefaultChilds, pVerbChildsPtr, pPrepInflWordPtr, pNextToken);
  if (childSpecThatMatchPtr != nullptr)
  {
    for (const auto& currConc : childSpecThatMatchPtr->verbConceptsToRemove)
      pVerbConcepts.erase(currConc);
    if (pPrepInflWordPtr != nullptr)
    {
      for (const auto& currConc : childSpecThatMatchPtr->conceptsToAdd)
        pPrepInflWordPtr->infos.concepts.emplace(currConc, 4);
    }
    auto itLemmaToSpecs = pDefaultChilds.chkLinkToChildSpecs.find(childSpecThatMatchPtr->chunkLinkType);
    if (itLemmaToSpecs != pDefaultChilds.chkLinkToChildSpecs.end())
    {
      if (pVerbChildsPtr != nullptr)
      {
        auto itVerbLemmaToSpecs = pVerbChildsPtr->chkLinkToChildSpecs.find(childSpecThatMatchPtr->chunkLinkType);
        if (itVerbLemmaToSpecs != pVerbChildsPtr->chkLinkToChildSpecs.end())
        {
          pWillBeAbleToSynthesizeIt = _willBeAbleToSynthesizeItFromTwoLists(itLemmaToSpecs->second, itVerbLemmaToSpecs->second,
                                                                            childSpecThatMatchPtr, pNextToken);
          return childSpecThatMatchPtr->chunkLinkType;
        }
      }
      pWillBeAbleToSynthesizeIt = _willBeAbleToSynthesizeIt(itLemmaToSpecs->second, childSpecThatMatchPtr, pNextToken);
      return childSpecThatMatchPtr->chunkLinkType;
    }
    else if (pVerbChildsPtr != nullptr)
    {
      auto itVerbLemmaToSpecs = pVerbChildsPtr->chkLinkToChildSpecs.find(childSpecThatMatchPtr->chunkLinkType);
      if (itVerbLemmaToSpecs != pVerbChildsPtr->chkLinkToChildSpecs.end())
      {
        pWillBeAbleToSynthesizeIt = _willBeAbleToSynthesizeIt(itVerbLemmaToSpecs->second, childSpecThatMatchPtr, pNextToken);
        return childSpecThatMatchPtr->chunkLinkType;
      }
    }
    pWillBeAbleToSynthesizeIt = false;
    return childSpecThatMatchPtr->chunkLinkType;
  }
  pWillBeAbleToSynthesizeIt = true;
  return mystd::optional<ChunkLinkType>();
}


bool _getIntroWordCheckConditions(const std::shared_ptr<LinguisticConditionTree>& pConditionTree,
                                  const GroundedExpression* pObjectGrdExpPtr,
                                  const std::list<WordToSynthesize>* pOut,
                                  SemanticNumberType pNumber,
                                  SemanticGenderType pGender,
                                  const linguistics::InflectedWord& pInflWord)
{
  if (!pConditionTree)
    return true;
  switch (pConditionTree->type)
  {
  case LinguisticConditionTreeType::VALUE:
  {
    if (pObjectGrdExpPtr != nullptr)
      return _doesConditionMatchWithGrounding(pConditionTree->getValue(), *pObjectGrdExpPtr,
                                              pOut, pNumber, pGender, pInflWord);
    return false;
  }
  case LinguisticConditionTreeType::OPERAND:
  {
    const auto& operandStruct = pConditionTree->getOperand();
    switch (operandStruct.operand)
    {
    case LinguisticConditionTreeOperandEnum::AND:
    {
      for (const auto& currChild : operandStruct.children)
        if (!_getIntroWordCheckConditions(currChild, pObjectGrdExpPtr, pOut, pNumber, pGender, pInflWord))
          return false;
      return true;
    }
    case LinguisticConditionTreeOperandEnum::NOT:
    {
      if (!operandStruct.children.empty())
        return !_getIntroWordCheckConditions(operandStruct.children.front(),
                                             pObjectGrdExpPtr, pOut, pNumber, pGender, pInflWord);
      break;
    }
    case LinguisticConditionTreeOperandEnum::OR:
    {
      for (const auto& currChild : operandStruct.children)
        if (_getIntroWordCheckConditions(currChild, pObjectGrdExpPtr, pOut, pNumber, pGender, pInflWord))
          return true;
      return false;
    }
    }
    break;
  }
  }
  return false;
}


bool _getIntroWordFromSpec(mystd::optional<SemanticWord>& pIntroWord,
                           const ChildSpecification& pChildSpec,
                           const std::map<std::string, char>& pVerbConcepts,
                           const GroundedExpression* pObjectGrdExpPtr,
                           const std::list<WordToSynthesize>* pOut,
                           SemanticNumberType pNumber,
                           SemanticGenderType pGender,
                           const linguistics::InflectedWord& pInflWord)
{
  bool conditionAreOk = _getIntroWordCheckConditions(pChildSpec.conditionTree, pObjectGrdExpPtr,
                                                     pOut, pNumber, pGender, pInflWord);
  if (conditionAreOk)
  {
    for (const auto& currCopncept : pChildSpec.verbConceptsToRemove)
    {
      if (ConceptSet::haveAConcept(pVerbConcepts, currCopncept))
      {
        conditionAreOk = false;
        break;
      }
    }
    if (conditionAreOk)
    {
      pIntroWord = pChildSpec.introWord;
      return true;
    }
  }
  return false;
}


bool _getIntroWordFromTwoLists(mystd::optional<SemanticWord>& pIntroWord,
                               const std::list<ChildSpecification*>& pChildSpecifications1,
                               const std::list<ChildSpecification*>& pChildSpecifications2,
                               const std::map<std::string, char>& pVerbConcepts,
                               const GroundedExpression* pObjectGrdExpPtr,
                               const std::list<WordToSynthesize>* pOut,
                               SemanticNumberType pNumber,
                               SemanticGenderType pGender,
                               const linguistics::InflectedWord& pInflWord)
{
  auto it1 = pChildSpecifications1.begin();
  auto it2 = pChildSpecifications2.begin();
  while (it1 != pChildSpecifications1.end() && it2 != pChildSpecifications2.end())
  {
    if ((*it1)->templatePos < (*it2)->templatePos)
    {
      if (_getIntroWordFromSpec(pIntroWord, **it1, pVerbConcepts, pObjectGrdExpPtr,
                                pOut, pNumber, pGender, pInflWord))
        return true;
      ++it1;
      continue;
    }
    if (_getIntroWordFromSpec(pIntroWord, **it2, pVerbConcepts, pObjectGrdExpPtr,
                              pOut, pNumber, pGender, pInflWord))
      return true;
    ++it2;
  }

  while (it1 != pChildSpecifications1.end())
  {
    if (_getIntroWordFromSpec(pIntroWord, **it1, pVerbConcepts, pObjectGrdExpPtr,
                              pOut, pNumber, pGender, pInflWord))
      return true;
    ++it1;
  }
  while (it2 != pChildSpecifications2.end())
  {
    if (_getIntroWordFromSpec(pIntroWord, **it2, pVerbConcepts, pObjectGrdExpPtr,
                              pOut, pNumber, pGender, pInflWord))
      return true;
    ++it2;
  }
  return false;
}


bool _getIntroWordFromOneList(mystd::optional<SemanticWord>& pIntroWord,
                              const std::list<ChildSpecification*>& pChildSpecifications,
                              const std::map<std::string, char>& pVerbConcepts,
                              const GroundedExpression* pObjectGrdExpPtr,
                              const std::list<WordToSynthesize>* pOut,
                              SemanticNumberType pNumber,
                              SemanticGenderType pGender,
                              const linguistics::InflectedWord& pInflWord)
{
  for (const auto& currChildSpec : pChildSpecifications)
    if (_getIntroWordFromSpec(pIntroWord, *currChildSpec, pVerbConcepts, pObjectGrdExpPtr,
                              pOut, pNumber, pGender, pInflWord))
      return true;
  return false;
}


bool _getIntroWord(mystd::optional<SemanticWord>& pIntroWord,
                   ChunkLinkType pChunkLinkType,
                   const std::map<std::string, char>& pVerbConcepts,
                   const ChildSpecificationsContainer& pDefaultChilds,
                   const ChildSpecificationsContainer* pVerbChildsPtr,
                   const GroundedExpression* pObjectGrdExpPtr,
                   const std::list<WordToSynthesize>* pOut,
                   SemanticNumberType pNumber,
                   SemanticGenderType pGender,
                   const linguistics::InflectedWord& pInflWord)
{
  auto itLemmaToSpecs = pDefaultChilds.chkLinkToChildSpecs.find(pChunkLinkType);
  if (itLemmaToSpecs != pDefaultChilds.chkLinkToChildSpecs.end())
  {
    if (pVerbChildsPtr != nullptr)
    {
      auto itVerbLemmaToSpecs = pVerbChildsPtr->chkLinkToChildSpecs.find(pChunkLinkType);
      if (itVerbLemmaToSpecs != pVerbChildsPtr->chkLinkToChildSpecs.end())
        return _getIntroWordFromTwoLists(pIntroWord, itLemmaToSpecs->second, itVerbLemmaToSpecs->second,
                                         pVerbConcepts, pObjectGrdExpPtr, pOut, pNumber, pGender,
                                         pInflWord);
      return _getIntroWordFromOneList(pIntroWord, itLemmaToSpecs->second, pVerbConcepts,
                                      pObjectGrdExpPtr, pOut, pNumber, pGender, pInflWord);
    }
    return _getIntroWordFromOneList(pIntroWord, itLemmaToSpecs->second, pVerbConcepts,
                                    pObjectGrdExpPtr, pOut, pNumber, pGender, pInflWord);
  }
  else if (pVerbChildsPtr != nullptr)
  {
    auto itVerbLemmaToSpecs = pVerbChildsPtr->chkLinkToChildSpecs.find(pChunkLinkType);
    if (itVerbLemmaToSpecs != pVerbChildsPtr->chkLinkToChildSpecs.end())
      return _getIntroWordFromOneList(pIntroWord, itVerbLemmaToSpecs->second, pVerbConcepts,
                                      pObjectGrdExpPtr, pOut, pNumber, pGender, pInflWord);
  }
  return false;
}

}


void ChildSpecificationsContainer::addChildSpec(ChildSpecification&& pChildSpec)
{
  listOfSpecifications.emplace_back(std::move(pChildSpec));
  auto& childSpec = listOfSpecifications.back();
  auto& lemmaTochildsSpec = lemmaToChildSpecs[childSpec.introWord ? childSpec.introWord->lemma : ""];
  bool childSpecInserted = false;
  for (auto it = lemmaTochildsSpec.begin(); it != lemmaTochildsSpec.end(); ++it)
  {
    if (pChildSpec.templatePos < (*it)->templatePos)
    {
      lemmaTochildsSpec.insert(it, &childSpec);
      childSpecInserted = true;
      break;
    }
  }
  if (!childSpecInserted)
    lemmaTochildsSpec.emplace_back(&childSpec);

  auto& childSpecs = chkLinkToChildSpecs[childSpec.chunkLinkType];
  for (auto it = childSpecs.begin(); it != childSpecs.end(); ++it)
  {
    if (childSpec.templatePos < (*it)->templatePos)
    {
      childSpecs.insert(it, &childSpec);
      return;
    }
  }
  childSpecs.emplace_back(&childSpec);
}




SemanticFrameDictionary::SemanticFrameDictionary()
  : _lastTemplatePos(0),
    _bookmarkToTemplatePos(),
    _childSpecificationsByDefault(std::make_unique<ChildSpecificationsContainer>()),
    _childSpecificationsWithoutVerbByDefault(std::make_unique<ChildSpecificationsContainer>()),
    _templateNameToChildSpecifications(),
    _wordToChildSpecifications()
{
}


SemanticFrameDictionary::~SemanticFrameDictionary()
{
}


int& SemanticFrameDictionary::getLastTemplatePos()
{
  return _lastTemplatePos;
}

std::map<std::string, int>& SemanticFrameDictionary::getBookmarkToTemplatePos()
{
  return _bookmarkToTemplatePos;
}



void SemanticFrameDictionary::addTemplateNameToChildSpecifications(const std::string& pTemplateName,
                                                                   ChildSpecification&& pChildSpec)
{
  auto& childSpecOnt = _templateNameToChildSpecifications[pTemplateName];
  if (!childSpecOnt)
    childSpecOnt = std::make_shared<ChildSpecificationsContainer>();
  childSpecOnt->templateNameOpt = pTemplateName;
  childSpecOnt->addChildSpec(std::move(pChildSpec));
}


void SemanticFrameDictionary::addWordToChildSpecifications(const SemanticWord& pWord,
                                                           ChildSpecification&& pChildSpec)
{
  auto it = _wordToChildSpecifications.find(pWord);
  if (it == _wordToChildSpecifications.end())
  {
    auto& childSpecOnt = _wordToChildSpecifications[pWord];
    if (!childSpecOnt)
      childSpecOnt = std::make_shared<ChildSpecificationsContainer>();
    childSpecOnt->addChildSpec(std::move(pChildSpec));
    return;
  }
  if (it->second->templateNameOpt)
  {
    it->second = std::make_shared<ChildSpecificationsContainer>(*it->second);
    it->second->templateNameOpt.reset();
  }
  it->second->addChildSpec(std::move(pChildSpec));
}


void SemanticFrameDictionary::addWordToTemplate(const SemanticWord& pWord,
                                                const std::string& pTemplateName)
{
  auto itTemplate = _templateNameToChildSpecifications.find(pTemplateName);
  if (itTemplate == _templateNameToChildSpecifications.end())
  {
    assert(false);
    return;
  }

  auto it = _wordToChildSpecifications.find(pWord);
  if (it == _wordToChildSpecifications.end())
  {
    _wordToChildSpecifications[pWord] = itTemplate->second;
    return;
  }

  if (it->second->templateNameOpt)
  {
    it->second = std::make_shared<ChildSpecificationsContainer>(*it->second);
    it->second->templateNameOpt.reset();
  }
  for (auto currElt : itTemplate->second->listOfSpecifications)
    it->second->addChildSpec(std::move(currElt));
}


void SemanticFrameDictionary::addAChildSpecificationsByDefault(ChildSpecification&& pChildSepcs)
{
  _childSpecificationsByDefault->addChildSpec(std::move(pChildSepcs));
}

void SemanticFrameDictionary::addAChildSpecificationsWithoutVerbByDefault(ChildSpecification&& pChildSepcs)
{
  _childSpecificationsWithoutVerbByDefault->addChildSpec(std::move(pChildSepcs));
}



mystd::optional<ChunkLinkType> SemanticFrameDictionary::getChunkLinkFromContext
(InflectedWord* pVerbInflectedWordPtr,
 bool& pWillBeAbleToSynthesizeIt,
 InflectedWord* pPrepInflWordPtr,
 const ConstTokenIterator* pNextToken) const
{
  if (pVerbInflectedWordPtr != nullptr)
  {
    const ChildSpecificationsContainer* verbChildsPtr = nullptr;
    auto itWordChildSpec = _wordToChildSpecifications.find(pVerbInflectedWordPtr->word);
    if (itWordChildSpec != _wordToChildSpecifications.end())
      verbChildsPtr = &*itWordChildSpec->second;
    return _getChunkLinkFromSchildsSpecs(pWillBeAbleToSynthesizeIt, pVerbInflectedWordPtr->infos.concepts, *_childSpecificationsByDefault,
                                         verbChildsPtr, pPrepInflWordPtr, pNextToken);
  }

  const auto* childSpecThatMatchPtr = _getChildSpecFromContainers(*_childSpecificationsWithoutVerbByDefault, nullptr, pPrepInflWordPtr, pNextToken);
  if (childSpecThatMatchPtr != nullptr)
    return childSpecThatMatchPtr->chunkLinkType;
  return mystd::optional<ChunkLinkType>();
}


bool SemanticFrameDictionary::getIntroWord(mystd::optional<SemanticWord>& pIntroWord,
                                           ChunkLinkType pChunkLinkType,
                                           const std::map<std::string, char>& pVerbConcepts,
                                           const InflectedWord& pInflectedWord,
                                           const GroundedExpression* pObjectGrdExpPtr,
                                           const std::list<WordToSynthesize>* pOut,
                                           SemanticNumberType pNumber,
                                           SemanticGenderType pGender,
                                           const linguistics::InflectedWord& pInflWord) const
{
  const ChildSpecificationsContainer* verbChildsPtr = nullptr;
  auto itWordChildSpec = _wordToChildSpecifications.find(pInflectedWord.word);
  if (itWordChildSpec != _wordToChildSpecifications.end())
      verbChildsPtr = &*itWordChildSpec->second;
  return _getIntroWord(pIntroWord, pChunkLinkType, pVerbConcepts, *_childSpecificationsByDefault, verbChildsPtr,
                       pObjectGrdExpPtr, pOut, pNumber, pGender, pInflWord);
}


bool SemanticFrameDictionary::getIntroWordWithoutVerb(mystd::optional<SemanticWord>& pIntroWord,
                                                      ChunkLinkType pChunkLinkType,
                                                      const GroundedExpression* pObjectGrdExpPtr,
                                                      const std::list<WordToSynthesize>* pOut,
                                                      SemanticNumberType pNumber,
                                                      SemanticGenderType pGender,
                                                      const linguistics::InflectedWord& pInflWord) const
{
  auto itLemmaToSpecs = _childSpecificationsWithoutVerbByDefault->chkLinkToChildSpecs.find(pChunkLinkType);
  if (itLemmaToSpecs != _childSpecificationsWithoutVerbByDefault->chkLinkToChildSpecs.end())
  {
    for (const auto& currChildSpec : itLemmaToSpecs->second)
    {
      if (_getIntroWordCheckConditions(currChildSpec->conditionTree, pObjectGrdExpPtr,
                                       pOut, pNumber, pGender, pInflWord))
      {
        pIntroWord = currChildSpec->introWord;
        return true;
      }
    }
  }
  return false;
}


bool SemanticFrameDictionary::getIntroWordWithoutConditions(mystd::optional<SemanticWord>& pIntroWord,
                                                            ChunkLinkType pChunkLinkType) const
{
  auto it = _childSpecificationsWithoutVerbByDefault->chkLinkToChildSpecs.find(pChunkLinkType);
  if (it != _childSpecificationsWithoutVerbByDefault->chkLinkToChildSpecs.end() && !it->second.empty())
  {
    pIntroWord = it->second.front()->introWord;
    return true;
  }
  return false;
}


bool SemanticFrameDictionary::doesIntroductionWordHasChunkLinkType(const SemanticWord& pIntroductionWord,
                                                                   ChunkLinkType pChunkLinkType) const
{
  auto it = _childSpecificationsWithoutVerbByDefault->lemmaToChildSpecs.find(pIntroductionWord.lemma);
  if (it != _childSpecificationsWithoutVerbByDefault->lemmaToChildSpecs.end())
    for (const auto& currChildSpec : it->second)
      if (currChildSpec->introWord &&
          *currChildSpec->introWord == pIntroductionWord &&
          currChildSpec->chunkLinkType == pChunkLinkType)
        return true;
   return false;
}


mystd::optional<ChunkLinkType> SemanticFrameDictionary::introductionWordToChunkLinkType(const SemanticWord& pIntroductionWord) const
{
  auto it = _childSpecificationsWithoutVerbByDefault->lemmaToChildSpecs.find(pIntroductionWord.lemma);
  if (it != _childSpecificationsWithoutVerbByDefault->lemmaToChildSpecs.end())
    for (const auto& currChildSpec : it->second)
      if (currChildSpec->introWord &&
          *currChildSpec->introWord == pIntroductionWord)
        return currChildSpec->chunkLinkType;
  return mystd::optional<ChunkLinkType>();
}


} // End of namespace linguistics
} // End of namespace onsem


