#ifndef ONSEM_CHATBOTPLANNER_CHATBOTPLANNER_HPP
#define ONSEM_CHATBOTPLANNER_CHATBOTPLANNER_HPP

#include <map>
#include <set>
#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <assert.h>
#include "api.hpp"
#include <onsem/chatbotplanner/alias.hpp>
#include <onsem/chatbotplanner/observableunsafe.hpp>


namespace onsem
{
namespace cp
{


#define ADD_HC_EXPRESSIONELEMENTTYPE_TABLE               \
  ADD_HC_EXPRESSIONELEMENTTYPE(OPERATOR, "operator")     \
  ADD_HC_EXPRESSIONELEMENTTYPE(VALUE, "value")           \
  ADD_HC_EXPRESSIONELEMENTTYPE(FACT, "fact")


#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) a,
enum class ExpressionElementType
{
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE



#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) {ExpressionElementType::a, b},
static const std::map<ExpressionElementType, std::string> _expressionElementType_toStr = {
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE

#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) {b, ExpressionElementType::a},
static const std::map<std::string, ExpressionElementType> _expressionElementType_fromStr = {
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE
#undef ADD_HC_EXPRESSIONELEMENTTYPE_TABLE


static inline std::string expressionElementType_toStr
(ExpressionElementType pHcActionType)
{
  return _expressionElementType_toStr.find(pHcActionType)->second;
}

static inline ExpressionElementType expressionElementType_fromStr
(const std::string& pStr)
{
  auto it = _expressionElementType_fromStr.find(pStr);
  if (it != _expressionElementType_fromStr.end())
    return it->second;
  assert(false);
  return ExpressionElementType::OPERATOR;
}






enum class ExpressionOperator
{
  PLUSPLUS,
  PLUS,
  MINUS,
  EQUAL,
  NOT
};



struct ONSEMCHATBOTPLANNER_API ExpressionElement
{
  ExpressionElement(ExpressionElementType pType,
                    const std::string& pValue)
    : type(pType),
      value(pValue)
  {
  }
  bool operator==(const ExpressionElement& pOther) const { return type == pOther.type &&
        value == pOther.value; }
  bool operator!=(const ExpressionElement& pOther) const { return !operator==(pOther); }
  ExpressionElementType type;
  std::string value;
};


struct ONSEMCHATBOTPLANNER_API Expression
{
  bool operator==(const Expression& pOther) const { return elts == pOther.elts; }
  bool operator!=(const Expression& pOther) const { return !operator==(pOther); }
  std::list<ExpressionElement> elts;
};


struct ONSEMCHATBOTPLANNER_API SetOfFacts
{
  SetOfFacts()
    : facts(),
      notFacts(),
      exps()
  {
  }
  SetOfFacts(const std::initializer_list<Fact>& pFacts,
             const std::initializer_list<Fact>& pNotFacts = {})
    : facts(pFacts),
      notFacts(pNotFacts),
      exps()
  {
  }
  bool empty() const { return facts.empty() && notFacts.empty() && exps.empty(); }
  bool operator==(const SetOfFacts& pOther) const
  { return facts == pOther.facts && notFacts == pOther.notFacts && exps == pOther.exps; }
  void add(const SetOfFacts& pOther)
  {
    facts.insert(pOther.facts.begin(), pOther.facts.end());
    notFacts.insert(pOther.notFacts.begin(), pOther.notFacts.end());
    exps.insert(exps.begin(), pOther.exps.begin(), pOther.exps.end());
  }
  bool containsFact(const Fact& pFact) const;
  void rename(const Fact& pOldFact,
              const Fact& pNewFact);
  std::list<std::pair<std::string, std::string>> toFactsStrs() const;
  std::list<std::string> toStrs() const;
  std::string toStr(const std::string& pSeparator) const;
  static SetOfFacts fromStr(const std::string& pStr,
                            const std::string& pSeparator);

  std::set<Fact> facts;
  std::set<Fact> notFacts;
  std::list<Expression> exps;
};


struct ONSEMCHATBOTPLANNER_API Action
{
  Action(const SetOfFacts& pPreconditions,
         const SetOfFacts& pEffects,
         const SetOfFacts& pPreferInContext = {},
         bool pShouldBeDoneAsapWithoutHistoryCheck = false)
    : preconditions(pPreconditions),
      preferInContext(pPreferInContext),
      effects(pEffects),
      shouldBeDoneAsapWithoutHistoryCheck(pShouldBeDoneAsapWithoutHistoryCheck)
  {
  }

  SetOfFacts preconditions;
  SetOfFacts preferInContext;
  SetOfFacts effects;
  // If this it's true it will have a very high priority for the planner.
  // It is approriate to use that for deduction actions.
  bool shouldBeDoneAsapWithoutHistoryCheck;
};

struct ONSEMCHATBOTPLANNER_API Domain
{
  Domain(const std::map<ActionId, Action>& pActions);

  const std::map<ActionId, Action>& actions() const { return _actions; }
  const std::map<Fact, std::set<ActionId>>& preconditionToActions() const { return _preconditionToActions; }
  const std::map<Fact, std::set<ActionId>>& preconditionToActionsExps() const { return _preconditionToActionsExps; }
  const std::map<Fact, std::set<ActionId>>& notPreconditionToActions() const { return _notPreconditionToActions; }
  const std::set<ActionId>& actionsWithoutPrecondition() const { return _actionsWithoutPrecondition; }

private:
  std::map<ActionId, Action> _actions;
  std::map<Fact, std::set<ActionId>> _preconditionToActions;
  std::map<Fact, std::set<ActionId>> _preconditionToActionsExps;
  std::map<Fact, std::set<ActionId>> _notPreconditionToActions;
  std::set<ActionId> _actionsWithoutPrecondition;
};



struct ONSEMCHATBOTPLANNER_API Historical
{
  void setMutex(std::shared_ptr<std::mutex> pMutex);
  void notifyActionDone(const ActionId& pActionId);
  bool hasActionId(const ActionId& pActionId) const;
  std::size_t getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const;
private:
  std::shared_ptr<std::mutex> _mutexPtr;
  std::map<ActionId, std::size_t> _actionToNumberOfTimeAleardyDone;

  void _notifyActionDone(const ActionId& pActionId);
  bool _hasActionId(const ActionId& pActionId) const;
  std::size_t _getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const;
};


struct ONSEMCHATBOTPLANNER_API Problem
{
  Problem() = default;
  Problem(const Problem& pOther);
  Historical historical{};
  cpstd::observable::ObservableUnsafe<void (const std::map<Fact, std::string>&)> onFactsToValueChanged{};
  cpstd::observable::ObservableUnsafe<void (const std::set<std::string>&)> onFactsChanged{};
  cpstd::observable::ObservableUnsafe<void (const std::vector<std::string>&)> onGoalsChanged{};

  std::string getCurrentGoal() const;
  void addFactsToValue(const std::map<Fact, std::string>& pFactsToValue);
  bool addFact(const Fact& pFact);
  template<typename FACTS>
  bool addFacts(const FACTS& pFacts);

  bool removeFact(const Fact& pFact);
  template<typename FACTS>
  bool removeFacts(const FACTS& pFacts);

  bool modifyFacts(const SetOfFacts& pSetOfFacts);
  void setFacts(const std::set<Fact>& pFacts);
  void addReachableFacts(const std::set<Fact>& pFacts);
  void addRemovableFacts(const std::set<Fact>& pFacts);
  void removeGoal(const Fact& pGoal);
  void noNeedToAddReachableFacts() { _needToAddReachableFacts = false; }
  bool needToAddReachableFacts() const { return _needToAddReachableFacts; }
  void setGoals(const std::vector<Fact>& pGoals);
  void addGoals(const std::vector<Fact>& pGoals);
  void pushBackGoal(const Fact& pGoal);
  void notifyActionDone(
      const std::string& pActionId,
      const cp::SetOfFacts& effect,
      const std::vector<cp::Fact>* pGoalsToAdd);
  const std::vector<Fact>& goals() const { return _goals; }
  const std::set<Fact>& facts() const { return _facts; }
  const std::map<Fact, std::string>& factsToValue() const { return _factsToValue; }
  const std::set<Fact>& reachableFacts() const { return _reachableFacts; }
  const std::set<Fact>& removableFacts() const { return _removableFacts; }

private:
  std::vector<Fact> _goals{};
  std::map<Fact, std::string> _factsToValue{};
  std::set<Fact> _facts{};
  std::set<Fact> _reachableFacts{};
  std::set<Fact> _removableFacts{};
  bool _needToAddReachableFacts = true;

  template<typename FACTS>
  bool _addFactsWithoutFactNotification(const FACTS& pFacts);

  template<typename FACTS>
  bool _removeFactsWithoutFactNotification(const FACTS& pFacts);

  void _removeDoneGoals();
  void _clearRechableAndRemovableFacts();
};


ONSEMCHATBOTPLANNER_API
void replaceVariables(std::string& pStr,
                      const Problem& pProblem);

ONSEMCHATBOTPLANNER_API
std::vector<cp::Fact> factsFromString(const std::string& pStr,
                                      const std::string& pSeparator);



ONSEMCHATBOTPLANNER_API
void fillReachableFacts(Problem& pProblem,
                        const Domain& pDomain);


ONSEMCHATBOTPLANNER_API
bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                  const Problem& pProblem);

ONSEMCHATBOTPLANNER_API
ActionId lookForAnActionToDo(Problem& pProblem,
                             const Domain& pDomain,
                             const Historical* pGlobalHistorical = nullptr);

ONSEMCHATBOTPLANNER_API
std::list<ActionId> solve(Problem& pProblem,
                          const Domain& pDomain,
                          Historical* pGlobalHistorical = nullptr);
} // !cp
} // !onsem


#endif // ONSEM_CHATBOTPLANNER_CHATBOTPLANNER_HPP