#include "semexpgenerator.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>

namespace onsem
{

namespace SemExpGenerator
{

std::unique_ptr<SemanticNameGrounding> makeNameGrd(const std::list<std::string>& pNames,
                                                   const std::map<std::string, char>* pConceptsPtr)
{
  auto nameGrd = mystd::make_unique<SemanticNameGrounding>(pNames);
  if (pConceptsPtr != nullptr)
    nameGrd->concepts.insert(pConceptsPtr->begin(), pConceptsPtr->end());
  nameGrd->concepts["name_*"] = 4;
  nameGrd->concepts["agent_*"] = 1;
  return nameGrd;
}

UniqueSemanticExpression makeCoreferenceExpression(CoreferenceDirectionEnum pDirection,
                                                   const mystd::optional<SemanticEntityType>& pEntityType)
{
  auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
  genGrd->coreference.emplace(pDirection);
  if (pEntityType)
    genGrd->entityType = *pEntityType;
  return mystd::make_unique<GroundedExpression>(std::move(genGrd));
}

UniqueSemanticExpression makeHumanCoreferenceBefore()
{
  auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
  genGrd->coreference.emplace(CoreferenceDirectionEnum::BEFORE);
  genGrd->entityType = SemanticEntityType::HUMAN;
  return mystd::make_unique<GroundedExpression>(std::move(genGrd));
}

UniqueSemanticExpression emptyStatementSemExp()
{
  return mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticStatementGrounding>());
}


UniqueSemanticExpression whatIs(UniqueSemanticExpression pSubjectSemExp)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->requests.set(SemanticRequestType::OBJECT);
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("verb_equal_be", 4);
    statementGrd->concepts.emplace("verb_equal_mean", 4);
    return statementGrd;
  }());

  rootGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pSubjectSemExp));
  return std::move(rootGrdExp);
}

UniqueSemanticExpression whatAbout(UniqueSemanticExpression pSubjectSemExp)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->requests.set(SemanticRequestType::ABOUT);
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("aboutQuestion", 4);
    return statementGrd;
  }());

  rootGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pSubjectSemExp));
  return std::move(rootGrdExp);
}


} // End of namespace SemExpGenerator

} // End of namespace onsem

