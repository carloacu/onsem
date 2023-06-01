#ifndef ONSEM_SEMANTICTOTEXT_OUTPUTTER_EXECUTIONDATAOUTPUTTER_HPP
#define ONSEM_SEMANTICTOTEXT_OUTPUTTER_EXECUTIONDATAOUTPUTTER_HPP

#include "virtualoutputter.hpp"
#include "../api.hpp"
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>


namespace onsem
{

struct ONSEMSEMANTICTOTEXT_API ExecutionData
{
  std::string text;
  SemanticLanguageEnum textLanguage;

  std::unique_ptr<SemanticResource> resource;
  std::map<std::string, std::vector<std::string>> resourceParameters;

  std::list<ExecutionData> toRunSequencially;
  std::list<ExecutionData> toRunInParallel;
  std::list<ExecutionData> toRunInBackground;
};


struct ONSEMSEMANTICTOTEXT_API ExecutionDataOutputter : public VirtualOutputter
{
  ExecutionDataOutputter(SemanticMemory& pSemanticMemory,
                         const linguistics::LinguisticDatabase& pLingDb,
                         VirtualOutputterLogger& pLogOnSynchronousExecutionCase);
  virtual ~ExecutionDataOutputter() {}

  ExecutionData rootExecutionData;


protected:
  void _exposeResource(const SemanticResource& pResource,
                       const std::map<std::string, std::vector<std::string>>& pParameters) override;

  void _exposeText(const std::string& pText,
                   SemanticLanguageEnum pLanguage) override;

  void _beginOfScope(Link pLink) override;
  void _insideScopeLink(Link pLink) override;
  void _insideScopeRepetition(int pNumberOfRepetitions) override;
  void _endOfScope() override;

private:
  ExecutionData* _currentExecutionDataPtr;
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_OUTPUTTER_EXECUTIONDATAOUTPUTTER_HPP

