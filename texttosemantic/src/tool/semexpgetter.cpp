#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/translationdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/inflection/pronominalinflections.hpp>

namespace onsem
{

namespace SemExpGetter
{

namespace
{

void _getAgentOrNameGrd
(const SemanticAgentGrounding** pAgentGrd,
 const SemanticNameGrounding** pNameGrd,
 const GroundedExpression& pGrdExp)
{
  *pAgentGrd = pGrdExp->getAgentGroundingPtr();
  if (*pAgentGrd == nullptr)
    *pNameGrd = pGrdExp->getNameGroundingPtr();
}

SemanticEntityType _getEntityGrdExp(const GroundedExpression& pGrdExp)
{
  const auto* genGrdPtr = pGrdExp->getGenericGroundingPtr();
  if (genGrdPtr != nullptr)
    return genGrdPtr->entityType;
  if (pGrdExp->getNameGroundingPtr() != nullptr)
    return SemanticEntityType::AGENTORTHING;
  if (pGrdExp->getAgentGroundingPtr() != nullptr)
    return SemanticEntityType::HUMAN;
  return SemanticEntityType::UNKNOWN;
}

template<typename USEMEXP, typename GRDEXP>
USEMEXP* _getCoreferenceFromGrdExp(GRDEXP& pGrdExp,
                                   CoreferenceDirectionEnum pCoreferenceDirection)
{
  for (auto& currChild : pGrdExp.children)
  {
    const GroundedExpression* grdExpPtr = currChild.second->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr &&
        (grdExpPtr->children.empty() ||
         (grdExpPtr->children.size() == 1 && grdExpPtr->children.begin()->first == GrammaticalType::INTRODUCTING_WORD)))
    {
      const SemanticGenericGrounding* genGrdPtr = grdExpPtr->grounding().getGenericGroundingPtr();
      if (genGrdPtr != nullptr &&
          genGrdPtr->coreference &&
          genGrdPtr->coreference->getDirection() == pCoreferenceDirection)
        return &currChild.second;
    }
  }
  return nullptr;
}


const SemanticGenericGrounding* _getGenericGrdPtr(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return grdExpPtr->grounding().getGenericGroundingPtr();
  return nullptr;
}

bool _isACoreferenceFromGrdExp(const GroundedExpression& pGrdExp,
                               CoreferenceDirectionEnum pDirection,
                               bool pShouldNotHaveALemma)
{
  const SemanticGenericGrounding* genGrdPtr = pGrdExp->getGenericGroundingPtr();
  return genGrdPtr != nullptr && isACoreferenceFromGenericGrounding(*genGrdPtr, pDirection, pShouldNotHaveALemma);
}


bool _isMandatoryGrdExp(const GroundedExpression& pGrdExp)
{
  const auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  return statGrdPtr != nullptr &&
      (statGrdPtr->isMandatoryInPresentTense() ||
       ConceptSet::haveAConcept(statGrdPtr->concepts, "verb_haveto"));
}

std::unique_ptr<SemanticGrounding> _extractQuantityFromGrdExp(const GroundedExpression& pGrdExp,
                                                              const SemanticUnityGrounding* pUnityGrdPtr)
{
  mystd::optional<int> res;
  const auto& grd = *pGrdExp;
  if (grd.type == SemanticGroundingType::GENERIC)
  {
    if (pUnityGrdPtr == nullptr)
    {
      const SemanticQuantity& repetitionQuantity = grd.getGenericGrounding().quantity;
      if (repetitionQuantity.type == SemanticQuantityType::NUMBER)
      {
        auto genGrd = std::make_unique<SemanticGenericGrounding>();
        genGrd->quantity.setNumber(repetitionQuantity.nb);
        genGrd->entityType = SemanticEntityType::THING;
        return genGrd;
      }
    }
  }
  else if (grd.type == SemanticGroundingType::ANGLE)
  {
    return pGrdExp.cloneGrounding();
  }
  else if (grd.type == SemanticGroundingType::LENGTH)
  {
    if (pUnityGrdPtr == nullptr)
      return pGrdExp.cloneGrounding();
    if (pUnityGrdPtr->typeOfUnity == TypeOfUnity::LENGTH)
    {
      auto res = pGrdExp.cloneGrounding();
      res->getLengthGrounding().length.convertToUnity(pUnityGrdPtr->getLengthUnity());
      return res;
    }
  }

  if (pUnityGrdPtr == nullptr)
  {
    auto genGrd = std::make_unique<SemanticGenericGrounding>();
    genGrd->quantity.setNumber(1);
    genGrd->entityType = SemanticEntityType::THING;
    return genGrd;
  }
  return {};
}


void _extractAskedChildrenFromGrdExp(
    std::set<GrammaticalType>& pAskedChildren,
    const GroundedExpression& pQuestionGrdExp)
{
  const auto* statGrdPtr = pQuestionGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
  {
    for (const auto& currRequest : statGrdPtr->requests.types)
    {
      auto grammTypes = requestToGrammaticalTypes(currRequest);
      for (const auto& currGramm : grammTypes)
        pAskedChildren.insert(currGramm);
    }
  }

  for (const auto& currChild : pQuestionGrdExp.children)
    extractAskedChildren(pAskedChildren, *currChild.second);
}


void _extractAskedChildrenByAResourceFromGrdExp(
    std::set<GrammaticalType>& pAskedChildren,
    const GroundedExpression& pGrdExp)
{
  const auto* resGrdPtr = pGrdExp->getResourceGroundingPtr();
  if (resGrdPtr != nullptr)
    for (const auto& currQuestions : resGrdPtr->resource.parameterLabelsToQuestions)
      for (const auto& currQuestion : currQuestions.second)
        extractAskedChildren(pAskedChildren, *currQuestion);

  for (const auto& currChild : pGrdExp.children)
    extractAskedChildrenByAResource(pAskedChildren, *currChild.second);
}

}


bool isGrdReflexive(const SemanticGrounding& pGrd)
{
  const auto* statGrdPtr = pGrd.getStatementGroundingPtr();
  return statGrdPtr != nullptr && statGrdPtr->word.isReflexive();
}

SemanticEntityType getEntity(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return _getEntityGrdExp(*grdExpPtr);

  const ListExpression* listExpPtr = pSemExp.getListExpPtr();
  if (listExpPtr != nullptr &&
      listExpPtr->elts.size() >= 1)
    return getEntity(*listExpPtr->elts.front());
  return SemanticEntityType::UNKNOWN;
}

VerbGoalEnum getGoal(const GroundedExpression& pGrdExp)
{
  const auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
    return statGrdPtr->verbGoal;
  return VerbGoalEnum::NOTIFICATION;
}


bool isPassive(const GroundedExpression& pGrdExp)
{
  const auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  return statGrdPtr != nullptr &&
      !statGrdPtr->requests.has(SemanticRequestType::SUBJECT) &&
      pGrdExp.children.count(GrammaticalType::SUBJECT) == 0;
}

SemanticVerbTense getVerbTense(const GroundedExpression& pGrdExp)
{
  const auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
    return statGrdPtr->verbTense;
  return SemanticVerbTense::UNKNOWN;
}

mystd::optional<bool> isUncountableFromGrd(const SemanticGenericGrounding& pGrounding,
                                           const linguistics::LinguisticDatabase& pLingDb)
{
  mystd::optional<bool> res;
  if (pGrounding.quantity.isPlural())
  {
    res.emplace(false);
    return res;
  }
  if (ConceptSet::haveAConceptIncompatibleWithSomethingCountable(pGrounding.concepts))
  {
    res.emplace(true);
    return res;
  }
  LinguisticMeaning lingMeaning;
  SemExpGetter::wordToAMeaning(lingMeaning, pGrounding.word, SemanticLanguageEnum::ENGLISH, pLingDb);
  res.emplace(pLingDb.hasContextualInfo(WordContextualInfos::UNCOUNTABLE, lingMeaning));
  return res;
}


mystd::optional<bool> isUncountableSemExp(const SemanticExpression& pSemExp,
                                           const linguistics::LinguisticDatabase& pLingDb)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    auto* genGrdPtr = grdExpPtr->grounding().getGenericGroundingPtr();
    if (genGrdPtr != nullptr)
      return isUncountableFromGrd(*genGrdPtr, pLingDb);
  }
  return mystd::optional<bool>();
}


SemanticNumberType getNumber(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExp = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp != nullptr)
    return getNumberFromGrd(**grdExp);

  const ListExpression* listExp = pSemExp.getListExpPtr();
  if (listExp != nullptr)
    return SemanticNumberType::PLURAL;
  return SemanticNumberType::UNKNOWN;
}


PartOfSpeech getMainPartOfSpeech(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const SemanticGenericGrounding* genGrdPtr = grdExpPtr->grounding().getGenericGroundingPtr();
    if (genGrdPtr != nullptr)
      return genGrdPtr->word.partOfSpeech;
    if (grdExpPtr->grounding().type == SemanticGroundingType::NAME)
      return PartOfSpeech::PROPER_NOUN;
  }
  return PartOfSpeech::UNKNOWN;
}


SemanticNumberType getNumberFromGrd(const SemanticGrounding& pGrounding)
{
  const SemanticGenericGrounding* genGrd = pGrounding.getGenericGroundingPtr();
  if (genGrd != nullptr && genGrd->quantity.isPlural())
    return SemanticNumberType::PLURAL;
  return SemanticNumberType::UNKNOWN;
}


SemanticReferenceType getReferenceTypeFromGrd(const SemanticGrounding& pGrd)
{
  const SemanticGenericGrounding* genGrd = pGrd.getGenericGroundingPtr();
  if (genGrd != nullptr)
    return genGrd->referenceType;
  auto* agentGrdPtr = pGrd.getAgentGroundingPtr();
  if (agentGrdPtr != nullptr)
  {
    if (agentGrdPtr->isSpecificUser())
      return SemanticReferenceType::DEFINITE;
    return SemanticReferenceType::UNDEFINED;
  }
  if (pGrd.getResourceGroundingPtr() != nullptr)
    return SemanticReferenceType::DEFINITE;
  return SemanticReferenceType::UNDEFINED;
}


template <typename INFLECTIONS>
SemanticNumberType _getNumberFromNominalOrPronominalInflections
(const INFLECTIONS& pInflections)
{
  std::set<SemanticNumberType> numbers;
  for (const auto& currInflection : pInflections.inflections)
    numbers.insert(currInflection.number);
  if (numbers.size() == 1 &&
      *numbers.begin() == SemanticNumberType::PLURAL)
    return SemanticNumberType::PLURAL;
  return SemanticNumberType::SINGULAR;
}


SemanticNumberType getNumberFromInflections
(const linguistics::InflectedWord& pInflections)
{
  const Inflections& infls = pInflections.inflections();
  switch (infls.type)
  {
  case InflectionType::NOMINAL:
    return _getNumberFromNominalOrPronominalInflections(infls.getNominalI());
  case InflectionType::PRONOMINAL:
    return _getNumberFromNominalOrPronominalInflections(infls.getPronominalI());
  default:
    return SemanticNumberType::UNKNOWN;
  }
}


SemanticGenderType possibleGendersToGender(const std::set<SemanticGenderType>& pPossibleGenders)
{
  SemanticGenderType res = SemanticGenderType::UNKNOWN;
  for (const auto& currGender : pPossibleGenders)
  {
    if (currGender == SemanticGenderType::UNKNOWN)
      continue;
    if (res != SemanticGenderType::UNKNOWN && res != currGender)
      return SemanticGenderType::UNKNOWN;
    res = currGender;
  }
  return res;
}


SemanticGenderType getGenderFromGenGrd(const SemanticGenericGrounding& pGenGrd)
{
  SemanticGenderType res = possibleGendersToGender(pGenGrd.possibleGenders);
  if (res == SemanticGenderType::UNKNOWN)
  {
    if (pGenGrd.concepts.find("gender_male") != pGenGrd.concepts.end())
      return SemanticGenderType::MASCULINE;
    if (pGenGrd.concepts.find("gender_female") != pGenGrd.concepts.end())
      return SemanticGenderType::FEMININE;
  }
  return res;
}


SemanticLanguageEnum getLanguage(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations)
{
  auto languageChild = pAnnotations.find(GrammaticalType::LANGUAGE);
  if (languageChild != pAnnotations.end())
  {
    const GroundedExpression* langGrdExpPtr = languageChild->second->getGrdExpPtr_SkipWrapperPtrs();
    if (langGrdExpPtr != nullptr &&
        langGrdExpPtr->grounding().type == SemanticGroundingType::LANGUAGE)
      return langGrdExpPtr->grounding().getLanguageGrounding().language;
  }
  return SemanticLanguageEnum::UNKNOWN;
}


mystd::optional<SemanticFloat> getNumberOfElementsFromGrdExp(const GroundedExpression& pGrdExp)
{
  mystd::optional<SemanticFloat> res;
  if (pGrdExp->type == SemanticGroundingType::GENERIC)
  {
    const SemanticQuantity& repetitionQuantity = pGrdExp->getGenericGrounding().quantity;
    if (repetitionQuantity.type == SemanticQuantityType::NUMBER)
      res.emplace(repetitionQuantity.nb);
    return res;
  }
  res.emplace(1);
  return res;
}

mystd::optional<SemanticFloat> getNumberOfElements(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return getNumberOfElementsFromGrdExp(*grdExpPtr);
  return mystd::optional<SemanticFloat>();
}


std::unique_ptr<SemanticGrounding> extractQuantity(const SemanticExpression& pSemExp,
                                                   const SemanticUnityGrounding* pUnityGrdPtr)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return _extractQuantityFromGrdExp(*grdExpPtr, pUnityGrdPtr);
  return {};
}

std::unique_ptr<SemanticGrounding> mergeQuantities(const SemanticGrounding& pPreviousQuantity,
                                                   std::unique_ptr<SemanticGrounding> pNewQuantity)
{
  if (pNewQuantity)
  {
    if (pPreviousQuantity.type == SemanticGroundingType::GENERIC &&
        pNewQuantity->type == SemanticGroundingType::GENERIC)
    {
      auto& prevQuantity = pPreviousQuantity.getGenericGrounding().quantity;
      auto& newQuantity = pNewQuantity->getGenericGrounding().quantity;
      if (prevQuantity.type == SemanticQuantityType::NUMBER &&
          newQuantity.type == SemanticQuantityType::NUMBER)
        newQuantity.setNumber(newQuantity.nb + prevQuantity.nb);
    }
  }
  return pNewQuantity;
}

int getNumberOfRepetitions(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations)
{
  auto itRepetitionChild = pAnnotations.find(GrammaticalType::REPETITION);
  if (itRepetitionChild != pAnnotations.end())
  {
    auto res = getNumberOfElements(*itRepetitionChild->second);
    if (res)
      return res->valueN;
  }
  return 1;
}


const SemanticExpression* getUntilChild(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations)
{
  auto itDurationChild = pAnnotations.find(GrammaticalType::DURATION);
  if (itDurationChild != pAnnotations.end())
  {
    const GroundedExpression* durationGrdExpPtr =
        itDurationChild->second->getGrdExpPtr_SkipWrapperPtrs();
    if (durationGrdExpPtr != nullptr)
    {
      const GroundedExpression& durationGrdExp = *durationGrdExpPtr;
      if (durationGrdExp->getRelDurationGroundingPtr() != nullptr &&
          durationGrdExp->getRelDurationGroundingPtr()->durationType == SemanticRelativeDurationType::UNTIL)
      {
        auto itSpecifier = durationGrdExp.children.find(GrammaticalType::SPECIFIER);
        if (itSpecifier != durationGrdExp.children.end())
          return &*itSpecifier->second;
      }
    }
  }
  return nullptr;
}

mystd::optional<int64_t> getTimeDurationInMilliseconds(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations)
{
  auto itDurationChild = pAnnotations.find(GrammaticalType::DURATION);
  if (itDurationChild != pAnnotations.end())
  {
    const GroundedExpression* durationGrdExpPtr =
        itDurationChild->second->getGrdExpPtr_SkipWrapperPtrs();
    if (durationGrdExpPtr != nullptr)
    {
      const GroundedExpression& durationGrdExp = *durationGrdExpPtr;
      const SemanticDurationGrounding* durationGrdPtr = durationGrdExp->getDurationGroundingPtr();
      if (durationGrdPtr != nullptr)
      {
        const SemanticDurationGrounding& durationGrd = *durationGrdPtr;
        return mystd::optional<int64_t>(durationGrd.duration.nbMilliseconds());
      }
    }
  }
  return mystd::optional<int64_t>();
}


bool hasASpecificWord(const SemanticGenericGrounding& pGrounding)
{
  if (!pGrounding.word.lemma.empty())
    return true;
  for (const auto& currCpt : pGrounding.concepts)
    if (!ConceptSet::isAConceptAny(currCpt.first))
      return true;
  return false;
}


bool doesSemExpContainsOnlyARequest(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr =
      pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const auto* reqListPtr = getRequestList(*grdExpPtr);
    return reqListPtr != nullptr &&
        !reqListPtr->empty();
  }
  return false;
}


const SemanticRequests* getRequestList
(const GroundedExpression& pGrdExp)
{
  auto& grd = pGrdExp.grounding();
  if (grd.type == SemanticGroundingType::STATEMENT)
  {
    auto& res = grd.getStatementGrounding().requests;
    if (!res.empty())
      return &res;
  }
  return nullptr;
}

SemanticRequests* getRequestList
(GroundedExpression& pGrdExp)
{
  auto& grd = pGrdExp.grounding();
  if (grd.type == SemanticGroundingType::STATEMENT)
  {
    auto& res = grd.getStatementGrounding().requests;
    if (!res.empty())
      return &res;
  }
  return nullptr;
}




GroundedExpression* getGrdExpChild(GroundedExpression& pGrdExp,
                                   GrammaticalType pChildType)
{
  auto itChild = pGrdExp.children.find(pChildType);
  if (itChild != pGrdExp.children.end())
    return itChild->second->getGrdExpPtr_SkipWrapperPtrs();
  return nullptr;
}


const GroundedExpression* getGrdExpChild(const GroundedExpression& pGrdExp,
                                         GrammaticalType pChildType)
{
  auto itChild = pGrdExp.children.find(pChildType);
  if (itChild != pGrdExp.children.end())
    return itChild->second->getGrdExpPtr_SkipWrapperPtrs();
  return nullptr;
}


const SemanticExpression* getChildFromGrdExp(const GroundedExpression& pGrdExp,
                                             GrammaticalType pChildType)
{
  auto itChild = pGrdExp.children.find(pChildType);
  if (itChild != pGrdExp.children.end())
    return &*itChild->second;
  return nullptr;
}


const SemanticExpression* getChildFromSemExp(const SemanticExpression& pSemExp,
                                             GrammaticalType pChildType)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    auto itChild = grdExpPtr->children.find(pChildType);
    if (itChild != grdExpPtr->children.end())
      return &*itChild->second;
  }
  return nullptr;
}


SemanticExpression& getDirectObjectOrIdentityRecursively(SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr == nullptr)
    return pSemExp;
  if (grdExpPtr->grounding().getStatementGroundingPtr() == nullptr)
    return pSemExp;
  auto itChild = grdExpPtr->children.find(GrammaticalType::OBJECT);
  if (itChild != grdExpPtr->children.end())
    return getDirectObjectOrIdentityRecursively(*itChild->second);
  return pSemExp;
}


bool hasChild(const GroundedExpression& pGrdExp,
              GrammaticalType pChildType)
{
  return pGrdExp.children.count(pChildType) > 0;
}


const GroundedExpression* getUnnamedGrdExpPtr(const SemanticExpression& pSemExp)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    return pSemExp.getGrdExpPtr();
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const auto& intExp = pSemExp.getIntExp();
    if (intExp.source == InterpretationSource::RECENTCONTEXT)
      return nullptr;
    const auto* origExpPtr = intExp.originalExp->getGrdExpPtr_SkipWrapperPtrs();
    if (origExpPtr != nullptr && origExpPtr->grounding().getNameGroundingPtr() != nullptr)
      return nullptr;
    return getUnnamedGrdExpPtr(*intExp.interpretedExp);
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return getUnnamedGrdExpPtr(*pSemExp.getFdkExp().concernedExp);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return getUnnamedGrdExpPtr(*pSemExp.getAnnExp().semExp);
  }
  case SemanticExpressionType::METADATA:
  {
    return getUnnamedGrdExpPtr(*pSemExp.getMetadataExp().semExp);
  }
  case SemanticExpressionType::COMMAND:
  {
    return getUnnamedGrdExpPtr(*pSemExp.getCmdExp().semExp);
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = pSemExp.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return getUnnamedGrdExpPtr(**originalFrom);
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    return getUnnamedGrdExpPtr(pSemExp.getFSynthExp().getSemExp());
  }
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::LIST:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


bool isNow(const GroundedExpression& pGrdExp)
{
  auto itTimeChild = pGrdExp.children.find(GrammaticalType::TIME);
  if (itTimeChild != pGrdExp.children.end())
  {
    const SemanticExpression& timeSepExp = *itTimeChild->second;
    const GroundedExpression* timeGrdExpPtr = timeSepExp.getGrdExpPtr_SkipWrapperPtrs();
    if (timeGrdExpPtr != nullptr)
    {
      const SemanticTimeGrounding* timeGrdPtr = timeGrdExpPtr->grounding().getTimeGroundingPtr();
      return timeGrdPtr != nullptr &&
          timeGrdPtr->isEqualMoreOrLess10Seconds(SemanticTimeGrounding::now());
    }
  }
  return false;
}


const SemanticExpression* getGrammTypeInfo(const SemanticExpression& pSemExp,
                                           GrammaticalType pGrammaticalType)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const GroundedExpression& grdExp = pSemExp.getGrdExp();
    auto itChild = grdExp.children.find(pGrammaticalType);
    if (itChild != grdExp.children.end())
      return &*itChild->second;
    return nullptr;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const AnnotatedExpression& annExp = pSemExp.getAnnExp();
    auto itChild = annExp.annotations.find(pGrammaticalType);
    if (itChild != annExp.annotations.end())
      return &*itChild->second;
    return getGrammTypeInfo(*annExp.semExp, pGrammaticalType);
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return getGrammTypeInfo(*pSemExp.getFdkExp().concernedExp, pGrammaticalType);
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    return getGrammTypeInfo(*pSemExp.getIntExp().interpretedExp, pGrammaticalType);
  }
  case SemanticExpressionType::METADATA:
  {
    return getGrammTypeInfo(*pSemExp.getMetadataExp().semExp, pGrammaticalType);
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = pSemExp.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return getGrammTypeInfo(**originalFrom, pGrammaticalType);
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    return getGrammTypeInfo(pSemExp.getFSynthExp().getSemExp(), pGrammaticalType);
  }
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::LIST:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


bool doesSemExpCanBeCompletedWithContext(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr();
  return grdExpPtr == nullptr ||
      grdExpPtr->grounding().type != SemanticGroundingType::AGENT;
}


const SemanticExpression* getTimeInfo(const SemanticExpression& pSemExp)
{
  const SemanticExpression* res =
      SemExpGetter::getGrammTypeInfo(pSemExp, GrammaticalType::TIME);
  if (res != nullptr)
    return res;

  const SemanticExpression* objectChildPtr =
      SemExpGetter::getGrammTypeInfo(pSemExp, GrammaticalType::OBJECT);
  if (objectChildPtr != nullptr)
  {
    const AnnotatedExpression* annExpPtr = objectChildPtr->getAnnExpPtr();
    if (annExpPtr != nullptr)
    {
      auto itTimeAnnoation = annExpPtr->annotations.find(GrammaticalType::TIME);
      if (itTimeAnnoation != annExpPtr->annotations.end())
        return &*itTimeAnnoation->second;

    }
  }
  return nullptr;
}


SemanticReferenceType getReference(const GroundedExpression& pGrdExp)
{
  const auto& grd = pGrdExp.grounding();
  auto* genGrdPtr = grd.getGenericGroundingPtr();
  if (genGrdPtr != nullptr)
    return genGrdPtr->referenceType;
  auto* timeGrdPtr = grd.getTimeGroundingPtr();
  if (timeGrdPtr != nullptr)
    return SemanticReferenceType::DEFINITE;
  auto* agentGrdPtr = grd.getAgentGroundingPtr();
  if (agentGrdPtr != nullptr && agentGrdPtr->isSpecificUser())
    return SemanticReferenceType::DEFINITE;
  if (grd.getResourceGroundingPtr() != nullptr)
    return SemanticReferenceType::DEFINITE;
  return SemanticReferenceType::UNDEFINED;
}


bool isDefinite(const SemanticExpression& pSemExp)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return getReference(*grdExpPtr) == SemanticReferenceType::DEFINITE;
  const auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    const auto& listExp = *listExpPtr;
    for (const auto& currElt : listExp.elts)
      if (isDefinite(*currElt))
        return true;
  }
  return false;
}


bool isDefiniteModifier(const SemanticExpression& pSemExp)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const auto& grd = grdExpPtr->grounding();
    auto* genGrdPtr = grd.getGenericGroundingPtr();
    if (genGrdPtr != nullptr &&
        genGrdPtr->referenceType == SemanticReferenceType::DEFINITE &&
        genGrdPtr->entityType == SemanticEntityType::MODIFIER)
      return true;
    return false;
  }
  const auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    const auto& listExp = *listExpPtr;
    for (const auto& currElt : listExp.elts)
      if (isDefiniteModifier(*currElt))
        return true;
  }
  return false;
}


bool hasSpecifications(const SemanticExpression& pSemExp)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const auto& grdExp = *grdExpPtr;
    if (getNumberFromGrd(*grdExp) == SemanticNumberType::PLURAL)
      return true;
    for (const auto& currChild : grdExp.children)
    {
      if (currChild.first == GrammaticalType::BETWEEN ||
          currChild.first == GrammaticalType::OWNER ||
          currChild.first == GrammaticalType::TIME ||
          currChild.first == GrammaticalType::TODO)
        return true;
      if (currChild.first == GrammaticalType::SPECIFIER)
      {
        std::list<const GroundedExpression*> currChidGrdExpPtrs;
        currChild.second->getGrdExpPtrs_SkipWrapperLists(currChidGrdExpPtrs);
        for (const auto* currGrdExpPtr : currChidGrdExpPtrs)
        {
          if (currGrdExpPtr->children.count(GrammaticalType::INTRODUCTING_WORD) == 0)
            return true;
        }
      }
    }
  }
  return false;
}

bool isItAnActionLabeling(const GroundedExpression*& pLabelSemExp,
                          const SemanticStatementGrounding*& pEqualityStatement,
                          const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr == nullptr)
    return false;
  const GroundedExpression& grdExp = *grdExpPtr;
  const SemanticStatementGrounding* statGrdptr = grdExp->getStatementGroundingPtr();
  if (statGrdptr == nullptr ||
      !ConceptSet::haveAConceptThatBeginWith(statGrdptr->concepts, "verb_equal_") ||
      !(isAPresentTense(statGrdptr->verbTense) || isAPastTense(statGrdptr->verbTense)))
    return false;

  auto itSubject = grdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject == grdExp.children.end())
    return false;
  const SemanticGenericGrounding* subjectGenGrdPtr = _getGenericGrdPtr(*itSubject->second);
  if (subjectGenGrdPtr == nullptr ||
      !subjectGenGrdPtr->coreference ||
      subjectGenGrdPtr->coreference->getDirection() != CoreferenceDirectionEnum::BEFORE ||
      subjectGenGrdPtr->referenceType != SemanticReferenceType::DEFINITE)
    return false;

  auto itObject = grdExp.children.find(GrammaticalType::OBJECT);
  if (itObject == grdExp.children.end())
    return false;
  const GroundedExpression* objectGrdExpPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
  if (objectGrdExpPtr == nullptr)
    return false;
  const GroundedExpression& objectGrdExp = *objectGrdExpPtr;
  const SemanticStatementGrounding* objectStatGrdPtr = objectGrdExp->getStatementGroundingPtr();
  if (objectStatGrdPtr == nullptr ||
      objectStatGrdPtr->verbTense != SemanticVerbTense::UNKNOWN)
    return false;

  pLabelSemExp = objectGrdExpPtr;
  pEqualityStatement = statGrdptr;
  return true;
}


bool isAModifierFromGrdExp(const GroundedExpression& pGrdExp)
{
  auto& grd = pGrdExp.grounding();
  const SemanticGenericGrounding* genGrdPtr = grd.getGenericGroundingPtr();
  return genGrdPtr != nullptr &&
      genGrdPtr->entityType == SemanticEntityType::MODIFIER;
}


bool isAModifier(const SemanticExpression& pSemExp, bool pFollowInterpretations)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
  if (grdExpPtr != nullptr)
    return isAModifierFromGrdExp(*grdExpPtr);
  return false;
}


bool hasAChildModifier(const GroundedExpression& pGrdExp)
{
  auto itSpecifier = pGrdExp.children.find(GrammaticalType::SPECIFIER);
  if (itSpecifier != pGrdExp.children.end())
    return isAModifier(*itSpecifier->second);
  return false;
}


SemanticRequestType convertSemGramToRequestType
(GrammaticalType pGramType)
{
  switch (pGramType)
  {
  case GrammaticalType::SUBJECT:
    return SemanticRequestType::SUBJECT;
  case GrammaticalType::OBJECT:
    return SemanticRequestType::OBJECT;
  case GrammaticalType::LOCATION:
    return SemanticRequestType::LOCATION;
  case GrammaticalType::MANNER:
    return SemanticRequestType::MANNER;
  case GrammaticalType::TIME:
    return SemanticRequestType::TIME;
  case GrammaticalType::PURPOSE:
    return SemanticRequestType::PURPOSE;
  case GrammaticalType::CAUSE:
    return SemanticRequestType::CAUSE;
  case GrammaticalType::TOPIC:
    return SemanticRequestType::TOPIC;
  default:
    return SemanticRequestType::NOTHING;
  }
}


bool isSemExpEqualToAnAgent
(const SemanticExpression& pSemExp,
 const SemanticAgentGrounding& pAgentGrd)
{
  const GroundedExpression* grdExp = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp != nullptr)
  {
    auto* agentGrdPtr = grdExp->grounding().getAgentGroundingPtr();
    return agentGrdPtr != nullptr &&
        agentGrdPtr->userId == pAgentGrd.userId;
  }
  return false;
}


bool doesSemExpHaveAnAgent
(const SemanticExpression& pSemExp,
 const SemanticAgentGrounding& pAgentGrd)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const auto& grdExp = pSemExp.getGrdExp();
    auto* agentGrdPtr = grdExp->getAgentGroundingPtr();
    if (agentGrdPtr != nullptr &&
        agentGrdPtr->userId == pAgentGrd.userId)
      return true;
    for (const auto& currChild : grdExp.children)
      if (doesSemExpHaveAnAgent(*currChild.second, pAgentGrd))
        return true;
    return false;
  }
  case SemanticExpressionType::LIST:
  {
    const auto& listExp = pSemExp.getListExp();
    if (!listExp.elts.empty())
      return doesSemExpHaveAnAgent(*listExp.elts.front(), pAgentGrd);
    return false;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const auto& fdkExp = pSemExp.getFdkExp();
    return doesSemExpHaveAnAgent(*fdkExp.concernedExp, pAgentGrd) ||
        doesSemExpHaveAnAgent(*fdkExp.feedbackExp, pAgentGrd);
  }
  case SemanticExpressionType::METADATA:
  {
    const auto& metadataExp = pSemExp.getMetadataExp();
    return doesSemExpHaveAnAgent(*metadataExp.semExp, pAgentGrd);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const auto& annExp = pSemExp.getAnnExp();
    if (doesSemExpHaveAnAgent(*annExp.semExp, pAgentGrd))
      return true;
    for (const auto& currAnn : annExp.annotations)
      if (doesSemExpHaveAnAgent(*currAnn.second, pAgentGrd))
        return true;
    return false;
  }
  case SemanticExpressionType::CONDITION:
  {
    const auto& condExp = pSemExp.getCondExp();
    return doesSemExpHaveAnAgent(*condExp.conditionExp, pAgentGrd) ||
        doesSemExpHaveAnAgent(*condExp.thenExp, pAgentGrd) ||
        (condExp.elseExp && doesSemExpHaveAnAgent(**condExp.elseExp, pAgentGrd));
  }
  case SemanticExpressionType::COMPARISON:
  {
    const auto& compExp = pSemExp.getCompExp();
    return doesSemExpHaveAnAgent(*compExp.leftOperandExp, pAgentGrd) ||
        (compExp.whatIsComparedExp && doesSemExpHaveAnAgent(**compExp.whatIsComparedExp, pAgentGrd)) ||
        (compExp.rightOperandExp && doesSemExpHaveAnAgent(**compExp.rightOperandExp, pAgentGrd));
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    const auto& setOfFormExp = pSemExp.getSetOfFormsExp();
    for (const auto& currForm : setOfFormExp.prioToForms)
    {
      for (const auto& currQForm : currForm.second)
        if (doesSemExpHaveAnAgent(*currQForm->exp, pAgentGrd))
          return true;
    }
    return false;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const auto& fSynthExp = pSemExp.getFSynthExp();
    return doesSemExpHaveAnAgent(fSynthExp.getSemExp(), pAgentGrd);
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const auto& intExp = pSemExp.getIntExp();
    return doesSemExpHaveAnAgent(*intExp.interpretedExp, pAgentGrd);
  }
  case SemanticExpressionType::COMMAND:
    return false;
  };
  assert(false);
  return false;
}


bool doSemExpHoldUserId
(const SemanticExpression& pSemExp,
 const std::string& pUserId)
{
  const GroundedExpression* grdExp = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp != nullptr)
  {
    return grdExp->grounding().type == SemanticGroundingType::AGENT &&
        (*grdExp)->getAgentGrounding().userId == pUserId;
  }
  const ListExpression* listExp = pSemExp.getListExpPtr();
  if (listExp != nullptr &&
      !listExp->elts.empty())
  {
    return doSemExpHoldUserId(*listExp->elts.front(), pUserId);
  }
  return false;
}


const GroundedExpression* getOriginalGrdExpForm(const SetOfFormsExpression& pSetOfFormsExp)
{
  for (const auto& currSetOfForm : pSetOfFormsExp.prioToForms)
    for (const auto& currForm : currSetOfForm.second)
      if (currForm->isOriginalForm)
        return currForm->exp->getGrdExpPtr_SkipWrapperPtrs();
  return nullptr;
}



const GroundedExpression* splitMainGrdAndOtherOnes
(std::list<const GroundedExpression*>& pOtherGrdExps,
 bool& pHasOriginalForm,
 const std::list<std::unique_ptr<QuestExpressionFrom>>& pForms)
{
  const GroundedExpression* mainGrd = nullptr;
  bool firstIteration = true;
  for (const auto& currForm : pForms)
  {
    if (currForm->isOriginalForm)
      pHasOriginalForm = true;
    const GroundedExpression* subGrd = currForm->exp->getGrdExpPtr_SkipWrapperPtrs();
    if (firstIteration)
    {
      mainGrd = subGrd;
    }
    else if (subGrd != nullptr)
    {
      pOtherGrdExps.emplace_back(subGrd);
    }
    firstIteration = false;
  }
  return mainGrd;
}


bool grdExpIsAnEmptyStatementGrd(const GroundedExpression& pGrdExp)
{
  if (pGrdExp.children.empty())
  {
    auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    return statGrdPtr != nullptr &&
        statGrdPtr->word.isEmpty() &&
        statGrdPtr->concepts.empty();
  }
  return false;
}


bool semExpIsAnEmptyStatementGrd(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  return grdExpPtr != nullptr &&
      grdExpIsAnEmptyStatementGrd(*grdExpPtr);
}


bool semExphasAStatementGrd(const SemanticExpression& pSemExp)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    return pSemExp.getGrdExp()->type == SemanticGroundingType::STATEMENT;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    return semExphasAStatementGrd(*pSemExp.getIntExp().interpretedExp);
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return semExphasAStatementGrd(*pSemExp.getFdkExp().concernedExp);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return semExphasAStatementGrd(*pSemExp.getAnnExp().semExp);
  }
  case SemanticExpressionType::METADATA:
  {
    return semExphasAStatementGrd(*pSemExp.getMetadataExp().semExp);
  }
  case SemanticExpressionType::LIST:
  {
    const ListExpression& listExp = pSemExp.getListExp();
    if (!listExp.elts.empty())
    {
      return semExphasAStatementGrd(*listExp.elts.front());
    }
    return false;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    return semExphasAStatementGrd(pSemExp.getFSynthExp().getSemExp());
  }
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
  {
    return true;
  }
  }
  assert(false);
  return false;
}


void extractReferences(std::list<std::string>& pReferences,
                       const SemanticExpression& pSemExp)
{
  std::list<const MetadataExpression*> metadateExps;
  pSemExp.getMetadataPtr_SkipWrapperAndLists(metadateExps);
  for (const auto& currMetadataExpPtr : metadateExps)
  {
    const MetadataExpression& currMetadataExp = *currMetadataExpPtr;
    for (const auto& currReference : currMetadataExp.references)
      MetadataExpression::addReference(pReferences, currReference);
    extractReferences(pReferences, *currMetadataExp.semExp);
  }
}


ContextualAnnotation extractContextualAnnotation(const SemanticExpression& pSemExp)
{
  std::list<const MetadataExpression*> metadateExps;
  pSemExp.getMetadataPtr_SkipWrapperAndLists(metadateExps);
  if (!metadateExps.empty())
    return metadateExps.back()->contextualAnnotation;
  return ContextualAnnotation::ANSWERNOTFOUND;
}



SemanticSourceEnum extractSource(const SemanticExpression& pSemExp)
{
  const MetadataExpression* metadataPtr = pSemExp.getMetadataPtr_SkipWrapperPtrs();
  if (metadataPtr != nullptr)
    return metadataPtr->from;
  return SemanticSourceEnum::UNKNOWN;
}


bool doesContainADialogSource(const SemanticExpression& pSemExp)
{
  SemanticSourceEnum from = extractSource(pSemExp);
  return from == SemanticSourceEnum::WRITTENTEXT ||
      from == SemanticSourceEnum::TTS ||
      from == SemanticSourceEnum::SEMREACTION ||
      from == SemanticSourceEnum::ASR;
}


const SemanticAgentGrounding* extractAgentGrdPtr(const SemanticExpression& pSemExp)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return grdExpPtr->grounding().getAgentGroundingPtr();
  return nullptr;
}

const SemanticAgentGrounding* extractAuthor(const SemanticExpression& pSemExp)
{
  auto* authorSemExpPtr = extractAuthorSemExp(pSemExp);
  if (authorSemExpPtr != nullptr)
    return extractAgentGrdPtr(*authorSemExpPtr);
  return nullptr;
}

const SemanticExpression* extractAuthorSemExp(const SemanticExpression& pSemExp)
{
  const MetadataExpression* metadataPtr = pSemExp.getMetadataPtr_SkipWrapperPtrs();
  if (metadataPtr != nullptr)
    return metadataPtr->getAuthorSemExpPtr();
  return nullptr;
}

bool semExpHasACoreferenceOrAnAgent(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    auto& grd = grdExpPtr->grounding();
    auto* genPtr = grd.getGenericGroundingPtr();
    if (genPtr != nullptr)
      return genPtr->coreference.has_value();
    auto* cptPtr = grd.getConceptualGroundingPtr();
    if (cptPtr != nullptr)
      return ConceptSet::haveAConcept(cptPtr->concepts, "tolink_1p");
    return grd.type == SemanticGroundingType::AGENT;
  }
  else
  {
    const ListExpression* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
    if (listExpPtr != nullptr)
      for (const auto& currElt : listExpPtr->elts)
        if (semExpHasACoreferenceOrAnAgent(*currElt))
          return true;
  }
  return false;
}

bool isAHumanFromGrd(const SemanticGrounding& pGrd)
{
  const SemanticGenericGrounding* genGrd = pGrd.getGenericGroundingPtr();
  if (genGrd != nullptr)
    return genGrd->entityType == SemanticEntityType::HUMAN;
  return pGrd.type == SemanticGroundingType::AGENT;
}

bool isASpecificHuman(const SemanticGenericGrounding& pGenGrd)
{
  return pGenGrd.quantity.isEqualToOne() &&
      pGenGrd.referenceType == SemanticReferenceType::DEFINITE &&
      pGenGrd.entityType == SemanticEntityType::HUMAN;
}

bool isASpecificHumanFromGrdExp(const GroundedExpression& pGrdExp)
{
  const SemanticGenericGrounding* genGrd = pGrdExp->getGenericGroundingPtr();
  if (genGrd != nullptr)
    return isASpecificHuman(*genGrd);
  return pGrdExp->getAgentGroundingPtr() != nullptr;
}


bool isAResourceFromGrdExp(const GroundedExpression& pGrdExp)
{
  return pGrdExp->type == SemanticGroundingType::RESOURCE;
}

bool isAResource(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return isAResourceFromGrdExp(*grdExpPtr);
  return false;
}


bool isAResourceOrATextFromGrdExp(const GroundedExpression& pGrdExp)
{
  return pGrdExp->type == SemanticGroundingType::RESOURCE ||
      pGrdExp->type == SemanticGroundingType::TEXT;
}

bool isAResourceOrAText(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return isAResourceOrATextFromGrdExp(*grdExpPtr);
  return false;
}


bool hasGroundingType(const SemanticExpression& pSemExp,
                      const std::set<SemanticGroundingType>& pGroundingType)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const SemanticGrounding& grd = grdExpPtr->grounding();
    return pGroundingType.find(grd.type) != pGroundingType.end();
  }

  const ListExpression* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (const auto& currElt : listExpPtr->elts)
      if (hasGroundingType(*currElt, pGroundingType))
        return true;

  return false;
}


SemanticRequestType getMainRequestTypeFromGrdExp
(const GroundedExpression& pGrdExp)
{
  const auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
    return statGrdPtr->requests.firstOrNothing();
  return SemanticRequestType::NOTHING;
}


bool isGrdExpAQuestion(const GroundedExpression& pGrdExp)
{
  SemanticRequestType reqType = getMainRequestTypeFromGrdExp(pGrdExp);
  return reqType != SemanticRequestType::NOTHING && reqType != SemanticRequestType::ACTION;
}

bool isSemExpAQuestion(const SemanticExpression& pSemExp)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::ANNOTATED:
    return isSemExpAQuestion(*pSemExp.getAnnExp().semExp);
  case SemanticExpressionType::COMMAND:
    return isSemExpAQuestion(*pSemExp.getCmdExp().semExp);
  case SemanticExpressionType::COMPARISON:
  {
    const auto& compExp = pSemExp.getCompExp();
    if (isSemExpAQuestion(*compExp.leftOperandExp))
      return true;
    if (compExp.rightOperandExp &&
        isSemExpAQuestion(**compExp.rightOperandExp))
      return true;
    return false;
  }
  case SemanticExpressionType::CONDITION:
    return false;
  case SemanticExpressionType::FEEDBACK:
    return isSemExpAQuestion(*pSemExp.getFdkExp().concernedExp);
  case SemanticExpressionType::GROUNDED:
    return isGrdExpAQuestion(pSemExp.getGrdExp());
  case SemanticExpressionType::INTERPRETATION:
    return isSemExpAQuestion(*pSemExp.getIntExp().interpretedExp);
  case SemanticExpressionType::LIST:
  {
    const auto& listExp = pSemExp.getListExp();
    if (!listExp.elts.empty())
    {
      auto itSemExp = --listExp.elts.end();
      while (true)
      {
        auto& semExp = **itSemExp;
        if (isAResource(semExp))
        {
          if (itSemExp == listExp.elts.begin())
            break;
          --itSemExp;
          continue;
        }
        return isSemExpAQuestion(semExp);
      }
    }
    return false;
  }
  case SemanticExpressionType::METADATA:
  {
    const auto& metadata = pSemExp.getMetadataExp();
    if (metadata.contextualAnnotation == ContextualAnnotation::QUESTION)
      return true;
    return isSemExpAQuestion(*metadata.semExp);
  }
  case SemanticExpressionType::SETOFFORMS:
    return true;
  case SemanticExpressionType::FIXEDSYNTHESIS:
    return isSemExpAQuestion(pSemExp.getFSynthExp().getSemExp());
  }
  return false;
}


void wordToAStaticMeaning(StaticLinguisticMeaning& pRes,
                          const SemanticWord& pWord,
                          SemanticLanguageEnum pToLanguage,
                          const linguistics::LinguisticDatabase& pLingDb)
{
  int32_t meaningId = wordToMeaningId(pWord, pToLanguage, pLingDb);
  if (meaningId != LinguisticMeaning_noMeaningId)
  {
    pRes = StaticLinguisticMeaning(pToLanguage, meaningId);
    return;
  }
  if (pToLanguage != SemanticLanguageEnum::UNKNOWN)
  {
    wordToAStaticMeaningInLanguage(pRes, pWord, SemanticLanguageEnum::UNKNOWN, pLingDb);
  }
}

void wordToAMeaning(LinguisticMeaning& pRes,
                    const SemanticWord& pWord,
                    SemanticLanguageEnum pToLanguage,
                    const linguistics::LinguisticDatabase& pLingDb)
{
  const auto& specLingDb = pLingDb.langToSpec[pWord.language];
  if (pLingDb.transDict.getTranslation(pRes, pWord, pToLanguage, specLingDb.lingDico))
    return;
  if (pToLanguage != SemanticLanguageEnum::UNKNOWN)
    wordToAMeaningInLanguage(pRes, pWord, SemanticLanguageEnum::UNKNOWN, pLingDb);
}

void wordToAStaticMeaningInLanguage(StaticLinguisticMeaning& pRes,
                                    const SemanticWord& pWord,
                                    SemanticLanguageEnum pToLanguage,
                                    const linguistics::LinguisticDatabase& pLingDb)
{
  int meaningId = wordToMeaningId(pWord, pToLanguage, pLingDb);
  if (meaningId != LinguisticMeaning_noMeaningId)
    pRes = StaticLinguisticMeaning(pToLanguage, meaningId);
}

void wordToAMeaningInLanguage(LinguisticMeaning& pRes,
                              const SemanticWord& pWord,
                              SemanticLanguageEnum pToLanguage,
                              const linguistics::LinguisticDatabase& pLingDb)
{
  int meaningId = wordToMeaningId(pWord, pToLanguage, pLingDb);
  if (meaningId != LinguisticMeaning_noMeaningId)
    pRes.emplace_id(pToLanguage, meaningId);
}


int32_t wordToMeaningId(const SemanticWord& pWord,
                        SemanticLanguageEnum pToLanguage,
                        const linguistics::LinguisticDatabase& pLingDb)
{
  int32_t res = LinguisticMeaning_noMeaningId;
  const auto& specLingDb = pLingDb.langToSpec[pWord.language];
  const auto& statLingDico = specLingDb.lingDico.statDb;
  pLingDb.transDict.getTranslation(res, pWord, pToLanguage, statLingDico);
  return res;
}


std::string getUserIdOfSubject(const GroundedExpression& pGrdExp)
{
  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject != pGrdExp.children.end())
  {
    const GroundedExpression* grdExpPtr = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
    {
      const SemanticAgentGrounding* subjAgentGrdPtr = grdExpPtr->grounding().getAgentGroundingPtr();
      if (subjAgentGrdPtr != nullptr)
        return subjAgentGrdPtr->userId;
    }
  }
  return SemanticAgentGrounding::userNotIdentified;
}


void extractSubjectAndObjectOfAVerbDefinition(
    const GroundedExpression*& pSubjectGrdPtr,
    const SemanticExpression*& pInfCommandToDo,
    const GroundedExpression& pGrdExp)
{
  const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr == nullptr ||
      !ConceptSet::haveAConcept(statGrdPtr->concepts, "verb_equal_mean"))
    return;

  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject == pGrdExp.children.end())
    return;
  const GroundedExpression* subjGrdExpPtr = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
  if (subjGrdExpPtr == nullptr ||
      !isAnInfinitiveGrdExp(*subjGrdExpPtr))
    return;

  auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
  if (itObject == pGrdExp.children.end())
    return;

  std::list<const GroundedExpression*> objectGrdExpPtrs;
  itObject->second->getGrdExpPtrs_SkipWrapperLists(objectGrdExpPtrs);
  if (objectGrdExpPtrs.empty())
    return;
  for (const auto& currObjPtr : objectGrdExpPtrs)
    if (!isAnInfinitiveGrdExp(*currObjPtr) &&
        currObjPtr->grounding().getMetaGroundingPtr() == nullptr &&
        currObjPtr->grounding().getResourceGroundingPtr() == nullptr)
      return;

  pSubjectGrdPtr = subjGrdExpPtr;
  pInfCommandToDo = &*itObject->second;
}


void extractTeachElements(
    const GroundedExpression*& pPurposeGrdPtr,
    const SemanticExpression*& pObjectSemExp,
    const GroundedExpression& pGrdExp)
{
  auto itPurpose = pGrdExp.children.find(GrammaticalType::PURPOSE);
  if (itPurpose == pGrdExp.children.end())
    return;
  const GroundedExpression* purposeGrdExpPtr = itPurpose->second->getGrdExpPtr_SkipWrapperPtrs();
  if (purposeGrdExpPtr == nullptr ||
      !isAnInfinitiveGrdExp(*purposeGrdExpPtr))
    return;

  auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
  if (itObject == pGrdExp.children.end())
    return;

  std::list<const GroundedExpression*> objectGrdExpPtrs;
  itObject->second->getGrdExpPtrs_SkipWrapperLists(objectGrdExpPtrs);
  if (objectGrdExpPtrs.empty())
    return;
  for (const auto& currObjPtr : objectGrdExpPtrs)
    if (!isATeachingElement(*currObjPtr))
      return;

  pPurposeGrdPtr = purposeGrdExpPtr;
  pObjectSemExp = &*itObject->second;
}

bool isAnActionDefinition(const GroundedExpression& pGrdExp)
{
  const GroundedExpression* subjectGrdPtr = nullptr;
  const SemanticExpression* objectSemExpPtr = nullptr;
  SemExpGetter::extractSubjectAndObjectOfAVerbDefinition(subjectGrdPtr, objectSemExpPtr, pGrdExp);
  return subjectGrdPtr != nullptr;
}


void _recIterateOnList(std::list<UniqueSemanticExpression*>& pRes,
                       UniqueSemanticExpression& pUSemExp)
{
  GroundedExpression* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    pRes.emplace_back(&pUSemExp);
    return;
  }

  ListExpression* listExpPtr = pUSemExp->getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    for (auto& currElt : listExpPtr->elts)
      _recIterateOnList(pRes, currElt);
    return;
  }
}

void _recIterateOnList(std::list<const SemanticExpression*>& pRes,
                       const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    pRes.emplace_back(&pSemExp);
    return;
  }

  auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    for (auto& currElt : listExpPtr->elts)
      _recIterateOnList(pRes, *currElt);
    return;
  }
}


template <typename TGRDEXP, typename TSEMEXP>
void _recIterateOnListOfGrdExps(std::list<TGRDEXP*>& pRes,
                                TSEMEXP& pSemExp)
{
  TGRDEXP* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    pRes.emplace_back(grdExpPtr);
    return;
  }

  auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    for (auto& currElt : listExpPtr->elts)
      _recIterateOnListOfGrdExps(pRes, *currElt);
    return;
  }
}

std::list<GroundedExpression*> iterateOnListOfGrdExps(SemanticExpression& pSemExp)
{
  std::list<GroundedExpression*> res;
  _recIterateOnListOfGrdExps(res, pSemExp);
  return res;
}

std::list<const GroundedExpression*> iterateOnListOfGrdExps(const SemanticExpression& pSemExp)
{
  std::list<const GroundedExpression*> res;
  _recIterateOnListOfGrdExps(res, pSemExp);
  return res;
}

std::list<UniqueSemanticExpression*> iterateOnList(UniqueSemanticExpression& pUSemExp)
{
  std::list<UniqueSemanticExpression*> res;
  _recIterateOnList(res, pUSemExp);
  return res;
}


std::list<const SemanticExpression*> iterateOnList(const SemanticExpression& pSemExp)
{
  std::list<const SemanticExpression*> res;
  _recIterateOnList(res, pSemExp);
  return res;
}


bool isAnInfinitiveGrdExp(const GroundedExpression& pGrdExp)
{
  const SemanticStatementGrounding* statGrdPtr =
      pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr &&
      statGrdPtr->isAtInfinitive())
    return true;
  return false;
}


bool isATeachingElement(const GroundedExpression& pGrdExp)
{
  return _isMandatoryGrdExp(pGrdExp) ||
      pGrdExp->getMetaGroundingPtr() != nullptr ||
      pGrdExp->getResourceGroundingPtr() != nullptr;
}

bool isNothing(const GroundedExpression& pGrdExp)
{
  const auto* genGrdPtr = pGrdExp->getGenericGroundingPtr();
  return genGrdPtr != nullptr &&
      genGrdPtr->quantity.isEqualToZero() &&
      genGrdPtr->concepts.empty() &&
      genGrdPtr->word.isEmpty();
}

bool isDoNothingSemExp(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const GroundedExpression& grdExp = *grdExpPtr;
    if (grdExp->getStatementGroundingPtr() != nullptr &&
        grdExp->concepts.find("verb_action") != grdExp->concepts.end())
    {
      auto itObject = grdExp.children.find(GrammaticalType::OBJECT);
      if (itObject != grdExp.children.end())
      {
        const GroundedExpression* ojectGrdExpPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
        return ojectGrdExpPtr != nullptr && isNothing(*ojectGrdExpPtr);
      }
    }
  }
  return false;
}


int getRank(const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const GroundedExpression& grdExp = *grdExpPtr;
    std::map<std::string, char> rankConcepts;
    static const std::string beginOfRankCptName = "rank_";
    ConceptSet::extractConceptsThatBeginWith(rankConcepts, grdExp->concepts, beginOfRankCptName);
    if (!rankConcepts.empty())
    {
      const std::string& rankCpt = rankConcepts.begin()->first;
      static const std::string beginOfrankInverseCptName = "rank_inverse_";
      bool isAnInverseRank = rankCpt.compare(0, beginOfrankInverseCptName.size(), beginOfrankInverseCptName) == 0;
      std::size_t posOfRankNumber = isAnInverseRank ?
            beginOfrankInverseCptName.size() : beginOfRankCptName.size();
      if (posOfRankNumber >= rankCpt.size())
        return 0;

      std::string aboluteRankNumber = rankCpt.substr(posOfRankNumber, rankCpt.size() - posOfRankNumber);
      int res = 0;
      try
      {
        res = mystd::lexical_cast<int>(aboluteRankNumber);
        if (isAnInverseRank)
          return -res;
        return res;
      }
      catch (...) {}
    }

    auto specifiierChildIt = grdExp.children.find(GrammaticalType::SPECIFIER);
    if (specifiierChildIt != grdExp.children.end())
      return getRank(*specifiierChildIt->second);
  }
  return 0;
}


const GroundedExpression* getGrdExpToDo(const GroundedExpression& pGrdExp,
                                        const SemanticStatementGrounding& pStatementGrd,
                                        const std::string& pAuthorUserId)
{
  if (!pStatementGrd.polarity ||
      !pStatementGrd.requests.empty())
    return nullptr;
  if (pStatementGrd.verbGoal == VerbGoalEnum::MANDATORY)
  {
    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject == pGrdExp.children.end() ||
        !SemExpGetter::doSemExpHoldUserId(*itSubject->second, SemanticAgentGrounding::me))
      return nullptr;
    return &pGrdExp;
  }
  else if (!pAuthorUserId.empty() &&
           pStatementGrd.concepts.count("verb_want") != 0)
  {
    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject == pGrdExp.children.end() ||
        !SemExpGetter::doSemExpHoldUserId(*itSubject->second, pAuthorUserId))
      return nullptr;

    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject != pGrdExp.children.end())
    {
      const GroundedExpression* res = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
      if (res != nullptr)
      {
        auto itResSubject = res->children.find(GrammaticalType::SUBJECT);
        if (itResSubject == res->children.end() ||
            !SemExpGetter::doSemExpHoldUserId(*itResSubject->second, SemanticAgentGrounding::me))
          return nullptr;

        const SemanticStatementGrounding* statGrdPtr = res->grounding().getStatementGroundingPtr();
        if (statGrdPtr != nullptr)
          return res;
      }
    }
  }
  return nullptr;
}


const GroundedExpression* getLastGrdExpPtr(const SemanticExpression& pSemExp)
{
  const GroundedExpression* res = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (res != nullptr)
    return res;

  const ListExpression* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr && !listExpPtr->elts.empty())
    return getLastGrdExpPtr(*listExpPtr->elts.back());
  return nullptr;
}


void getConceptsOfGrdExp
(std::set<std::string>& pConcepts,
 const GroundedExpression& pGrdExp)
{
  for (const auto& currCpt : pGrdExp->concepts)
    pConcepts.insert(currCpt.first);
  for (const auto& currChild : pGrdExp.children)
    getConceptsOfSemExp(pConcepts, *currChild.second);
}


void getConceptsOfSemExp
(std::set<std::string>& pConcepts,
 const SemanticExpression& pSemExp)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    getConceptsOfGrdExp(pConcepts, *grdExpPtr);
    return;
  }

  const ListExpression* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (const auto& currListElt : listExpPtr->elts)
      getConceptsOfSemExp(pConcepts, *currListElt);
}


std::unique_ptr<GroundedExpression> getCopyExceptChild
(const GroundedExpression& pGrdExp,
 GrammaticalType pGrammaticalTypeToSkip)
{
  auto res = std::make_unique<GroundedExpression>(pGrdExp.cloneGrounding());
  for (const auto& currChild : pGrdExp.children)
    if (currChild.first != pGrammaticalTypeToSkip)
      res->children.emplace(currChild.first,
                            currChild.second->clone());
  return res;
}


bool agentIsTheSubject(const GroundedExpression& pGrdExp,
                       const std::string& pSubjectUserId)
{
  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject != pGrdExp.children.end())
  {
    const GroundedExpression* subjectGrdExpPtr = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
    if (subjectGrdExpPtr != nullptr)
    {
      const SemanticAgentGrounding* agentGrdPtr = subjectGrdExpPtr->grounding().getAgentGroundingPtr();
      return agentGrdPtr != nullptr &&
          agentGrdPtr->userId == pSubjectUserId;
    }
  }
  return false;
}


bool isACoreference(const SemanticExpression& pSemExp,
                    CoreferenceDirectionEnum pDirection,
                    bool pShouldNotHaveALemma)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  return grdExpPtr != nullptr &&
      _isACoreferenceFromGrdExp(*grdExpPtr, pDirection, pShouldNotHaveALemma);
}


bool isACoreferenceFromGenericGrounding(const SemanticGenericGrounding& pGenGrd,
                                        CoreferenceDirectionEnum pDirection,
                                        bool pShouldNotHaveALemma)
{
  return pGenGrd.coreference &&
      (pDirection == CoreferenceDirectionEnum::UNKNOWN || pGenGrd.coreference->getDirection() == pDirection) &&
      (!pShouldNotHaveALemma || pGenGrd.word.isEmpty());
}

bool isACoreferenceFromStatementGrounding(const SemanticStatementGrounding& pStatGrd,
                                          CoreferenceDirectionEnum pDirection,
                                          bool pShouldNotHaveALemma)
{
  return pStatGrd.coreference &&
      (pDirection == CoreferenceDirectionEnum::UNKNOWN || pStatGrd.coreference->getDirection() == pDirection) &&
      (!pShouldNotHaveALemma || pStatGrd.word.isEmpty());
}


GrammaticalType childGrammaticalTypeOfParentCoreference(const GroundedExpression& pGrdExp)
{
  for (auto& currChild : pGrdExp.children)
    if (hasACoreference(*currChild.second, CoreferenceDirectionEnum::PARENT))
      return currChild.first;
  return GrammaticalType::UNKNOWN;
}


bool hasACoreference(const SemanticExpression& pSemExp,
                     CoreferenceDirectionEnum pCorefenceDirection)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    if (_isACoreferenceFromGrdExp(*grdExpPtr, pCorefenceDirection, false)) //TODO: check
      return true;
    for (auto& currChild : grdExpPtr->children)
      if (hasACoreference(*currChild.second))
        return true;
  }
  else
  {
    const ListExpression* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
    if (listExpPtr != nullptr)
      for (auto& currElt : listExpPtr->elts)
        if (hasACoreference(*currElt))
          return true;
  }
  return false;
}


const UniqueSemanticExpression* getCoreferenceAfterFromGrdExp(const GroundedExpression& pGrdExp)
{
  return _getCoreferenceFromGrdExp<const UniqueSemanticExpression, const GroundedExpression>(pGrdExp, CoreferenceDirectionEnum::AFTER);
}

UniqueSemanticExpression* getCoreferenceAfterFromGrdExp(GroundedExpression& pGrdExp)
{
  return _getCoreferenceFromGrdExp<UniqueSemanticExpression, GroundedExpression>(pGrdExp, CoreferenceDirectionEnum::AFTER);
}


bool isGrdExpPositive(const GroundedExpression& pGrdExp)
{
  const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
    return statGrdPtr->polarity;
  return false;
}


bool getSubjectAndObjectUserIdsOfNameAssignement(std::string& pSubjectUserId,
                                                 std::string& pObjectUserId,
                                                 const GroundedExpression*& pSubjectGrdExpPtr,
                                                 const GroundedExpression*& pObjectGrdExpPtr,
                                                 const GroundedExpression& pGrdExp)
{
  const SemanticStatementGrounding& statGrd = pGrdExp->getStatementGrounding();
  if (!statGrd.polarity)
    return false;
  bool equCpt = statGrd.concepts.count("verb_equal_be") > 0;
  if (!equCpt &&
      statGrd.concepts.count("predicate_hasName") == 0)
    return false;

  const GroundedExpression* objectGrdExpPtr =
      SemExpGetter::getGrdExpChild(pGrdExp,GrammaticalType::OBJECT);
  if (objectGrdExpPtr == nullptr)
    return false;

  const GroundedExpression* subjectGrdExpPtr =
      SemExpGetter::getGrdExpChild(pGrdExp,GrammaticalType::SUBJECT);
  if (subjectGrdExpPtr == nullptr)
    return false;

  if (equCpt)
  {
    auto tryToReplaceByOwnerName = [](const GroundedExpression*& pGrdExpPtr)
    {
      if (pGrdExpPtr->grounding().concepts.count("name") > 0)
      {
        auto itOwner = pGrdExpPtr->children.find(GrammaticalType::OWNER);
        if (itOwner != pGrdExpPtr->children.end())
        {
          auto* newGrdExpPtr = itOwner->second->getGrdExpPtr_SkipWrapperPtrs();
          if (newGrdExpPtr != nullptr)
            pGrdExpPtr = newGrdExpPtr;
        }
      }
    };
    tryToReplaceByOwnerName(subjectGrdExpPtr);
    tryToReplaceByOwnerName(objectGrdExpPtr);
  }

  const SemanticAgentGrounding* subjectAgentGrd = nullptr;
  const SemanticNameGrounding* subjectNameGrdPtr = nullptr;
  _getAgentOrNameGrd(&subjectAgentGrd, &subjectNameGrdPtr, *subjectGrdExpPtr);
  const SemanticAgentGrounding* objectAgentGrd = nullptr;
  const SemanticNameGrounding* objectNameGrdPtr = nullptr;
  _getAgentOrNameGrd(&objectAgentGrd, &objectNameGrdPtr, *objectGrdExpPtr);

  if (subjectAgentGrd != nullptr && subjectAgentGrd->isSpecificUser())
  {
    if (objectNameGrdPtr != nullptr)
    {
      if (ConceptSet::extractUserId(pObjectUserId, objectNameGrdPtr->concepts))
      {
        pSubjectUserId = subjectAgentGrd->userId;
        return true;
      }
    }
    else if (objectAgentGrd != nullptr)
    {
      if (objectAgentGrd->isSpecificUser())
      {
        pSubjectUserId = subjectAgentGrd->userId;
        pObjectUserId = objectAgentGrd->userId;
        return true;
      }
    }
    else if (equCpt && SemExpGetter::isASpecificHumanFromGrdExp(*objectGrdExpPtr))
    {
      pSubjectUserId = subjectAgentGrd->userId;
      pObjectGrdExpPtr = objectGrdExpPtr;
      return true;
    }
  }
  else if (objectAgentGrd != nullptr && objectAgentGrd->isSpecificUser())
  {
    if (subjectNameGrdPtr != nullptr)
    {
      if (ConceptSet::extractUserId(pSubjectUserId, subjectNameGrdPtr->concepts))
      {
        pObjectUserId = objectAgentGrd->userId;
        return true;
      }
    }
    else if (subjectAgentGrd != nullptr)
    {
      if (subjectAgentGrd->isSpecificUser())
      {
        pSubjectUserId = subjectAgentGrd->userId;
        pObjectUserId = objectAgentGrd->userId;
        return true;
      }
    }
    else if (SemExpGetter::isASpecificHumanFromGrdExp(*subjectGrdExpPtr))
    {
      pSubjectGrdExpPtr = subjectGrdExpPtr;
      pObjectUserId = objectAgentGrd->userId;
      return true;
    }
  }
  else if (subjectNameGrdPtr != nullptr)
  {
    if (ConceptSet::extractUserId(pSubjectUserId, subjectNameGrdPtr->concepts))
    {
      pObjectGrdExpPtr = objectGrdExpPtr;
      return true;
    }
  }
  else if (objectNameGrdPtr != nullptr)
  {
    if (ConceptSet::extractUserId(pObjectUserId, objectNameGrdPtr->concepts))
    {
      pSubjectGrdExpPtr = subjectGrdExpPtr;
      return true;
    }
  }

  pSubjectGrdExpPtr = subjectGrdExpPtr;
  return false;
}


bool isANameAssignement(const GroundedExpression& pGrdExp)
{
  std::string subjectUserId;
  std::string objectUserId;
  const GroundedExpression* subjectGrdExpPtr = nullptr;
  const GroundedExpression* objectGrdExpPtr = nullptr;
  return getSubjectAndObjectUserIdsOfNameAssignement(subjectUserId, objectUserId,
                                                     subjectGrdExpPtr, objectGrdExpPtr,
                                                     pGrdExp);
}


UniqueSemanticExpression returnAPositiveSemExpBasedOnAnInput(const GroundedExpression& pInputGrdExp)
{
  if (pInputGrdExp.children.empty() &&
      ConceptSet::haveAConceptThatBeginWith(pInputGrdExp.grounding().concepts, "sentiment_positive_"))
    return pInputGrdExp.clone();
  return std::make_unique<GroundedExpression>(std::make_unique<SemanticConceptualGrounding>("sentiment_positive_joy"));
}


bool extractExternalTeachingLabel(const GroundedExpression& pGrdExp,
                                  const SemanticStatementGrounding& pStatGrd,
                                  const SemanticExpression*& pExternalTeachingLabelSemExpPtr,
                                  SemanticLanguageEnum& pTeachingLanguage,
                                  const std::string& pAuthorUserId)
{
  if (pStatGrd.verbTense != SemanticVerbTense::FUTURE ||
      pAuthorUserId.empty() ||
      !SemExpGetter::agentIsTheSubject(pGrdExp, pAuthorUserId))
    return false;

  auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
  if (itObject == pGrdExp.children.end())
    return false;
  const SemanticExpression& objectSemExp = *itObject->second;
  auto objectGrdExPtr = objectSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (objectGrdExPtr == nullptr)
    return false;

  const SemanticStatementGrounding* subStatGrdPtr = objectGrdExPtr->grounding().getStatementGroundingPtr();
  if (subStatGrdPtr == nullptr ||
      subStatGrdPtr->verbTense != SemanticVerbTense::UNKNOWN)
    return false;

  pExternalTeachingLabelSemExpPtr = &objectSemExp;
  pTeachingLanguage = subStatGrdPtr->word.language;
  return true;
}


bool isAnExtractExternalTeaching(const GroundedExpression& pGrdExp,
                                  const std::string& pAuthorUserId)
{
  const auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr == nullptr ||
      !ConceptSet::haveAConcept(statGrdPtr->concepts, "verb_action_teach"))
    return false;
  const SemanticExpression* externalTeachingLabelSemExpPtr = nullptr;
  SemanticLanguageEnum teachingLanguage = SemanticLanguageEnum::UNKNOWN;
  return extractExternalTeachingLabel(pGrdExp, *statGrdPtr,
                                      externalTeachingLabelSemExpPtr, teachingLanguage,
                                      pAuthorUserId);
}


bool isWhoSemExp(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const SemanticGenericGrounding* genGrdPtr = (*grdExpPtr)->getGenericGroundingPtr();
    if (genGrdPtr != nullptr)
    {
      const SemanticGenericGrounding& genGrd = *genGrdPtr;
      return genGrd.word.isEmpty() &&
          !genGrd.coreference &&
          genGrd.entityType == SemanticEntityType::HUMAN &&
          ConceptSet::haveAConcept(genGrd.concepts, "agent");
    }
  }
  return false;
}

bool isAnything(const GroundedExpression& pGrdExpToLookFor)
{
  const std::map<std::string, char>& conceptOfTheGrdExp = pGrdExpToLookFor->concepts;
  if (conceptOfTheGrdExp.empty())
  {
    const SemanticGenericGrounding* genGrdPtr = pGrdExpToLookFor->getGenericGroundingPtr();
    if (genGrdPtr != nullptr)
    {
      const SemanticGenericGrounding& genGrd = *genGrdPtr;
      return genGrd.word.isEmpty() &&
          !genGrd.coreference &&
          genGrd.referenceType == SemanticReferenceType::INDEFINITE;
    }
    return false;
  }

  return ConceptSet::haveAConceptOrAHyponym(conceptOfTheGrdExp, "stuff");
}


bool isAnythingFromSemExp(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return isAnything(*grdExpPtr);
  return false;
}


void getStatementSubordinates(std::set<const SemanticExpression*>& pStatementSubordinates,
                              const SemanticExpression& pSemExp,
                              bool pSearchOnRootSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    auto& grdExp = *grdExpPtr;
    if (pSearchOnRootSemExp &&
        grdExp->type == SemanticGroundingType::STATEMENT)
      pStatementSubordinates.insert(&pSemExp);

    auto itSpec = grdExp.children.find(GrammaticalType::SPECIFIER);
    if (itSpec != grdExp.children.end())
      getStatementSubordinates(pStatementSubordinates, *itSpec->second, true);
    return;
  }

  auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (const auto& currElt : listExpPtr->elts)
      getStatementSubordinates(pStatementSubordinates, *currElt, pSearchOnRootSemExp);
}

GrammaticalType invertGrammaticalType(GrammaticalType pGrammaticalType)
{
  if (pGrammaticalType == GrammaticalType::SUBJECT)
    return GrammaticalType::OBJECT;
  if (pGrammaticalType == GrammaticalType::OBJECT)
    return GrammaticalType::SUBJECT;
  return pGrammaticalType;
}


std::unique_ptr<GroundedExpression> getASimplifiedVersionFromGrdExp(
    const GroundedExpression& pGrdExp)
{
  auto res = std::make_unique<GroundedExpression>(pGrdExp.cloneGrounding());
  if (pGrdExp->type != SemanticGroundingType::AGENT &&
      pGrdExp->type != SemanticGroundingType::NAME)
    for (const auto& currChild : pGrdExp.children)
      res->children.emplace(currChild.first, getASimplifiedVersion(*currChild.second));
  return res;
}

UniqueSemanticExpression getASimplifiedVersion(
    const SemanticExpression& pSemExp)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    auto& grdExp = pSemExp.getGrdExp();
    return getASimplifiedVersionFromGrdExp(grdExp);
  }
  case SemanticExpressionType::LIST:
  {
    auto& listExp = pSemExp.getListExp();
    auto res = std::make_unique<ListExpression>(listExp.listType);
    for (const auto& currRefElt : listExp.elts)
      res->elts.emplace_back(getASimplifiedVersion(*currRefElt));
    return std::move(res);
  }
  case SemanticExpressionType::CONDITION:
  {
    auto& condExp = pSemExp.getCondExp();
    auto res = std::make_unique<ConditionExpression>
        (condExp.isAlwaysActive,
         condExp.conditionPointsToAffirmations,
         getASimplifiedVersion(*condExp.conditionExp),
         getASimplifiedVersion(*condExp.thenExp));
    if (condExp.elseExp)
      res->elseExp.emplace(getASimplifiedVersion(**condExp.elseExp));
    return std::move(res);
  }
  case SemanticExpressionType::COMPARISON:
  {
    const auto& compExp = pSemExp.getCompExp();
    auto res = std::make_unique<ComparisonExpression>
        (compExp.op, getASimplifiedVersion(*compExp.leftOperandExp));
    res->tense = compExp.tense;
    res->request = compExp.request;
    if (compExp.whatIsComparedExp)
      res->whatIsComparedExp.emplace(getASimplifiedVersion(**compExp.whatIsComparedExp));
    if (compExp.rightOperandExp)
      res->rightOperandExp.emplace(getASimplifiedVersion(**compExp.rightOperandExp));
    return std::move(res);
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const auto& intExp = pSemExp.getIntExp();
    return getASimplifiedVersion(*intExp.interpretedExp);
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const auto& fdkExp = pSemExp.getFdkExp();
    return getASimplifiedVersion(*fdkExp.concernedExp);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const auto& annExp = pSemExp.getAnnExp();
    return getASimplifiedVersion(*annExp.semExp);
  }
  case SemanticExpressionType::METADATA:
  {
    const auto& metadataExp = pSemExp.getMetadataExp();
    return getASimplifiedVersion(*metadataExp.semExp);
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    const auto& setOfFromsExp = pSemExp.getSetOfFormsExp();
    auto* originalFromPtr = setOfFromsExp.getOriginalForm();
    if (originalFromPtr != nullptr)
      return getASimplifiedVersion(**originalFromPtr);
    break;
  }
  case SemanticExpressionType::COMMAND:
  {
    const auto& cmdExp = pSemExp.getCmdExp();
    return getASimplifiedVersion(*cmdExp.semExp);
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const auto& fSynthExp = pSemExp.getFSynthExp();
    return getASimplifiedVersion(fSynthExp.getSemExp());
  }
  }
  return {};
}



bool hasGenericConcept(const UniqueSemanticExpression* pUSemExpPtr)
{
  if (pUSemExpPtr == nullptr)
    return false;
  auto* grdExpPtr = pUSemExpPtr->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
  return grdExpPtr != nullptr &&
      ConceptSet::haveAConcept(grdExpPtr->grounding().concepts, "generic");
}


void extractAskedChildren(
    std::set<GrammaticalType>& pAskedChildren,
    const SemanticExpression& pQuestion)
{
  std::list<const GroundedExpression*> grdExpPtrs;
  pQuestion.getGrdExpPtrs_SkipWrapperLists(grdExpPtrs);
  for (const auto& currGrdExp : grdExpPtrs)
    _extractAskedChildrenFromGrdExp(pAskedChildren, *currGrdExp);
}


void extractAskedChildrenByAResource(
    std::set<GrammaticalType>& pAskedChildren,
    const SemanticExpression& pSemExp)
{
  std::list<const GroundedExpression*> grdExpPtrs;
  pSemExp.getGrdExpPtrs_SkipWrapperLists(grdExpPtrs);
  for (const auto& currGrdExp : grdExpPtrs)
    _extractAskedChildrenByAResourceFromGrdExp(pAskedChildren, *currGrdExp);
}


std::vector<GrammaticalType> requestToGrammaticalTypes(SemanticRequestType pRequestType)
{
  switch (pRequestType)
  {
  case SemanticRequestType::CAUSE:
    return {GrammaticalType::CAUSE};
  case SemanticRequestType::DURATION:
    return {GrammaticalType::DURATION};
  case SemanticRequestType::LENGTH:
    return {GrammaticalType::LENGTH};
  case SemanticRequestType::LOCATION:
    return {GrammaticalType::LOCATION};
  case SemanticRequestType::MANNER:
    return {GrammaticalType::MANNER};
  case SemanticRequestType::PURPOSE:
    return {GrammaticalType::PURPOSE};
  case SemanticRequestType::SUBJECT:
    return {GrammaticalType::SUBJECT};
  case SemanticRequestType::TIME:
    return {GrammaticalType::TIME};
  case SemanticRequestType::TOPIC:
    return {GrammaticalType::TOPIC};
  case SemanticRequestType::WAY:
    return {GrammaticalType::WAY};

  case SemanticRequestType::OBJECT:
  case SemanticRequestType::CHOICE:
    return {GrammaticalType::OBJECT};

  case SemanticRequestType::QUANTITY:
    return {GrammaticalType::OBJECT, GrammaticalType::LENGTH, GrammaticalType::LOCATION};

  case SemanticRequestType::ABOUT:
  case SemanticRequestType::ACTION:
  case SemanticRequestType::NOTHING:
  case SemanticRequestType::TIMES:
  case SemanticRequestType::VERB:
  case SemanticRequestType::YESORNO:
    return {};
  }

  return {};
}



} // End of namespace SemExpGetter

} // End of namespace onsem

