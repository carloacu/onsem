#ifndef ONSEM_SEMANTICTOTEXT_OUTPUTTER_TEXTOUTPUTTER_HPP
#define ONSEM_SEMANTICTOTEXT_OUTPUTTER_TEXTOUTPUTTER_HPP

#include "virtualoutputter.hpp"
#include "../api.hpp"
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>


namespace onsem
{

struct ONSEMSEMANTICTOTEXT_API TextOutputter : public VirtualOutputter
{
  TextOutputter(SemanticMemory& pSemanticMemory,
                const linguistics::LinguisticDatabase& pLingDb,
                VirtualOutputterLogger& pLogOnSynchronousExecutionCase);
  virtual ~TextOutputter() {}

protected:
  void _synchronicityWrapper(std::function<void()> pFunction) override;
  void _usageOfMemory(std::function<void(SemanticMemory&)> pFunction) override;
  void _usageOfMemblock(std::function<void(const SemanticMemoryBlock&, const std::string&)> pFunction) override;
  void _usageOfLingDb(std::function<void(const linguistics::LinguisticDatabase&)> pFunction) override;

  void _exposeResource(const SemanticResource& pResource,
                       const SemanticExpression* pInputSemExpPtr) override;

private:
  SemanticMemory& _semanticMemory;
  const linguistics::LinguisticDatabase& _lingDb;
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_OUTPUTTER_TEXTOUTPUTTER_HPP

