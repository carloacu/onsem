#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_INTERACTIONCONTEXT_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_INTERACTIONCONTEXT_HPP

#include <map>
#include <memory>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>


namespace onsem
{

struct InteractionContext
{
  UniqueSemanticExpression textToSay;
  std::unique_ptr<UniqueSemanticExpression> fallbackTextToSay;
  std::map<UniqueSemanticExpression, int> answerPossibilities;
};


struct InteractionContextContainer
{
  std::map<int, InteractionContext> interactionContexts;
  InteractionContext* currentPosition = nullptr;
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_INTERACTIONCONTEXT_HPP
