#include "semexpgenerator.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
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
  auto nameGrd = std::make_unique<SemanticNameGrounding>(pNames);
  if (pConceptsPtr != nullptr)
    nameGrd->concepts.insert(pConceptsPtr->begin(), pConceptsPtr->end());
  nameGrd->concepts["name_*"] = 4;
  nameGrd->concepts["agent_*"] = 1;
  return nameGrd;
}

UniqueSemanticExpression makeCoreferenceExpression(CoreferenceDirectionEnum pDirection,
                                                   const mystd::optional<SemanticEntityType>& pEntityType)
{
  auto genGrd = std::make_unique<SemanticGenericGrounding>();
  genGrd->coreference.emplace(pDirection);
  if (pEntityType)
    genGrd->entityType = *pEntityType;
  return std::make_unique<GroundedExpression>(std::move(genGrd));
}

UniqueSemanticExpression emptyStatementSemExp()
{
  return std::make_unique<GroundedExpression>
      (std::make_unique<SemanticStatementGrounding>());
}


UniqueSemanticExpression whatIs(UniqueSemanticExpression pSubjectSemExp,
                                SemanticLanguageEnum pLanguage)
{
  auto rootGrdExp = std::make_unique<GroundedExpression>
      ([&]()
  {
    auto statementGrd = std::make_unique<SemanticStatementGrounding>();
    statementGrd->word.language = pLanguage;
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
  auto rootGrdExp = std::make_unique<GroundedExpression>
      ([]()
  {
    auto statementGrd = std::make_unique<SemanticStatementGrounding>();
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

