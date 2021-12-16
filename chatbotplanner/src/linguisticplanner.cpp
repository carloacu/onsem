#include <onsem/chatbotplanner/linguisticplanner.hpp>
#include <assert.h>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/chatbotplanner/arithmeticevaluator.hpp>

namespace onsem
{
namespace lp
{

namespace
{
const std::map<std::string, ExpressionOperator> _strToBeginOfTextOperators
{{"++", ExpressionOperator::PLUSPLUS}};
const std::map<char, ExpressionOperator> _charToOperators
{{'=', ExpressionOperator::EQUAL}, {'+', ExpressionOperator::PLUS}, {'+', ExpressionOperator::MINUS}};

struct FactsAlreadychecked
{
  std::set<Fact> factsToAdd;
  std::set<Fact> factsToRemove;
};

template<typename CONTAINER_TYPE>
static inline std::string _listOfStrToStr(const CONTAINER_TYPE& pStrs,
                                          const std::string& pSeparator = "\n")
{
  std::string res;
  for (const auto& curStr : pStrs)
  {
    if (!res.empty())
      res += pSeparator;
    res += curStr;
  }
  return res;
}

void _incrementStr(std::string& pStr)
{
  if (pStr.empty())
  {
    pStr = "1";
  }
  else
  {
    try
    {
      std::stringstream ss;
      ss << mystd::lexical_cast<int>(pStr) + 1;
      pStr = ss.str();
    }
    catch (...) {}
  }
}

struct PotentialNextAction
{
 PotentialNextAction()
   : actionId(""),
     actionPtr(nullptr)
 {
 }
  PotentialNextAction(const ActionId& pActionId,
                      const Action& pAction);

  ActionId actionId;
  const Action* actionPtr;

  bool isMoreImportantThan(const PotentialNextAction& pOther,
                           const State& pState,
                           const Historical* pGlobalHistorical) const;
};


PotentialNextAction::PotentialNextAction(const ActionId& pActionId,
                                         const Action& pAction)
  : actionId(pActionId),
    actionPtr(&pAction)
{
}


void _getPrecoditionStatistics(std::size_t& nbOfPreconditionsSatisfied,
                               std::size_t& nbOfPreconditionsNotSatisfied,
                               const Action& pAction,
                               const std::set<Fact>& pFacts)
{
  nbOfPreconditionsSatisfied = pAction.preconditions.facts.size();
  for (const auto& currOptPrecond : pAction.preferInContext.facts)
  {
    if (pFacts.count(currOptPrecond) > 0)
      ++nbOfPreconditionsSatisfied;
    else
      ++nbOfPreconditionsNotSatisfied;
  }
}


bool PotentialNextAction::isMoreImportantThan(const PotentialNextAction& pOther,
                                              const State& pState,
                                              const Historical* pGlobalHistorical) const
{
  if (actionPtr == nullptr)
    return false;
  if (pOther.actionPtr == nullptr)
    return true;
  auto& action = *actionPtr;
  if (action.shouldBeDoneAsapWithoutHistoryCheck)
    return true;
  auto& otherAction = *pOther.actionPtr;

  auto nbOfTimesAlreadyDone = pState.historical.getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
  auto otherNbOfTimesAlreadyDone = pState.historical.getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);
  if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
    return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;

  std::size_t nbOfPreconditionsSatisfied = 0;
  std::size_t nbOfPreconditionsNotSatisfied = 0;
  _getPrecoditionStatistics(nbOfPreconditionsSatisfied, nbOfPreconditionsNotSatisfied, action, pState.facts());
  std::size_t otherNbOfPreconditionsSatisfied = 0;
  std::size_t otherNbOfPreconditionsNotSatisfied = 0;
  _getPrecoditionStatistics(otherNbOfPreconditionsSatisfied, otherNbOfPreconditionsNotSatisfied, otherAction, pState.facts());
  if (nbOfPreconditionsSatisfied != otherNbOfPreconditionsSatisfied)
    return nbOfPreconditionsSatisfied > otherNbOfPreconditionsSatisfied;
  if (nbOfPreconditionsNotSatisfied != otherNbOfPreconditionsNotSatisfied)
    return nbOfPreconditionsNotSatisfied < otherNbOfPreconditionsNotSatisfied;

  if (pGlobalHistorical != nullptr)
  {
    nbOfTimesAlreadyDone = pGlobalHistorical->getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
    otherNbOfTimesAlreadyDone = pGlobalHistorical->getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);
    if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
      return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;
  }

  return actionId < pOther.actionId;
}

std::string _expressionEltToValue(const ExpressionElement& pExpElt,
                                  const std::map<Fact, std::string>& pFactsToValue)
{
  if (pExpElt.type == ExpressionElementType::FACT)
  {
    auto it = pFactsToValue.find(pExpElt.value);
    if (it != pFactsToValue.end())
      return it->second;
    return "";
  }
  return pExpElt.value;
}

bool _areExpsValid(const std::list<Expression>& pExps,
                   const std::map<Fact, std::string>& pFactsToValue)
{
  for (const auto& currExp : pExps)
  {
    if (currExp.elts.size() >= 3)
    {
      auto it = currExp.elts.begin();
      auto val1 = _expressionEltToValue(*it, pFactsToValue);
      ++it;
      if (it->type != ExpressionElementType::OPERATOR ||
          it->value != "=")
        return false;
      ++it;
      auto val2 = _expressionEltToValue(*it, pFactsToValue);
      ++it;
      while (it != currExp.elts.end())
      {
        if (it->type != ExpressionElementType::OPERATOR)
          return false;
        auto op = it->value;
        ++it;
        if (it == currExp.elts.end())
          break;
        auto val3 = _expressionEltToValue(*it, pFactsToValue);
        if (op == "+" || op == "-")
          val2 = evaluteToStr(val2 + op + val3);
        else
          return false;
        ++it;
      }
      if (it != currExp.elts.end() || val1 != val2)
        return false;
    }
  }
  return true;
}


bool _canFactsBecomeTrue(const SetOfFacts& pSetOfFacts,
                         const State& pState)
{
  auto& facts = pState.facts();
  auto& reachableFacts = pState.reachableFacts();
  for (const auto& currPrecond : pSetOfFacts.facts)
    if (facts.count(currPrecond) == 0 &&
        reachableFacts.count(currPrecond) == 0)
      return false;
  auto& removableFacts = pState.removableFacts();
  for (const auto& currPrecond : pSetOfFacts.notFacts)
    if (facts.count(currPrecond) > 0 &&
        removableFacts.count(currPrecond) == 0)
      return false;
  return _areExpsValid(pSetOfFacts.exps, pState.factsToValue());
}


bool _willTheActionAddOrRemoveAFact(const Action& pAction,
                                    const std::set<Fact>& pFacts)
{
  for (const auto& currPrecond : pAction.effects.facts)
    if (pFacts.count(currPrecond) == 0)
      return true;
  for (const auto& currPrecond : pAction.effects.notFacts)
    if (pFacts.count(currPrecond) > 0)
      return true;
  return false;
}


void _getTheFactsToAddFromTheActionEffects(std::set<Fact>& pNewFacts,
                                           const Action& pAction,
                                           const std::set<Fact>& pFacts1,
                                           const std::set<Fact>& pFacts2)
{
  for (const auto& currPrecond : pAction.effects.facts)
    if (pFacts1.count(currPrecond) == 0 &&
        pFacts2.count(currPrecond) == 0)
      pNewFacts.insert(currPrecond);
}

void _getTheFactsToRemoveFromTheActionEffects(std::set<Fact>& pFactsToRemove,
                                              const Action& pAction,
                                              const std::set<Fact>& pFacts1,
                                              const std::set<Fact>& pFacts2)
{
  for (const auto& currPrecond : pAction.effects.notFacts)
    if (pFacts1.count(currPrecond) > 0 &&
        pFacts2.count(currPrecond) == 0)
      pFactsToRemove.insert(currPrecond);
}

bool _lookForAPossibleEffect(const SetOfFacts& pEffectsToCheck,
                             const Fact& pEffectToLookFor,
                             const State& pState,
                             const CompiledProblem& pProblem,
                             FactsAlreadychecked& pFactsAlreadychecked);


bool _lookForAPossibleExistingOrNotFact(
    const std::set<Fact>& pFacts,
    const std::map<Fact, std::set<ActionId>>& pPreconditionToActions,
    const Fact& pEffectToLookFor,
    const State& pState,
    const CompiledProblem& pProblem,
    FactsAlreadychecked& pFactsAlreadychecked)
{
  for (const auto& currEffect : pFacts)
  {
    if (!pFactsAlreadychecked.factsToAdd.insert(currEffect).second)
      continue;
    auto it = pPreconditionToActions.find(currEffect);
    if (it != pPreconditionToActions.end())
    {
      for (const auto& currActionId : it->second)
      {
        auto itAction = pProblem.actions.find(currActionId);
        if (itAction != pProblem.actions.end())
        {
          auto& action = itAction->second;
          if (_canFactsBecomeTrue(action.preconditions, pState) &&
              _willTheActionAddOrRemoveAFact(action, pState.facts()) &&
              _lookForAPossibleEffect(action.effects, pEffectToLookFor, pState, pProblem, pFactsAlreadychecked))
            return true;
        }
      }
    }
  }
  return false;
}



bool _lookForAPossibleEffect(const SetOfFacts& pEffectsToCheck,
                             const Fact& pEffectToLookFor,
                             const State& pState,
                             const CompiledProblem& pProblem,
                             FactsAlreadychecked& pFactsAlreadychecked)
{
  if (pEffectsToCheck.facts.count(pEffectToLookFor) > 0)
    return true;
  if (_lookForAPossibleExistingOrNotFact(pEffectsToCheck.facts, pProblem.preconditionToActions, pEffectToLookFor,
                                         pState, pProblem, pFactsAlreadychecked))
    return true;
  if (_lookForAPossibleExistingOrNotFact(pEffectsToCheck.notFacts, pProblem.notPreconditionToActions, pEffectToLookFor,
                                         pState, pProblem, pFactsAlreadychecked))
    return true;
  return false;
}

void _feedReachableFacts(State& pState,
                         const Fact& pFact,
                         const CompiledProblem& pProblem);

void _feedReachableFactsFromSetOfActions(State& pState,
                                         const std::set<ActionId>& pActions,
                                         const CompiledProblem& pProblem)
{
  for (const auto& currAction : pActions)
  {
    auto itAction = pProblem.actions.find(currAction);
    if (itAction != pProblem.actions.end())
    {
      auto& action = itAction->second;
      if (_canFactsBecomeTrue(action.preconditions, pState))
      {
        std::set<Fact> reachableFactsToAdd;
        _getTheFactsToAddFromTheActionEffects(reachableFactsToAdd, action, pState.facts(), pState.reachableFacts());
        std::set<Fact> removableFactsToAdd;
        _getTheFactsToRemoveFromTheActionEffects(removableFactsToAdd, action, pState.facts(), pState.removableFacts());
        if (!reachableFactsToAdd.empty() || !removableFactsToAdd.empty())
        {
          pState.addReachableFacts(reachableFactsToAdd);
          pState.addRemovableFacts(removableFactsToAdd);
          for (const auto& currNewFact : reachableFactsToAdd)
            _feedReachableFacts(pState, currNewFact, pProblem);
          for (const auto& currNewFact : removableFactsToAdd)
            _feedReachableFacts(pState, currNewFact, pProblem);
        }
      }
    }
  }
}


void _feedReachableFacts(State& pState,
                         const Fact& pFact,
                         const CompiledProblem& pProblem)
{
  auto itPrecToActions = pProblem.preconditionToActions.find(pFact);
  if (itPrecToActions != pProblem.preconditionToActions.end())
    _feedReachableFactsFromSetOfActions(pState, itPrecToActions->second, pProblem);
}


bool _nextStepOfTheProblemForAGoalAndSetOfActions(PotentialNextAction& pCurrentResult,
                                                  const std::set<ActionId>& pActions,
                                                  const Fact& pGoal,
                                                  const State& pState,
                                                  const CompiledProblem& pProblem,
                                                  const Historical* pGlobalHistorical)
{
  PotentialNextAction newPotNextAction;
  for (const auto& currAction : pActions)
  {
    auto itAction = pProblem.actions.find(currAction);
    if (itAction != pProblem.actions.end())
    {
      auto& action = itAction->second;
      FactsAlreadychecked factsAlreadychecked;
      if (areFactsTrue(action.preconditions, pState) &&
          _willTheActionAddOrRemoveAFact(action, pState.facts()) &&
          _lookForAPossibleEffect(action.effects, pGoal, pState, pProblem, factsAlreadychecked))
      {
        auto newPotRes = PotentialNextAction(currAction, action);
        if (newPotRes.isMoreImportantThan(newPotNextAction, pState, pGlobalHistorical))
        {
          assert(newPotRes.actionPtr != nullptr);
          newPotNextAction = newPotRes;
          if (newPotRes.actionPtr->shouldBeDoneAsapWithoutHistoryCheck)
            break;
        }
      }
    }
  }

  if (newPotNextAction.isMoreImportantThan(pCurrentResult, pState, pGlobalHistorical))
  {
    assert(newPotNextAction.actionPtr != nullptr);
    pCurrentResult = newPotNextAction;
    if (newPotNextAction.actionPtr->shouldBeDoneAsapWithoutHistoryCheck)
      return true;
  }
  return false;
}


ActionId _nextStepOfTheProblemForAGoal(const Fact& pGoal,
                                       const State& pState,
                                       const CompiledProblem& pProblem,
                                       const Historical* pGlobalHistorical)
{
  PotentialNextAction res;
  for (const auto& currFact : pState.facts())
  {
    auto itPrecToActions = pProblem.preconditionToActions.find(currFact);
    if (itPrecToActions != pProblem.preconditionToActions.end() &&
        _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pState,
                                                     pProblem, pGlobalHistorical))
      return res.actionId;
  }
  for (const auto& currFact : pState.factsToValue())
  {
    auto itPrecToActions = pProblem.preconditionToActionsExps.find(currFact.first);
    if (itPrecToActions != pProblem.preconditionToActionsExps.end() &&
        _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pState,
                                                     pProblem, pGlobalHistorical))
      return res.actionId;
  }
  _nextStepOfTheProblemForAGoalAndSetOfActions(res, pProblem.actionsWithoutPrecondition, pGoal, pState,
                                               pProblem, pGlobalHistorical);
  return res.actionId;
}

}


bool SetOfFacts::containsFact(const Fact& pFact) const
{
  if (facts.count(pFact) > 0 || notFacts.count(pFact) > 0)
    return true;
  for (auto& currExp : exps)
    for (auto& currElt : currExp.elts)
      if (currElt.type == ExpressionElementType::FACT && currElt.value == pFact)
        return true;
  return false;
}

void SetOfFacts::rename(const Fact& pOldFact,
                        const Fact& pNewFact)
{
  auto it = facts.find(pOldFact);
  if (it != facts.end())
  {
    facts.erase(it);
    facts.insert(pNewFact);
  }
  auto itNot = notFacts.find(pOldFact);
  if (itNot != notFacts.end())
  {
    notFacts.erase(itNot);
    notFacts.insert(pNewFact);
  }
  for (auto& currExp : exps)
    for (auto& currElt : currExp.elts)
      if (currElt.type == ExpressionElementType::FACT && currElt.value == pOldFact)
        currElt.value = pNewFact;
}



std::list<std::pair<std::string, std::string>> SetOfFacts::toFactsStrs() const
{
  std::list<std::pair<std::string, std::string>> res;
  for (const auto& currFact : facts)
    res.emplace_back(currFact, currFact);
  for (const auto& currNotFact : notFacts)
    res.emplace_back(currNotFact, "!" + currNotFact);
  return res;
}


std::list<std::string> SetOfFacts::toStrs() const
{
  std::list<std::string> res(facts.begin(), facts.end());
  for (const auto& currNotFact : notFacts)
    res.emplace_back("!" + currNotFact);
  for (const auto& currExp : exps)
  {
    std::string ExpStr;
    for (const auto& currElt : currExp.elts)
    {
      if (currElt.type == ExpressionElementType::FACT)
        ExpStr += "${" +  currElt.value + "}";
      else
        ExpStr += currElt.value;
    }
    if (!ExpStr.empty())
      res.emplace_back(ExpStr);
  }
  return res;
}


std::string SetOfFacts::toStr(const std::string& pSeparator) const
{
  return _listOfStrToStr(toStrs(), pSeparator);
}



SetOfFacts SetOfFacts::fromStr(const std::string& pStr,
                               const std::string& pSeparator)
{
  std::vector<std::string> vect;
  mystd::split(vect, pStr, pSeparator);
  SetOfFacts res;

  for (const auto& currStr : vect)
  {
    if (currStr.empty())
      continue;
    std::string currentToken;
    Expression exp;
    auto fillCurrentToken = [&]
    {
      if (!currentToken.empty())
      {
        exp.elts.emplace_back(ExpressionElementType::VALUE, currentToken);
        currentToken.clear();
      }
    };

    for (std::size_t i = 0; i < currStr.size();)
    {
      bool needToContinue = false;
      if (i == 0)
      {
        for (const auto& currOp : _strToBeginOfTextOperators)
        {
          if (currStr.compare(0, currOp.first.size(), currOp.first) == 0)
          {
            fillCurrentToken();
            exp.elts.emplace_back(ExpressionElementType::OPERATOR, currOp.first);
            i += currOp.first.size();
            needToContinue = true;
            break;
          }
        }
      }
      if (needToContinue)
        continue;
      for (const auto& charToOp : _charToOperators)
      {
        if (charToOp.first == currStr[i])
        {
          fillCurrentToken();
          exp.elts.emplace_back(ExpressionElementType::OPERATOR, std::string(1, charToOp.first));
          ++i;
          needToContinue = true;
          break;
        }
      }
      if (needToContinue)
        continue;
      if (currStr[i] == '$' && currStr[i+1] == '{')
      {
        auto endOfVar = currStr.find('}', i + 2);
        if (endOfVar != std::string::npos)
        {
          fillCurrentToken();
          auto begPos = i + 2;
          exp.elts.emplace_back(ExpressionElementType::FACT, currStr.substr(begPos, endOfVar - begPos));
          i = endOfVar + 1;
          continue;
        }
      }
      currentToken += currStr[i++];
    }
    fillCurrentToken();

    if (exp.elts.size() == 1)
    {
      auto& token = exp.elts.front().value;
      if (!token.empty())
      {
        if (token[0] == '!')
          res.notFacts.insert(token.substr(1, token.size() - 1));
        else
          res.facts.insert(token);
      }
    }
    else
    {
      res.exps.emplace_back(std::move(exp));
    }
  }
  return res;
}


void Historical::setMutex(std::shared_ptr<std::mutex> pMutex)
{
  _mutexPtr = std::move(pMutex);
}

void Historical::notifyActionDone(const ActionId& pActionId)
{
  if (_mutexPtr)
  {
    std::lock_guard<std::mutex> lock(*_mutexPtr);
    _notifyActionDone(pActionId);
  }
  else
  {
    _notifyActionDone(pActionId);
  }
}


void Historical::_notifyActionDone(const ActionId& pActionId)
{
  _actionToNumberOfTimeAleardyDone.emplace(pActionId, 0);
  ++_actionToNumberOfTimeAleardyDone[pActionId];
}

bool Historical::hasActionId(const ActionId& pActionId) const
{
  if (_mutexPtr)
  {
    std::lock_guard<std::mutex> lock(*_mutexPtr);
    return _hasActionId(pActionId);
  }
  return _hasActionId(pActionId);
}

std::size_t Historical::getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const
{
  if (_mutexPtr)
  {
    std::lock_guard<std::mutex> lock(*_mutexPtr);
    return _getNbOfTimeAnActionHasAlreadyBeenDone(pActionId);
  }
  return _getNbOfTimeAnActionHasAlreadyBeenDone(pActionId);
}

bool Historical::_hasActionId(const ActionId& pActionId) const
{
  return _actionToNumberOfTimeAleardyDone.count(pActionId) > 0;
}

std::size_t Historical::_getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const
{
  auto it = _actionToNumberOfTimeAleardyDone.find(pActionId);
  if (it == _actionToNumberOfTimeAleardyDone.end())
    return 0;
  return it->second;
}


State::State(const State& pOther)
 : historical(pOther.historical),
   onFactsToValueChanged(),
   onFactsChanged(),
   onGoalsChanged(),
   _goals(pOther._goals),
   _factsToValue(pOther._factsToValue),
   _facts(pOther._facts),
   _reachableFacts(pOther._reachableFacts),
   _removableFacts(pOther._removableFacts),
   _needToAddReachableFacts(pOther._needToAddReachableFacts)
{
}


std::string State::getCurrentGoal() const
{
  return _goals.empty() ? "" : _goals.front();
}

void State::addFactsToValue(const std::map<Fact, std::string>& pFactsToValue)
{
  if (!pFactsToValue.empty())
    for (const auto& currFactToVal : pFactsToValue)
      _factsToValue[currFactToVal.first] = currFactToVal.second;
  onFactsToValueChanged(_factsToValue);
}

bool State::addFact(const Fact& pFact)
{
  return addFacts(std::vector<Fact>{pFact});
}

template<typename FACTS>
bool State::addFacts(const FACTS& pFacts)
{
  bool res = _addFactsWithoutFactNotification(pFacts);
  if (res)
    onFactsChanged(_facts);
  return res;
}

template bool State::addFacts<std::set<Fact>>(const std::set<Fact>&);
template bool State::addFacts<std::vector<Fact>>(const std::vector<Fact>&);


template<typename FACTS>
bool State::_addFactsWithoutFactNotification(const FACTS& pFacts)
{
  bool res = false;
  bool aGoalHasBeenRemoved = false;
  for (const auto& currFact : pFacts)
  {
    if (_facts.count(currFact) > 0)
      continue;
    res = true;
    _facts.insert(currFact);
    if (!_goals.empty() && currFact == _goals.front())
    {
      removeGoal(currFact);
      aGoalHasBeenRemoved = true;
    }

    auto itReachable = _reachableFacts.find(currFact);
    if (itReachable != _reachableFacts.end())
      _reachableFacts.erase(itReachable);
    else
      _clearRechableAndRemovableFacts();
  }
  if (aGoalHasBeenRemoved)
    _removeDoneGoals();
  return res;
}

template bool State::_addFactsWithoutFactNotification<std::set<Fact>>(const std::set<Fact>&);
template bool State::_addFactsWithoutFactNotification<std::vector<Fact>>(const std::vector<Fact>&);


void State::_clearRechableAndRemovableFacts()
{
  _needToAddReachableFacts = true;
  _reachableFacts.clear();
  _removableFacts.clear();
}

bool State::modifyFacts(const SetOfFacts& pSetOfFacts)
{
  bool factsChanged = _addFactsWithoutFactNotification(pSetOfFacts.facts);
  for (const auto& currFact : pSetOfFacts.notFacts)
  {
    auto it = _facts.find(currFact);
    if (it == _facts.end())
      continue;
    factsChanged = true;
    _facts.erase(it);
    _clearRechableAndRemovableFacts();
  }
  bool factsToValueChanged = false;
  for (auto& currExp : pSetOfFacts.exps)
  {
    if (currExp.elts.size() >= 2)
    {
      auto it = currExp.elts.begin();
      if (it->type == ExpressionElementType::OPERATOR)
      {
        auto op = it->value;
        ++it;
        if (op == "++" &&
            it->type == ExpressionElementType::FACT)
        {
          _incrementStr(_factsToValue[it->value]);
          factsToValueChanged = true;
        }
      }
      else if (it->type == ExpressionElementType::FACT)
      {
        auto factToSet = it->value;
        ++it;
        if (it->type == ExpressionElementType::OPERATOR)
        {
          auto op = it->value;
          ++it;
          if (op == "=" &&
              it->type == ExpressionElementType::VALUE)
          {
            _factsToValue[factToSet] = it->value;
            factsToValueChanged = true;
          }
        }
      }
    }
  }
  if (factsChanged)
    onFactsChanged(_facts);
  if (factsToValueChanged)
    onFactsToValueChanged(_factsToValue);
  return factsChanged;
}


void State::setFacts(const std::set<Fact>& pFacts)
{
  if (_facts != pFacts)
  {
    _facts = pFacts;
    _clearRechableAndRemovableFacts();
    onFactsChanged(_facts);
  }
}

void State::addReachableFacts(const std::set<Fact>& pFacts)
{
  _reachableFacts.insert(pFacts.begin(), pFacts.end());
}

void State::addRemovableFacts(const std::set<Fact>& pFacts)
{
  _removableFacts.insert(pFacts.begin(), pFacts.end());
}


void State::_removeDoneGoals()
{
  bool hasRemovedAGoal = false;
  for (auto itGoal = _goals.begin(); itGoal != _goals.end(); )
  {
    if (_facts.count(*itGoal) > 0)
    {
      itGoal = _goals.erase(itGoal);
      hasRemovedAGoal = true;
    }
    else
    {
      break;
    }
  }
  if (hasRemovedAGoal)
    onGoalsChanged(_goals);
}


void State::removeGoal(const Fact& pGoal)
{
  for (auto itGoal = _goals.begin(); itGoal != _goals.end(); ++itGoal)
  {
    if (*itGoal == pGoal)
    {
      _goals.erase(itGoal);
      onGoalsChanged(_goals);
      break;
    }
  }
}

void State::setGoals(const std::vector<Fact>& pGoals)
{
  if (_goals != pGoals)
  {
    _goals = pGoals;
    onGoalsChanged(_goals);
  }
}

void State::addGoals(const std::vector<Fact>& pGoals)
{
  if (pGoals.empty())
    return;
  _goals.insert(_goals.begin(), pGoals.begin(), pGoals.end());
  onGoalsChanged(_goals);
}

void State::pushBackGoal(const Fact& pGoal)
{
  _goals.push_back(pGoal);
  onGoalsChanged(_goals);
}



void replaceVariables(std::string& pStr,
                      const State& pState)
{
  // replace variable
  auto currentPos = pStr.find_first_of("${");
  while (currentPos != std::string::npos)
  {
    auto beginOfVarName = currentPos + 2;
    auto endVarPos = pStr.find("}", beginOfVarName);
    if (endVarPos != std::string::npos)
    {
      auto varName = pStr.substr(beginOfVarName, endVarPos - beginOfVarName);

      auto& factsToValue = pState.factsToValue();
      auto it = factsToValue.find(varName);
      if (it != factsToValue.end())
      {
        auto& varValue = it->second;
        pStr.replace(currentPos, endVarPos - currentPos + 1, varValue);
        currentPos += varValue.size();
      }
      else
      {
        currentPos = endVarPos;
      }

    }
    currentPos = pStr.find_first_of("${", currentPos);
  }

  // evalute expressions
  currentPos = pStr.find("`");
  while (currentPos != std::string::npos)
  {
    auto beginOfExp = currentPos + 1;
    auto endExpPos = pStr.find("`", beginOfExp);
    if (endExpPos == std::string::npos)
      break;
    pStr.replace(currentPos, endExpPos - currentPos + 1, evaluteToStr(pStr, beginOfExp));
    currentPos = endExpPos + 1;
    currentPos = pStr.find("`", currentPos);
  }
}


std::vector<lp::Fact> factsFromString(const std::string& pStr,
                                      const std::string& pSeparator)
{
  std::vector<lp::Fact> res;
  mystd::split(res, pStr, pSeparator);
  for (auto it = res.begin(); it != res.end(); )
  {
    if (it->empty())
      it = res.erase(it);
    else
      ++it;
  }
  return res;
}

CompiledProblem compileProblem(const std::map<ActionId, Action>& pActions)
{
  CompiledProblem res;
  for (const auto& currAction : pActions)
  {
    if (currAction.second.preconditions == currAction.second.effects ||
        currAction.second.effects.empty())
      continue;
    res.actions.emplace(currAction.first, currAction.second);
    for (const auto& currPrecondition : currAction.second.preferInContext.facts)
      res.preconditionToActions[currPrecondition].insert(currAction.first);
    for (const auto& currPrecondition : currAction.second.preconditions.facts)
      res.preconditionToActions[currPrecondition].insert(currAction.first);

    for (const auto& currPrecondition : currAction.second.preferInContext.notFacts)
      res.notPreconditionToActions[currPrecondition].insert(currAction.first);
    for (const auto& currPrecondition : currAction.second.preconditions.notFacts)
      res.notPreconditionToActions[currPrecondition].insert(currAction.first);

    for (auto& currExp : currAction.second.preconditions.exps)
      for (auto& currElt : currExp.elts)
        if (currElt.type == ExpressionElementType::FACT)
          res.preconditionToActionsExps[currElt.value].insert(currAction.first);
    if (currAction.second.preconditions.facts.empty() && currAction.second.preconditions.exps.empty())
      res.actionsWithoutPrecondition.insert(currAction.first);
  }
  return res;
}


void fillReachableFacts(State& pState,
                        const CompiledProblem& pProblem)
{
  if (!pState.needToAddReachableFacts())
    return;
  for (const auto& currFact : pState.facts())
  {
    if (pState.reachableFacts().count(currFact) == 0)
      _feedReachableFacts(pState, currFact, pProblem);
  }
  _feedReachableFactsFromSetOfActions(pState, pProblem.actionsWithoutPrecondition, pProblem);
  pState.noNeedToAddReachableFacts();
}


bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                  const State& pState)
{
  auto& facts = pState.facts();
  for (const auto& currPrecond : pSetOfFacts.facts)
    if (facts.count(currPrecond) == 0)
      return false;
  for (const auto& currPrecond : pSetOfFacts.notFacts)
    if (facts.count(currPrecond) > 0)
      return false;
  return _areExpsValid(pSetOfFacts.exps, pState.factsToValue());
}


ActionId lookForAnActionToDo(State& pState,
                             const CompiledProblem& pProblem,
                             const Historical* pGlobalHistorical)
{
  fillReachableFacts(pState, pProblem);
  while (!pState.goals().empty())
  {
    auto& currGoal = pState.goals().front();
    if (pState.facts().count(currGoal) == 0)
    {
      auto res = _nextStepOfTheProblemForAGoal(currGoal, pState,
                                               pProblem, pGlobalHistorical);
      if (!res.empty())
        return res;
    }
    pState.removeGoal(currGoal);
  }
  return "";
}


std::list<ActionId> solve(State& pState,
                          const CompiledProblem& pProblem,
                          Historical* pGlobalHistorical)
{
  std::list<std::string> res;
  while (!pState.goals().empty())
  {
    auto actionToDo = lookForAnActionToDo(pState, pProblem, pGlobalHistorical);
    if (actionToDo.empty())
      break;
    res.emplace_back(actionToDo);
    pState.historical.notifyActionDone(actionToDo);
    if (pGlobalHistorical != nullptr)
      pGlobalHistorical->notifyActionDone(actionToDo);
    auto itAction = pProblem.actions.find(actionToDo);
    if (itAction != pProblem.actions.end())
      pState.modifyFacts(itAction->second.effects);
  }
  return res;
}


} // !lp
} // !onsem
