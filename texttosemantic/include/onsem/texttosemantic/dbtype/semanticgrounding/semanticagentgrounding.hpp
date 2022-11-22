#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICAGENTGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICAGENTGROUNDING_HPP

#include <memory>
#include <onsem/common/utility/optional.hpp>
#include "semanticgrounding.hpp"
#include "semanticnamegrounding.hpp"
#include "../../api.hpp"


namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticAgentGrounding : public SemanticGrounding
{
  SemanticAgentGrounding();
  SemanticAgentGrounding(const std::string& pUserId);
  SemanticAgentGrounding(const std::string& pUserId,
                         const std::string& pUserIdWithoutContext,
                         const NameInfos& pNameInfos);
  SemanticAgentGrounding(const std::string& pUserId,
                         const std::string& pUserIdWithoutContext,
                         const std::vector<std::string>& pNames);
  SemanticAgentGrounding(const std::string& pUserId,
                         const std::string& pUserIdWithoutContext,
                         const std::list<std::string>& pNames);
  SemanticAgentGrounding(const std::vector<std::string>& pNames);
  SemanticAgentGrounding(const SemanticAgentGrounding& pOther);

  const SemanticAgentGrounding& getAgentGrounding() const override { return *this; }
  SemanticAgentGrounding& getAgentGrounding() override { return *this; }
  const SemanticAgentGrounding* getAgentGroundingPtr() const override { return this; }
  SemanticAgentGrounding* getAgentGroundingPtr() override { return this; }

  static std::unique_ptr<SemanticAgentGrounding> getRobotAgentPtr();
  static std::string namesToUserId(const std::vector<std::string>& pNames);
  static std::string namesToUserId(const std::list<std::string>& pNames);

  bool operator==(const SemanticAgentGrounding& pOther) const { return this->isEqual(pOther); }
  bool isEqual(const SemanticAgentGrounding& pOther) const
  { return _isMotherClassEqual(pOther) && userId == pOther.userId &&
        nameInfos == pOther.nameInfos; }

  bool isSpecificUser() const
  { return userId != userNotIdentified && userId != anyUser; }

  const std::string userId;
  const std::string userIdWithoutContext;
  mystd::optional<NameInfos> nameInfos;

  static const std::string currentUser;
  static const std::string anyUser;
  static const std::string userNotIdentified;
  static const std::string me;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICAGENTGROUNDING_HPP
