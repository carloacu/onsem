#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_INTERACTIONCONTEXT_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_INTERACTIONCONTEXT_HPP

#include <map>
#include <memory>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include "../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API InteractionContext
{
  UniqueSemanticExpression textToSay;
  std::unique_ptr<UniqueSemanticExpression> fallbackTextToSay;
  std::list<std::pair<UniqueSemanticExpression, int>> answerPossibilities;

  InteractionContext clone() const;
};


struct ONSEM_TEXTTOSEMANTIC_API InteractionContextContainer
{
  mystd::optional<int> currentPosition;

  int addInteractionContext(InteractionContext pInteractionContext);
  const InteractionContext* getCurrentInteractionContextPtr() const;
  const InteractionContext* getInteractionContextPtr(int pId) const;
  InteractionContext* getInteractionContextPtr(int pId);

  std::unique_ptr<InteractionContextContainer> clone() const;

private:
  std::map<int, InteractionContext> _interactionContexts;
};



} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_INTERACTIONCONTEXT_HPP
