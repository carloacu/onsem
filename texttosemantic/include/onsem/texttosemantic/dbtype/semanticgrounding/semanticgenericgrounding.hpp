#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICGENERICGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICGENERICGROUNDING_HPP

#include <memory>
#include <set>
#include <onsem/common/utility/optional.hpp>
#include <onsem/common/enum/semanticentitytype.hpp>
#include <onsem/common/enum/semanticquantitytype.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticsubjectivequantity.hpp>
#include <onsem/texttosemantic/dbtype/semanticquantity.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/misc/coreference.hpp>
#include "semanticgrounding.hpp"
#include "../../api.hpp"

namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticGenericGrounding : public SemanticGrounding
{
  SemanticGenericGrounding()
    : SemanticGrounding(SemanticGroundingType::GENERIC),
      referenceType(SemanticReferenceType::UNDEFINED),
      coreference(),
      entityType(SemanticEntityType::UNKNOWN),
      quantity(),
      word(),
      possibleGenders()
  {
  }

  SemanticGenericGrounding
  (SemanticReferenceType pReferenceType,
   SemanticEntityType pAgentType);

  const SemanticGenericGrounding& getGenericGrounding() const override { return *this; }
  SemanticGenericGrounding& getGenericGrounding() override { return *this; }
  const SemanticGenericGrounding* getGenericGroundingPtr() const override { return this; }
  SemanticGenericGrounding* getGenericGroundingPtr() override { return this; }

  bool operator==(const SemanticGenericGrounding& pOther) const;
  bool isEqual(const SemanticGenericGrounding& pOther) const;

  static std::unique_ptr<SemanticGenericGrounding> makeThingThatHasToBeCompletedFromContext();

  SemanticReferenceType referenceType;
  mystd::optional<Coreference> coreference;
  SemanticEntityType entityType; // TODO: remove this attribute
  SemanticQuantity quantity;
  SemanticWord word;
  std::set<SemanticGenderType> possibleGenders;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICGENERICGROUNDING_HPP
