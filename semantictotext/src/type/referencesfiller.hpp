#ifndef ONSEM_SEMANTICTOTEXT_SRC_TYPE_REFERENCESFILLER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_TYPE_REFERENCESFILLER_HPP

#include <string>
#include <list>

namespace onsem
{
struct UniqueSemanticExpression;
struct ReferencesGetter;

class ReferencesFiller
{
public:
  ReferencesFiller(const ReferencesGetter& pReferencesGetter);
  ReferencesFiller(const std::list<std::string>& pReferences);

  void addReferences(UniqueSemanticExpression& pUSemExp) const;

private:
  const ReferencesGetter* _referencesGetterPtr;
  const std::list<std::string>* _references;
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_TYPE_REFERENCESFILLER_HPP
