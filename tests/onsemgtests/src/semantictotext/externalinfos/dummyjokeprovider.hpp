#ifndef SEMANTICENGINE_MOCK_DUMMYJOKEPROVIDER_HPP
#define SEMANTICENGINE_MOCK_DUMMYJOKEPROVIDER_HPP

#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

namespace onsem
{
struct SemanticExpression;
namespace linguistics
{
struct LinguisticDatabase;
}


class DummyJokeProvider : public MemBlockAndExternalCallback
{
public:
  DummyJokeProvider(const linguistics::LinguisticDatabase& pLingDb);

  virtual mystd::unique_propagate_const<UniqueSemanticExpression> getAnswer
  (const IndexToSubNameToParameterValue& pParams,
   SemanticLanguageEnum pLanguageType) const;

  virtual std::string idStr() const { return idStrOfProv; }

  virtual std::list<UniqueSemanticExpression>& getSemExpThatCanBeAnswered()
  { return _semExpsThatCanBeAnswered; }
  virtual std::list<UniqueSemanticExpression>& getTriggers()
  { return _trigger; }

  static std::string idStrOfProv;

  static std::string frenchJoke();
  static std::string englishJoke();


private:
  std::list<UniqueSemanticExpression> _semExpsThatCanBeAnswered;
  std::list<UniqueSemanticExpression> _trigger;
};
}


#endif // !SEMANTICENGINE_MOCK_DUMMYJOKEPROVIDER_HPP
