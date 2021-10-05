#include <onsem/texttosemantic/dbtype/linguisticdatabase/treeconverter.hpp>
#include "statictreeconverter.hpp"


namespace onsem
{
std::mutex TreeConverter::_pathToStatDbsMutex{};
std::unique_ptr<StaticTreeConverter> TreeConverter::_statDbs{};


const StaticTreeConverter& TreeConverter::_getStatDbInstance(linguistics::LinguisticDatabaseStreams& pIStreams)
{
  std::lock_guard<std::mutex> lock(_pathToStatDbsMutex);
  if (!_statDbs)
    _statDbs = mystd::make_unique<StaticTreeConverter>(pIStreams);
  return *_statDbs;
}


TreeConverter::TreeConverter(linguistics::LinguisticDatabaseStreams& pIStreams)
 : _statDb(_getStatDbInstance(pIStreams))
{
}


void TreeConverter::refactorSemExp
(UniqueSemanticExpression& pUSemExp,
 TreePatternConventionEnum pFromConvention,
 TreePatternConventionEnum pToConvention,
 SemanticLanguageEnum pLanguage,
 std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const
{
  _statDb.refactorSemExp(pUSemExp, pFromConvention, pToConvention, pLanguage, pDebugOutput);
}

void TreeConverter::splitPossibilitiesOfQuestions
(UniqueSemanticExpression& pSemExp,
 SemanticLanguageEnum pLanguage,
 std::list<std::list<ALSemLineToPrint>>* pDebugOutput) const
{
  _statDb.splitPossibilitiesOfQuestions(pSemExp, pLanguage, true, pDebugOutput);
}

void TreeConverter::splitEquivalentQuestions
(UniqueSemanticExpression& pSemExp,
 SemanticLanguageEnum pLanguage,
 std::list<std::list<ALSemLineToPrint>>* pDebugOutput) const
{
  _statDb.splitPossibilitiesOfQuestions(pSemExp, pLanguage, false, pDebugOutput);
}


GrammaticalType TreeConverter::getChildThatShouldBeUnique
(mystd::unique_propagate_const<UniqueSemanticExpression>& pChildThatShouldBeUnique,
 const GroundedExpression& pGrdExp) const
{
  return _statDb.getChildThatShouldBeUnique(pChildThatShouldBeUnique, pGrdExp);
}


std::size_t TreeConverter::getSize(std::string& pErrorStr,
                                   bool& pIsLoaded) const
{
  return _statDb.getSize(pErrorStr, pIsLoaded);
}




} // End of namespace onsem
