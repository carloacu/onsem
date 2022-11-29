#include <onsem/semantictotext/recommendations.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include "controller/steps/semanticmemorygetter.hpp"

namespace onsem
{
namespace
{

void _linkToMemBlock(ExpressionWithLinks& pExpHandleInMemory,
                     const SemanticExpression& pSemExp,
                     const linguistics::LinguisticDatabase& pLingDb)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    pExpHandleInMemory.addAxiomForARecommendation(*grdExpPtr, pLingDb);
    return;
  }

  const auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (const auto& currElt : listExpPtr->elts)
      _linkToMemBlock(pExpHandleInMemory, *currElt, pLingDb);
}


void _getLinksFromMemBlock(std::map<const ExpressionWithLinks*, int>& pLinks,
                           const SemanticExpression& pSemExp,
                           const SemanticRecommendationsContainer& pContainer,
                           const linguistics::LinguisticDatabase& pLingDb,
                           SemanticLanguageEnum pLanguage)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::METADATA:
  {
    auto& metadaExp = pSemExp.getMetadataExp();
    auto language = metadaExp.fromLanguage != SemanticLanguageEnum::UNKNOWN ? metadaExp.fromLanguage : pLanguage;
    _getLinksFromMemBlock(pLinks, *metadaExp.semExp, pContainer, pLingDb, language);
    break;
  }
  case SemanticExpressionType::GROUNDED:
  {
    const auto& grdExp = pSemExp.getGrdExp();
    mystd::optional<int> groundingCoef;

    std::set<const ExpressionWithLinks*> expPtr;
    semanticMemoryGetter::findGrdExpInRecommendationLinks(expPtr, grdExp, pContainer.goundingsToCoef.memBlock,
                                                        pLingDb, pLanguage);
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
      semanticMemoryGetter::findGrdExpWithCoefInRecommendationLinks(pLinks, grdExp, groundingCoef,
                                                                  pContainer.memBlock, pLingDb, pLanguage);
    for (const auto& currChild : grdExp.children)
      if (currChild.first != GrammaticalType::INTRODUCTING_WORD)
        _getLinksFromMemBlock(pLinks, *currChild.second, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    const auto& listExp = pSemExp.getListExp();
    for (const auto& currElt : listExp.elts)
      _getLinksFromMemBlock(pLinks, *currElt, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pSemExp.getIntExp();
    _getLinksFromMemBlock(pLinks, *intExp.interpretedExp, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    auto& fdkExp = pSemExp.getFdkExp();
    _getLinksFromMemBlock(pLinks, *fdkExp.concernedExp, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    auto& annExp = pSemExp.getAnnExp();
    _getLinksFromMemBlock(pLinks, *annExp.semExp, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::COMMAND:
  {
    auto& cmdExp = pSemExp.getCmdExp();
    _getLinksFromMemBlock(pLinks, *cmdExp.semExp, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = pSemExp.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      _getLinksFromMemBlock(pLinks, **originalFrom, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = pSemExp.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      _getLinksFromMemBlock(pLinks, *semExp, pContainer, pLingDb, pLanguage);
    break;
  }
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
    break;
  }
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
                        std::size_t pMaxNbOfRecommendationsToAdd,
                        const SemanticExpression& pInput,
                        const SemanticRecommendationsContainer& pContainer,
                        const linguistics::LinguisticDatabase& pLingDb,
                        const std::set<std::string>& pForbiddenRecommendations)
{
  std::map<const ExpressionWithLinks*, int> expToSimilarities;
  _getLinksFromMemBlock(expToSimilarities, pInput, pContainer, pLingDb, SemanticLanguageEnum::UNKNOWN);
  int minCoefToAllowANewRecommendation = 0;

  for (const auto& currExpToSimilarity : expToSimilarities)
  {
    auto itToName = pContainer.recommendationsToNumberOfLinks.find(currExpToSimilarity.first);
    if (itToName != pContainer.recommendationsToNumberOfLinks.end())
    {
      int coef = currExpToSimilarity.second - itToName->second.second;
      if (coef > minCoefToAllowANewRecommendation && pForbiddenRecommendations.count(itToName->second.first) == 0)
      {
        pRecommendations[coef].insert(itToName->second.first);
        if (pMaxNbOfRecommendationsToAdd == 0)
        {
          // Remove the less important recommendation
          pRecommendations.begin()->second.erase(pRecommendations.begin()->second.begin());
          if (pRecommendations.begin()->second.empty())
            pRecommendations.erase(pRecommendations.begin());
          if (!pRecommendations.empty())
            minCoefToAllowANewRecommendation = pRecommendations.begin()->first;
        }
        else
        {
          if (pMaxNbOfRecommendationsToAdd == 1)
            minCoefToAllowANewRecommendation = pRecommendations.begin()->first;
          --pMaxNbOfRecommendationsToAdd;
        }
      }
    }
    else
    {
      assert(false);
    }
  }
}



} // End of namespace onsem
