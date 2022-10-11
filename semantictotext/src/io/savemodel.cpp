#include "savemodel.hpp"
#include <iostream>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentence.hpp>
#include <onsem/semantictotext/semanticmemory/semanticcontextaxiom.hpp>

namespace onsem
{
static const std::string trueStr = "True";
namespace serializationprivate
{
struct GrdExpLinks
{
  std::map<const SemanticExpression*, int> semExpPtrToLink{};
  std::map<const GroundedExpression*, int> grdExpPtrToLink{};
  int nextLink{1};

  int getNewLink()
  {
    return nextLink++;
  }
  int getLinkOfSemExp(const SemanticExpression& pSemExp) const
  {
    auto itSemExp = semExpPtrToLink.find(&pSemExp);
    assert(itSemExp != semExpPtrToLink.end());
    return itSemExp->second;
  }
  int getLinkOfGrdExp(const GroundedExpression& pGrdExp) const
  {
    auto itGrdExp = grdExpPtrToLink.find(&pGrdExp);
    assert(itGrdExp != grdExpPtrToLink.end());
    return itGrdExp->second;
  }
};

void _saveSemExp(boost::property_tree::ptree& pTree,
                 const SemanticExpression& pSemExp,
                 GrdExpLinks* pGrdExpLinks);




void _saveConcepts(boost::property_tree::ptree& pTree,
                   const std::map<std::string, char>& pConcepts)
{
  if (!pConcepts.empty())
  {
    boost::property_tree::ptree& conceptsChild = pTree.put_child("concepts", {});
    for (const auto& currCpt : pConcepts)
      conceptsChild.put(currCpt.first, currCpt.second);
  }
}


void _saveGrd(boost::property_tree::ptree& pTree,
              const SemanticGrounding& pGrd)
{
  if (pGrd.type != SemanticGroudingType::GENERIC)
      pTree.put("type", semanticGroudingsType_toStr(pGrd.type));
  if (!pGrd.polarity)
    pTree.put("polarity", pGrd.polarity);
  _saveConcepts(pTree, pGrd.concepts);
}


void _saveNameInfos(boost::property_tree::ptree& pTree,
                    const NameInfos& pNameInfos)
{
  {
    boost::property_tree::ptree& namesChild = pTree.put_child("names", {});
    for (const auto& currName : pNameInfos.names)
      namesChild.add("e", currName);
  }
  if (!pNameInfos.possibleGenders.empty())
  {
    boost::property_tree::ptree& gendersChild = pTree.put_child("possibleGenders", {});
    for (const auto& currGender : pNameInfos.possibleGenders)
      gendersChild.add("e", semanticGenderType_toStr(currGender));
  }
}

void _saveNameGrd(boost::property_tree::ptree& pTree,
                  const SemanticNameGrounding& pNameGrd)
{
  _saveGrd(pTree, pNameGrd);
  _saveNameInfos(pTree, pNameGrd.nameInfos);
}

void _saveUnityGrd(boost::property_tree::ptree& pTree,
                   const SemanticUnityGrounding& pUnityGrd)
{
  _saveGrd(pTree, pUnityGrd);
  pTree.put("typeOfUnity", typeOfUnity_toStr(pUnityGrd.typeOfUnity));
  pTree.put("value", pUnityGrd.getValueStr());
}

void _saveAgentGrd(boost::property_tree::ptree& pTree,
                   const SemanticAgentGrounding& pAgentGrd)
{
  _saveGrd(pTree, pAgentGrd);
  if (pAgentGrd.userId != SemanticAgentGrounding::userNotIdentified)
    pTree.put("userId", pAgentGrd.userId);
  if (pAgentGrd.nameInfos)
    _saveNameInfos(pTree.put_child("nameInfos", {}), *pAgentGrd.nameInfos);
}

void _saveSemanticLength(boost::property_tree::ptree& pTree,
                         const SemanticLength& pSemanticLength)
{
  for (const auto& currLengthInfo : pSemanticLength.lengthInfos)
    pTree.put(semanticLengthUnity_toStr(currLengthInfo.first), currLengthInfo.second);
}

void _saveSemanticDuration(boost::property_tree::ptree& pTree,
                           const SemanticDuration& pSemanticDuration)
{
  if (pSemanticDuration.sign != SemanticDurationSign::POSITIVE)
    pTree.put("sign", false);
  for (const auto& currTimeInfo : pSemanticDuration.timeInfos)
    pTree.put(semanticTimeUnity_toStr(currTimeInfo.first), currTimeInfo.second);
}

void _saveSemanticDate(boost::property_tree::ptree& pTree,
                       const SemanticDate& pSemanticDate)
{
  if (pSemanticDate.year)
    pTree.put("year", *pSemanticDate.year);
  if (pSemanticDate.month)
    pTree.put("month", *pSemanticDate.month);
  if (pSemanticDate.day)
    pTree.put("day", *pSemanticDate.day);
}

void _saveTimeGrd(boost::property_tree::ptree& pTree,
                  const SemanticTimeGrounding& pTimeGrd)
{
  _saveGrd(pTree, pTimeGrd);
  _saveSemanticDate(pTree.put_child("date", {}), pTimeGrd.date);
  _saveSemanticDuration(pTree.put_child("timeOfDay", {}), pTimeGrd.timeOfDay);
  _saveSemanticDuration(pTree.put_child("length", {}), pTimeGrd.length);
}


void _saveTextGrd(boost::property_tree::ptree& pTree,
                  const SemanticTextGrounding& pTextGrd)
{
  _saveGrd(pTree, pTextGrd);
  pTree.put("text", pTextGrd.text);
  if (pTextGrd.forLanguage != SemanticLanguageEnum::UNKNOWN)
    pTree.put("forLanguage", semanticLanguageEnum_toStr(pTextGrd.forLanguage));
  if (pTextGrd.hasQuotationMark)
    pTree.put("hasQuotationMark", trueStr);
}

void _saveLengthGrd(boost::property_tree::ptree& pTree,
                    const SemanticLengthGrounding& pLengthGrd)
{
  _saveGrd(pTree, pLengthGrd);
  _saveSemanticLength(pTree.put_child("length", {}), pLengthGrd.length);
}

void _saveDurationGrd(boost::property_tree::ptree& pTree,
                      const SemanticDurationGrounding& pDurationGrd)
{
  _saveGrd(pTree, pDurationGrd);
  _saveSemanticDuration(pTree.put_child("duration", {}), pDurationGrd.duration);
}

void _saveLanguageGrd(boost::property_tree::ptree& pTree,
                      const SemanticLanguageGrounding& pLangGrd)
{
  _saveGrd(pTree, pLangGrd);
  if (pLangGrd.language != SemanticLanguageEnum::UNKNOWN)
    pTree.put("language", semanticLanguageEnum_toStr(pLangGrd.language));
}

void _saveRelativeLocationGrd(boost::property_tree::ptree& pTree,
                              const SemanticRelativeLocationGrounding& pRelLocGrd)
{
  _saveGrd(pTree, pRelLocGrd);
  pTree.put("locationType", semanticRelativeLocationType_toStr(pRelLocGrd.locationType));
}

void _saveRelativeTimeGrd(boost::property_tree::ptree& pTree,
                          const SemanticRelativeTimeGrounding& pRelTimeGrd)
{
  _saveGrd(pTree, pRelTimeGrd);
  pTree.put("timeType", semanticRelativeTimeType_toStr(pRelTimeGrd.timeType));
}

void _saveRelativeDurationGrd(boost::property_tree::ptree& pTree,
                              const SemanticRelativeDurationGrounding& pRelDurationGrd)
{
  _saveGrd(pTree, pRelDurationGrd);
  pTree.put("durationType", semanticRelativeDurationType_toStr(pRelDurationGrd.durationType));
}

void _saveResourceGrd(boost::property_tree::ptree& pTree,
                      const SemanticResourceGrounding& pResourceGrd)
{
  _saveGrd(pTree, pResourceGrd);
  pTree.put("label", pResourceGrd.resource.label);
  if (pResourceGrd.resource.language != SemanticLanguageEnum::UNKNOWN)
    pTree.put("language", semanticLanguageEnum_toStr(pResourceGrd.resource.language));
  pTree.put("value", pResourceGrd.resource.value);
}

void _saveMetaGrd(boost::property_tree::ptree& pTree,
                  const SemanticMetaGrounding& pMetaGrd)
{
  _saveGrd(pTree, pMetaGrd);
  pTree.put("refToType", semanticGroudingsType_toStr(pMetaGrd.refToType));
  pTree.put("paramId", pMetaGrd.paramId);
  pTree.put("attibuteName", pMetaGrd.attibuteName);
}


void _saveSemanticQuantity(boost::property_tree::ptree& pTree,
                           const SemanticQuantity& pSemanticQuantity)
{
  if (pSemanticQuantity.type != SemanticQuantityType::UNKNOWN)
    pTree.put("type", semanticQuantityType_toStr(pSemanticQuantity.type));
  if (pSemanticQuantity.nb != 0)
    pTree.put("nb", pSemanticQuantity.nb);
}


void _saveSemanticWord(boost::property_tree::ptree& pTree,
                       const SemanticWord& pWord)
{
  if (pWord.language != SemanticLanguageEnum::UNKNOWN)
    pTree.put("language", semanticLanguageEnum_toStr(pWord.language));
  if (!pWord.lemma.empty())
    pTree.put("lemma", pWord.lemma);
  if (pWord.partOfSpeech != PartOfSpeech::UNKNOWN)
    pTree.put("partOfSpeech", partOfSpeech_toStr(pWord.partOfSpeech));
}


void _saveGenericGrd(boost::property_tree::ptree& pTree,
                     const SemanticGenericGrounding& pGenGrd)
{
  _saveGrd(pTree, pGenGrd);
  if (pGenGrd.referenceType != SemanticReferenceType::UNDEFINED)
    pTree.put("referenceType", semanticReferenceType_toStr(pGenGrd.referenceType));
  if (pGenGrd.coreference)
    pTree.put("coreference", coreferenceDirectionEnum_toStr(pGenGrd.coreference->getDirection()));
  if (pGenGrd.entityType != SemanticEntityType::UNKNOWN)
    pTree.put("entityType", semanticEntityType_toStr(pGenGrd.entityType));
  if (!pGenGrd.quantity.isEqualToInit())
    _saveSemanticQuantity(pTree.put_child("quantity", {}), pGenGrd.quantity);
  if (!pGenGrd.word.isEmpty())
    _saveSemanticWord(pTree.put_child("word", {}), pGenGrd.word);
  if (!pGenGrd.possibleGenders.empty())
  {
    boost::property_tree::ptree& gendersChild = pTree.put_child("possibleGenders", {});
    for (const auto& currGender : pGenGrd.possibleGenders)
      gendersChild.add("e", semanticGenderType_toStr(currGender));
  }
}


void _saveStatementGrd(boost::property_tree::ptree& pTree,
                       const SemanticStatementGrounding& pStatGrd)
{
  _saveGrd(pTree, pStatGrd);
  if (!pStatGrd.requests.empty())
  {
    boost::property_tree::ptree& requestTypesChild = pTree.put_child("requestTypes", {});
    for (const auto& currReq : pStatGrd.requests.types)
      requestTypesChild.add("e", semanticRequestType_toStr(currReq));
  }
  if (!pStatGrd.word.isEmpty())
    _saveSemanticWord(pTree.put_child("word", {}), pStatGrd.word);
  if (pStatGrd.verbTense != SemanticVerbTense::UNKNOWN)
    pTree.put("verbTense", semanticVerbTense_toStr(pStatGrd.verbTense));
  if (pStatGrd.verbGoal != VerbGoalEnum::NOTIFICATION)
    pTree.put("verbGoal", semVerbGoalEnum_toStr(pStatGrd.verbGoal));
  if (pStatGrd.coreference)
    pTree.put("coreference", coreferenceDirectionEnum_toStr(pStatGrd.coreference->getDirection()));
  if (pStatGrd.isPassive)
    pTree.put("isPassive", *pStatGrd.isPassive);
}


void _saveGrounding(boost::property_tree::ptree& pTree,
                    const SemanticGrounding& pGrouding)
{
  switch (pGrouding.type)
  {
  case SemanticGroudingType::AGENT:
    _saveAgentGrd(pTree, pGrouding.getAgentGrounding());
    return;
  case SemanticGroudingType::GENERIC:
    _saveGenericGrd(pTree, pGrouding.getGenericGrounding());
    return;
  case SemanticGroudingType::STATEMENT:
    _saveStatementGrd(pTree, pGrouding.getStatementGrounding());
    return;
  case SemanticGroudingType::TIME:
    _saveTimeGrd(pTree, pGrouding.getTimeGrounding());
    return;
  case SemanticGroudingType::TEXT:
    _saveTextGrd(pTree, pGrouding.getTextGrounding());
    return;
  case SemanticGroudingType::DURATION:
    _saveDurationGrd(pTree, pGrouding.getDurationGrounding());
    return;
  case SemanticGroudingType::LANGUAGE:
    _saveLanguageGrd(pTree, pGrouding.getLanguageGrounding());
    return;
  case SemanticGroudingType::RELATIVELOCATION:
    _saveRelativeLocationGrd(pTree, pGrouding.getRelLocationGrounding());
    return;
  case SemanticGroudingType::RELATIVETIME:
    _saveRelativeTimeGrd(pTree, pGrouding.getRelTimeGrounding());
    return;
  case SemanticGroudingType::RELATIVEDURATION:
    _saveRelativeDurationGrd(pTree, pGrouding.getRelDurationGrounding());
    return;
  case SemanticGroudingType::RESOURCE:
    _saveResourceGrd(pTree, pGrouding.getResourceGrounding());
    return;
  case SemanticGroudingType::LENGTH:
    _saveLengthGrd(pTree, pGrouding.getLengthGrounding());
    return;
  case SemanticGroudingType::META:
    _saveMetaGrd(pTree, pGrouding.getMetaGrounding());
    return;
  case SemanticGroudingType::NAME:
    _saveNameGrd(pTree, pGrouding.getNameGrounding());
    return;
  case SemanticGroudingType::CONCEPTUAL:
    _saveGrd(pTree, pGrouding);
    return;
  case SemanticGroudingType::UNITY:
    _saveUnityGrd(pTree, pGrouding.getUnityGrounding());
    return;
  }
  assert(false);
}


void _saveGrdExp(boost::property_tree::ptree& pTree,
                 const GroundedExpression& pGrdExp,
                 GrdExpLinks* pGrdExpLinks)
{
  _saveGrounding(pTree.put_child("grounding", {}), pGrdExp.grounding());
  for (const auto& currChild : pGrdExp.children)
    _saveSemExp(pTree.put_child
                (grammaticalType_toStr(currChild.first), {}), *currChild.second,
                pGrdExpLinks);
}


void _saveListExp(boost::property_tree::ptree& pTree,
                  const ListExpression& pListExp,
                  GrdExpLinks* pGrdExpLinks)
{
  if (pListExp.listType != ListExpressionType::UNRELATED)
    pTree.put("listType", listExpressionType_toStr(pListExp.listType));
  for (const auto& currElt : pListExp.elts)
    _saveSemExp(pTree.add_child("e", {}), *currElt, pGrdExpLinks);
}

void _saveConditionExp(boost::property_tree::ptree& pTree,
                       const ConditionExpression& pCondExp,
                       GrdExpLinks* pGrdExpLinks)
{
  if (pCondExp.isAlwaysActive)
    pTree.put("isAlwaysActive", trueStr);
  if (pCondExp.conditionPointsToAffirmations)
    pTree.put("conditionPointsToAffirmations", trueStr);
  _saveSemExp(pTree.put_child("conditionExp", {}), *pCondExp.conditionExp,
              pGrdExpLinks);
  _saveSemExp(pTree.put_child("thenExp", {}), *pCondExp.thenExp,
              pGrdExpLinks);
  if (pCondExp.elseExp)
    _saveSemExp(pTree.put_child("elseExp", {}), **pCondExp.elseExp,
                pGrdExpLinks);
}

void _saveComparisonExp(boost::property_tree::ptree& pTree,
                        const ComparisonExpression& pCompExp,
                        GrdExpLinks* pGrdExpLinks)
{
  pTree.put("op", ComparisonOperator_toStr(pCompExp.op));
  if (pCompExp.tense != SemanticVerbTense::PRESENT)
    pTree.put("tense", semanticVerbTense_toStr(pCompExp.tense));
  if (pCompExp.request != SemanticRequestType::NOTHING)
    pTree.put("request", semanticRequestType_toStr(pCompExp.request));
  if (pCompExp.whatIsComparedExp)
    _saveSemExp(pTree.put_child("whatIsComparedExp", {}), **pCompExp.whatIsComparedExp,
                pGrdExpLinks);
  _saveSemExp(pTree.put_child("leftOperandExp", {}), *pCompExp.leftOperandExp,
              pGrdExpLinks);
  if (pCompExp.rightOperandExp)
    _saveSemExp(pTree.put_child("rightOperandExp", {}), **pCompExp.rightOperandExp,
                pGrdExpLinks);
}

void _saveInterpretationExp(boost::property_tree::ptree& pTree,
                            const InterpretationExpression& pIntExp,
                            GrdExpLinks* pGrdExpLinks)
{
  pTree.put("source", interpretationFrom_toStr(pIntExp.source));
  _saveSemExp(pTree.put_child("interpretedExp", {}), *pIntExp.interpretedExp,
              pGrdExpLinks);
  _saveSemExp(pTree.put_child("originalExp", {}), *pIntExp.originalExp,
              pGrdExpLinks);
}


void _saveFeedbackExp(boost::property_tree::ptree& pTree,
                      const FeedbackExpression& pFdkExp,
                      GrdExpLinks* pGrdExpLinks)
{
  _saveSemExp(pTree.put_child("feedbackExp", {}), *pFdkExp.feedbackExp,
              pGrdExpLinks);
  _saveSemExp(pTree.put_child("concernedExp", {}), *pFdkExp.concernedExp,
              pGrdExpLinks);
}


void _saveAnnotatedExp(boost::property_tree::ptree& pTree,
                       const AnnotatedExpression& pAnnExp,
                       GrdExpLinks* pGrdExpLinks)
{
  _saveSemExp(pTree.put_child("semExp", {}), *pAnnExp.semExp, pGrdExpLinks);
  if (pAnnExp.synthesizeAnnotations)
    pTree.put("synthesizeAnnotations", pAnnExp.synthesizeAnnotations);
  for (const auto& currAnn : pAnnExp.annotations)
    _saveSemExp(pTree.put_child
                (grammaticalType_toStr(currAnn.first), {}), *currAnn.second,
                pGrdExpLinks);
}

void _saveMetadataExp(boost::property_tree::ptree& pTree,
                      const MetadataExpression& pMetadataExp,
                      GrdExpLinks* pGrdExpLinks)
{
  if (pMetadataExp.from != SemanticSourceEnum::UNKNOWN)
    pTree.put("from", semanticSourceEnum_toStr(pMetadataExp.from));
  if (pMetadataExp.contextualAnnotation != ContextualAnnotation::PROACTIVE)
    pTree.put("contextualAnnotation", contextualAnnotation_toStr(pMetadataExp.contextualAnnotation));
  if (pMetadataExp.fromLanguage != SemanticLanguageEnum::UNKNOWN)
    pTree.put("fromLanguage", semanticLanguageEnum_toStr(pMetadataExp.fromLanguage));
  if (!pMetadataExp.fromText.empty())
    pTree.put("fromText", pMetadataExp.fromText);
  if (!pMetadataExp.references.empty())
  {
    boost::property_tree::ptree& referencesChild = pTree.put_child("references", {});
    for (const auto& currRef : pMetadataExp.references)
      referencesChild.add("e", currRef);
  }
  if (pMetadataExp.source)
    _saveSemExp(pTree.put_child("source", {}), **pMetadataExp.source, pGrdExpLinks);
  _saveSemExp(pTree.put_child("semExp", {}), *pMetadataExp.semExp, pGrdExpLinks);
}


void _saveQuestExpressionForm(boost::property_tree::ptree& pTree,
                              const QuestExpressionFrom& pQuestForm,
                              GrdExpLinks* pGrdExpLinks)
{
  _saveSemExp(pTree.put_child("exp", {}), *pQuestForm.exp, pGrdExpLinks);
  if (pQuestForm.isOriginalForm)
    pTree.put("isOriginalForm", trueStr);
}


void _saveCommandExp(boost::property_tree::ptree& pTree,
                     const CommandExpression& pCmdExp,
                     GrdExpLinks* pGrdExpLinks)
{
  _saveSemExp(pTree.put_child("semExp", {}), *pCmdExp.semExp, pGrdExpLinks);
  if (pCmdExp.description)
    _saveSemExp(pTree.put_child("description", {}), **pCmdExp.description,
                pGrdExpLinks);
}

void _saveFSynthExp(boost::property_tree::ptree& pTree,
                    const FixedSynthesisExpression& pFSynthExp,
                    GrdExpLinks* pGrdExpLinks)
{
  if (!pFSynthExp.langToSynthesis.empty())
  {
    auto langToSynthesisTree = pTree.put_child("langToSynthesis", {});
    for (const auto& currSynth : pFSynthExp.langToSynthesis)
      langToSynthesisTree.put(semanticLanguageEnum_toStr(currSynth.first), currSynth.second);
  }
  _saveSemExp(pTree.put_child("semExp", {}), pFSynthExp.getSemExp(), pGrdExpLinks);
}

void _saveSetOfFormsExp(boost::property_tree::ptree& pTree,
                        const SetOfFormsExpression& pSetOfFormsExp,
                        GrdExpLinks* pGrdExpLinks)
{
  for (const auto& currSetOfForm : pSetOfFormsExp.prioToForms)
  {
    for (const auto& currQuestForm : currSetOfForm.second)
    {
      boost::property_tree::ptree currChild;
      _saveQuestExpressionForm(currChild, *currQuestForm, pGrdExpLinks);
      std::stringstream ss;
      ss << currSetOfForm.first;
      pTree.push_back(std::make_pair(ss.str(), currChild));
    }
  }
}


void _saveSemExp(boost::property_tree::ptree& pTree,
                 const SemanticExpression& pSemExp,
                 GrdExpLinks* pGrdExpLinks)
{
  int newLink = 0;
  if (pGrdExpLinks != nullptr)
  {
    newLink = pGrdExpLinks->getNewLink();
    pGrdExpLinks->semExpPtrToLink.emplace(&pSemExp, newLink);
    pTree.put("serializationLink", newLink);
  }
  if (pSemExp.type != SemanticExpressionType::GROUNDED)
    pTree.put("type", semanticExpressionType_toStr(pSemExp.type));
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const GroundedExpression& grdExp = pSemExp.getGrdExp();
    if (pGrdExpLinks != nullptr)
      pGrdExpLinks->grdExpPtrToLink.emplace(&grdExp, newLink);
    _saveGrdExp(pTree, grdExp, pGrdExpLinks);
    return;
  }
  case SemanticExpressionType::LIST:
    _saveListExp(pTree, pSemExp.getListExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::CONDITION:
    _saveConditionExp(pTree, pSemExp.getCondExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::COMPARISON:
    _saveComparisonExp(pTree, pSemExp.getCompExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::INTERPRETATION:
    _saveInterpretationExp(pTree, pSemExp.getIntExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::FEEDBACK:
    _saveFeedbackExp(pTree, pSemExp.getFdkExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::ANNOTATED:
    _saveAnnotatedExp(pTree, pSemExp.getAnnExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::METADATA:
    _saveMetadataExp(pTree, pSemExp.getMetadataExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::SETOFFORMS:
    _saveSetOfFormsExp(pTree, pSemExp.getSetOfFormsExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::COMMAND:
    _saveCommandExp(pTree, pSemExp.getCmdExp(), pGrdExpLinks);
    return;
  case SemanticExpressionType::FIXEDSYNTHESIS:
    _saveFSynthExp(pTree, pSemExp.getFSynthExp(), pGrdExpLinks);
    return;
  }
  assert(false);
}



void saveSemExp(boost::property_tree::ptree& pTree,
                const SemanticExpression& pSemExp)
{
  _saveSemExp(pTree.put_child("semExp", {}), pSemExp, nullptr);
}


void _saveMemorySentence(boost::property_tree::ptree& pTree,
                         const SemanticMemorySentence& pMemSentence,
                         const GrdExpLinks& pGrdExpLinks)
{
  pTree.put("id", pMemSentence.id);
  pTree.put("grdExp", pGrdExpLinks.getLinkOfGrdExp(pMemSentence.grdExp));
  if (pMemSentence.gatherAllTheLinks)
    pTree.put("gatherAllTheLinks", trueStr);
  const auto& annotations = pMemSentence.getAnnotations();
  if (!annotations.empty())
  {
    boost::property_tree::ptree& annotationsTree = pTree.put_child("annotations", {});
    for (const auto& currAnn : annotations)
      annotationsTree.put(grammaticalType_toStr(currAnn.first),
                          pGrdExpLinks.getLinkOfSemExp(*currAnn.second));
  }
  if (pMemSentence.isAConditionToSatisfy())
    pTree.put("isAConditionToSatisfy", trueStr);
  if (pMemSentence.isEnabled())
    pTree.put("isEnabled", trueStr);
}


void _saveContextAxiom(boost::property_tree::ptree& pTree,
                       const SemanticContextAxiom& pMemContextAxiom,
                       const GrdExpLinks& pGrdExpLinks)
{
  if (pMemContextAxiom.informationType != InformationType::INFORMATION)
    pTree.put("informationType", informationType_toStr(pMemContextAxiom.informationType));
  if (pMemContextAxiom.triggerAxiomId.isEmpty())
  {
    boost::property_tree::ptree& triggerAxiomIdChild = pTree.put_child("triggerAxiomId", {});
    if (pMemContextAxiom.triggerAxiomId.nbOfAxioms != 1)
      triggerAxiomIdChild.put("nbOfAxioms", pMemContextAxiom.triggerAxiomId.nbOfAxioms);
    if (pMemContextAxiom.triggerAxiomId.idOfAxiom != 0)
      triggerAxiomIdChild.put("idOfAxiom", pMemContextAxiom.triggerAxiomId.idOfAxiom);
    if (pMemContextAxiom.triggerAxiomId.listExpType != ListExpressionType::UNRELATED)
      triggerAxiomIdChild.put("listExpType", listExpressionType_toStr(pMemContextAxiom.triggerAxiomId.listExpType));
  }
  if (pMemContextAxiom.semExpToDoIsAlwaysActive)
    pTree.put("semExpToDoIsAlwaysActive", trueStr);
  if (pMemContextAxiom.semExpToDo != nullptr)
    pTree.put("semExpToDo", pGrdExpLinks.getLinkOfSemExp(*pMemContextAxiom.semExpToDo));
  if (pMemContextAxiom.semExpToDoElse != nullptr)
    pTree.put("semExpToDoElse", pGrdExpLinks.getLinkOfSemExp(*pMemContextAxiom.semExpToDoElse));
  if (pMemContextAxiom.infCommandToDo != nullptr)
    pTree.put("infCommandToDo", pGrdExpLinks.getLinkOfSemExp(*pMemContextAxiom.infCommandToDo));
  boost::property_tree::ptree& memorySentencesChild = pTree.put_child("memorySentences", {});
  if (pMemContextAxiom.memorySentences.and_or)
    memorySentencesChild.put("and_or", trueStr);
  for (const auto& currmMmorySentence : pMemContextAxiom.memorySentences.elts)
    _saveMemorySentence(memorySentencesChild.add_child("e", {}), currmMmorySentence, pGrdExpLinks);
}


void _saveMemoryKnowledge(boost::property_tree::ptree& pTree,
                          const ExpressionHandleInMemory& pMemKnowledge)
{
  GrdExpLinks grdExpLinks;
  if (!pMemKnowledge.linkedInfos.empty())
  {
    boost::property_tree::ptree& linkedInfosChild = pTree.put_child("linkedInfos", {});
    for (const auto& currLinkedInfo : pMemKnowledge.linkedInfos)
      linkedInfosChild.add(currLinkedInfo.first, currLinkedInfo.second);
  }
  _saveSemExp(pTree.put_child("semExp", {}), *pMemKnowledge.semExp, &grdExpLinks);
  boost::property_tree::ptree& contextAxiomsChild = pTree.put_child("contextAxioms", {});
  for (const auto& currContextAxiom : pMemKnowledge.contextAxioms)
    _saveContextAxiom(contextAxiomsChild.add_child("e", {}), currContextAxiom, grdExpLinks);
  if (pMemKnowledge.outputToAnswerIfTriggerHasMatched)
    _saveSemExp(pTree.put_child("outputToAnswerIfTriggerHasMatched", {}),
                **pMemKnowledge.outputToAnswerIfTriggerHasMatched, nullptr);
}


void _saveMemoryBlock(boost::property_tree::ptree& pTree,
                      const SemanticMemoryBlock& pMemBlock)
{
  if (pMemBlock.nbOfKnowledges() > 0)
  {
    boost::property_tree::ptree& expressionsMemoriesChild = pTree.put_child("expressionsMemories", {});
    for (const auto& currExpressionHandleInMemory : pMemBlock.getExpressionHandleInMemories())
      _saveMemoryKnowledge(expressionsMemoriesChild.add_child("e", {}), *currExpressionHandleInMemory);
  }
  pTree.put("maxNbOfKnowledgesInAMemoryBloc", pMemBlock.maxNbOfExpressionsInAMemoryBlock);

  const SemanticMemoryBlock* fallbacksBlockPtr = pMemBlock.getFallbacksBlockPtr();
  if (fallbacksBlockPtr != nullptr)
    _saveMemoryBlock(pTree.put_child("fallbacksBlock", {}), *fallbacksBlockPtr);
}

void _saveProativeSpecifications(boost::property_tree::ptree& pTree,
                                 const ProativeSpecifications& pProativeSpecifications)
{
  pTree.put("informTheUserHowToTeachMe", pProativeSpecifications.informTheUserHowToTeachMe);
}


void _saveSemMemory(boost::property_tree::ptree& pTree,
                     const SemanticMemory& pSemMemory)
{
  _saveMemoryBlock(pTree.put_child("memBloc", {}), pSemMemory.memBloc);
  if (pSemMemory.defaultLanguage != SemanticLanguageEnum::UNKNOWN)
    pTree.put("defaultLanguage", semanticLanguageEnum_toStr(pSemMemory.defaultLanguage));
  _saveProativeSpecifications(pTree.put_child("proativeSpecifications", {}), pSemMemory.proativeSpecifications);

  const std::map<std::string, std::list<UniqueSemanticExpression>>&
      newUserFocusedToSemExps = pSemMemory.getNewUserFocusedToSemExps();
  if (!newUserFocusedToSemExps.empty())
  {
    boost::property_tree::ptree& newUserFocusedToKnowledgesChild =
        pTree.put_child("newUserFocusedToSemExps", {});
    for (const auto& currNewUserFocusedToSemExps : newUserFocusedToSemExps)
    {
      boost::property_tree::ptree currEltChild;
      for (const auto& currSemExp : currNewUserFocusedToSemExps.second)
      {
        GrdExpLinks grdExpLinks;
        _saveSemExp(currEltChild.add_child("e", {}), *currSemExp, &grdExpLinks);
      }
      newUserFocusedToKnowledgesChild.add_child(currNewUserFocusedToSemExps.first, currEltChild);
    }
  }

  const std::string currUserId = pSemMemory.getCurrUserId();
  if (currUserId != SemanticAgentGrounding::currentUser)
    pTree.put("currUserId", currUserId);
}


void saveSemMemory(boost::property_tree::ptree& pTree,
                   const SemanticMemory& pSemMemory)
{
  _saveSemMemory(pTree.put_child("semanticMemory", {}), pSemMemory);
}


void _saveConcetSet(boost::property_tree::ptree& pTree,
                    const ConceptSet& pConceptSet)
{
  const auto& localConcepts = pConceptSet.getLocalConcepts();
  if (!localConcepts.empty())
  {
    boost::property_tree::ptree& localConceptsChild =
        pTree.put_child("localConcepts", {});
    for (const auto& currLocalConcept : localConcepts)
      localConceptsChild.put("e", currLocalConcept);
  }

  const auto& oppositeConcepts = pConceptSet.getOppositeConcepts();
  if (!oppositeConcepts.empty())
  {
    boost::property_tree::ptree& oppositeConceptsChild =
        pTree.put_child("oppositeConcepts", {});
    for (const auto& currOppositeConcept : oppositeConcepts)
    {
      boost::property_tree::ptree currEltChild;
      for (const auto& currOppCpt : currOppositeConcept.second)
        currEltChild.put("e", currOppCpt);
      oppositeConceptsChild.add_child(currOppositeConcept.first, currEltChild);
    }
  }
}


void _saveLingWordsGroup(boost::property_tree::ptree& pTree,
                         const linguistics::LingWordsGroup& pLingWordsGroup)
{
  if (pLingWordsGroup.rootWord)
    _saveSemanticWord(pTree.put_child("rootWord", {}), *pLingWordsGroup.rootWord);

  if (!pLingWordsGroup.linkedMeanings.empty())
  {
    boost::property_tree::ptree& linkedMeaningsChild = pTree.put_child("linkedMeanings", {});
    for (const auto& currLinkedMeaning : pLingWordsGroup.linkedMeanings)
    {
      boost::property_tree::ptree currEltChild;
      if (currLinkedMeaning.first)
        _saveSemanticWord(currEltChild.put_child("word", {}), *currLinkedMeaning.first);
      currEltChild.put("direction", linkedMeaningDirection_toStr(currLinkedMeaning.second));
      linkedMeaningsChild.add_child("e", currEltChild);
    }
  }
}


void _saveWordAssociatedInfos(boost::property_tree::ptree& pTree,
                              const linguistics::WordAssociatedInfos& pInfos)
{
  _saveConcepts(pTree, pInfos.concepts);
  if (!pInfos.contextualInfos.empty())
  {
    boost::property_tree::ptree& contextualInfosChild = pTree.put_child("contextualInfos", {});
    for (const auto& currContextualInfo : pInfos.contextualInfos)
      contextualInfosChild.put("e",
                               wordContextualInfos_toStr(currContextualInfo));
  }
  if (pInfos.linkedMeanings)
    _saveLingWordsGroup(pTree.put_child("linkedMeanings", {}), *pInfos.linkedMeanings);

  if (!pInfos.metaMeanings.empty())
  {
    boost::property_tree::ptree& metaMeaningsChild = pTree.put_child("metaMeanings", {});
    for (const auto& currMetaMeaning : pInfos.metaMeanings)
      _saveLingWordsGroup(metaMeaningsChild.put_child("e", {}), currMetaMeaning);
  }
}


void _saveSpecificLingDatabase(boost::property_tree::ptree& pTree,
                               const linguistics::SpecificLinguisticDatabase& pSpecLingDb)
{
  const auto& wordToSavedInfos = pSpecLingDb.getWordToSavedInfos();
  if (!wordToSavedInfos.empty())
  {
    boost::property_tree::ptree& wordToIgramChild = pTree.put_child("wordToSavedInfos", {});
    for (const auto& currWordToSavedInfos : wordToSavedInfos)
    {
      const SemanticWord& word = currWordToSavedInfos.first;
      boost::property_tree::ptree& subTree = wordToIgramChild.add_child("e", {});
      _saveSemanticWord(subTree.put_child("word", {}), word);
      _saveWordAssociatedInfos(subTree.put_child("infos", {}), currWordToSavedInfos.second.infos);
      boost::property_tree::ptree& inflectedFormInfosTree = subTree.add_child("inflectedFormInfos", {});
      for (const auto& currInflectedFormInfos : currWordToSavedInfos.second.inflectedFormInfos)
      {
        boost::property_tree::ptree& inflTree = inflectedFormInfosTree.add_child("e", {});
        if (currInflectedFormInfos.inflectedFrom != word.lemma)
          inflTree.put("inflectedFrom", currInflectedFormInfos.inflectedFrom);
        if (currInflectedFormInfos.inflections)
        {
          const Inflections& inflections = *currInflectedFormInfos.inflections;
          if (inflections.type != InflectionType::EMPTY)
          {
            boost::property_tree::ptree& inflectionChild = inflTree.put_child("inflections", {});
            std::stringstream ss;
            inflections.concisePrint(ss);
            inflectionChild.put(inflectionType_toStr(inflections.type), ss.str());
          }
        }
        if (currInflectedFormInfos.frequency != 1u)
          inflTree.put("frequency", currInflectedFormInfos.frequency);
      }
    }
  }
}


void _saveTranslationDictionary(boost::property_tree::ptree& pTree,
                                const linguistics::TranslationDictionary& pTransDic)
{
  boost::property_tree::ptree& translationsTree = pTree.put_child("translations", {});
  for (const auto& currInLangTrans : pTransDic.getAllTranslations())
  {
    for (const auto& currOutLangTrans : currInLangTrans.second)
    {
      for (const auto& currWordToWord : currOutLangTrans.second)
      {
        boost::property_tree::ptree newEltChild;
        _saveSemanticWord(newEltChild.put_child("inWord", {}), currWordToWord.first);
        _saveSemanticWord(newEltChild.put_child("outWord", {}), currWordToWord.second);
        translationsTree.put_child("e", newEltChild);
      }
    }
  }
}


void _saveLingDatabase(boost::property_tree::ptree& pTree,
                       const linguistics::LinguisticDatabase& pLingDb)
{
  _saveConcetSet(pTree.put_child("conceptSet", {}), pLingDb.conceptSet);
  boost::property_tree::ptree& langToSpecChild = pTree.put_child("langToSpec", {});
  auto langToSpec_size = pLingDb.langToSpec.size();
  for (std::size_t i = 0; i < langToSpec_size; ++i)
  {
    boost::property_tree::ptree currEltChild;
    _saveSpecificLingDatabase(currEltChild, pLingDb.langToSpec[i]);
    langToSpecChild.add_child(semanticLanguageEnum_toStr(semanticLanguageEnum_fromChar(static_cast<char>(i))),
                              currEltChild);
  }
  _saveTranslationDictionary(pTree.put_child("transDict", {}), pLingDb.transDict);
}


void saveLingDatabase(boost::property_tree::ptree& pTree,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  _saveLingDatabase(pTree.put_child("lingDb", {}), pLingDb);
}


} // End of namespace serializationprivate
} // End of namespace onsem
