#include <onsem/texttosemantic/printer/expressionprinter.hpp>
#include <onsem/texttosemantic/printer/semlinetoprint.hpp>
#include <sstream>
#include <iostream>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>


namespace onsem
{
namespace printer
{
namespace
{
static const std::string _humanResult = "human";
static const std::string _robotResult = "robot";
static const std::string _contextResult = "context";
static const std::string _thingResult = "thing";
static const std::string _currentUserResult = "currentUser";
static const std::string _sentenceResult = "sentence";
}

struct PrinterBuffer
{
  PrinterBuffer()
    : elts(),
      offsetCurrLine(0),
      offsetNewLine(0)
  {
  }

  std::list<std::string> elts;
  std::size_t offsetCurrLine;
  std::size_t offsetNewLine;
};
void _prettyPrintSemExp(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const SemanticExpression& pSemExp);


void _flushStringStream(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const SemanticExpression* pSemExp)
{
  pLines.emplace_back(pPrinterBuff.offsetCurrLine, pPrinterBuff.elts, pSemExp);
  pPrinterBuff.offsetCurrLine = pPrinterBuff.offsetNewLine;
}


void _prettyPrintConcepts(std::list<std::string>& pElts,
                          const std::map<std::string, char>& pConcepts,
                          bool pPolarity)
{
  for (const auto& currConcept : pConcepts)
  {
    std::stringstream ss;
    ss << "concept(" << currConcept.first << ", "
       << static_cast<int>(pPolarity ? currConcept.second : -currConcept.second) << ")";
    pElts.emplace_back(ss.str());
  }
}


void _prettyPrintWord(PrinterBuffer& pPrinterBuff,
                      const SemanticWord& pWord)
{
  if (!pWord.lemma.empty())
  {
    if (pWord.partOfSpeech == PartOfSpeech::UNKNOWN)
      pPrinterBuff.elts.emplace_back("word(" + pWord.lemma + ")");
    else
      pPrinterBuff.elts.emplace_back("word(" + pWord.lemma + ", " +
                                     partOfSpeech_toStr(pWord.partOfSpeech) + ")");
  }
}

void _prettyPrintLength(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const std::string& pLabelName,
                        const SemanticLength& pLength)
{
  pLength.printLength(pPrinterBuff.elts, pLabelName);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}

void _prettyPrintDuration(std::list<SemLineToPrint>& pLines,
                          PrinterBuffer& pPrinterBuff,
                          const std::string& pDurationLabelName,
                          const SemanticDuration& pDuration)
{
  pDuration.printDuration(pPrinterBuff.elts, pDurationLabelName);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticConceptualGrounding& pGrounding)
{
  if (!pGrounding.concepts.empty())
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticLanguageGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("language(" + semanticLanguageEnum_toStr(pGrounding.language) + ")");
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticTextGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("text(\"" + pGrounding.text + "\")");
  if (!pGrounding.concepts.empty())
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}

void _prettyPrintPossibleGenders(PrinterBuffer& pPrinterBuff,
                                 const std::set<SemanticGenderType>& pPossibleGenders)
{
  if (!pPossibleGenders.empty())
  {
    std::stringstream ss;
    if (pPossibleGenders.size() == 1)
      ss << "gender(";
    else
      ss << "gendersPoss(";
    bool firstIteration = true;
    for (const auto& currGender : pPossibleGenders)
    {
      if (firstIteration)
        firstIteration = false;
      else
        ss << ", ";
      ss << semanticGenderType_toStr(currGender);
    }
    ss << ")";
    pPrinterBuff.elts.emplace_back(ss.str());
  }
}

void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticNameGrounding& pGrounding)
{
  std::string nameStr = "name(\"";
  bool firstIt = true;
  for (const auto& currName : pGrounding.nameInfos.names)
  {
    if (firstIt)
      firstIt = false;
    else
      nameStr += ", ";
    nameStr += currName;
  }
  nameStr += "\")";
  pPrinterBuff.elts.emplace_back(nameStr);
  _prettyPrintPossibleGenders(pPrinterBuff, pGrounding.nameInfos.possibleGenders);

  if (!pGrounding.concepts.empty())
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}

void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticResourceGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("label(" + pGrounding.resource.label + ")");
  if (pGrounding.resource.language != SemanticLanguageEnum::UNKNOWN)
    pPrinterBuff.elts.emplace_back("language(" + semanticLanguageEnum_toStr(pGrounding.resource.language) + ")");
  pPrinterBuff.elts.emplace_back("value(" + pGrounding.resource.value + ")");
  if (!pGrounding.concepts.empty())
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticMetaGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("refToType(\"" + semanticGroundingsType_toStr(pGrounding.refToType) + "\")");
  {
    std::stringstream ssIdParam;
    ssIdParam << "paramId(" << pGrounding.paramId << ")";
    pPrinterBuff.elts.emplace_back(ssIdParam.str());
  }
  if (!pGrounding.attibuteName.empty())
  {
    std::stringstream ssAttibuteName;
    ssAttibuteName << "attibuteName(" << pGrounding.attibuteName << ")";
    pPrinterBuff.elts.emplace_back(ssAttibuteName.str());
  }

  if (!pGrounding.concepts.empty())
  {
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  }
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticRelativeLocationGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("relativeLocation(" + semanticRelativeLocationType_toStr(pGrounding.locationType) + ")");
  if (!pGrounding.concepts.empty())
  {
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  }
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticRelativeTimeGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("relativeTime(" + semanticRelativeTimeType_toStr(pGrounding.timeType) + ")");
  if (!pGrounding.concepts.empty())
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts, pGrounding.polarity);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}

void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticRelativeDurationGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("relativeDuration(" + semanticRelativeDurationType_toStr(pGrounding.durationType) + ")");
  if (!pGrounding.concepts.empty())
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts, pGrounding.polarity);
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticStatementGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back("statement:");

  _prettyPrintWord(pPrinterBuff, pGrounding.word);
  {
    std::string requestsPrint;
    for (const auto& currRequest : pGrounding.requests.types)
    {
      if (!requestsPrint.empty())
        requestsPrint += ", ";
      requestsPrint += semanticRequestType_toStr(currRequest);
    }
    if (!requestsPrint.empty())
    {
      if (pGrounding.requests.types.size() > 1)
        pPrinterBuff.elts.emplace_back("requests(" + requestsPrint + ")");
      else
        pPrinterBuff.elts.emplace_back("request(" + requestsPrint + ")");
    }
  }
  if (!pGrounding.polarity)
  {
    pPrinterBuff.elts.emplace_back("polarity(negative)");
  }
  if (pGrounding.verbTense != SemanticVerbTense::UNKNOWN)
  {
    pPrinterBuff.elts.emplace_back("time(" +
                                   semanticVerbTense_toStr(pGrounding.verbTense) + ")");
  }
  if (pGrounding.verbGoal != VerbGoalEnum::NOTIFICATION)
  {
    pPrinterBuff.elts.emplace_back("goal(" +
                                   semVerbGoalEnum_toStr(pGrounding.verbGoal) + ")");
  }

  if (pGrounding.coreference)
  {
    pPrinterBuff.elts.emplace_back("coreference(" +
                                   coreferenceDirectionEnum_toStr(pGrounding.coreference->getDirection()) + ")");
  }

  if (pGrounding.isPassive)
  {
    if (*pGrounding.isPassive)
      pPrinterBuff.elts.emplace_back("isPassive(true)");
    else
      pPrinterBuff.elts.emplace_back("isPassive(false)");
  }

  if (!pGrounding.concepts.empty())
  {
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  }

  _flushStringStream(pLines, pPrinterBuff, nullptr);
}



void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticAgentGrounding& pGrounding)
{
  if (pGrounding.userId != SemanticAgentGrounding::userNotIdentified)
  {
    pPrinterBuff.elts.emplace_back("userId(" + pGrounding.userId + ")");
  }

  if (!pGrounding.concepts.empty())
  {
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts,
                         pGrounding.polarity);
  }

  _flushStringStream(pLines, pPrinterBuff, nullptr);
}



void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticGenericGrounding& pGrounding)
{
  _prettyPrintWord(pPrinterBuff, pGrounding.word);

  if (pGrounding.referenceType != SemanticReferenceType::UNDEFINED)
  {
    pPrinterBuff.elts.emplace_back
        ("ref(" + semanticReferenceType_toStr(pGrounding.referenceType) + ")");
  }

  if (pGrounding.coreference)
  {
    pPrinterBuff.elts.emplace_back("coreference(" +
                                   coreferenceDirectionEnum_toStr(pGrounding.coreference->getDirection()) + ")");
  }

  if (!pGrounding.polarity)
  {
    pPrinterBuff.elts.emplace_back("polarity(negative)");
  }

  std::stringstream ssQuantityToPrint;
  switch (pGrounding.quantity.type)
  {
  case SemanticQuantityType::NUMBER:
    ssQuantityToPrint << "nb, " << pGrounding.quantity.nb;
    break;
  case SemanticQuantityType::NUMBERTOFILL:
    ssQuantityToPrint << "nbToFill, " << pGrounding.quantity.paramSpec;
    break;
  case SemanticQuantityType::MOREOREQUALTHANNUMBER:
    ssQuantityToPrint << "nb >= " << pGrounding.quantity.nb;
    break;
  case SemanticQuantityType::ANYTHING:
    ssQuantityToPrint << "anything";
    break;
  case SemanticQuantityType::EVERYTHING:
    ssQuantityToPrint << "everything";
    break;
  case SemanticQuantityType::MAXNUMBER:
    ssQuantityToPrint << "maxNumber";
    break;
  case SemanticQuantityType::UNKNOWN:
    break;
  }
  if (pGrounding.quantity.subjectiveValue != SemanticSubjectiveQuantity::UNKNOWN)
  {
    if (!ssQuantityToPrint.str().empty())
      ssQuantityToPrint << ", ";
    ssQuantityToPrint << semanticSubjectiveQuantity_toStr(pGrounding.quantity.subjectiveValue);
  }
  auto quantityToPrint = ssQuantityToPrint.str();
  if (!quantityToPrint.empty())
    pPrinterBuff.elts.emplace_back("quantity(" + quantityToPrint + ")");

  if (pGrounding.entityType != SemanticEntityType::UNKNOWN)
  {
    pPrinterBuff.elts.emplace_back("type(" +
                                   semanticEntityType_toStr(pGrounding.entityType) + ")");
  }

  _prettyPrintPossibleGenders(pPrinterBuff, pGrounding.possibleGenders);

  if (!pGrounding.concepts.empty())
  {
    _prettyPrintConcepts(pPrinterBuff.elts, pGrounding.concepts, pGrounding.polarity);
  }
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticTimeGrounding& pGroundings)
{
  std::stringstream ss;
  pGroundings.date.prettyPrint(ss);
  pPrinterBuff.elts.emplace_back(ss.str());
  pGroundings.timeOfDay.printDuration(pPrinterBuff.elts, "timeOfDay");
  _prettyPrintDuration(pLines, pPrinterBuff, "length", pGroundings.length);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticLengthGrounding& pGrounding)
{
  _prettyPrintLength(pLines, pPrinterBuff, "length", pGrounding.length);
}


void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticDurationGrounding& pGrounding)
{
  _prettyPrintDuration(pLines, pPrinterBuff, "duration", pGrounding.duration);
}

void _prettyPrintTypedGroundings(std::list<SemLineToPrint>& pLines,
                                 PrinterBuffer& pPrinterBuff,
                                 const SemanticUnityGrounding& pGrounding)
{
  pPrinterBuff.elts.emplace_back(typeOfUnity_toStr(pGrounding.typeOfUnity) + "(" +
                                 pGrounding.getValueStr() + ")");
  _flushStringStream(pLines, pPrinterBuff, nullptr);
}

void _prettyPrintGroundings(std::list<SemLineToPrint>& pLines,
                            PrinterBuffer& pPrinterBuff,
                            const SemanticGrounding& pGroundings)
{
  switch (pGroundings.type)
  {
  case SemanticGroundingType::GENERIC:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getGenericGrounding());
    break;
  }
  case SemanticGroundingType::STATEMENT:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getStatementGrounding());
    break;
  }
  case SemanticGroundingType::AGENT:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getAgentGrounding());
    break;
  }
  case SemanticGroundingType::TIME:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getTimeGrounding());
    break;
  }
  case SemanticGroundingType::TEXT:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getTextGrounding());
    break;
  }
  case SemanticGroundingType::DURATION:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getDurationGrounding());
    break;
  }
  case SemanticGroundingType::LANGUAGE:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getLanguageGrounding());
    break;
  }
  case SemanticGroundingType::RESOURCE:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getResourceGrounding());
    break;
  }
  case SemanticGroundingType::LENGTH:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getLengthGrounding());
    break;
  }
  case SemanticGroundingType::META:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getMetaGrounding());
    break;
  }
  case SemanticGroundingType::NAME:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getNameGrounding());
    break;
  }
  case SemanticGroundingType::RELATIVELOCATION:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getRelLocationGrounding());
    break;
  }
  case SemanticGroundingType::RELATIVETIME:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getRelTimeGrounding());
    break;
  }
  case SemanticGroundingType::RELATIVEDURATION:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getRelDurationGrounding());
    break;
  }
  case SemanticGroundingType::CONCEPTUAL:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getConceptualGrounding());
    break;
  }
  case SemanticGroundingType::UNITY:
  {
    _prettyPrintTypedGroundings(pLines, pPrinterBuff,
                                pGroundings.getUnityGrounding());
    break;
  }
  }
}

void _printLabel(PrinterBuffer& pPrinterBuff,
                 const std::string& pLabel)
{
  pPrinterBuff.offsetNewLine += pLabel.size();
  pPrinterBuff.elts.push_back(pLabel);
}

void _prettyPrintListOfSemExps(std::list<SemLineToPrint>& pLines,
                               PrinterBuffer& pPrinterBuff,
                               const std::list<UniqueSemanticExpression>& pSemExps)
{
  bool firstIteration = true;
  for (const auto& currSemExp : pSemExps)
  {
    std::size_t pivOffsetNewLine = pPrinterBuff.offsetNewLine;
    if (pivOffsetNewLine == 0 && !firstIteration)
    {
      _flushStringStream(pLines, pPrinterBuff, nullptr);
    }

    PrinterBuffer subPrinterBuff(pPrinterBuff);
    if (pSemExps.size() > 1)
    {
      if (!firstIteration)
      {
        subPrinterBuff.offsetCurrLine = pPrinterBuff.offsetNewLine;
      }
      _printLabel(subPrinterBuff, "->");
    }
    _prettyPrintSemExp(pLines, subPrinterBuff, *currSemExp);
    firstIteration = false;
  }
}


void _prettyPrintGrdExp(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const GroundedExpression& pGrdExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  _prettyPrintGroundings(pLines, pPrinterBuff, pGrdExp.grounding());
  if (!pLines.empty())
  {
    pLines.back().semExp = &pGrdExp;
  }

  for (const auto& currChild : pGrdExp.children)
  {
    PrinterBuffer subPrinterBuffer;
    subPrinterBuffer.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuffer.offsetNewLine += subPrinterBuffer.offsetCurrLine;
    _printLabel(subPrinterBuffer, grammaticalType_toStr(currChild.first) + ":");
    _prettyPrintSemExp(pLines, subPrinterBuffer, *currChild.second);
  }
}


void _prettyPrintCondExp(std::list<SemLineToPrint>& pLines,
                         PrinterBuffer& pPrinterBuff,
                         const ConditionExpression& pCondExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  {
    _printLabel(pPrinterBuff, "condition:");
    if (pCondExp.conditionPointsToAffirmations)
    {
      std::stringstream ss;
      ss << "conditionPointsToAffirmations(";
      if (pCondExp.conditionPointsToAffirmations)
      {
        ss << "true";
      }
      else
      {
        ss << "false";
      }
      ss << ")";
      pPrinterBuff.elts.emplace_back(ss.str());
    }
    else
    {
      std::stringstream ss;
      ss << "isAlwaysActive(";
      if (pCondExp.isAlwaysActive)
      {
        ss << "true";
      }
      else
      {
        ss << "false";
      }
      ss << ")";
      pPrinterBuff.elts.emplace_back(ss.str());
    }
    _flushStringStream(pLines, pPrinterBuff, &pCondExp);
  }

  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "conditionExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pCondExp.conditionExp);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "thenExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pCondExp.thenExp);
  }
  if (pCondExp.elseExp)
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "elseExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, **pCondExp.elseExp);
  }
}

void _prettyPrintCompExp(std::list<SemLineToPrint>& pLines,
                         PrinterBuffer& pPrinterBuff,
                         const ComparisonExpression& pCompExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  {
    _printLabel(pPrinterBuff, "comparison:");
    {
      std::stringstream ss;
      ss << "comparisonType(" <<
            ComparisonOperator_toStr(pCompExp.op) << ")";
      pPrinterBuff.elts.emplace_back(ss.str());
    }
    if (pCompExp.tense != SemanticVerbTense::UNKNOWN)
    {
      std::stringstream ss;
      ss << "tense(" <<
            semanticVerbTense_toStr(pCompExp.tense) << ")";
      pPrinterBuff.elts.emplace_back(ss.str());
    }
    if (pCompExp.request != SemanticRequestType::NOTHING)
    {
      std::stringstream ss;
      ss << "request(" <<
            semanticRequestType_toStr(pCompExp.request) << ")";
      pPrinterBuff.elts.emplace_back(ss.str());
    }
    _flushStringStream(pLines, pPrinterBuff, &pCompExp);
  }
  if (pCompExp.whatIsComparedExp)
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "whatIsComparedExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, **pCompExp.whatIsComparedExp);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "leftOperandExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pCompExp.leftOperandExp);
  }
  if (pCompExp.rightOperandExp)
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "rightOperandExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, **pCompExp.rightOperandExp);
  }
}


void _prettyPrintIntExp(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const InterpretationExpression& pIntExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  {
    _printLabel(pPrinterBuff, "interpretation:");
    pPrinterBuff.elts.emplace_back("from(" + interpretationFrom_toStr(pIntExp.source) + ")");
    _flushStringStream(pLines, pPrinterBuff, &pIntExp);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "interpretedExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pIntExp.interpretedExp);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "originalExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pIntExp.originalExp);
  }
}


void _prettyPrintAnnExp(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const AnnotatedExpression& pAnnExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  {
    _printLabel(pPrinterBuff, "annotation:");
    if (pAnnExp.synthesizeAnnotations)
      pPrinterBuff.elts.emplace_back("synthesizeAnnotations(true)");
    _flushStringStream(pLines, pPrinterBuff, &pAnnExp);
  }

  for (const auto& currAnn : pAnnExp.annotations)
  {
    PrinterBuffer subPrinterBuffer;
    subPrinterBuffer.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuffer.offsetNewLine += subPrinterBuffer.offsetCurrLine;
    _printLabel(subPrinterBuffer, grammaticalType_toStr(currAnn.first) + ":");
    _prettyPrintSemExp(pLines, subPrinterBuffer, *currAnn.second);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "semExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pAnnExp.semExp);
  }
}


void _prettyPrintFdkExp(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const FeedbackExpression& pFdkExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  {
    _printLabel(pPrinterBuff, "feedback:");
    _flushStringStream(pLines, pPrinterBuff, &pFdkExp);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "feedbackExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pFdkExp.feedbackExp);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "concernedExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pFdkExp.concernedExp);
  }
}


void _prettyPrintListExp(std::list<SemLineToPrint>& pLines,
                         PrinterBuffer& pPrinterBuff,
                         const ListExpression& pListExp)
{
  pPrinterBuff.elts.emplace_back
      ("listType(" + listExpressionType_toStr(pListExp.listType) + ")");
  ++pPrinterBuff.offsetNewLine;
  _flushStringStream(pLines, pPrinterBuff, &pListExp);
  _prettyPrintListOfSemExps(pLines, pPrinterBuff, pListExp.elts);
}


void _prettyPrintMetadataExp(std::list<SemLineToPrint>& pLines,
                             PrinterBuffer& pPrinterBuff,
                             const MetadataExpression& pMetadataExp)
{
  PrinterBuffer childrenPrinterBuff;
  childrenPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenPrinterBuff.offsetNewLine;

  {
    _printLabel(pPrinterBuff, "metadata:");
    _flushStringStream(pLines, pPrinterBuff, &pMetadataExp);
  }

  pPrinterBuff.elts.emplace_back("extractorType(" + semanticSourceEnum_toStr(pMetadataExp.from) + ")");
  if (pMetadataExp.contextualAnnotation != ContextualAnnotation::PROACTIVE)
    pPrinterBuff.elts.emplace_back("contextualAnnotation(" + contextualAnnotation_toStr(pMetadataExp.contextualAnnotation) + ")");
  if (pMetadataExp.fromLanguage != SemanticLanguageEnum::UNKNOWN)
    pPrinterBuff.elts.emplace_back("fromLanguage(" + semanticLanguageEnum_toStr(pMetadataExp.fromLanguage) + ")");
  if (!pMetadataExp.fromText.empty())
    pPrinterBuff.elts.emplace_back("fromText(\"" + pMetadataExp.fromText + "\")");

  if (pMetadataExp.source)
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "source:");
    _prettyPrintSemExp(pLines, subPrinterBuff, **pMetadataExp.source);
  }

  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "semExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pMetadataExp.semExp);
  }
}


void _prettyPrintCmdExp(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const CommandExpression& pCmdExp)
{
  PrinterBuffer childrenPrinterBuff;
  childrenPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenPrinterBuff.offsetNewLine;

  _printLabel(pPrinterBuff, "command:");
  _flushStringStream(pLines, pPrinterBuff, &pCmdExp);

  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "semExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, *pCmdExp.semExp);
  }

  if (pCmdExp.description)
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "description:");
    _prettyPrintSemExp(pLines, subPrinterBuff, **pCmdExp.description);
  }
}


void _prettyPrintFSynthExp(std::list<SemLineToPrint>& pLines,
                           PrinterBuffer& pPrinterBuff,
                           const FixedSynthesisExpression& pFSynthExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  {
    _printLabel(pPrinterBuff, "fSynth:");
    _flushStringStream(pLines, pPrinterBuff, &pFSynthExp);
  }
  for (const auto& currLangToSynthesis : pFSynthExp.langToSynthesis)
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, semanticLanguageEnum_toStr(currLangToSynthesis.first) + ":");
    subPrinterBuff.elts.emplace_back("\"" + currLangToSynthesis.second + "\"");
    _flushStringStream(pLines, subPrinterBuff, nullptr);
  }
  {
    PrinterBuffer subPrinterBuff;
    subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
    subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
    _printLabel(subPrinterBuff, "semExp:");
    _prettyPrintSemExp(pLines, subPrinterBuff, pFSynthExp.getSemExp());
  }
}

void _prettyPrintSetOfFormsExp(std::list<SemLineToPrint>& pLines,
                               PrinterBuffer& pPrinterBuff,
                               const SetOfFormsExpression& pSetOfFormsExp)
{
  PrinterBuffer childrenbPrinterBuff;
  childrenbPrinterBuff.offsetNewLine = pPrinterBuff.offsetNewLine + semLineToPrint_subLabelOffsets;
  if (!pPrinterBuff.elts.empty())
    ++childrenbPrinterBuff.offsetNewLine;

  {
    pPrinterBuff.elts.emplace_back("forms:");
    _flushStringStream(pLines, pPrinterBuff, &pSetOfFormsExp);
  }

  for (const auto& currSetOfForms : pSetOfFormsExp.prioToForms)
  {
    for (const auto& currForm : currSetOfForms.second)
    {
      pPrinterBuff.elts.emplace_back("newFormOfExp:");
      {
        std::stringstream ss;
        ss << "priority(" << -currSetOfForms.first << ")";
        pPrinterBuff.elts.emplace_back(ss.str());
      }
      if (currForm->isOriginalForm)
      {
        pPrinterBuff.elts.emplace_back("isOriginalForm(true)");
      }
      _flushStringStream(pLines, pPrinterBuff, &pSetOfFormsExp);

      PrinterBuffer subPrinterBuff;
      subPrinterBuff.offsetCurrLine = childrenbPrinterBuff.offsetNewLine;
      subPrinterBuff.offsetNewLine = subPrinterBuff.offsetCurrLine;
      _prettyPrintSemExp(pLines, subPrinterBuff, *currForm->exp);
    }
  }
}



void _prettyPrintSemExp(std::list<SemLineToPrint>& pLines,
                        PrinterBuffer& pPrinterBuff,
                        const SemanticExpression& pSemExp)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const GroundedExpression& grdExp = pSemExp.getGrdExp();
    _prettyPrintGrdExp(pLines, pPrinterBuff, grdExp);
    return;
  }
  case SemanticExpressionType::CONDITION:
  {
    const ConditionExpression& condExp = pSemExp.getCondExp();
    _prettyPrintCondExp(pLines, pPrinterBuff, condExp);
    return;
  }
  case SemanticExpressionType::COMPARISON:
  {
    const ComparisonExpression& compExp = pSemExp.getCompExp();
    _prettyPrintCompExp(pLines, pPrinterBuff, compExp);
    return;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const InterpretationExpression& intExp = pSemExp.getIntExp();
    _prettyPrintIntExp(pLines, pPrinterBuff, intExp);
    return;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const FeedbackExpression& fdkExp = pSemExp.getFdkExp();
    _prettyPrintFdkExp(pLines, pPrinterBuff, fdkExp);
    return;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const AnnotatedExpression& annExp = pSemExp.getAnnExp();
    _prettyPrintAnnExp(pLines, pPrinterBuff, annExp);
    return;
  }
  case SemanticExpressionType::LIST:
  {
    const ListExpression& listExp = pSemExp.getListExp();
    _prettyPrintListExp(pLines, pPrinterBuff, listExp);
    return;
  }
  case SemanticExpressionType::METADATA:
  {
    const MetadataExpression& metaDataExp = pSemExp.getMetadataExp();
    _prettyPrintMetadataExp(pLines, pPrinterBuff, metaDataExp);
    return;
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    const SetOfFormsExpression& setOfFormsExp = pSemExp.getSetOfFormsExp();
    _prettyPrintSetOfFormsExp(pLines, pPrinterBuff, setOfFormsExp);
    return;
  }
  case SemanticExpressionType::COMMAND:
  {
    const CommandExpression& cmdExp = pSemExp.getCmdExp();
    _prettyPrintCmdExp(pLines, pPrinterBuff, cmdExp);
    return;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const FixedSynthesisExpression& fSynthExp = pSemExp.getFSynthExp();
    _prettyPrintFSynthExp(pLines, pPrinterBuff, fSynthExp);
    return;
  }
  }
  assert(false);
}



void oneWordPrint(std::string& pRes,
                  const UniqueSemanticExpression& pSemanticExp,
                  const std::string& pCurrentUserId)
{
  const GroundedExpression* grdExp = pSemanticExp->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp != nullptr)
  {
    const SemanticGrounding& grdExpGrounding = grdExp->grounding();
    switch (grdExpGrounding.type)
    {
    case SemanticGroundingType::GENERIC:
    {
      const SemanticGenericGrounding& genGr = grdExpGrounding.getGenericGrounding();
      if (genGr.entityType == SemanticEntityType::AGENTORTHING ||
          genGr.entityType == SemanticEntityType::HUMAN)
      {
        pRes = _humanResult;
        return;
      }
      if (genGr.entityType == SemanticEntityType::ROBOT)
      {
        pRes = _robotResult;
        return;
      }
      if (genGr.coreference &&
          genGr.coreference->getDirection() == CoreferenceDirectionEnum::BEFORE &&
          genGr.word.isEmpty())
      {
        pRes = _contextResult;
        return;
      }
      pRes = _thingResult;
      return;
    }
    case SemanticGroundingType::AGENT:
    {
      const SemanticAgentGrounding& agentGr = grdExpGrounding.getAgentGrounding();
      if (agentGr.userId == pCurrentUserId)
      {
        pRes = _currentUserResult;
        return;
      }
      if (agentGr.userId == SemanticAgentGrounding::me)
      {
        pRes = _robotResult;
        return;
      }
      pRes = _humanResult;
      return;
    }
    case SemanticGroundingType::NAME:
    {
      pRes = _humanResult;
      return;
    }
    case SemanticGroundingType::STATEMENT:
    {
      pRes = _sentenceResult;
      return;
    }
    default:
      break;
    }
  }

  const ListExpression* listExp = pSemanticExp->getListExpPtr();
  if (listExp != nullptr &&
      !listExp->elts.empty())
  {
    oneWordPrint(pRes, listExp->elts.front(), pCurrentUserId);
    return;
  }

  pRes = semanticEntityType_toStr(SemanticEntityType::THING);
}


void prettyPrintSemExp(std::list<SemLineToPrint>& pLines,
                       const SemanticExpression& pSemExp)
{
  PrinterBuffer printerBuff;
  _prettyPrintSemExp(pLines, printerBuff, pSemExp);
}


} // End of namespace printer
} // End of namespace onsem
