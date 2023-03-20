#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_TREECONVERTER_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_TREECONVERTER_HPP

#include <mutex>
#include <onsem/common/keytostreams.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/common/enum/treepatternconventionenum.hpp>
#include "../../api.hpp"

namespace onsem
{
class StaticTreeConverter;
struct SemLineToPrint;


class ONSEM_TEXTTOSEMANTIC_API TreeConverter
{
public:
  TreeConverter(linguistics::LinguisticDatabaseStreams& pIStreams);

  void refactorSemExp(UniqueSemanticExpression& pUSemExp,
                      TreePatternConventionEnum pFromConvention,
                      TreePatternConventionEnum pToConvention,
                      SemanticLanguageEnum pLanguage,
                      std::list<std::list<SemLineToPrint> >* pDebugOutput = nullptr) const;

  void addDifferentForms(UniqueSemanticExpression& pSemExp,
                         SemanticLanguageEnum pLanguage,
                         std::list<std::list<SemLineToPrint> >* pDebugOutput) const;
  void addBothDirectionForms(UniqueSemanticExpression& pSemExp,
                             SemanticLanguageEnum pLanguage,
                             std::list<std::list<SemLineToPrint>>* pDebugOutput) const;

  GrammaticalType getChildThatShouldBeUnique(mystd::unique_propagate_const<UniqueSemanticExpression>& pChildThatShouldBeUnique,
                                             const GroundedExpression& pGrdExp) const;


  std::size_t getSize(std::string& pErrorStr,
                      bool& pIsLoaded) const;


private:
  const StaticTreeConverter& _statDb;

  static std::mutex _pathToStatDbsMutex;
  static std::unique_ptr<StaticTreeConverter> _statDbs;
  static const StaticTreeConverter& _getStatDbInstance(linguistics::LinguisticDatabaseStreams& pIStreams);
};



} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_TREECONVERTER_HPP
