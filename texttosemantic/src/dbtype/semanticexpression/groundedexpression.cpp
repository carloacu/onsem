#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>

namespace onsem
{

GroundedExpression::GroundedExpression()
  : SemanticExpression(SemanticExpressionType::GROUNDED),
    GroundedExpressionContainer(),
    children(),
    _grounding(mystd::make_unique<SemanticConceptualGrounding>())
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
  auto res = mystd::make_unique<GroundedExpression>(cloneGrounding(pParams));
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
  case SemanticGroudingType::GENERIC:
  {
    const SemanticGenericGrounding& genGrd = _grounding->getGenericGrounding();
    if (pParams != nullptr)
    {
      int paramId = 0;
      std::string attributeName;
      if (genGrd.quantity.getNumberToFill(paramId, attributeName))
      {
        auto res = mystd::make_unique<SemanticGenericGrounding>(genGrd);
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
    return mystd::make_unique<SemanticGenericGrounding>(genGrd);
  }
  case SemanticGroudingType::STATEMENT:
    return mystd::make_unique<SemanticStatementGrounding>(_grounding->getStatementGrounding());
  case SemanticGroudingType::AGENT:
    return mystd::make_unique<SemanticAgentGrounding>(_grounding->getAgentGrounding());
  case SemanticGroudingType::TIME:
    return mystd::make_unique<SemanticTimeGrounding>(_grounding->getTimeGrounding());
  case SemanticGroudingType::TEXT:
    return mystd::make_unique<SemanticTextGrounding>(_grounding->getTextGrounding());
  case SemanticGroudingType::DISTANCE:
    return mystd::make_unique<SemanticDistanceGrounding>(_grounding->getDistanceGrounding());
  case SemanticGroudingType::DURATION:
    return mystd::make_unique<SemanticDurationGrounding>(_grounding->getDurationGrounding());
  case SemanticGroudingType::LANGUAGE:
    return mystd::make_unique<SemanticLanguageGrounding>(_grounding->getLanguageGrounding());
  case SemanticGroudingType::RELATIVELOCATION:
    return mystd::make_unique<SemanticRelativeLocationGrounding>(_grounding->getRelLocationGrounding());
  case SemanticGroudingType::RELATIVETIME:
    return mystd::make_unique<SemanticRelativeTimeGrounding>(_grounding->getRelTimeGrounding());
  case SemanticGroudingType::RELATIVEDURATION:
    return mystd::make_unique<SemanticRelativeDurationGrounding>(_grounding->getRelDurationGrounding());
  case SemanticGroudingType::RESOURCE:
    return mystd::make_unique<SemanticResourceGrounding>(_grounding->getResourceGrounding());
  case SemanticGroudingType::META:
    return mystd::make_unique<SemanticMetaGrounding>(_grounding->getMetaGrounding());
  case SemanticGroudingType::NAME:
    return mystd::make_unique<SemanticNameGrounding>(_grounding->getNameGrounding());
  case SemanticGroudingType::CONCEPTUAL:
    return mystd::make_unique<SemanticConceptualGrounding>(_grounding->getConceptualGrounding());
  }
  assert(false);
  return std::unique_ptr<SemanticGrounding>();
}



} // End of namespace onsem
