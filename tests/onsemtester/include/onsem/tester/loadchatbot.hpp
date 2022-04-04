#ifndef ONSEM_SEMANTICTOTEXT_IO_LOADCHATBOT_HPP
#define ONSEM_SEMANTICTOTEXT_IO_LOADCHATBOT_HPP

#include <istream>
#include <contextualplanner/contextualplanner.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "api.hpp"

namespace onsem
{
const std::string beginOfActionId = "actionId-";

struct ONSEMTESTER_API ChatbotParam
{
  std::string text{};
  cp::SetOfFacts effect{};
  std::vector<cp::Goal> goalsToAdd{};
};

struct ONSEMTESTER_API ChatbotInput
{
  cp::Fact fact{};
  cp::SetOfFacts effect{};
};


struct ONSEMTESTER_API ChatbotAction
{
  SemanticLanguageEnum language{SemanticLanguageEnum::UNKNOWN};
  std::string trigger{};
  std::string text{};
  std::vector<ChatbotParam> parameters{};
  std::unique_ptr<ChatbotInput> inputPtr{};
  cp::SetOfFacts precondition{};
  cp::SetOfFacts preferInContext{};
  cp::SetOfFacts effect{};
  cp::SetOfFacts potentialEffect{};
  std::vector<cp::Goal> goalsToAdd{};
  bool shouldBeDoneAsapWithoutHistoryCheck{};
};

struct ONSEMTESTER_API ChatbotDomain
{
  std::map<SemanticLanguageEnum, std::vector<std::string>> inform{};
  std::map<cp::ActionId, ChatbotAction> actions{};
  std::unique_ptr<cp::Domain> compiledDomain{};
};


struct ONSEMTESTER_API ChatbotProblem
{
  SemanticLanguageEnum language{SemanticLanguageEnum::UNKNOWN};
  cp::Problem problem{};
};


ONSEMTESTER_API
void loadChatbotDomain(ChatbotDomain& pChatbotDomain,
                       std::istream& pIstream);


ONSEMTESTER_API
void loadChatbotProblem(ChatbotProblem& pChatbotProblem,
                        std::istream& pIstream);



ONSEMTESTER_API
void addChatbotDomaintoASemanticMemory(
    SemanticMemory& pSemanticMemory,
    ChatbotDomain& pChatbotDomain,
    const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_IO_LOADCHATBOT_HPP
