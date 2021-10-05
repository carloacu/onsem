#ifndef ONSEM_SEMANTICTOTEXT_SEMEXPSIMPLIFER_HPP
#define ONSEM_SEMANTICTOTEXT_SEMEXPSIMPLIFER_HPP

#include <memory>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "api.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemoryBlock;

namespace simplifier
{

ONSEMSEMANTICTOTEXT_API
void processFromMemBlock(UniqueSemanticExpression& pSemExp,
                         const SemanticMemoryBlock& pMemBlock,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pConvertUnrelatedListToAndList = false);

ONSEMSEMANTICTOTEXT_API
void process(UniqueSemanticExpression& pSemExp,
             const SemanticMemory& pSemanticMemory,
             const linguistics::LinguisticDatabase& pLingDb,
             bool pConvertUnrelatedListToAndList = false);


ONSEMSEMANTICTOTEXT_API
void solveConditionsInplace(UniqueSemanticExpression& pSemExp,
                            const SemanticMemoryBlock& pMemBlock,
                            const linguistics::LinguisticDatabase& pLingDb);




template <typename TUniqueSemExp>
struct ConditionSolvedResult
{
  ConditionSolvedResult(TUniqueSemExp& pRootSemExp,
                        TruenessValue pTruenessValue)
    : rootSemExp(pRootSemExp),
      truenessValue(pTruenessValue)
  {
  }

  TUniqueSemExp& rootSemExp;
  TruenessValue truenessValue;
};


template <typename TUniqueSemExp>
void solveConditions(std::list<ConditionSolvedResult<TUniqueSemExp>>& pRes,
                     TUniqueSemExp& pUSemExp,
                     const SemanticMemoryBlock& pMemBlock,
                     const linguistics::LinguisticDatabase& pLingDb)
{
  switch (pUSemExp->type)
  {
  case SemanticExpressionType::LIST:
  {
    auto& listExp = pUSemExp->getListExp();
    for (auto& currElt : listExp.elts)
      solveConditions<TUniqueSemExp>(pRes, pUSemExp.wrapInContainer(currElt), pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pUSemExp->getIntExp();
    solveConditions<TUniqueSemExp>(pRes, pUSemExp.wrapInContainer(intExp.interpretedExp), pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    auto& fdkExp = pUSemExp->getFdkExp();
    solveConditions<TUniqueSemExp>(pRes, pUSemExp.wrapInContainer(fdkExp.concernedExp), pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    auto& annExp = pUSemExp->getAnnExp();
    solveConditions<TUniqueSemExp>(pRes, pUSemExp.wrapInContainer(annExp.semExp), pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metadataExp = pUSemExp->getMetadataExp();
    solveConditions<TUniqueSemExp>(pRes, pUSemExp.wrapInContainer(metadataExp.semExp), pMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    auto& condExp = pUSemExp->getCondExp();
    TruenessValue truenessVal = memoryOperation::check(*condExp.conditionExp, pMemBlock, pLingDb);
    pRes.emplace_back(pUSemExp, truenessVal);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::GROUNDED:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}

} // End of namespace simplifier
} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMEXPSIMPLIFER_HPP
