#include <onsem/semantictotext/semanticrecommendations.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include "controller/steps/semanticmemorygetter.hpp"

namespace onsem
{
namespace
{

void _linkToMemBlock(ExpressionHandleInMemory& pExpHandleInMemory,
                     const SemanticExpression& pSemExp,
                     const linguistics::LinguisticDatabase& pLingDb)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    pExpHandleInMemory.addAxiomWhereGatherAllTheLinks(*grdExpPtr, pLingDb);
    return;
  }

  const auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (const auto& currElt : listExpPtr->elts)
      _linkToMemBlock(pExpHandleInMemory, *currElt, pLingDb);
}


void _getLinksFromMemBlock(std::map<const ExpressionHandleInMemory*, int>& pLinks,
                           const SemanticExpression& pSemExp,
                           const SemanticRecommendationsContainer& pContainer,
                           const linguistics::LinguisticDatabase& pLingDb)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const auto& grdExp = *grdExpPtr;
    mystd::optional<int> groundingCoef;

    std::set<const ExpressionHandleInMemory*> expPtr;
    semanticMemoryGetter::findGrdExpInNominalGroupLinks(expPtr, grdExp, pContainer.goundingsToCoef.memBlock, pLingDb);
    for (const auto& currExp : expPtr)
    {
      auto itExpToCoef = pContainer.goundingsToCoef.expToCoef.find(currExp);
      if (itExpToCoef != pContainer.goundingsToCoef.expToCoef.end())
      {
        groundingCoef.emplace(itExpToCoef->second);
        break;
      }
    }

    if (!groundingCoef || *groundingCoef != 0)
      semanticMemoryGetter::findGrdExpWithCoefInNominalGroupLinks(pLinks, grdExp, groundingCoef, pContainer.memBlock, pLingDb);
    for (const auto& currChild : grdExp.children)
      if (currChild.first != GrammaticalType::INTRODUCTING_WORD)
        _getLinksFromMemBlock(pLinks, *currChild.second, pContainer, pLingDb);
    return;
  }

  const auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (const auto& currElt : listExpPtr->elts)
      _getLinksFromMemBlock(pLinks, *currElt, pContainer, pLingDb);
}


int _getNumberOfLinkedGroundings(const SemanticExpression& pSemExp)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const auto& grdExp = pSemExp.getGrdExp();
    int res = 1;
    for (const auto& currChild : grdExp.children)
      if (currChild.first != GrammaticalType::INTRODUCTING_WORD)
        res += _getNumberOfLinkedGroundings(*currChild.second);
    return res;
  }
  case SemanticExpressionType::LIST:
  {
    const auto& listExp = pSemExp.getListExp();
    int res = 0;
    for (const auto& currElt : listExp.elts)
      res += _getNumberOfLinkedGroundings(*currElt);
    return res;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return _getNumberOfLinkedGroundings(*pSemExp.getFdkExp().concernedExp);
  }
  case SemanticExpressionType::METADATA:
  {
    return _getNumberOfLinkedGroundings(*pSemExp.getMetadataExp().semExp);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return _getNumberOfLinkedGroundings(*pSemExp.getAnnExp().semExp);
  }
  case SemanticExpressionType::CONDITION:
  {
    const auto& condExp = pSemExp.getCondExp();
    int res = 0;
    res += _getNumberOfLinkedGroundings(*condExp.conditionExp);
    res += _getNumberOfLinkedGroundings(*condExp.thenExp);
    if (condExp.elseExp)
      res += _getNumberOfLinkedGroundings(**condExp.elseExp);
    return res;
  }
  case SemanticExpressionType::COMPARISON:
  {
    const auto& compExp = pSemExp.getCompExp();
    int res = 0;
    res += _getNumberOfLinkedGroundings(*compExp.leftOperandExp);
    if (compExp.rightOperandExp)
      res += _getNumberOfLinkedGroundings(**compExp.rightOperandExp);
    if (compExp.whatIsComparedExp)
      res += _getNumberOfLinkedGroundings(**compExp.whatIsComparedExp);
    return res;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    return _getNumberOfLinkedGroundings(*pSemExp.getIntExp().interpretedExp);
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    return _getNumberOfLinkedGroundings(pSemExp.getFSynthExp().getSemExp());
  }
  case SemanticExpressionType::SETOFFORMS:
  case SemanticExpressionType::COMMAND:
  {
    break;
  }
  }
  return 0;
}

}



void addGroundingCoef(SemanticGroundingsToRecommendationCoef& pGrdContainer,
                      UniqueSemanticExpression pGroundings,
                      int pCoef,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  auto expWrappedForMemory = pGrdContainer.memBlock.addRootSemExp(std::move(pGroundings), pLingDb);
  pGrdContainer.expsWrappedInMemory.emplace_back(expWrappedForMemory);
  pGrdContainer.expToCoef.emplace(&*expWrappedForMemory, pCoef);
  _linkToMemBlock(*expWrappedForMemory, *expWrappedForMemory->semExp, pLingDb);
}


void addARecommendation(SemanticRecommendationsContainer& pContainer,
                        UniqueSemanticExpression pRecommendation,
                        const std::string& pRecommendationId,
                        const linguistics::LinguisticDatabase& pLingDb)
{
  if (!pContainer.recommendationIds.insert(pRecommendationId).second)
    return;

  auto expWrappedForMemory = pContainer.memBlock.addRootSemExp(std::move(pRecommendation), pLingDb);
  pContainer.expsWrappedInMemory.emplace_back(expWrappedForMemory);
  int sizeOfSemExp = _getNumberOfLinkedGroundings(*expWrappedForMemory->semExp);
  pContainer.recommendationsToNumberOfLinks.emplace
      (&*expWrappedForMemory, std::pair<std::string, int>(pRecommendationId, sizeOfSemExp));
  _linkToMemBlock(*expWrappedForMemory, *expWrappedForMemory->semExp, pLingDb);
}


void getRecommendations(std::map<int, std::set<std::string>>& pRecommendations,
                        const SemanticExpression& pInput,
                        const SemanticRecommendationsContainer& pContainer,
                        const linguistics::LinguisticDatabase& pLingDb,
                        const std::set<std::string>& pForbiddenRecommendations)
{
  std::map<const ExpressionHandleInMemory*, int> expToSimilarities;
  _getLinksFromMemBlock(expToSimilarities, pInput, pContainer, pLingDb);

  for (const auto& currExpToSimilarity : expToSimilarities)
  {
    auto itToName = pContainer.recommendationsToNumberOfLinks.find(currExpToSimilarity.first);
    if (itToName != pContainer.recommendationsToNumberOfLinks.end())
    {
      int coef = currExpToSimilarity.second - itToName->second.second;
      if (coef > 0 && pForbiddenRecommendations.count(itToName->second.first) == 0)
        pRecommendations[coef].insert(itToName->second.first);
    }
    else
    {
      assert(false);
    }
  }
}



} // End of namespace onsem
