#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/common/utility/string.hpp>

namespace onsem {

SemanticAgentGrounding::SemanticAgentGrounding()
    : SemanticGrounding(SemanticGroundingType::AGENT)
    , userId(getUserNotIdentified())
    , userIdWithoutContext(getUserNotIdentified())
    , nameInfos() {
    if (userId == getMe())
        concepts["agent_*"] = 4;
    else
        concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::string& pUserId)
    : SemanticGrounding(SemanticGroundingType::AGENT)
    , userId(pUserId)
    , userIdWithoutContext(pUserId)
    , nameInfos() {
    if (userId == getMe())
        concepts["agent_*"] = 4;
    else
        concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::string& pUserId, const std::string& pUserIdWithoutContext)
    : SemanticGrounding(SemanticGroundingType::AGENT)
    , userId(pUserId)
    , userIdWithoutContext(pUserIdWithoutContext)
    , nameInfos() {
    if (userId == getMe())
        concepts["agent_*"] = 4;
    else
        concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::string& pUserId,
                                               const std::string& pUserIdWithoutContext,
                                               const NameInfos& pNameInfos)
    : SemanticGrounding(SemanticGroundingType::AGENT)
    , userId(pUserId)
    , userIdWithoutContext(pUserIdWithoutContext)
    , nameInfos(pNameInfos) {
    if (userId == getMe())
        concepts["agent_*"] = 4;
    else
        concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::string& pUserId,
                                               const std::string& pUserIdWithoutContext,
                                               const std::vector<std::string>& pNames)
    : SemanticGrounding(SemanticGroundingType::AGENT)
    , userId(pUserId)
    , userIdWithoutContext(pUserIdWithoutContext)
    , nameInfos(pNames) {
    if (userId == getMe())
        concepts["agent_*"] = 4;
    else
        concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::string& pUserId,
                                               const std::string& pUserIdWithoutContext,
                                               const std::list<std::string>& pNames)
    : SemanticGrounding(SemanticGroundingType::AGENT)
    , userId(pUserId)
    , userIdWithoutContext(pUserIdWithoutContext)
    , nameInfos(pNames) {
    if (userId == getMe())
        concepts["agent_*"] = 4;
    else
        concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::vector<std::string>& pNames)
    : SemanticGrounding(SemanticGroundingType::AGENT)
    , userId(namesToUserId(pNames))
    , userIdWithoutContext(userId)
    , nameInfos(pNames) {
    if (userId == getMe())
        concepts["agent_*"] = 4;
    else
        concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const SemanticAgentGrounding& pOther)
    : SemanticGrounding(pOther)
    , userId(pOther.userId)
    , userIdWithoutContext(pOther.userIdWithoutContext)
    , nameInfos(pOther.nameInfos) {}

std::unique_ptr<SemanticAgentGrounding> SemanticAgentGrounding::getRobotAgentPtr() {
    return std::make_unique<SemanticAgentGrounding>(getMe());
}

std::string SemanticAgentGrounding::namesToUserId(const std::vector<std::string>& pNames) {
    return mystd::join(pNames, "-");
}

std::string SemanticAgentGrounding::namesToUserId(const std::list<std::string>& pNames) {
    return mystd::join(pNames, "-");
}


const std::string& SemanticAgentGrounding::getCurrentUser()
{
  static const std::string currentUser = "currentUser";
  return currentUser;
}

const std::string& SemanticAgentGrounding::getAnyUser()
{
  static const std::string anyUser = "any";
  return anyUser;
}

const std::string& SemanticAgentGrounding::getUserNotIdentified()
{
  static const std::string userNotIdentified = "-1";
  return userNotIdentified;
}

const std::string& SemanticAgentGrounding::getMe()
{
  static const std::string me = "me";
  return me;
}



}    // End of namespace onsem
