#include <onsem/tester/loadchatbot.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/fixedsynthesisexpression.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>

namespace onsem
{
namespace
{
std::size_t nextId = 0;

std::string newId(const std::map<cp::ActionId, ChatbotAction>& pActions) {
  while (true)
  {
    std::stringstream ss;
    ss << nextId++;
    std::string res;
    res = ss.str();
    if (pActions.count(res) == 0)
      return res;
  }
  assert(false);
  return "";
}
}

void loadChatbotDomain(ChatbotDomain& pChatbotDomain,
                       std::istream& pIstream)
{
  boost::property_tree::ptree pTree;
  boost::property_tree::read_json(pIstream, pTree);
  auto language = SemanticLanguageEnum::UNKNOWN;

  for (auto& currChatbotAttr : pTree)
  {
    if (currChatbotAttr.first == "language")
    {
      auto languageStr = currChatbotAttr.second.get_value<std::string>();
      if (languageStr == "en")
        language = SemanticLanguageEnum::ENGLISH;
      else if (languageStr == "fr")
        language = SemanticLanguageEnum::FRENCH;
      else if (languageStr == "ja")
        language = SemanticLanguageEnum::JAPANESE;
      else
        language = SemanticLanguageEnum::UNKNOWN;
    }
    else if (currChatbotAttr.first == "inform")
    {
      auto& inform = pChatbotDomain.inform[language];
      for (auto& currActionTree : currChatbotAttr.second)
        inform.push_back(currActionTree.second.get_value<std::string>());
    }
    else if (currChatbotAttr.first == "actions")
    {
      for (auto& currActionTree : currChatbotAttr.second)
      {
        cp::ActionId actionId = currActionTree.second.get("id", "");
        if (actionId.empty())
          actionId = newId(pChatbotDomain.actions);
        auto& currChatbotAction = pChatbotDomain.actions[actionId];
        currChatbotAction.language = language;

        currChatbotAction.trigger = currActionTree.second.get("trigger", "");
        currChatbotAction.text = currActionTree.second.get("text", "");

        auto parametersTreeOpt = currActionTree.second.get_child_optional("parameters");
        if (parametersTreeOpt)
        {
          for (auto& currParameterTree : *parametersTreeOpt)
          {
            currChatbotAction.parameters.emplace_back();
            auto& currParam = currChatbotAction.parameters.back();
            currParam.text = currParameterTree.second.get("text", "");
            currParam.effect = cp::SetOfFacts::fromStr(currParameterTree.second.get("effect", ""), ',');

            auto goalsToAddTreeOpt = currParameterTree.second.get_child_optional("goalsToAdd");
            if (goalsToAddTreeOpt)
              for (auto& currGoalTree : *goalsToAddTreeOpt)
                currParam.goalsToAdd.push_back(currGoalTree.second.get_value<std::string>());
          }
        }


        auto inputTreeOpt = currActionTree.second.get_child_optional("input");
        if (inputTreeOpt)
        {
          currChatbotAction.inputPtr = mystd::make_unique<ChatbotInput>();
          currChatbotAction.inputPtr->fact = inputTreeOpt->get("fact", "");
          currChatbotAction.inputPtr->effect = cp::SetOfFacts::fromStr(inputTreeOpt->get("effect", ""), ',');
        }

        currChatbotAction.precondition = cp::SetOfFacts::fromStr(currActionTree.second.get("precondition", ""), ',');
        currChatbotAction.preferInContext = cp::SetOfFacts::fromStr(currActionTree.second.get("preferInContext", ""), ',');
        currChatbotAction.effect = cp::SetOfFacts::fromStr(currActionTree.second.get("effect", ""), ',');
        currChatbotAction.potentialEffect = cp::SetOfFacts::fromStr(currActionTree.second.get("potentialEffect", ""), ',');
        currChatbotAction.shouldBeDoneAsapWithoutHistoryCheck = currActionTree.second.get("shouldBeDoneAsapWithoutHistoryCheck", false);

        auto goalsToAddTreeOpt = currActionTree.second.get_child_optional("goalsToAdd");
        if (goalsToAddTreeOpt)
          for (auto& currGoalTree : *goalsToAddTreeOpt)
            currChatbotAction.goalsToAdd.push_back(currGoalTree.second.get_value<std::string>());

      }
    }
  }
}


void loadChatbotProblem(ChatbotProblem& pChatbotProblem,
                        std::istream& pIstream)
{
  boost::property_tree::ptree pTree;
  boost::property_tree::read_json(pIstream, pTree);

  for (auto& currChatbotAttr : pTree)
  {
    if (currChatbotAttr.first == "language")
    {
      auto languageStr = currChatbotAttr.second.get_value<std::string>();
      if (languageStr == "en")
        pChatbotProblem.language = SemanticLanguageEnum::ENGLISH;
      else if (languageStr == "fr")
        pChatbotProblem.language = SemanticLanguageEnum::FRENCH;
      else if (languageStr == "ja")
        pChatbotProblem.language = SemanticLanguageEnum::JAPANESE;
      else
        pChatbotProblem.language = SemanticLanguageEnum::UNKNOWN;
    }
    else if (currChatbotAttr.first == "facts")
    {
      for (auto& currFactTree : currChatbotAttr.second)
        pChatbotProblem.problem.addFact(cp::Fact::fromStr(currFactTree.second.get_value<std::string>()));
    }
    else if (currChatbotAttr.first == "goals")
    {
      for (auto& currGoalTree : currChatbotAttr.second)
        pChatbotProblem.problem.pushBackGoal(currGoalTree.second.get_value<std::string>());
    }
  }
}


void addChatbotDomaintoASemanticMemory(
    SemanticMemory& pSemanticMemory,
    ChatbotDomain& pChatbotDomain,
    const linguistics::LinguisticDatabase& pLingDb)
{
  for (const auto& currInform : pChatbotDomain.inform)
  {
    auto textProc = TextProcessingContext::getTextProcessingContextToRobot(currInform.first);
    for (const auto& currText : currInform.second)
    {
      auto semExp = converter::textToContextualSemExp(currText, textProc, SemanticSourceEnum::UNKNOWN, pLingDb);
      memoryOperation::addAgentInterpretations(semExp, pSemanticMemory, pLingDb);
      memoryOperation::informAxiom(std::move(semExp), pSemanticMemory, pLingDb);
    }
  }

  std::map<cp::ActionId, cp::Action> actions;
  for (const auto& currActionWithId : pChatbotDomain.actions)
  {
    auto& currAction = currActionWithId.second;
    if (!currAction.trigger.empty())
    {
      auto textProcToRobot = TextProcessingContext::getTextProcessingContextToRobot(currAction.language);
      auto triggerSemExp = converter::textToContextualSemExp(currAction.trigger, textProcToRobot, SemanticSourceEnum::UNKNOWN, pLingDb);

      auto textProcFromRobot = TextProcessingContext::getTextProcessingContextFromRobot(currAction.language);
      const std::list<std::string> references{1, beginOfActionId + currActionWithId.first};
      auto semExpWithFiexedSynthesis = mystd::make_unique<FixedSynthesisExpression>(
            converter::textToContextualSemExp(currAction.text, textProcFromRobot, SemanticSourceEnum::UNKNOWN, pLingDb, &references));
      semExpWithFiexedSynthesis->langToSynthesis.emplace(currAction.language, currAction.text);

      memoryOperation::resolveAgentAccordingToTheContext(triggerSemExp, pSemanticMemory, pLingDb);
      memoryOperation::addATrigger(std::move(triggerSemExp),
                                   std::move(semExpWithFiexedSynthesis),
                                   pSemanticMemory, pLingDb);
    }

    cp::Action action(currAction.precondition, currAction.effect,
                      currAction.preferInContext);
    action.effects.add(currAction.potentialEffect);
    action.shouldBeDoneAsapWithoutHistoryCheck = currAction.shouldBeDoneAsapWithoutHistoryCheck;
    for (const auto& currParam : currAction.parameters)
      action.effects.add(currParam.effect);
    if (currAction.inputPtr)
      action.effects.add(currAction.inputPtr->effect);
    actions.emplace(currActionWithId.first, std::move(action));
  }
  pChatbotDomain.compiledDomain = mystd::make_unique<cp::Domain>(actions);
}


} // End of namespace onsem
