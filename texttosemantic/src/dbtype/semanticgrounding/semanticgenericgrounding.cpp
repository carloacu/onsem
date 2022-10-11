#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/common/utility/make_unique.hpp>


namespace onsem
{


SemanticGenericGrounding::SemanticGenericGrounding
(SemanticReferenceType pReferenceType,
 SemanticEntityType pAgentType)
  : SemanticGrounding(SemanticGroundingType::GENERIC),
    referenceType(pReferenceType),
    coreference(),
    entityType(pAgentType),
    quantity(),
    word(),
    possibleGenders()
{
}


bool SemanticGenericGrounding::operator==(const SemanticGenericGrounding& pOther) const
{
  return this->isEqual(pOther);
}


bool SemanticGenericGrounding::isEqual(const SemanticGenericGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      referenceType == pOther.referenceType &&
      coreference == pOther.coreference &&
      entityType == pOther.entityType &&
      quantity == pOther.quantity &&
      word == pOther.word &&
      possibleGenders == pOther.possibleGenders;
}


std::unique_ptr<SemanticGenericGrounding> SemanticGenericGrounding::makeThingThatHasToBeCompletedFromContext()
{
  auto res = mystd::make_unique<SemanticGenericGrounding>
      (SemanticReferenceType::DEFINITE, SemanticEntityType::THING);
  res->coreference.emplace();
  return res;
}


} // End of namespace onsem
