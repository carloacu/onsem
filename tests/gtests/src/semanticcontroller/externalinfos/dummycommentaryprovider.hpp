#ifndef SEMANTICENGINE_MOCK_DUMMYCOMMENTARYPROVIDER_HPP
#define SEMANTICENGINE_MOCK_DUMMYCOMMENTARYPROVIDER_HPP

#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

namespace onsem
{
struct SemanticExpression;
namespace linguistics
{
struct LinguisticDatabase;
}



class DummyCommentaryProvider : public MemBlockAndExternalCallback
{
public:
  DummyCommentaryProvider(const linguistics::LinguisticDatabase& pLingDb);

  virtual mystd::unique_propagate_const<UniqueSemanticExpression> getAnswer
  (const IndexToSubNameToParameterValue& pParams,
   SemanticLanguageEnum pLanguageType) const;

  virtual std::string idStr() const { return idStrOfProv; }

  virtual std::list<UniqueSemanticExpression>& getSemExpThatCanBeAnswered()
  { return _semExpsThatCanBeAnswered; }
  virtual std::list<UniqueSemanticExpression>& getTriggers()
  { return _triggers; }

  static std::string idStrOfProv;

  static std::string frenchCommentary();
  static std::string englishCommentary();


private:
  std::list<UniqueSemanticExpression> _semExpsThatCanBeAnswered;
  std::list<UniqueSemanticExpression> _triggers;
};
}


#endif // SEMANTICENGINE_MOCK_DUMMYCOMMENTARYPROVIDER_HPP
