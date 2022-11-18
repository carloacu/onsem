#ifndef ONSEM_GTESTS_SEMANTICCONTROLLER_EXTERNALFALLBACK_TABLETFALLBACK_HPP
#define ONSEM_GTESTS_SEMANTICCONTROLLER_EXTERNALFALLBACK_TABLETFALLBACK_HPP

#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

namespace onsem
{


class TabletFallback : public ExternalFallback
{
public:
  TabletFallback();

  virtual void addFallback(UniqueSemanticExpression& pSemExp,
                           const std::string& pUserId,
                           const GroundedExpression& pOriginalGrdExp) const;

};


} // end namespace onsem


#endif // ONSEM_GTESTS_SEMANTICCONTROLLER_EXTERNALFALLBACK_TABLETFALLBACK_HPP
