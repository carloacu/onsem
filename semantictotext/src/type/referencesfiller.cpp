#include "referencesfiller.hpp"
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/referencesgetter.hpp>

namespace onsem
{


ReferencesFiller::ReferencesFiller(const ReferencesGetter& pReferencesGetter)
  : _referencesGetterPtr(&pReferencesGetter),
    _references(nullptr)
{
}

ReferencesFiller::ReferencesFiller(const std::list<std::string>& pReferences)
  : _referencesGetterPtr(nullptr),
    _references(&pReferences)
{
}

void ReferencesFiller::addReferences(UniqueSemanticExpression& pUSemExp) const
{
  if (_referencesGetterPtr != nullptr)
  {
    std::list<std::string> references;
    _referencesGetterPtr->getReferences(references);
    if (!references.empty())
      SemExpModifier::addReferences(pUSemExp, references);
  }
  else if (_references != nullptr &&
           !_references->empty())
  {
    SemExpModifier::addReferences(pUSemExp, *_references);
  }
}


} // End of namespace onsem
