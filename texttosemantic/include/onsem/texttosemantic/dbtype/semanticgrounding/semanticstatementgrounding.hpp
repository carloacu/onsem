#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICSTATEMENTGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICSTATEMENTGROUNDING_HPP

#include <onsem/common/utility/optional.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/misc/coreference.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include "semanticgrounding.hpp"
#include "../../api.hpp"


namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticStatementGrounding : public SemanticGrounding
{
  SemanticStatementGrounding()
    : SemanticGrounding(SemanticGroudingType::STATEMENT),
      requests(),
      word(),
      verbTense(SemanticVerbTense::UNKNOWN),
      verbGoal(VerbGoalEnum::NOTIFICATION),
      coreference(),
      isPassive()
  {
  }

  static std::unique_ptr<SemanticStatementGrounding> makeCoreference();

  const SemanticStatementGrounding& getStatementGrounding() const override { return *this; }
  SemanticStatementGrounding& getStatementGrounding() override { return *this; }
  const SemanticStatementGrounding* getStatementGroundingPtr() const override { return this; }
  SemanticStatementGrounding* getStatementGroundingPtr() override { return this; }

  bool operator==(const SemanticStatementGrounding& pOther) const;
  bool isEqual(const SemanticStatementGrounding& pOther) const;

  bool isAtInfinitive() const { return verbTense == SemanticVerbTense::UNKNOWN; }
  bool isMandatoryInPresentTense() const {
    return verbGoal == VerbGoalEnum::MANDATORY &&
        verbTense == SemanticVerbTense::PRESENT; }

  SemanticRequests requests;

  /// The word here is in fact the verb in the statement.
  /// @todo Rename to "verb"?
  SemanticWord word;

  SemanticVerbTense verbTense;
  VerbGoalEnum verbGoal;
  mystd::optional<Coreference> coreference;
  mystd::optional<bool> isPassive;
};





inline bool SemanticStatementGrounding::operator==(const SemanticStatementGrounding& pOther) const
{
  return this->isEqual(pOther);
}


inline bool SemanticStatementGrounding::isEqual(const SemanticStatementGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      requests == pOther.requests &&
      word == pOther.word &&
      verbTense == pOther.verbTense &&
      verbGoal == pOther.verbGoal &&
      coreference == pOther.coreference &&
      isPassive == pOther.isPassive;
}


inline std::unique_ptr<SemanticStatementGrounding> SemanticStatementGrounding::makeCoreference()
{
  auto coreferenceStatementGrdExp = mystd::make_unique<SemanticStatementGrounding>();
  coreferenceStatementGrdExp->coreference.emplace();
  return coreferenceStatementGrdExp;
}



} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICSTATEMENTGROUNDING_HPP
