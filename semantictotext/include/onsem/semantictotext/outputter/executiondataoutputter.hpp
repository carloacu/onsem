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

  std::unique_ptr<UniqueSemanticExpression> punctualAssertion;
  std::unique_ptr<UniqueSemanticExpression> permanentAssertion;
  std::unique_ptr<UniqueSemanticExpression> informationToTeach;

  int numberOfRepetitions = 1;

  std::list<ExecutionData> toRunSequencially;
  std::list<ExecutionData> toRunInParallel;
  std::list<ExecutionData> toRunInBackground;

  bool hasData() const;
  std::list<ExecutionData>& linkToChildList(VirtualOutputter::Link pLink);
  std::string run(SemanticMemory& pSemanticMemory,
                  const linguistics::LinguisticDatabase &pLingDb);

private:
  std::string _dataToStr() const;
};


struct ONSEMSEMANTICTOTEXT_API ExecutionDataOutputter : public VirtualOutputter
{
  ExecutionDataOutputter(SemanticMemory& pSemanticMemory,
                         const linguistics::LinguisticDatabase& pLingDb);
  virtual ~ExecutionDataOutputter() {}

  ExecutionData rootExecutionData;


protected:
  void _exposeResource(const SemanticResource& pResource,
                       const std::map<std::string, std::vector<std::string>>& pParameters) override;

  void _exposeText(const std::string& pText,
                   SemanticLanguageEnum pLanguage) override;

  void _assertPunctually(UniqueSemanticExpression pUSemExp) override;
  void _teachInformation(UniqueSemanticExpression pUSemExp) override;
  void _assertPermanently(UniqueSemanticExpression pUSemExp) override;

  void _beginOfScope(Link pLink) override;
  void _endOfScope() override;
  void _insideScopeRepetition(int pNumberOfRepetitions) override;

private:
  std::list<Link> _linksStack;
  std::list<ExecutionData*> _executionDataStack;

  ExecutionData& _getOrCreateNewElt();
  ExecutionData& _getLasExectInStack();
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_OUTPUTTER_EXECUTIONDATAOUTPUTTER_HPP

