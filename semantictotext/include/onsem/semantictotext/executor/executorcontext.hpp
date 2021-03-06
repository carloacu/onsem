#ifndef ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_EXECUTORSPECS_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_EXECUTORSPECS_HPP

#include <atomic>
#include <memory>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include "../api.hpp"

namespace onsem
{
struct SemanticMemory;


struct ONSEMSEMANTICTOTEXT_API ExecutorContext
{
  ExecutorContext(const TextProcessingContext& pTextProcessingContext)
    : textProcContext(pTextProcessingContext),
      annotations(std::make_shared<std::map<GrammaticalType, UniqueSemanticExpression>>()),
      contAnnotation(ContextualAnnotation::PROACTIVE),
      sayOrExecute(false)
  {
  }

  void updateAnnotation(
      const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations)
  {
    for (const auto& currAnnotation : pAnnotations)
      (*annotations)[currAnnotation.first] = currAnnotation.second->clone();
  }

  TextProcessingContext textProcContext;
  std::shared_ptr<std::map<GrammaticalType, UniqueSemanticExpression>> annotations;
  ContextualAnnotation contAnnotation;
  bool sayOrExecute;
};



} // End of namespace onsem

#endif // !ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_EXECUTORSPECS_HPP
