#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>

namespace onsem
{

GroundedExpression::GroundedExpression()
  : SemanticExpression(SemanticExpressionType::GROUNDED),
    GroundedExpressionContainer(),
    children(),
    _grounding(std::make_unique<SemanticConceptualGrounding>())
{
}


void GroundedExpression::assertEltsEqual(const GroundedExpression& pOther) const
{
  _assertChildrenEqual(children, pOther.children);
  assert(areEquals(_grounding, pOther._grounding));
}


std::unique_ptr<GroundedExpression> GroundedExpression::clone
(const IndexToSubNameToParameterValue* pParams,
 bool pRemoveRecentContextInterpretations,
 const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const
{
  auto res = std::make_unique<GroundedExpression>(cloneGrounding(pParams));
  for (const auto& currChild : children)
    res->children.emplace(currChild.first,
                          currChild.second->clone(pParams, pRemoveRecentContextInterpretations,
                                                  pExpressionTypesToSkip));
  return res;
}


std::unique_ptr<SemanticGrounding> GroundedExpression::cloneGrounding(const IndexToSubNameToParameterValue* pParams) const
{
  switch (_grounding->type)
  {
  case SemanticGroundingType::GENERIC:
  {
    const SemanticGenericGrounding& genGrd = _grounding->getGenericGrounding();
    if (pParams != nullptr)
    {
      int paramId = 0;
      std::string attributeName;
      if (genGrd.quantity.getNumberToFill(paramId, attributeName))
      {
        auto res = std::make_unique<SemanticGenericGrounding>(genGrd);
        for (const auto& currParam : *pParams)
          if (paramId == currParam.first)
            for (const auto& currAttrName : currParam.second)
              if (attributeName == currAttrName.first &&
                  currAttrName.second)
              {
                auto optNb = SemExpGetter::getNumberOfElements(currAttrName.second->getSemExp());
                if (optNb)
                  res->quantity.setNumber(*optNb);
              }
        return std::move(res);
      }
    }
    return std::make_unique<SemanticGenericGrounding>(genGrd);
  }
  case SemanticGroundingType::STATEMENT:
    return std::make_unique<SemanticStatementGrounding>(_grounding->getStatementGrounding());
  case SemanticGroundingType::AGENT:
    return std::make_unique<SemanticAgentGrounding>(_grounding->getAgentGrounding());
  case SemanticGroundingType::TIME:
    return std::make_unique<SemanticTimeGrounding>(_grounding->getTimeGrounding());
  case SemanticGroundingType::TEXT:
    return std::make_unique<SemanticTextGrounding>(_grounding->getTextGrounding());
  case SemanticGroundingType::DURATION:
    return std::make_unique<SemanticDurationGrounding>(_grounding->getDurationGrounding());
  case SemanticGroundingType::LANGUAGE:
    return std::make_unique<SemanticLanguageGrounding>(_grounding->getLanguageGrounding());
  case SemanticGroundingType::RELATIVELOCATION:
    return std::make_unique<SemanticRelativeLocationGrounding>(_grounding->getRelLocationGrounding());
  case SemanticGroundingType::RELATIVETIME:
    return std::make_unique<SemanticRelativeTimeGrounding>(_grounding->getRelTimeGrounding());
  case SemanticGroundingType::RELATIVEDURATION:
    return std::make_unique<SemanticRelativeDurationGrounding>(_grounding->getRelDurationGrounding());
  case SemanticGroundingType::RESOURCE:
    return std::make_unique<SemanticResourceGrounding>(_grounding->getResourceGrounding());
  case SemanticGroundingType::LENGTH:
    return std::make_unique<SemanticLengthGrounding>(_grounding->getLengthGrounding());
  case SemanticGroundingType::META:
    return std::make_unique<SemanticMetaGrounding>(_grounding->getMetaGrounding());
  case SemanticGroundingType::NAME:
    return std::make_unique<SemanticNameGrounding>(_grounding->getNameGrounding());
  case SemanticGroundingType::CONCEPTUAL:
    return std::make_unique<SemanticConceptualGrounding>(_grounding->getConceptualGrounding());
  case SemanticGroundingType::UNITY:
    return std::make_unique<SemanticUnityGrounding>(_grounding->getUnityGrounding());
  }
  assert(false);
  return std::unique_ptr<SemanticGrounding>();
}



} // End of namespace onsem
