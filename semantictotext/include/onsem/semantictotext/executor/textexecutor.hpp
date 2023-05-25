#ifndef ONSEM_SEMANTICTOTEXT_EXECUTOR_TEXTEXECUTOR_HPP
#define ONSEM_SEMANTICTOTEXT_EXECUTOR_TEXTEXECUTOR_HPP

#include "virtualexecutor.hpp"
#include "../api.hpp"
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>


namespace onsem
{

struct ONSEMSEMANTICTOTEXT_API TextExecutor : public VirtualExecutor
{
  TextExecutor(SemanticMemory& pSemanticMemory,
               const linguistics::LinguisticDatabase& pLingDb,
               VirtualExecutorLogger& pLogOnSynchronousExecutionCase);
  virtual ~TextExecutor() {}

protected:
  void _synchronicityWrapper(std::function<void()> pFunction) override;
  void _usageOfMemory(std::function<void(SemanticMemory&)> pFunction) override;
  void _usageOfMemblock(std::function<void(const SemanticMemoryBlock&, const std::string&)> pFunction) override;
  void _usageOfLingDb(std::function<void(const linguistics::LinguisticDatabase&)> pFunction) override;

  FutureVoid _exposeResource(const SemanticResource& pResource,
                             const SemanticExpression* pInputSemExpPtr,
                             const FutureVoid&) override;

private:
  SemanticMemory& _semanticMemory;
  const linguistics::LinguisticDatabase& _lingDb;
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_EXECUTOR_TEXTEXECUTOR_HPP

