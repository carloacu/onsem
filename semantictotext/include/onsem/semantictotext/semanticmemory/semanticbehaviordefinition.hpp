#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICBEHAVIORDEFINITION_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICBEHAVIORDEFINITION_HPP

#include "../api.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>


namespace onsem
{

struct ONSEMSEMANTICTOTEXT_API SemanticBehaviorDefinition
{
  SemanticBehaviorDefinition(UniqueSemanticExpression pLabel = UniqueSemanticExpression(),
                             UniqueSemanticExpression pComposition = UniqueSemanticExpression())
    : label(std::move(pLabel)),
      composition(std::move(pComposition))
  {
  }

  UniqueSemanticExpression label;
  UniqueSemanticExpression composition;
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICBEHAVIORDEFINITION_HPP
