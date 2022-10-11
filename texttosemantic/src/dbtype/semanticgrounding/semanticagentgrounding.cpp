#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/common/utility/string.hpp>


namespace onsem
{
const std::string SemanticAgentGrounding::currentUser = "currentUser";
const std::string SemanticAgentGrounding::anyUser = "any";
const std::string SemanticAgentGrounding::userNotIdentified = "-1";
const std::string SemanticAgentGrounding::me = "me";


SemanticAgentGrounding::SemanticAgentGrounding()
  : SemanticGrounding(SemanticGroundingType::AGENT),
    userId(userNotIdentified),
    nameInfos()
{
  if (userId == SemanticAgentGrounding::me)
    concepts["agent_*"] = 4;
  else
    concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding
(const std::string& pUserId)
  : SemanticGrounding(SemanticGroundingType::AGENT),
    userId(pUserId),
    nameInfos()
{
  if (userId == SemanticAgentGrounding::me)
    concepts["agent_*"] = 4;
  else
    concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding
(const std::string& pUserId,
 const NameInfos& pNameInfos)
  : SemanticGrounding(SemanticGroundingType::AGENT),
    userId(pUserId),
    nameInfos(pNameInfos)
{
  if (userId == SemanticAgentGrounding::me)
    concepts["agent_*"] = 4;
  else
    concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::string& pUserId,
                                               const std::vector<std::string>& pNames)
  : SemanticGrounding(SemanticGroundingType::AGENT),
    userId(pUserId),
    nameInfos(pNames)
{
  if (userId == SemanticAgentGrounding::me)
    concepts["agent_*"] = 4;
  else
    concepts["agent_human_*"] = 4;
}


SemanticAgentGrounding::SemanticAgentGrounding(const std::string& pUserId,
                                               const std::list<std::string>& pNames)
  : SemanticGrounding(SemanticGroundingType::AGENT),
    userId(pUserId),
    nameInfos(pNames)
{
  if (userId == SemanticAgentGrounding::me)
    concepts["agent_*"] = 4;
  else
    concepts["agent_human_*"] = 4;
}

SemanticAgentGrounding::SemanticAgentGrounding(const std::vector<std::string>& pNames)
  : SemanticGrounding(SemanticGroundingType::AGENT),
    userId(namesToUserId(pNames)),
    nameInfos(pNames)
{
  if (userId == SemanticAgentGrounding::me)
    concepts["agent_*"] = 4;
  else
    concepts["agent_human_*"] = 4;
}


SemanticAgentGrounding::SemanticAgentGrounding(const SemanticAgentGrounding& pOther)
  : SemanticGrounding(pOther),
    userId(pOther.userId),
    nameInfos(pOther.nameInfos)
{
}


std::unique_ptr<SemanticAgentGrounding> SemanticAgentGrounding::getRobotAgentPtr()
{
  return mystd::make_unique<SemanticAgentGrounding>(SemanticAgentGrounding::me);
}


std::string SemanticAgentGrounding::namesToUserId(const std::vector<std::string>& pNames)
{
  return mystd::join(pNames, "-");
}


} // End of namespace onsem
