#ifndef ONSEM_SEMANTICTOTEXT_SRC_TOOL_USERINFOSFILLER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_TOOL_USERINFOSFILLER_HPP

#include <vector>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>

namespace onsem
{
namespace SemExpUserInfosFiller
{

struct UserInfosContainer
{
  virtual ~UserInfosContainer() {}

  virtual void addNames(const std::string& pUserId,
                        const std::vector<std::string>& pNames) = 0;
  virtual void addGenders(const std::string& pUserId,
                          const std::set<SemanticGenderType>& pPossibleGenders) = 0;
  virtual void addEquivalentUserIds(const std::string& pSubjectUserId,
                                    const std::string& pObjectUserId) = 0;
  virtual void addGrdExpToUserId(const GroundedExpression& pSubjectGrdExp,
                                 const std::string& pObjectUserId) = 0;
};

void tryToAddUserInfosWithTense(UserInfosContainer& pUserInfosContainer,
                                const GroundedExpression& pGrdExp,
                                SemanticVerbTense pRootVerbTense);

void tryToAddUserInfos
(UserInfosContainer& pUserInfosContainer,
 const GroundedExpression& pGrdExp);

} // End of namespace SemExpUserInfosFiller

} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_TOOL_USERINFOSFILLER_HPP
