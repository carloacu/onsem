#include "linguisticsynthesizer.hpp"
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/common/enum/treepatternconventionenum.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/printer/semlinetoprint.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include "linguisticsynthesizerenglish.hpp"
#include "linguisticsynthesizerfrench.hpp"

namespace onsem
{
namespace
{

void _grdExpSubstract(std::map<GrammaticalType, UniqueSemanticExpression>& pRes,
                      GroundedExpression& pGrdExp,
                      const GroundedExpression& pSemExpBase)
{
  for (auto itChild = pGrdExp.children.begin(); itChild != pGrdExp.children.end(); )
  {
    if (pSemExpBase.children.count(itChild->first) == 0)
    {
      pRes.emplace(itChild->first, std::move(itChild->second));
      itChild = pGrdExp.children.erase(itChild);
    }
    else
    {
      ++itChild;
    }
  }
}


UniqueSemanticExpression _semExpReplacement(UniqueSemanticExpression pSemExp,
                                            const SemanticExpression& pSemExpBase,
                                            const GroundedExpression& pGrdExpToReplace,
                                            const SemanticMemoryBlock& pMemBlock,
                                            const linguistics::LinguisticDatabase& pLingDb)
{
  GroundedExpression* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const GroundedExpression* grdExpBase = pSemExpBase.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpBase != nullptr)
    {
      std::map<GrammaticalType, UniqueSemanticExpression> childrenToAdd;
      _grdExpSubstract(childrenToAdd, *grdExpPtr, *grdExpBase);
      auto newGrdExp = pGrdExpToReplace.clone();
      for (auto& currChildToAdd : childrenToAdd)
        SemExpModifier::addChild(*newGrdExp, currChildToAdd.first, std::move(currChildToAdd.second));
      return std::move(newGrdExp);
    }
    return pSemExp;
  }

  ListExpression* listExpPtr = pSemExp->getListExpPtr();
  if (listExpPtr != nullptr &&
      !listExpPtr->elts.empty())
  {
    const ListExpression* listExpBasePtr = pSemExpBase.getListExpPtr();
    if (listExpBasePtr != nullptr &&
        !listExpBasePtr->elts.empty())
    {
      auto& lastListExpElt = listExpPtr->elts.back();
      const auto& lastListExpBase = listExpBasePtr->elts.back();
      return _semExpReplacement(std::move(lastListExpElt), *lastListExpBase, pGrdExpToReplace, pMemBlock, pLingDb);
    }
  }
  return pSemExp;
}


void _replaceSemExpThatNeedToBeReplaced(UniqueSemanticExpression& pSemExp,
                                        const SemanticExpression& pUsSemExp,
                                        const GroundedExpression& pGrdExpToReplace,
                                        const SemanticMemoryBlock& pMemBlock,
                                        const linguistics::LinguisticDatabase& pLingDb)
{
  auto& semExp = *pSemExp;
  if (SemExpComparator::semExpsAreEqualFromMemBlock(semExp, pUsSemExp, pMemBlock, pLingDb, nullptr))
  {
    pSemExp = _semExpReplacement(std::move(pSemExp), pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    return;
  }

  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    for (auto& currElt : grdExp.children)
      _replaceSemExpThatNeedToBeReplaced(currElt.second, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::COMPARISON:
  {
    ComparisonExpression& compExp = semExp.getCompExp();
    if (compExp.whatIsComparedExp)
      _replaceSemExpThatNeedToBeReplaced(*compExp.whatIsComparedExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    _replaceSemExpThatNeedToBeReplaced(compExp.leftOperandExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    if (compExp.rightOperandExp)
      _replaceSemExpThatNeedToBeReplaced(*compExp.rightOperandExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    ConditionExpression& condExp = semExp.getCondExp();
    _replaceSemExpThatNeedToBeReplaced(condExp.conditionExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    _replaceSemExpThatNeedToBeReplaced(condExp.thenExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    if (condExp.elseExp)
      _replaceSemExpThatNeedToBeReplaced(*condExp.elseExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    InterpretationExpression& intExp = semExp.getIntExp();
    _replaceSemExpThatNeedToBeReplaced(intExp.interpretedExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    FeedbackExpression& fdkExp = semExp.getFdkExp();
    _replaceSemExpThatNeedToBeReplaced(fdkExp.feedbackExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    _replaceSemExpThatNeedToBeReplaced(fdkExp.concernedExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    AnnotatedExpression& annExp = semExp.getAnnExp();
    for (auto& currElt : annExp.annotations)
      _replaceSemExpThatNeedToBeReplaced(currElt.second, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    _replaceSemExpThatNeedToBeReplaced(annExp.semExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    MetadataExpression& metadataExp = semExp.getMetadataExp();
    if (metadataExp.source)
      _replaceSemExpThatNeedToBeReplaced(*metadataExp.source, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    _replaceSemExpThatNeedToBeReplaced(metadataExp.semExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = semExp.getListExp();
    for (auto& currElt : listExp.elts)
      _replaceSemExpThatNeedToBeReplaced(currElt, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    SetOfFormsExpression& setOfFormsExp = semExp.getSetOfFormsExp();
    UniqueSemanticExpression* originalFormExp = setOfFormsExp.getOriginalForm();
    if (originalFormExp != nullptr)
      _replaceSemExpThatNeedToBeReplaced(*originalFormExp, pUsSemExp, pGrdExpToReplace, pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
    break;
  }
}

}

void synthesize
(std::list<std::unique_ptr<SynthesizerResult>>& pNaturalLanguageResult,
UniqueSemanticExpression pSemExp,
 bool pOneLinePerSentence,
 const SemanticMemoryBlock& pMemBlock,
 const std::string& pCurrentUserId,
 const TextProcessingContext& pTextProcContext,
 const linguistics::LinguisticDatabase& pLingDb,
 std::list<std::list<SemLineToPrint> >* pDebugOutput)
{
  pLingDb.treeConverter.refactorSemExp(pSemExp,
                                       TREEPATTERN_MIND, TREEPATTERN_OUTTEXT,
                                       pTextProcContext.langType, pDebugOutput);
  _replaceSemExpThatNeedToBeReplaced(pSemExp, *pTextProcContext.usSemExp,
                                     *std::make_unique<GroundedExpression>(std::make_unique<SemanticConceptualGrounding>("tolink_1p")),
                                     pMemBlock, pLingDb);

  const LinguisticSynthesizerPrivate& synthPriv =
      LinguisticSynthesizerPrivate::getInstance(pTextProcContext.langType);
  synthPriv.getTranslationToNaturalLanguage
      (pNaturalLanguageResult, *pSemExp,
       pOneLinePerSentence, pMemBlock, pCurrentUserId,
       pTextProcContext, pLingDb);
}



} // End of namespace onsem
