#include "statictreeconverter.hpp"
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/printer/expressionprinter.hpp>


namespace onsem
{

StaticTreeConverter::StaticTreeConverter(linguistics::LinguisticDatabaseStreams& pIStreams)
  : SemExpTreeConversionDatabase(pIStreams)
{
}


void StaticTreeConverter::refactorSemExp
(UniqueSemanticExpression& pUSemExp,
 TreePatternConventionEnum pFromConvention,
 TreePatternConventionEnum pToConvention,
 SemanticLanguageEnum pLanguage,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  if (pDebugOutput != nullptr)
  {
    _addExpsToDebugOutput(*pDebugOutput, *pUSemExp);
  }

  _refactorSemExpForALanguage(pUSemExp, pFromConvention, pToConvention,
                              pLanguage, pDebugOutput);
  if (pLanguage != SemanticLanguageEnum::UNKNOWN)
  {
    _refactorSemExpForALanguage(pUSemExp, pFromConvention, pToConvention,
                                SemanticLanguageEnum::UNKNOWN, pDebugOutput);
  }
}


void StaticTreeConverter::addDifferentForms
(UniqueSemanticExpression& pSemExp,
 SemanticLanguageEnum pLanguage,
 bool pAllOrJustTheOneThatAreInBothDirections,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  if (pDebugOutput != nullptr)
    _addExpsToDebugOutput(*pDebugOutput, *pSemExp);

  _searchRootOfSplitPossibilities(pSemExp, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                  *pSemExp, pDebugOutput);
}


GrammaticalType StaticTreeConverter::getChildThatShouldBeUnique
(mystd::unique_propagate_const<UniqueSemanticExpression>& pChildThatShouldBeUnique,
 const GroundedExpression& pGrdExp) const
{
  std::set<std::string> semExpConcepts;
  SemExpGetter::getConceptsOfGrdExp(semExpConcepts, pGrdExp);

  std::list<const UniqueInformationRule*> uniqueInfosRules;
  _getRulesThatHaveTheseConcepts(uniqueInfosRules, fTreesOfSemUniquePattern, semExpConcepts);

  for (const auto& currInfoRulePtr : uniqueInfosRules)
  {
    const UniqueInformationRule& currInfoRule = *currInfoRulePtr;
    const SemExpTreePatternNode& rootNode = *currInfoRule.treePattern;
    std::set<const SemanticExpression*> alreadyTreatedRoots;
    const SemanticExpressionContainer* rootInSemExp = nullptr;
    std::map<std::string, std::list<const SemanticExpressionContainer*>> links;
    ReferenceOfSemanticExpressionContainer semExpWrapped(pGrdExp);
    if (_doesASemExpMatchAPatternTree<const SemanticExpressionContainer>
        (rootInSemExp, links, semExpWrapped,
         alreadyTreatedRoots, rootNode, rootNode, false))
    {
      auto itOfUniqueChildInPattern = rootNode.children.find(currInfoRule.childThatIsUnique);
      if (itOfUniqueChildInPattern != rootNode.children.end())
      {
        std::map<std::string, std::list<std::shared_ptr<SemanticExpression>>> repLinks;
        pChildThatShouldBeUnique.emplace(_patternNodeToSemExp(itOfUniqueChildInPattern->second, repLinks, true));
      }
      return currInfoRule.childThatIsUnique;
    }
  }
  return GrammaticalType::UNKNOWN;
}



void StaticTreeConverter::_searchRootOfSplitPossibilities
(UniqueSemanticExpression& pSemExp,
 SemanticLanguageEnum pLanguage,
 bool pAllOrJustTheOneThatAreInBothDirections,
 const SemanticExpression& pRootSemExpForDebug,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (grdExp->type == SemanticGroundingType::STATEMENT)
    {
      const SemanticStatementGrounding& statGr = grdExp->getStatementGrounding();
      if (!statGr.requests.empty())
      {
        _addDifferentFormsOfExpressionForALanguage(pSemExp, pAllOrJustTheOneThatAreInBothDirections,
                                                   pRootSemExpForDebug, pDebugOutput);
        return;
      }
    }

    for (auto& currChild : grdExp.children)
    {
      _searchRootOfSplitPossibilities(currChild.second, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                        pRootSemExpForDebug, pDebugOutput);
    }
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    for (auto& currElt : listExp.elts)
    {
      _searchRootOfSplitPossibilities(currElt, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                        pRootSemExpForDebug, pDebugOutput);
    }
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    ConditionExpression& condExp = pSemExp->getCondExp();
    _searchRootOfSplitPossibilities(condExp.conditionExp, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                      pRootSemExpForDebug, pDebugOutput);
    _searchRootOfSplitPossibilities(condExp.thenExp, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                      pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    InterpretationExpression& intExp = pSemExp->getIntExp();
    _searchRootOfSplitPossibilities(intExp.interpretedExp, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                      pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    FeedbackExpression& fdkExp = pSemExp->getFdkExp();
    _searchRootOfSplitPossibilities(fdkExp.concernedExp, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                      pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    AnnotatedExpression& annExp = pSemExp->getAnnExp();
    _searchRootOfSplitPossibilities(annExp.semExp, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                      pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    MetadataExpression& metadataExp = pSemExp->getMetadataExp();
    _searchRootOfSplitPossibilities(metadataExp.semExp, pLanguage, pAllOrJustTheOneThatAreInBothDirections,
                                      pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
  {
    break;
  }
  }
}


void StaticTreeConverter::_addDifferentFormsOfExpressionForALanguage
(UniqueSemanticExpression& pSemExp,
 bool pAllOrJustTheOneThatAreInBothDirections,
 const SemanticExpression& pRootSemExpForDebug,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  std::set<std::string> alreadyDoneConvIds;
  std::list<std::pair<int, UniqueSemanticExpression> > newFormsOfTheExp;

  const auto& semanticForms = pAllOrJustTheOneThatAreInBothDirections ?
        _semanticForms : _semanticFormsBothDirections;
  _recurssivelyAddForms(newFormsOfTheExp, pSemExp, alreadyDoneConvIds,
                        semanticForms, 10, pRootSemExpForDebug, pDebugOutput);

  if (!newFormsOfTheExp.empty())
  {
    auto setOfFormsExp = std::make_unique<SetOfFormsExpression>();
    setOfFormsExp->prioToForms[-10].emplace_back
        (std::make_unique<QuestExpressionFrom>(std::move(pSemExp), true));
    for (auto& currForm : newFormsOfTheExp)
    {
      setOfFormsExp->prioToForms[-currForm.first].emplace_back
          (std::make_unique<QuestExpressionFrom>(std::move(currForm.second), false));
    }
    pSemExp = std::move(setOfFormsExp);
  }
}




void StaticTreeConverter::_recurssivelyAddForms
(std::list<std::pair<int, UniqueSemanticExpression> >& pNewFormsOfTheExp,
 UniqueSemanticExpression& pSemExp,
 std::set<std::string>& pAlreadyDoneConvIds,
 const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
 int pCurrPrio,
 const SemanticExpression& pRootSemExpForDebug,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  std::list<std::pair<int, UniqueSemanticExpression>> provNewFormsOfTheExp;
  _extractNewForms(provNewFormsOfTheExp, pSemExp, pAlreadyDoneConvIds, pTreeOfConvs,
                   pCurrPrio, pRootSemExpForDebug, pDebugOutput);

  if (provNewFormsOfTheExp.empty())
  {
    return;
  }
  for (auto& currElt : provNewFormsOfTheExp)
  {
    std::set<std::string> alreadyDoneConvIds(pAlreadyDoneConvIds);
    _recurssivelyAddForms(pNewFormsOfTheExp, currElt.second, alreadyDoneConvIds, pTreeOfConvs,
                          currElt.first, pRootSemExpForDebug, pDebugOutput);
  }
  pNewFormsOfTheExp.splice(pNewFormsOfTheExp.end(), provNewFormsOfTheExp);
}


void StaticTreeConverter::_addExpsToDebugOutput
(std::list<std::list<SemLineToPrint> >& pDebugOutput,
 const SemanticExpression& pSemExp)
{
  pDebugOutput.emplace_back();
  printer::prettyPrintSemExp(pDebugOutput.back(), pSemExp);
}


void StaticTreeConverter::_refactorSemExpForALanguage
(UniqueSemanticExpression& pSemExp,
 TreePatternConventionEnum pFromConvention,
 TreePatternConventionEnum pToConvention,
 SemanticLanguageEnum pLanguage,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  auto itConvForLang = fConversions.find(pLanguage);
  if (itConvForLang == fConversions.end())
  {
    return;
  }
  auto itConvs = itConvForLang->second.find(TreePatternEnumPair(pFromConvention, pToConvention));
  if (itConvs != itConvForLang->second.end())
  {
    std::map<const ConversionRule*, std::set<const SemanticExpression*> > alreadyDoneConv;
    _refactSemExpRec(pSemExp, alreadyDoneConv, itConvs->second,
                     *pSemExp, pDebugOutput);
  }
}


void StaticTreeConverter::_convertFindLinksToReplacementLinks
(std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
 const std::map<std::string, std::list<UniqueSemanticExpression*> >& pLinks) const
{
  for (const auto& currLink : pLinks)
  {
    std::list<std::shared_ptr<SemanticExpression> >& repSemExp = pRepLinks[currLink.first];
    for (const auto& currSemExpLink : currLink.second)
    {
      repSemExp.emplace_back(currSemExpLink->getSharedPtr());
    }
  }
}


void StaticTreeConverter::_refactSemExpRec
(UniqueSemanticExpression& pSemExp,
 std::map<const ConversionRule*, std::set<const SemanticExpression*> >& pAlreadyDoneConv,
 const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
 const SemanticExpression& pRootSemExpForDebug,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::ANNOTATED:
  {
    auto& annExp = pSemExp->getAnnExp();
    _refactSemExpRec(annExp.semExp, pAlreadyDoneConv, pTreeOfConvs,
                     pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    auto& condExp = pSemExp->getCondExp();
    _refactSemExpRec(condExp.thenExp, pAlreadyDoneConv, pTreeOfConvs,
                     pRootSemExpForDebug, pDebugOutput);
    if (condExp.elseExp)
      _refactSemExpRec(*condExp.elseExp, pAlreadyDoneConv, pTreeOfConvs,
                       pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    auto& fdkdExp = pSemExp->getFdkExp();
    _refactSemExpRec(fdkdExp.concernedExp, pAlreadyDoneConv, pTreeOfConvs,
                     pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::GROUNDED:
  {
    _refactSemExp(pSemExp, pAlreadyDoneConv, pTreeOfConvs,
                  pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pSemExp->getIntExp();
    _refactSemExpRec(intExp.interpretedExp, pAlreadyDoneConv, pTreeOfConvs,
                     pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    auto& listExp = pSemExp->getListExp();
    if (listExp.listType == ListExpressionType::UNRELATED)
    {
      for (auto& currElt : listExp.elts)
      {
        std::map<const ConversionRule*, std::set<const SemanticExpression*> > alreadyDoneConv;
        _refactSemExpRec(currElt, alreadyDoneConv, pTreeOfConvs,
                         pRootSemExpForDebug, pDebugOutput);
      }
    }
    _refactSemExp(pSemExp, pAlreadyDoneConv, pTreeOfConvs,
                  pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metadataExp = pSemExp->getMetadataExp();
    _refactSemExpRec(metadataExp.semExp, pAlreadyDoneConv, pTreeOfConvs,
                     pRootSemExpForDebug, pDebugOutput);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}


void StaticTreeConverter::_refactSemExp
(UniqueSemanticExpression& pSemExp,
 std::map<const ConversionRule*, std::set<const SemanticExpression*> >& pAlreadyDoneConv,
 const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
 const SemanticExpression& pRootSemExpForDebug,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  bool conversionHasBeenDone = false;
  do
  {
    conversionHasBeenDone = false;
    // get sem exp concepts
    // TODO: do it only one time for current language and unknown language
    std::set<std::string> semExpConcepts;
    SemExpGetter::getConceptsOfSemExp(semExpConcepts, *pSemExp);

    // get conversions that have the same concepts that the sem exp
    std::list<const ConversionRule*> possiblesConv;
    _getRulesThatHaveTheseConcepts(possiblesConv, pTreeOfConvs, semExpConcepts);

    // try to apply all the possible conversions
    for (const auto& currPossConv : possiblesConv)
    {
      std::set<const SemanticExpression*>& alreadyTreatedRoots = pAlreadyDoneConv[currPossConv];
      UniqueSemanticExpression* rootInSemExp = nullptr;
      std::map<std::string, std::list<UniqueSemanticExpression*> > links;
      const SemExpTreePatternNode& rootNode = *currPossConv->treePatternIn;
      while (_doesASemExpMatchAPatternTree(rootInSemExp, links, pSemExp,
                                           alreadyTreatedRoots, rootNode, rootNode, true))
      {
        alreadyTreatedRoots.insert(&**rootInSemExp);

        std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > > repLinks;
        _convertFindLinksToReplacementLinks(repLinks, links);
        _applyModifsOnSemExp(**rootInSemExp, repLinks,
                             *currPossConv->treePatternOut);
        conversionHasBeenDone = true;

        if (pDebugOutput != nullptr)
        {
          _printAConversionInfos(*pDebugOutput, *currPossConv);
          _addExpsToDebugOutput(*pDebugOutput, pRootSemExpForDebug);
        }
        links.clear();
      }
    }
  } while (conversionHasBeenDone);
}



void StaticTreeConverter::_extractNewForms
(std::list<std::pair<int, UniqueSemanticExpression>>& pNewFormsOfTheExp,
 UniqueSemanticExpression& pSemExp,
 std::set<std::string>& pAlreadyDoneConvIds,
 const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
 int pCurrPrio,
 const SemanticExpression& pRootSemExpForDebug,
 std::list<std::list<SemLineToPrint> >* pDebugOutput) const
{
  // get sem exp concepts
  // TODO: do it only one time for current language and unknown language
  std::set<std::string> semExpConcepts;
  SemExpGetter::getConceptsOfSemExp(semExpConcepts, *pSemExp);

  // get conversions that have the same concepts that the sem exp
  std::list<const ConversionRule*> possiblesConv;
  _getRulesThatHaveTheseConcepts(possiblesConv, pTreeOfConvs, semExpConcepts);

  // try to apply all the possible conversions
  for (const ConversionRule* currPossConv : possiblesConv)
  {
    if (pAlreadyDoneConvIds.find(currPossConv->id) != pAlreadyDoneConvIds.end())
      continue;

    UniqueSemanticExpression* rootInSemExp = nullptr;
    std::map<std::string, std::list<UniqueSemanticExpression*> > links;
    const SemExpTreePatternNode& rootNode = *currPossConv->treePatternIn;
    if (_doesASemExpMatchAPatternTree(rootInSemExp, links, pSemExp,
                                      std::set<const SemanticExpression*>(),
                                      rootNode, rootNode, false))
    {
      pAlreadyDoneConvIds.insert(currPossConv->id);

      auto semExpCopied = pSemExp->clone();
      std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > > repLinks;
      _convertFindLinksToReplacementLinks(repLinks, links);
      _applyModifsOnSemExp(**rootInSemExp, repLinks,
                           *currPossConv->treePatternOut);
      if (pDebugOutput != nullptr)
      {
        _printAConversionInfos(*pDebugOutput, *currPossConv);
        _addExpsToDebugOutput(*pDebugOutput, pRootSemExpForDebug);
      }

      pNewFormsOfTheExp.emplace_back(pCurrPrio + currPossConv->priorityOfOutPattern,
                                     std::move(pSemExp));
      pSemExp = std::move(semExpCopied);
    }
  }
}




void StaticTreeConverter::_printAConversionInfos
(std::list<std::list<SemLineToPrint> >& pDebugOutput,
 const ConversionRule& pConvInfos) const
{
  pDebugOutput.emplace_back();
  std::list<SemLineToPrint>& outLines = pDebugOutput.back();
  {
    std::stringstream ss;
    ss << "Conversion: " << pConvInfos.filename << " (" << pConvInfos.convesionNb << ")";
    outLines.emplace_back(0, ss.str());
  }
  outLines.emplace_back(0, "From:");
  {
    std::stringstream ss;
    _printAPatternNode(outLines, ss, semLineToPrint_subLabelOffsets, *pConvInfos.treePatternIn);
  }
  outLines.emplace_back(0, "To:");
  {
    std::stringstream ss;
    _printAPatternNode(outLines, ss, semLineToPrint_subLabelOffsets, *pConvInfos.treePatternOut);
  }
}


void StaticTreeConverter::_printAPatternNode
(std::list<SemLineToPrint>& pOutLines,
 std::stringstream& pSs,
 std::size_t pOffsetNewLine,
 const SemExpTreePatternNode& pRootPattern) const
{
  if (pRootPattern.groundingType)
  {
    pSs << " grounding(" << semanticGroundingsType_toStr(*pRootPattern.groundingType) + ")";
  }
  if (!pRootPattern.id.empty())
  {
    pSs << " id(" << pRootPattern.id << ")";
  }
  for (const auto& currCpt : pRootPattern.concepts)
    pSs << " concept(" << currCpt.first << ")";
  for (const auto& currNotCpt : pRootPattern.notConcepts)
    pSs << " notConcept(" << currNotCpt << ")";
  for (const auto& currBegOfCpt : pRootPattern.beginOfConcepts)
    pSs << " beginOfConcepts(" << currBegOfCpt << ")";
  for (const auto& currCpt : pRootPattern.conceptsOrHyponyms)
    pSs << " conceptOrHyponym(" << currCpt << ")";
  for (const auto& currCpt : pRootPattern.notConceptsOrHyponyms)
    pSs << " notConceptOrHyponym(" << currCpt << ")";
  if (pRootPattern.nb)
    pSs << " nb(" << *pRootPattern.nb << ")";
  for (const auto& currRequest : pRootPattern.requests.types)
  {
    pSs << " request(" << semanticRequestType_toStr(currRequest) << ")";
  }
  for (const auto& currNotRequest : pRootPattern.notRequests.types)
  {
    pSs << " notRequest(" << semanticRequestType_toStr(currNotRequest) << ")";
  }
  if (pRootPattern.time)
  {
    pSs << " time(" << semanticVerbTense_toStr(*pRootPattern.time) << ")";
  }
  if (pRootPattern.type)
  {
    pSs << " type(" << semanticEntityType_toStr(*pRootPattern.type) << ")";
  }
  for (const auto& currNotType : pRootPattern.notTypes)
  {
    pSs << " notType(" << semanticEntityType_toStr(currNotType) << ")";
  }
  if (pRootPattern.reference)
  {
    pSs << " ref(" << semanticReferenceType_toStr(*pRootPattern.reference) << ")";
  }
  if (pRootPattern.word)
  {
    const SemanticWord& word = *pRootPattern.word;
    if (word.isEmpty())
      pSs << " word()";
    else
      pSs << " word(" << word.lemma << "|"
          << partOfSpeech_toStr(word.partOfSpeech)
          << semanticLanguageEnum_toStr(word.language) << ")";
  }
  if (pRootPattern.timeType)
  {
    pSs << " timeType(" << semanticRelativeTimeType_toStr(*pRootPattern.timeType) << ")";
  }
  if (pRootPattern.hasToBeCompletedFromContext)
  {
    pSs << " hasToBeCompletedFromContext(";
    if (*pRootPattern.hasToBeCompletedFromContext)
      pSs << "true";
    else
      pSs << "false";
    pSs << ")";
  }
  for (const auto& currNotChild : pRootPattern.notChildren)
  {
    pSs << " notChild(" << grammaticalType_toStr(currNotChild) << ")";
  }
  pOutLines.emplace_back(pOffsetNewLine, pSs.str());
  pSs.str("");

  for (const auto& currChild : pRootPattern.children)
  {
    pSs << grammaticalType_toStr(currChild.first) + ":";
    std::size_t offset = pOffsetNewLine;
    offset += semLineToPrint_subLabelOffsets;
    _printAPatternNode(pOutLines, pSs, offset, currChild.second);
  }
}


template <typename RULE>
void StaticTreeConverter::_getRulesThatHaveTheseConcepts
(std::list<const RULE*>& pPossiblesConv,
 const ConceptTreeOfRules<RULE>& pTreeOfConvs,
 const std::set<std::string>& pSemExpConcepts) const
{
  for (const auto& currCv : pTreeOfConvs.rules)
    pPossiblesConv.push_back(&currCv);
  for (const auto& currChld : pTreeOfConvs.children)
    if (pSemExpConcepts.find(currChld.first) != pSemExpConcepts.end())
      _getRulesThatHaveTheseConcepts(pPossiblesConv, currChld.second, pSemExpConcepts);
}


void StaticTreeConverter::_applyModifsOnSemExp
(SemanticExpression& pRootInSemExp,
 const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
 const SemExpTreePatternNode& pRootPattern) const
{
  GroundedExpression* grdExpPtr = pRootInSemExp.getGrdExpPtr_SkipWrapperPtrs(false);
  if (grdExpPtr != nullptr)
  {
    _applyModifsOnGrdExp(*grdExpPtr, pRepLinks, pRootPattern);
    return;
  }

  ListExpression* listExpPtr = pRootInSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (auto& currElt : listExpPtr->elts)
      _applyModifsOnSemExp(*currElt, pRepLinks, pRootPattern);
}



void StaticTreeConverter::_applyModifsOnGrdExp
(GroundedExpression& pRootInGrExp,
 const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
 const SemExpTreePatternNode& pRootPattern,
 bool pSetMotherConceptOfConceptsAny) const
{
  // fill grounding infos
  {
    SemanticGrounding& semExpGrd = pRootInGrExp.grounding();

    for (const auto& currCpt : pRootPattern.concepts)
    {
      std::string conceptName = currCpt.first;
      if (pSetMotherConceptOfConceptsAny)
        ConceptSet::getMotherConceptOfConceptAny(conceptName);
      if (semExpGrd.concepts.find(conceptName) == semExpGrd.concepts.end())
        semExpGrd.concepts[conceptName] = currCpt.second.empty() ?
              static_cast<char>(4) : currCpt.second.front();
    }

    for (const auto& currNotCpt : pRootPattern.notConcepts)
    {
      if (currNotCpt == "*")
      {
        semExpGrd.concepts.clear();
      }
      else
      {
        auto itCptToRem = semExpGrd.concepts.find(currNotCpt);
        if (itCptToRem != semExpGrd.concepts.end())
          semExpGrd.concepts.erase(itCptToRem);
      }
    }

    for (const auto& currBegOfCpt : pRootPattern.beginOfConcepts)
    {
      if (!ConceptSet::haveAConceptThatBeginWith(semExpGrd.concepts, currBegOfCpt))
      {
        std::string conceptName = currBegOfCpt + "*";
        if (pSetMotherConceptOfConceptsAny)
          ConceptSet::getMotherConceptOfConceptAny(conceptName);
        semExpGrd.concepts.emplace(conceptName, 4);
      }
    }

    for (const auto& currCpt : pRootPattern.conceptsOrHyponyms)
    {
      if (ConceptSet::haveAConceptOrAHyponym(semExpGrd.concepts, currCpt))
        semExpGrd.concepts[currCpt] = static_cast<char>(4);
    }

    for (const auto& currCpt : pRootPattern.notConceptsOrHyponyms)
      ConceptSet::removeConceptsOrHyponyms(semExpGrd.concepts, currCpt);

    if (pRootPattern.groundingType)
    {
      switch (*pRootPattern.groundingType)
      {
      case SemanticGroundingType::STATEMENT:
      {
        SemanticStatementGrounding* statGrdPtr = semExpGrd.getStatementGroundingPtr();
        if (statGrdPtr != nullptr)
        {
          SemanticStatementGrounding& statGrd = *statGrdPtr;
          for (const auto& currNoReq : pRootPattern.notRequests.types)
            statGrd.requests.erase(currNoReq);
          for (const auto& currReq : pRootPattern.requests.types)
            statGrd.requests.add(currReq);
          if (pRootPattern.time.has_value())
            statGrd.verbTense = *pRootPattern.time;
          if (pRootPattern.word)
            statGrd.word = *pRootPattern.word;
        }
        break;
      }
      case SemanticGroundingType::GENERIC:
      {
        SemanticGenericGrounding* genGrdPtr = semExpGrd.getGenericGroundingPtr();
        if (genGrdPtr != nullptr)
        {
          SemanticGenericGrounding& genGrd = *genGrdPtr;
          if (pRootPattern.nb.has_value())
            genGrd.quantity.setNumber(*pRootPattern.nb);
          if (pRootPattern.type.has_value())
            genGrd.entityType = *pRootPattern.type;
          if (pRootPattern.reference.has_value())
            genGrd.referenceType = *pRootPattern.reference;
          if (pRootPattern.word)
            genGrd.word = *pRootPattern.word;
          if (pRootPattern.hasToBeCompletedFromContext)
          {
            if (*pRootPattern.hasToBeCompletedFromContext)
              genGrd.coreference.emplace();
            else
              genGrd.coreference.reset();
          }
        }
        break;
      }
      case SemanticGroundingType::RELATIVETIME:
      {
        SemanticRelativeTimeGrounding* relTimeGrdPtr = semExpGrd.getRelTimeGroundingPtr();
        if (relTimeGrdPtr != nullptr)
        {
          SemanticRelativeTimeGrounding& relTimeGrd = *relTimeGrdPtr;
          if (pRootPattern.timeType.has_value())
            relTimeGrd.timeType = *pRootPattern.timeType;
        }
        break;
      }
      default:
        break;
      }
    }
  }

  // remove children that should be removed
  for (const auto& currNotChild : pRootPattern.notChildren)
  {
    auto itChildToRemove = pRootInGrExp.children.find(currNotChild);
    if (itChildToRemove != pRootInGrExp.children.end())
    {
      pRootInGrExp.children.erase(itChildToRemove);
    }
  }

  // consider each children
  for (const auto& currPattChild : pRootPattern.children)
  {
    _applyModifsForAChild(pRootInGrExp, pRepLinks,
                          currPattChild.first, currPattChild.second);
  }
}


bool StaticTreeConverter::_aSubSemExpIsInTheList
(const ListExpression& pListExp,
 const std::list<std::shared_ptr<SemanticExpression> >& pSubSemExp) const
{
  for (const auto& currElt : pListExp.elts)
  {
    for (const auto& currSubSemExp : pSubSemExp)
    {
      if (&*currElt == &*currSubSemExp)
      {
        return true;
      }
    }
  }
  return false;
}



void StaticTreeConverter::_applyModifsForAChild
(GroundedExpression& pRootInGrdExp,
 const std::map<std::string, std::list<std::shared_ptr<SemanticExpression>>>& pRepLinks,
 GrammaticalType pChildGramType,
 const SemExpTreePatternNode& pChildPattern) const
{
  if (!pChildPattern.id.empty())
  {
    auto itLk = pRepLinks.find(pChildPattern.id);
    if (itLk != pRepLinks.end())
    {
      auto itChild = pRootInGrdExp.children.find(pChildGramType);
      if (itChild != pRootInGrdExp.children.end())
      {
        ListExpression* listSubExp = itChild->second->getListExpPtr();
        if (listSubExp != nullptr &&
            _aSubSemExpIsInTheList(*listSubExp, itLk->second))
        {
          for (auto& currSemExpsToModify : itLk->second)
          {
            bool eltAlreadyExist = false;
            for (auto& currElt : listSubExp->elts)
            {
              if (&*currElt == &*currSemExpsToModify)
              {
                _applyModifsOnSemExp(*currElt, pRepLinks, pChildPattern);
                eltAlreadyExist = true;
                break;
              }
            }
            if (!eltAlreadyExist)
            {
              auto newElt = currSemExpsToModify->clone();
              _applyModifsOnSemExp(*newElt, pRepLinks, pChildPattern);
              listSubExp->elts.emplace_back(std::move(newElt));
            }
          }
          return;
        }

        if (itLk->second.size() == 1 &&
            &*itChild->second == &*itLk->second.front())
        {
          _applyModifsOnSemExp(*itLk->second.front(),
                               pRepLinks, pChildPattern);
        }
        else
        {
          pRootInGrdExp.children.erase(itChild);
        }
      }

      // go to the specified child
      if (itLk->second.size() == 1)
      {
        auto newChild = itLk->second.front()->clone();
        _applyModifsOnSemExp(*newChild, pRepLinks, pChildPattern);
        pRootInGrdExp.children.emplace(pChildGramType, std::move(newChild));
      }
      else if (itLk->second.size() > 1)
      {
        auto newListExp = std::make_unique<ListExpression>();
        for (auto& currSemExpsToModify : itLk->second)
        {
          auto newElt = currSemExpsToModify->clone();
          _applyModifsOnSemExp(*newElt, pRepLinks, pChildPattern);
          newListExp->elts.emplace_back(std::move(newElt));
        }
        pRootInGrdExp.children.emplace(pChildGramType, std::move(newListExp));
      }
      return;
    }
  }

  // create a new empty child
  pRootInGrdExp.children.erase(pChildGramType);
  pRootInGrdExp.children.emplace(pChildGramType, _patternNodeToSemExp(pChildPattern, pRepLinks));
}


UniqueSemanticExpression StaticTreeConverter::_patternNodeToSemExp
(const SemExpTreePatternNode& pTreePattern,
 const std::map<std::string, std::list<std::shared_ptr<SemanticExpression>>>& pRepLinks,
 bool pSetMotherConceptOfConceptsAny) const
{
  auto newChild = std::make_unique<GroundedExpression>
      (pTreePattern.groundingType ?
         SemanticGrounding::make(*pTreePattern.groundingType) :
         std::make_unique<SemanticConceptualGrounding>());
  _applyModifsOnGrdExp(*newChild, pRepLinks, pTreePattern, pSetMotherConceptOfConceptsAny);
  return std::move(newChild);
}


template <typename SEMEXPCONTAINER>
bool StaticTreeConverter::_doesASemExpMatchAPatternTree
(SEMEXPCONTAINER*& pRootInSemExp,
 std::map<std::string, std::list<SEMEXPCONTAINER*> >& pLinks,
 SEMEXPCONTAINER& pSemExp,
 const std::set<const SemanticExpression*>& pAlreadyTreatedRoots,
 const SemExpTreePatternNode& pRootNode,
 const SemExpTreePatternNode& pCurrNode,
 bool pCanMatchWithAChildNode) const
{
  auto* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs(false);
  if (grdExpPtr != nullptr)
  {
    if (pAlreadyTreatedRoots.find(&*pSemExp) == pAlreadyTreatedRoots.end())
    {
      const SemanticGrounding& grd = grdExpPtr->grounding();
      if ((!pCurrNode.groundingType ||
           grd.type == *pCurrNode.groundingType) &&
          xIsASubSetOnMapLabel(pCurrNode.concepts, grd.concepts))
      {
        bool res = true;

        for (auto itNotCpt = pCurrNode.notConcepts.begin();
             itNotCpt != pCurrNode.notConcepts.end(); ++itNotCpt)
        {
          if (grd.concepts.find(*itNotCpt) != grd.concepts.end())
          {
            res = false;
            break;
          }
        }

        for (const auto& currBegOfCpt : pCurrNode.beginOfConcepts)
        {
          if (!ConceptSet::haveAConceptThatBeginWith(grd.concepts, currBegOfCpt))
          {
            res = false;
            break;
          }
        }

        for (const auto& currCpt : pCurrNode.conceptsOrHyponyms)
        {
          if (!ConceptSet::haveAConceptOrAHyponym(grd.concepts, currCpt))
          {
            res = false;
            break;
          }
        }

        for (const auto& currCpt : pCurrNode.notConceptsOrHyponyms)
        {
          if (ConceptSet::haveAConceptOrAHyponym(grd.concepts, currCpt))
          {
            res = false;
            break;
          }
        }

        if (res && pCurrNode.groundingType)
        {
          switch (*pCurrNode.groundingType)
          {
          case SemanticGroundingType::STATEMENT:
          {
            const auto* statGrd = grd.getStatementGroundingPtr();
            if (statGrd != nullptr &&
                pCurrNode.requests.isASubSetOf(statGrd->requests))
            {
              for (const auto& currNotReq : pCurrNode.notRequests.types)
              {
                if (statGrd->requests.has(currNotReq))
                {
                  res = false;
                  break;
                }
              }

              if (res && pCurrNode.time.has_value() &&
                  statGrd->verbTense != *pCurrNode.time)
              {
                res = false;
              }

              if (res && pCurrNode.word && statGrd->word != pCurrNode.word)
              {
                res = false;
              }
            }
            else
            {
              res = false;
            }
            break;
          }
          case SemanticGroundingType::GENERIC:
          {
            const SemanticGenericGrounding* genGrd = grd.getGenericGroundingPtr();
            if (genGrd != nullptr)
            {
              if (pCurrNode.type.has_value() &&
                  genGrd->entityType != *pCurrNode.type)
              {
                res = false;
              }

              if (pCurrNode.nb.has_value() &&
                  !genGrd->quantity.isEqualTo(*pCurrNode.nb))
              {
                res = false;
              }

              if (pCurrNode.reference.has_value() &&
                  genGrd->referenceType != *pCurrNode.reference)
              {
                res = false;
              }

              for (auto itNotType = pCurrNode.notTypes.begin();
                   itNotType != pCurrNode.notTypes.end(); ++itNotType)
              {
                if (*itNotType == genGrd->entityType)
                {
                  res = false;
                  break;
                }
              }

              if (res && pCurrNode.word && genGrd->word != pCurrNode.word)
              {
                res = false;
              }
              if (res && pCurrNode.hasToBeCompletedFromContext.has_value())
              {
                if (*pCurrNode.hasToBeCompletedFromContext && !genGrd->coreference)
                  res = false;
                else if (!*pCurrNode.hasToBeCompletedFromContext && genGrd->coreference)
                  res = false;
              }
            }
            else
            {
              res = false;
            }
            break;
          }
          case SemanticGroundingType::AGENT:
          {
            const SemanticAgentGrounding* agentGrd = grd.getAgentGroundingPtr();
            if (agentGrd == nullptr ||
                pCurrNode.reference.has_value())
            {
              res = false;
            }
            break;
          }
          case SemanticGroundingType::RELATIVETIME:
          {
            const SemanticRelativeTimeGrounding* relTimeGrdPtr = grd.getRelTimeGroundingPtr();
            if (relTimeGrdPtr != nullptr)
            {
              if (pCurrNode.timeType.has_value() &&
                  relTimeGrdPtr->timeType != *pCurrNode.timeType)
                res = false;
            }
            else
            {
              res = false;
            }
            break;
          }
          default:
          {
            break;
          }
          }
        }

        if (res)
        {
          for (auto itPatternNotChild = pCurrNode.notChildren.begin();
               itPatternNotChild != pCurrNode.notChildren.end(); ++itPatternNotChild)
          {
            if (grdExpPtr->children.find(*itPatternNotChild) != grdExpPtr->children.end())
            {
              res = false;
              break;
            }
          }
        }

        if (res)
        {
          for (auto itPatternChild = pCurrNode.children.begin();
               itPatternChild != pCurrNode.children.end(); ++itPatternChild)
          {
            auto itSemExpChild = grdExpPtr->children.find(itPatternChild->first);
            if (itSemExpChild == grdExpPtr->children.end() ||
                !_doesASemExpMatchAPatternTree<SEMEXPCONTAINER>(pRootInSemExp, pLinks,
                                                                itSemExpChild->second,
                                                                std::set<const SemanticExpression*>(),
                                                                pRootNode, itPatternChild->second,
                                                                pCanMatchWithAChildNode))
            {
              res = false;
              break;
            }
          }
        }

        if (res)
        {
          if (!pCurrNode.id.empty())
          {
            pLinks[pCurrNode.id].emplace_back(&pSemExp);
          }
          pRootInSemExp = &pSemExp;
          return true;
        }
      }
    }

    if (pCanMatchWithAChildNode)
    {
      for (auto& currChild : grdExpPtr->children)
      {
        if (_doesASemExpMatchAPatternTree<SEMEXPCONTAINER>(pRootInSemExp, pLinks, currChild.second,
                                                           pAlreadyTreatedRoots, pRootNode, pRootNode,
                                                           pCanMatchWithAChildNode))
        {
          return true;
        }
      }
    }
    return false;
  }

  if (pCanMatchWithAChildNode)
  {
    auto* listExpPtr = pSemExp->getListExpPtr();
    if (listExpPtr != nullptr)
    {
      for (auto& currElt : listExpPtr->elts)
      {
        if (_doesASemExpMatchAPatternTree<SEMEXPCONTAINER>(pRootInSemExp, pLinks, currElt,
                                                           pAlreadyTreatedRoots, pRootNode, pCurrNode,
                                                           pCanMatchWithAChildNode))
        {
          return true;
        }
      }
      return false;
    }
  }
  return false;
}



} // End of namespace onsem
