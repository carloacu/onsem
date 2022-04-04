#ifndef INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
#define INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP

#include <map>
#include <set>
#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <assert.h>
#include "api.hpp"
#include <contextualplanner/alias.hpp>
#include <contextualplanner/fact.hpp>
#include <contextualplanner/goal.hpp>
#include <contextualplanner/observableunsafe.hpp>


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



struct CONTEXTUALPLANNER_API ExpressionElement
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


struct CONTEXTUALPLANNER_API Expression
{
  bool operator==(const Expression& pOther) const { return elts == pOther.elts; }
  bool operator!=(const Expression& pOther) const { return !operator==(pOther); }
  std::list<ExpressionElement> elts;
};


struct CONTEXTUALPLANNER_API SetOfFacts
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
                            char pSeparator);

  std::set<Fact> facts;
  std::set<Fact> notFacts;
  std::list<Expression> exps;
};


struct CONTEXTUALPLANNER_API Action
{
  Action(const SetOfFacts& pPreconditions,
         const SetOfFacts& pEffects,
         const SetOfFacts& pPreferInContext = {},
         bool pShouldBeDoneAsapWithoutHistoryCheck = false)
    : parameters(),
      preconditions(pPreconditions),
      preferInContext(pPreferInContext),
      effects(pEffects),
      shouldBeDoneAsapWithoutHistoryCheck(pShouldBeDoneAsapWithoutHistoryCheck)
  {
  }

  std::vector<std::string> parameters;
  SetOfFacts preconditions;
  SetOfFacts preferInContext;
  SetOfFacts effects;
  // If this it's true it will have a very high priority for the planner.
  // It is approriate to use that for deduction actions.
  bool shouldBeDoneAsapWithoutHistoryCheck;
};

struct CONTEXTUALPLANNER_API Domain
{
  Domain(const std::map<ActionId, Action>& pActions);

  const std::map<ActionId, Action>& actions() const { return _actions; }
  const std::map<std::string, std::set<ActionId>>& preconditionToActions() const { return _preconditionToActions; }
  const std::map<std::string, std::set<ActionId>>& preconditionToActionsExps() const { return _preconditionToActionsExps; }
  const std::map<std::string, std::set<ActionId>>& notPreconditionToActions() const { return _notPreconditionToActions; }
  const std::set<ActionId>& actionsWithoutPrecondition() const { return _actionsWithoutPrecondition; }

private:
  std::map<ActionId, Action> _actions;
  std::map<std::string, std::set<ActionId>> _preconditionToActions;
  std::map<std::string, std::set<ActionId>> _preconditionToActionsExps;
  std::map<std::string, std::set<ActionId>> _notPreconditionToActions;
  std::set<ActionId> _actionsWithoutPrecondition;
};



struct CONTEXTUALPLANNER_API Historical
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


struct CONTEXTUALPLANNER_API Problem
{
  Problem() = default;
  Problem(const Problem& pOther);
  Historical historical{};
  cpstd::observable::ObservableUnsafe<void (const std::map<std::string, std::string>&)> onVariablesToValueChanged{};
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsChanged{};
  cpstd::observable::ObservableUnsafe<void (const std::vector<Goal>&)> onGoalsChanged{};

  std::string getCurrentGoal() const;
  void addVariablesToValue(const std::map<std::string, std::string>& pVariablesToValue);
  bool addFact(const Fact& pFact);
  template<typename FACTS>
  bool addFacts(const FACTS& pFacts);

  bool removeFact(const Fact& pFact);
  template<typename FACTS>
  bool removeFacts(const FACTS& pFacts);

  bool modifyFacts(const SetOfFacts& pSetOfFacts);
  void setFacts(const std::set<Fact>& pFacts);
  void addReachableFacts(const std::set<Fact>& pFacts);
  void addReachableFactsWithAnyValues(const std::vector<Fact>& pFacts);
  void addRemovableFacts(const std::set<Fact>& pFacts);
  void noNeedToAddReachableFacts() { _needToAddReachableFacts = false; }
  bool needToAddReachableFacts() const { return _needToAddReachableFacts; }
  void iterateOnGoalAndRemoveNonPersistent(
      const std::function<bool(const Goal&)>& pManageGoal);
  void setGoals(const std::vector<Goal>& pGoals);
  void addGoals(const std::vector<Goal>& pGoals);
  void pushBackGoal(const Goal& pGoal);
  void notifyActionDone(const std::string& pActionId,
                        const std::map<std::string, std::string>& pParameters,
                        const SetOfFacts& pEffect,
                        const std::vector<Goal>* pGoalsToAdd);
  const std::vector<Goal>& goals() const { return _goals; }
  const std::set<Fact>& facts() const { return _facts; }
  const std::map<std::string, std::size_t>& factNamesToNbOfFactOccurences() const { return _factNamesToNbOfFactOccurences; }
  const std::map<std::string, std::string>& variablesToValue() const { return _variablesToValue; }
  const std::set<Fact>& reachableFacts() const { return _reachableFacts; }
  const std::set<Fact>& reachableFactsWithAnyValues() const { return _reachableFactsWithAnyValues; }
  const std::set<Fact>& removableFacts() const { return _removableFacts; }

private:
  std::vector<Goal> _goals{};
  std::map<std::string, std::string> _variablesToValue{};
  std::set<Fact> _facts{};
  std::map<std::string, std::size_t> _factNamesToNbOfFactOccurences{};
  std::set<Fact> _reachableFacts{};
  std::set<Fact> _reachableFactsWithAnyValues{};
  std::set<Fact> _removableFacts{};
  bool _needToAddReachableFacts = true;

  template<typename FACTS>
  bool _addFactsWithoutFactNotification(const FACTS& pFacts);

  template<typename FACTS>
  bool _removeFactsWithoutFactNotification(const FACTS& pFacts);

  void _clearRechableAndRemovableFacts();
  void _addFactNameRef(const std::string& pFactName);
};


CONTEXTUALPLANNER_API
void replaceVariables(std::string& pStr,
                      const Problem& pProblem);

CONTEXTUALPLANNER_API
std::vector<cp::Fact> factsFromString(const std::string& pStr,
                                      char pSeparator);



CONTEXTUALPLANNER_API
void fillReachableFacts(Problem& pProblem,
                        const Domain& pDomain);


CONTEXTUALPLANNER_API
bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                  const Problem& pProblem);

CONTEXTUALPLANNER_API
ActionId lookForAnActionToDo(std::map<std::string, std::string>& pParameters,
                             Problem& pProblem,
                             const Domain& pDomain,
                             const Historical* pGlobalHistorical = nullptr);

CONTEXTUALPLANNER_API
std::string printActionIdWithParameters(
    const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters);

CONTEXTUALPLANNER_API
std::list<ActionId> solve(Problem& pProblem,
                          const Domain& pDomain,
                          Historical* pGlobalHistorical = nullptr);
} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
