#include "semexpsimilaritycoef.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>

namespace onsem
{
namespace
{

int _getSimilarityCoefFromGrd(const SemanticGrounding& pGrdExp1,
                              const SemanticGrounding& pGrdExp2)
{
  if (pGrdExp1.type != pGrdExp2.type)
    return 0;
  switch (pGrdExp1.type)
  {
  case SemanticGroudingType::AGENT:
  {
    const auto& agentGrd1 = pGrdExp1.getAgentGrounding();
    const auto& agentGrd2 = pGrdExp2.getAgentGrounding();
    if (agentGrd1.userId == agentGrd2.userId)
      return 1;
    break;
  }
  default:
    break;
  }
  return 0;
}


int _getSimilarityCoefFromGrdExp(const GroundedExpression& pGrdExp1,
                                 const GroundedExpression& pGrdExp2)
{
  int res = _getSimilarityCoefFromGrd(pGrdExp1.grounding(), pGrdExp2.grounding());
  for (const auto& currChild1 : pGrdExp1.children)
  {
    auto itChild2 = pGrdExp2.children.find(currChild1.first);
    if (itChild2 != pGrdExp2.children.end())
      res += getSimilarityCoef(*currChild1.second, *itChild2->second);
  }
  return res;
}

}


int getSimilarityCoef(const SemanticExpression& pSemExp1,
                      const SemanticExpression& pSemExp2)
{
  auto* grdExpPtr1 = pSemExp1.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr1 != nullptr)
  {
    auto& grdExp1 = *grdExpPtr1;
    auto* grdExpPtr2 = pSemExp2.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr2 != nullptr)
    {
      auto& grdExp2 = *grdExpPtr2;
      return _getSimilarityCoefFromGrdExp(grdExp1, grdExp2);
    }
  }
  return 0;
}


} // End of namespace onsem
