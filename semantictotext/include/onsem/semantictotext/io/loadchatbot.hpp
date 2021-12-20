#ifndef ONSEM_SEMANTICTOTEXT_IO_LOADCHATBOT_HPP
#define ONSEM_SEMANTICTOTEXT_IO_LOADCHATBOT_HPP

#include <istream>
#include <onsem/chatbotplanner/chatbotplanner.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../api.hpp"

namespace onsem
{
const std::string beginOfActionId = "actionId-";

struct ONSEMSEMANTICTOTEXT_API ChatbotParam
{
  std::string text{};
  lp::SetOfFacts effect{};
  std::vector<lp::Fact> goalsToAdd{};
};

struct ONSEMSEMANTICTOTEXT_API ChatbotInput
{
  lp::Fact fact{};
  lp::SetOfFacts effect{};
};


struct ONSEMSEMANTICTOTEXT_API ChatbotAction
{
  SemanticLanguageEnum language{SemanticLanguageEnum::UNKNOWN};
  std::string trigger{};
  std::string text{};
  std::vector<ChatbotParam> parameters{};
  std::unique_ptr<ChatbotInput> inputPtr{};
  lp::SetOfFacts precondition{};
  lp::SetOfFacts preferInContext{};
  lp::SetOfFacts effect{};
  lp::SetOfFacts potentialEffect{};
  std::vector<lp::Fact> goalsToAdd{};
  bool shouldBeDoneAsapWithoutHistoryCheck{};
};

struct ONSEMSEMANTICTOTEXT_API ChatbotDomain
{
  std::map<SemanticLanguageEnum, std::vector<std::string>> inform{};
  std::map<lp::ActionId, ChatbotAction> actions{};
  std::unique_ptr<lp::CompiledProblem> compiledDomain{};
};


struct ONSEMSEMANTICTOTEXT_API ChatbotProblem
{
  SemanticLanguageEnum language{SemanticLanguageEnum::UNKNOWN};
  lp::State state{};
};


ONSEMSEMANTICTOTEXT_API
void loadChatbotDomain(ChatbotDomain& pChatbotDomain,
                       std::istream& pIstream);


ONSEMSEMANTICTOTEXT_API
void loadChatbotProblem(ChatbotProblem& pChatbotProblem,
                        std::istream& pIstream);



ONSEMSEMANTICTOTEXT_API
void addChatbotDomaintoASemanticMemory(
    SemanticMemory& pSemanticMemory,
    ChatbotDomain& pChatbotDomain,
    const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_IO_LOADCHATBOT_HPP
