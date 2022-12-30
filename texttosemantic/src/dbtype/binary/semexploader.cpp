#include <onsem/texttosemantic/dbtype/binary/semexploader.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>

namespace onsem
{
namespace semexploader
{
namespace
{
union BinaryHeader
{
  int intValues[1];
  char charValues[4];
};


void _loadConcepts(
    std::map<std::string, char>& pConcepts,
    const unsigned char*& pPtr,
    const StaticConceptSet& pCptSet)
{
  const bool writeCptIds = binaryloader::loadChar_6(pPtr);
  const bool writeCptStrings = binaryloader::loadChar_7(pPtr);
  ++pPtr;
  if (writeCptIds)
  {
    const unsigned char nbCptIds = *(pPtr++);
    for (unsigned char i = 0; i < nbCptIds; ++i)
    {
      std::string cptName = pCptSet.conceptName(binaryloader::alignedDecToInt(binaryloader::loadInt(pPtr)));
      pPtr += 3;
      pConcepts.emplace(std::move(cptName), *(pPtr++));
    }
  }
  if (writeCptStrings)
  {
    const unsigned char nbCptStrs = *(pPtr++);
    for (unsigned char i = 0; i < nbCptStrs; ++i)
    {
      std::string cptName = binaryloader::loadString(pPtr);
      pConcepts.emplace(std::move(cptName), *(pPtr++));
    }
  }
}

std::unique_ptr<SemanticGrounding> _loadMotherClass(
    const unsigned char*& pPtr,
    const StaticConceptSet& pCptSet)
{
  SemanticGroundingType grdType = semanticGroundingsType_fromChar
      (binaryloader::loadChar_0To4(pPtr));
  auto res = SemanticGrounding::make(grdType);
  if (grdType == SemanticGroundingType::AGENT)
  {
    ++pPtr;
    return res;
  }
  res->polarity = binaryloader::loadChar_5(pPtr);
  _loadConcepts(res->concepts, pPtr, pCptSet);
  return res;
}


void _loadCoreference(mystd::optional<Coreference>& pCoreference,
                      const unsigned char* pUChar)
{
  bool hasACoreference = binaryloader::loadChar_3(pUChar);
  if (hasACoreference)
    pCoreference.emplace(coreferenceDirectionEnum_fromChar(binaryloader::loadChar_4To7(pUChar)));
}


void _loadWord(SemanticWord& pWord,
               const unsigned char*& pPtr,
               const linguistics::LinguisticDatabase& pLingDb)
{
  if (binaryloader::loadChar_0(pPtr)) // if a word is written
  {
    const bool writeWordId = binaryloader::loadChar_1(pPtr);
    pWord.language = semanticLanguageEnum_fromChar(binaryloader::loadChar_2To7(pPtr));
    ++pPtr;
    if (writeWordId) // if a word id is written
    {
      auto& statDico = pLingDb.langToSpec[pWord.language].lingDico.statDb;
      statDico.getSemanticWord(pWord, binaryloader::alignedDecToInt(binaryloader::loadInt(pPtr)));
      pPtr += 3;
    }
    else
    {
      pWord.lemma = binaryloader::loadString(pPtr);
      pWord.partOfSpeech = partOfSpeech_fromChar(*(pPtr++));
    }
  }
}


void _loadGenders(
    std::set<SemanticGenderType>& pGenders,
    const unsigned char*& pPtr)
{
  unsigned char nbOfGenders = binaryloader::loadChar_1To2(pPtr);
  if (nbOfGenders > 0)
  {
    pGenders.insert(semanticGenderType_fromChar(binaryloader::loadChar_4To5(pPtr)));
    --nbOfGenders;
    if (nbOfGenders > 0)
    {
      pGenders.insert(semanticGenderType_fromChar(binaryloader::loadChar_6To7(pPtr)));
      ++pPtr;
      --nbOfGenders;
      if (nbOfGenders > 0)
      {
        pGenders.insert(semanticGenderType_fromChar(binaryloader::loadChar_0To1(pPtr)));
        --nbOfGenders;
        if (nbOfGenders > 0)
        {
          pGenders.insert(semanticGenderType_fromChar(binaryloader::loadChar_2To3(pPtr)));
          assert(nbOfGenders == 1);
        }
        ++pPtr;
      }
    }
    else
    {
      ++pPtr;
    }
  }
  else
  {
    ++pPtr;
  }
}


void _loadNameInfos(NameInfos& pNameInfos,
                    const unsigned char*& pPtr)
{
  _loadGenders(pNameInfos.possibleGenders, pPtr);
  unsigned char nbOfNames = *(pPtr++);
  pNameInfos.names.resize(nbOfNames);
  for (unsigned char nameId = 0; nameId < nbOfNames; ++nameId)
    pNameInfos.names[nameId] = binaryloader::loadString(pPtr);
}


int _loadInt(const unsigned char*& pPtr)
{
  auto resPtr = pPtr;
  pPtr += 4;
  return binaryloader::loadInt(resPtr);
}

int _loadCharOrInt(const unsigned char*& pPtr,
                   bool pCharOrInt)
{
  if (pCharOrInt)
    return static_cast<int>(*((const char*)(pPtr++)));
  return _loadInt(pPtr);
}


void _loadAngle(SemanticAngle& pAngle,
                const unsigned char*& pPtr)
{
  unsigned char nbOfAngleInfos = binaryloader::loadChar_0To6(pPtr);
  ++pPtr;
  for (unsigned char i = 0; i < nbOfAngleInfos; ++i)
  {
    const bool charOrInt = binaryloader::loadChar_0(pPtr);
    auto angleUnity = semanticAngleUnity_fromChar(binaryloader::loadChar_1To7(pPtr));
    ++pPtr;
    pAngle.angleInfos.emplace(angleUnity, _loadCharOrInt(pPtr, charOrInt));
  }
}

void _loadLength(SemanticLength& pLength,
                 const unsigned char*& pPtr)
{
  unsigned char nbOflengthInfos = binaryloader::loadChar_0To6(pPtr);
  ++pPtr;
  for (unsigned char i = 0; i < nbOflengthInfos; ++i)
  {
    const bool charOrInt = binaryloader::loadChar_0(pPtr);
    auto lengthUnity = semanticLengthUnity_fromChar(binaryloader::loadChar_1To7(pPtr));
    ++pPtr;
    pLength.lengthInfos.emplace(lengthUnity, _loadCharOrInt(pPtr, charOrInt));
  }
}


void _loadDuration(SemanticDuration& pDuration,
                   const unsigned char*& pPtr)
{
  pDuration.sign = binaryloader::loadChar_0(pPtr) ?
        SemanticDurationSign::POSITIVE : SemanticDurationSign::NEGATIVE;
  unsigned char nbOfTimeInfos = binaryloader::loadChar_1To7(pPtr);
  ++pPtr;
  for (unsigned char i = 0; i < nbOfTimeInfos; ++i)
  {
    const bool charOrInt = binaryloader::loadChar_0(pPtr);
    auto timeUnity = semanticTimeUnity_fromChar(binaryloader::loadChar_1To7(pPtr));
    ++pPtr;
    pDuration.timeInfos.emplace(timeUnity, _loadCharOrInt(pPtr, charOrInt));
  }
}


std::unique_ptr<SemanticGrounding> _loadGrd(
    const unsigned char*& pPtr,
    const linguistics::LinguisticDatabase& pLingDb)
{
  auto res = _loadMotherClass(pPtr, pLingDb.conceptSet.statDb);
  switch (res->type)
  {
  case SemanticGroundingType::GENERIC:
  {
    auto& genGrd = res->getGenericGrounding();
    genGrd.referenceType = semanticReferenceType_fromchar(binaryloader::loadChar_0To2(pPtr));
    _loadCoreference(genGrd.coreference, pPtr++);
    genGrd.entityType = semanticEntityType_fromChar(binaryloader::loadChar_0To5(pPtr));
    if (binaryloader::loadChar_6(pPtr)) // if quantity is written
    {
      bool nbWrittenInACharOrInAInt = binaryloader::loadChar_7(pPtr++);
      genGrd.quantity.type = semanticQuantityType_fromChar(binaryloader::loadChar_0To3(pPtr));
      genGrd.quantity.subjectiveValue = semanticSubjectiveQuantity_fromChar(binaryloader::loadChar_4To7(pPtr));
      ++pPtr;
      genGrd.quantity.nb.value = _loadCharOrInt(pPtr, nbWrittenInACharOrInAInt);
      genGrd.quantity.nb.valueAfterTheDecimalPoint = nbWrittenInACharOrInAInt ? 0 : _loadInt(pPtr);
      genGrd.quantity.nb.nbOfSignificantDigit = nbWrittenInACharOrInAInt ? 0u : *(pPtr++);
      genGrd.quantity.paramSpec = binaryloader::loadString(pPtr);
    }
    else if (!binaryloader::loadChar_7(pPtr++)) // if quantity is 1
    {
      genGrd.quantity.setNumber(1);
    }
    _loadWord(genGrd.word, pPtr, pLingDb);
    _loadGenders(genGrd.possibleGenders, pPtr);
    return res;
  }
  case SemanticGroundingType::STATEMENT:
  {
    auto& statGrd = res->getStatementGrounding();
    _loadWord(statGrd.word, pPtr, pLingDb);
    const bool verbGoalAndCoreferenceAndIsPassiveAreWritten = binaryloader::loadChar_1(pPtr);
    unsigned char nbOfRequests = binaryloader::loadChar_2To3(pPtr);
    statGrd.verbTense = semanticVerbTense_fromChar(binaryloader::loadChar_4To7(pPtr));
    ++pPtr;
    if (verbGoalAndCoreferenceAndIsPassiveAreWritten)
    {
      statGrd.verbGoal = semVerbGoalEnum_fromChar(binaryloader::loadChar_0To2(pPtr));
      _loadCoreference(statGrd.coreference, pPtr++);
      bool hasIsValueInformation = binaryloader::loadChar_1(pPtr);
      if (hasIsValueInformation)
        statGrd.isPassive.emplace(binaryloader::loadChar_2(pPtr));
      pPtr++;
    }
    for (unsigned char idRequest = 0; idRequest < nbOfRequests; ++idRequest)
      statGrd.requests.addWithoutCollisionCheck(semanticRequestType_fromChar(*(pPtr++)));
    return res;
  }
  case SemanticGroundingType::AGENT:
  {
    auto userId = binaryloader::loadString(pPtr);
    auto agentGrd = std::make_unique<SemanticAgentGrounding>
        (userId, binaryloader::loadString(pPtr));
    const bool writeNameInfos = binaryloader::loadChar_0(pPtr);
    if (writeNameInfos)
    {
      agentGrd->nameInfos.emplace();
      _loadNameInfos(*agentGrd->nameInfos, pPtr);
    }
    else
    {
      ++pPtr;
    }
    return agentGrd;
  }
  case SemanticGroundingType::ANGLE:
  {
    auto& angleGrd = res->getAngleGrounding();
    _loadAngle(angleGrd.angle, pPtr);
    return res;
  }
  case SemanticGroundingType::NAME:
  {
    auto& nameGrd = res->getNameGrounding();
    _loadNameInfos(nameGrd.nameInfos, pPtr);
    return res;
  }
  case SemanticGroundingType::TIME:
  {
    auto& timeGrd = res->getTimeGrounding();
    const bool writeYear = binaryloader::loadChar_0(pPtr);
    const bool writeYearInAChar = binaryloader::loadChar_1(pPtr);
    const bool writeMonth = binaryloader::loadChar_2(pPtr);
    const bool writeMonthInAChar = binaryloader::loadChar_3(pPtr);
    const bool writeDay = binaryloader::loadChar_4(pPtr);
    const bool writeDayInAChar = binaryloader::loadChar_5(pPtr);
    ++pPtr;
    if (writeYear)
      timeGrd.date.year.emplace(_loadCharOrInt(pPtr, writeYearInAChar));
    if (writeMonth)
      timeGrd.date.month.emplace(_loadCharOrInt(pPtr, writeMonthInAChar));
    if (writeDay)
      timeGrd.date.day.emplace(_loadCharOrInt(pPtr, writeDayInAChar));
    _loadDuration(timeGrd.timeOfDay, pPtr);
    _loadDuration(timeGrd.length, pPtr);
    _loadConcepts(timeGrd.fromConcepts, pPtr, pLingDb.conceptSet.statDb);
    return res;
  }
  case SemanticGroundingType::DURATION:
  {
    auto& durationGrd = res->getDurationGrounding();
    _loadDuration(durationGrd.duration, pPtr);
    return res;
  }
  case SemanticGroundingType::TEXT:
  {
    auto& textGrd = res->getTextGrounding();
    textGrd.text = binaryloader::loadString(pPtr);
    textGrd.forLanguage = semanticLanguageEnum_fromChar(binaryloader::loadChar_0To6(pPtr));
    textGrd.hasQuotationMark = binaryloader::loadChar_7(pPtr);
    ++pPtr;
    return res;
  }
  case SemanticGroundingType::LENGTH:
  {
    auto& lengthGrd = res->getLengthGrounding();
    _loadLength(lengthGrd.length, pPtr);
    return res;
  }
  case SemanticGroundingType::META:
  {
    auto& metaGrd = res->getMetaGrounding();
    metaGrd.refToType = semanticGroundingsType_fromChar(binaryloader::loadChar_0To6(pPtr));
    const bool paramIdIsWrittenInAChar = binaryloader::loadChar_7(pPtr);
    ++pPtr;
    metaGrd.paramId = _loadCharOrInt(pPtr, paramIdIsWrittenInAChar);
    metaGrd.attibuteName = binaryloader::loadString(pPtr);
    return res;
  }
  case SemanticGroundingType::RESOURCE:
  {
    auto& resGrd = res->getResourceGrounding();
    resGrd.resource.label = binaryloader::loadString(pPtr);
    resGrd.resource.language = semanticLanguageEnum_fromChar(*(pPtr++));
    resGrd.resource.value = binaryloader::loadString(pPtr);
    return res;
  }
  case SemanticGroundingType::LANGUAGE:
  {
    auto& langGrd = res->getLanguageGrounding();
    langGrd.language = semanticLanguageEnum_fromChar(*(pPtr++));
    return res;
  }
  case SemanticGroundingType::CONCEPTUAL:
    return res;
  case SemanticGroundingType::RELATIVETIME:
  {
    auto& relTimeGrd = res->getRelTimeGrounding();
    relTimeGrd.timeType = semanticRelativeTimeType_fromChar(*(pPtr++));
    return res;
  }
  case SemanticGroundingType::RELATIVEDURATION:
  {
    auto& relDurationGrd = res->getRelDurationGrounding();
    relDurationGrd.durationType = semanticRelativeDurationType_fromChar(*(pPtr++));
    return res;
  }
  case SemanticGroundingType::RELATIVELOCATION:
  {
    auto& relLocationGrd = res->getRelLocationGrounding();
    relLocationGrd.locationType = semanticRelativeLocationType_fromChar(*(pPtr++));
    return res;
  }
  case SemanticGroundingType::UNITY:
  {
    auto& unityGrd = res->getUnityGrounding();
    unityGrd.typeOfUnity = typeOfUnity_fromChar(*(pPtr++));
    unityGrd.value = *(pPtr++);
    return res;
  }
  }
  assert(false);
  return res;
}


void _loadChildren(std::map<GrammaticalType, UniqueSemanticExpression>& pChildren,
                   const unsigned char*& pPtr,
                   const linguistics::LinguisticDatabase& pLingDb)
{
  unsigned char nbOfChildren = *(pPtr++);
  for (unsigned char i = 0; i < nbOfChildren; ++i)
  {
    GrammaticalType grammType  = grammaticalType_fromChar(*(pPtr++));
    pChildren.emplace(grammType, loadSemExp(pPtr, pLingDb));
  }
}


void _loadSemExpOpt(
    mystd::unique_propagate_const<UniqueSemanticExpression>& pSemExpOpt,
    const unsigned char*& pPtr,
    const linguistics::LinguisticDatabase& pLingDb)
{
  if (*(pPtr++)) // if the description is initialized
    pSemExpOpt.emplace(loadSemExp(pPtr, pLingDb));
}

}


std::unique_ptr<GroundedExpression> loadGrdExp(
    const unsigned char*& pPtr,
    const linguistics::LinguisticDatabase& pLingDb)
{
  pPtr++;
  auto res = std::make_unique<GroundedExpression>(_loadGrd(pPtr, pLingDb));
  _loadChildren(res->children, pPtr, pLingDb);
  return res;
}


UniqueSemanticExpression loadSemExp(
    const unsigned char*& pPtr,
    const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticExpressionType semExpType(semanticExpressionType_fromChar(*(pPtr++)));
  switch (semExpType)
  {
  case SemanticExpressionType::GROUNDED:
  {
    auto res = std::make_unique<GroundedExpression>(_loadGrd(pPtr, pLingDb));
    _loadChildren(res->children, pPtr, pLingDb);
    return std::move(res);
  }
  case SemanticExpressionType::LIST:
  {
    auto res = std::make_unique<ListExpression>(listExpressionType_fromChar(*(pPtr++)));
    unsigned char nbOfElts = *(pPtr++);
    for (unsigned char i = 0; i < nbOfElts; ++i)
      res->elts.emplace_back(loadSemExp(pPtr, pLingDb));
    return std::move(res);
  }
  case SemanticExpressionType::COMMAND:
  {
    auto res = std::make_unique<CommandExpression>(loadSemExp(pPtr, pLingDb));
    _loadSemExpOpt(res->description, pPtr, pLingDb);
    return std::move(res);
  }
  case SemanticExpressionType::FEEDBACK:
  {
    auto feedbackExp = loadSemExp(pPtr, pLingDb);
    return std::make_unique<FeedbackExpression>(std::move(feedbackExp), loadSemExp(pPtr, pLingDb));
  }
  case SemanticExpressionType::METADATA:
  {
    SemanticSourceEnum from = semanticSourceEnum_fromChar(*(pPtr++));
    ContextualAnnotation contextualAnnotation = contextualAnnotation_fromChar(*(pPtr++));
    SemanticLanguageEnum fromLanguage = semanticLanguageEnum_fromChar(*(pPtr++));
    std::string fromText = binaryloader::loadString(pPtr);
    unsigned char nbOfReferences = *(pPtr++);
    std::list<std::string> references;
    for (unsigned char i = 0; i < nbOfReferences; ++i)
      references.emplace_back(binaryloader::loadString(pPtr));
    mystd::unique_propagate_const<UniqueSemanticExpression> source;
    _loadSemExpOpt(source, pPtr, pLingDb);

    auto res = std::make_unique<MetadataExpression>(loadSemExp(pPtr, pLingDb));
    res->from = from;
    res->contextualAnnotation = contextualAnnotation;
    res->fromLanguage = fromLanguage;
    res->fromText = std::move(fromText);
    res->references = std::move(references);
    res->source = std::move(source);
    return std::move(res);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    bool synthesizeAnnotations = *(pPtr++);
    std::map<GrammaticalType, UniqueSemanticExpression> annotations;
    _loadChildren(annotations, pPtr, pLingDb);
    auto res = std::make_unique<AnnotatedExpression>(loadSemExp(pPtr, pLingDb));
    res->synthesizeAnnotations = synthesizeAnnotations;
    res->annotations = std::move(annotations);
    return std::move(res);
  }
  case SemanticExpressionType::CONDITION:
  {
    bool isAlwaysActive = *(pPtr++);
    bool conditionPointsToAffirmations = *(pPtr++);
    auto conditionExp = loadSemExp(pPtr, pLingDb);
    auto res = std::make_unique<ConditionExpression>
        (isAlwaysActive, conditionPointsToAffirmations,
         std::move(conditionExp), loadSemExp(pPtr, pLingDb));
    _loadSemExpOpt(res->elseExp, pPtr, pLingDb);
    return std::move(res);
  }
  case SemanticExpressionType::COMPARISON:
  {
    ComparisonOperator compOperator = ComparisonOperator_fromChar(binaryloader::loadChar_0To3(pPtr));
    SemanticVerbTense tense = semanticVerbTense_fromChar(binaryloader::loadChar_4To7(pPtr));
    ++pPtr;
    SemanticRequestType request = semanticRequestType_fromChar(*(pPtr++));
    mystd::unique_propagate_const<UniqueSemanticExpression> whatIsComparedExp;
    _loadSemExpOpt(whatIsComparedExp, pPtr, pLingDb);
    auto res = std::make_unique<ComparisonExpression>
        (compOperator, loadSemExp(pPtr, pLingDb));
    res->tense = tense;
    res->request = request;
    res->whatIsComparedExp = std::move(whatIsComparedExp);
    _loadSemExpOpt(res->rightOperandExp, pPtr, pLingDb);
    return std::move(res);
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    auto res = std::make_unique<SetOfFormsExpression>();
    unsigned char nbOfPrioToForms = *(pPtr++);
    for (unsigned char i = 0; i < nbOfPrioToForms; ++i)
    {
      int priorityValue = binaryloader::loadInt(pPtr);
      pPtr += 4;
      std::list<std::unique_ptr<QuestExpressionFrom>> qefList;
      unsigned char nbOfQuestionExpForm = *(pPtr++);
      for (unsigned char j = 0; j < nbOfQuestionExpForm; ++j)
      {
        auto semExp = loadSemExp(pPtr, pLingDb);
        qefList.emplace_back(std::make_unique<QuestExpressionFrom>
                             (std::move(semExp), *(pPtr++)));
      }
      res->prioToForms.emplace(priorityValue, std::move(qefList));
    }
    return std::move(res);
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    unsigned char nbOfLangToSynt = *(pPtr++);
    std::map<SemanticLanguageEnum, std::string> langToSynthesis;
    for (unsigned char i = 0; i < nbOfLangToSynt; ++i)
    {
      auto synthLanguage = semanticLanguageEnum_fromChar(*(pPtr++));
      langToSynthesis.emplace(synthLanguage, binaryloader::loadString(pPtr));
    }
    auto res = std::make_unique<FixedSynthesisExpression>(loadSemExp(pPtr, pLingDb));
    res->langToSynthesis = std::move(langToSynthesis);
    return std::move(res);
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    InterpretationSource source = interpretationFrom_fromChar(*(pPtr++));
    auto interpretedExp = loadSemExp(pPtr, pLingDb);
    auto originalExp = loadSemExp(pPtr, pLingDb);
    auto res = std::make_unique<InterpretationExpression>
        (source, std::move(interpretedExp), std::move(originalExp));
    return std::move(res);
  }
  }
  assert(false);
  return UniqueSemanticExpression();
}



} // End of namespace semexploader
} // End of namespace onsem
