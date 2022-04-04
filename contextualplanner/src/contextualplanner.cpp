#include <contextualplanner/contextualplanner.hpp>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <contextualplanner/arithmeticevaluator.hpp>


namespace cp
{

namespace
{
const std::map<std::string, ExpressionOperator> _strToBeginOfTextOperators
{{"++", ExpressionOperator::PLUSPLUS}};
const std::map<char, ExpressionOperator> _charToOperators
{{'=', ExpressionOperator::EQUAL}, {'+', ExpressionOperator::PLUS}, {'+', ExpressionOperator::MINUS}};

template <typename T>
T _lexical_cast(const std::string& pStr)
{
  bool firstChar = true;
  for (const auto& currChar : pStr)
  {
    if ((currChar < '0' || currChar > '9') &&
        !(firstChar && currChar == '-'))
      throw std::runtime_error("bad lexical cast: source type value could not be interpreted as target");
    firstChar = false;
  }
  return atoi(pStr.c_str());
}


void _splitFacts(
    std::vector<cp::Fact>& pFacts,
    const std::string& pStr,
    char pSeparator)
{
  std::size_t pos = 0u;
  cp::Fact currFact;
  while (pos < pStr.size())
  {
    pos = currFact.fillFactFromStr(pStr, pos, pSeparator) + 1;
    if (!currFact.name.empty())
    {
      pFacts.emplace_back(std::move(currFact));
      currFact = cp::Fact();
    }
  }
  if (!currFact.name.empty())
    pFacts.emplace_back(std::move(currFact));
}



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
      ss << _lexical_cast<int>(pStr) + 1;
      pStr = ss.str();
    }
    catch (...) {}
  }
}

struct PotentialNextAction
{
 PotentialNextAction()
   : actionId(""),
     actionPtr(nullptr),
     parameters()
 {
 }
  PotentialNextAction(const ActionId& pActionId,
                      const Action& pAction);

  ActionId actionId;
  const Action* actionPtr;
  std::map<std::string, std::string> parameters;

  bool isMoreImportantThan(const PotentialNextAction& pOther,
                           const Problem& pProblem,
                           const Historical* pGlobalHistorical) const;
};


PotentialNextAction::PotentialNextAction(const ActionId& pActionId,
                                         const Action& pAction)
  : actionId(pActionId),
    actionPtr(&pAction),
    parameters()
{
  for (const auto& currParam : pAction.parameters)
    parameters[currParam];
}


void _getPrecoditionStatistics(std::size_t& nbOfPreconditionsSatisfied,
                               std::size_t& nbOfPreconditionsNotSatisfied,
                               const Action& pAction,
                               const std::set<Fact>& pFacts)
{
  nbOfPreconditionsSatisfied = pAction.preconditions.facts.size();
  for (const auto& currFact : pAction.preferInContext.facts)
  {
    if (pFacts.count(currFact) > 0)
      ++nbOfPreconditionsSatisfied;
    else
      ++nbOfPreconditionsNotSatisfied;
  }
  for (const auto& currFact : pAction.preferInContext.notFacts)
  {
    if (pFacts.count(currFact) == 0)
      ++nbOfPreconditionsSatisfied;
    else
      ++nbOfPreconditionsNotSatisfied;
  }
}


bool PotentialNextAction::isMoreImportantThan(const PotentialNextAction& pOther,
                                              const Problem& pProblem,
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

  auto nbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
  auto otherNbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);
  if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
    return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;

  std::size_t nbOfPreconditionsSatisfied = 0;
  std::size_t nbOfPreconditionsNotSatisfied = 0;
  _getPrecoditionStatistics(nbOfPreconditionsSatisfied, nbOfPreconditionsNotSatisfied, action, pProblem.facts());
  std::size_t otherNbOfPreconditionsSatisfied = 0;
  std::size_t otherNbOfPreconditionsNotSatisfied = 0;
  _getPrecoditionStatistics(otherNbOfPreconditionsSatisfied, otherNbOfPreconditionsNotSatisfied, otherAction, pProblem.facts());
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
                                  const std::map<std::string, std::string>& pVariablesToValue)
{
  if (pExpElt.type == ExpressionElementType::FACT)
  {
    auto it = pVariablesToValue.find(pExpElt.value);
    if (it != pVariablesToValue.end())
      return it->second;
    return "";
  }
  return pExpElt.value;
}

bool _areExpsValid(const std::list<Expression>& pExps,
                   const std::map<std::string, std::string>& pVariablesToValue)
{
  for (const auto& currExp : pExps)
  {
    if (currExp.elts.size() >= 3)
    {
      auto it = currExp.elts.begin();
      auto val1 = _expressionEltToValue(*it, pVariablesToValue);
      ++it;
      if (it->type != ExpressionElementType::OPERATOR ||
          it->value != "=")
        return false;
      ++it;
      auto val2 = _expressionEltToValue(*it, pVariablesToValue);
      ++it;
      while (it != currExp.elts.end())
      {
        if (it->type != ExpressionElementType::OPERATOR)
          return false;
        auto op = it->value;
        ++it;
        if (it == currExp.elts.end())
          break;
        auto val3 = _expressionEltToValue(*it, pVariablesToValue);
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
                         const Problem& pProblem)
{
  auto& facts = pProblem.facts();
  auto& reachableFacts = pProblem.reachableFacts();
  auto& reachableFactsWithAnyValues = pProblem.reachableFactsWithAnyValues();
  for (const auto& currFact : pSetOfFacts.facts)
  {
    if (facts.count(currFact) == 0 &&
        reachableFacts.count(currFact) == 0)
    {
      bool reableFactFound = false;
      for (const auto& currReachableFact : reachableFactsWithAnyValues)
      {
        if (currFact.areEqualExceptAnyValues(currReachableFact))
        {
          reableFactFound = true;
          break;
        }
      }
      if (!reableFactFound)
        return false;
    }
  }
  auto& removableFacts = pProblem.removableFacts();
  for (const auto& currFact : pSetOfFacts.notFacts)
    if (facts.count(currFact) > 0 &&
        removableFacts.count(currFact) == 0)
      return false;
  return _areExpsValid(pSetOfFacts.exps, pProblem.variablesToValue());
}


bool _doesFactInFacts(
    std::map<std::string, std::string>& pParameters,
    const Fact& pFact,
    const std::set<Fact>& pFacts,
    bool pParametersAreForTheFact)
{
  for (const auto& currFact : pFacts)
  {
    if (currFact.name != pFact.name ||
        currFact.parameters.size() != pFact.parameters.size())
      continue;

    auto doesItMatch = [&](const std::string& pFactValue, const std::string& pValueToLookFor) {
      if (pFactValue == pValueToLookFor)
        return true;
      if (pParameters.empty())
        return false;

      auto itParam = pParameters.find(pFactValue);
      if (itParam != pParameters.end())
      {
        if (!itParam->second.empty())
        {
          if (itParam->second == pValueToLookFor)
            return true;
        }
        else
        {
          pParameters[pFactValue] = pValueToLookFor;
          return true;
        }
      }
      return false;
    };

    {
      bool doesParametersMatches = true;
      auto itFactParameters = currFact.parameters.begin();
      auto itLookForParameters = pFact.parameters.begin();
      while (itFactParameters != currFact.parameters.end())
      {
        if (*itFactParameters != *itLookForParameters)
        {
          if (!itFactParameters->parameters.empty() ||
              !itFactParameters->value.empty() ||
              !itLookForParameters->parameters.empty() ||
              !itLookForParameters->value.empty() ||
              (!pParametersAreForTheFact && !doesItMatch(itFactParameters->name, itLookForParameters->name)) ||
              (pParametersAreForTheFact && !doesItMatch(itLookForParameters->name, itFactParameters->name)))
            doesParametersMatches = false;
        }
        ++itFactParameters;
        ++itLookForParameters;
      }
      if (!doesParametersMatches)
        continue;
    }

    if (pParametersAreForTheFact)
    {
      if (doesItMatch(pFact.value, currFact.value))
        return true;
    }
    else
    {
      if (doesItMatch(currFact.value, pFact.value))
        return true;
    }
  }
  return false;
}


bool _areFactsTrue(std::map<std::string, std::string>& pParameters,
                   const SetOfFacts& pSetOfFacts,
                   const Problem& pProblem)
{
  auto& facts = pProblem.facts();
  for (const auto& currPrecond : pSetOfFacts.facts)
    if (!_doesFactInFacts(pParameters, currPrecond, facts, true))
      return false;
  for (const auto& currPrecond : pSetOfFacts.notFacts)
    if (_doesFactInFacts(pParameters, currPrecond, facts, true))
      return false;
  return _areExpsValid(pSetOfFacts.exps, pProblem.variablesToValue());
}


void _getTheFactsToAddFromTheActionEffects(std::set<Fact>& pNewFacts,
                                           std::vector<Fact>& pNewFactsWithAnyValues,
                                           const Action& pAction,
                                           const std::set<Fact>& pFacts1,
                                           const std::set<Fact>& pFacts2)
{
  for (const auto& currFact : pAction.effects.facts)
  {
    if (pFacts1.count(currFact) == 0 &&
        pFacts2.count(currFact) == 0)
    {
      auto factToInsert = currFact;
      if (factToInsert.replaceParametersByAny(pAction.parameters))
        pNewFactsWithAnyValues.push_back(std::move(factToInsert));
      else
        pNewFacts.insert(std::move(factToInsert));
    }
  }
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

bool _lookForAPossibleEffect(std::map<std::string, std::string>& pParameters,
                             const SetOfFacts& pEffectsToCheck,
                             const Fact& pEffectToLookFor,
                             const Problem& pProblem,
                             const Domain& pDomain,
                             FactsAlreadychecked& pFactsAlreadychecked);


bool _lookForAPossibleExistingOrNotFact(
    const Fact& pFact,
    std::map<std::string, std::string>& pParentParameters,
    const std::map<std::string, std::set<ActionId>>& pPreconditionToActions,
    const Fact& pEffectToLookFor,
    const Problem& pProblem,
    const Domain& pDomain,
    FactsAlreadychecked& pFactsAlreadychecked)
{
  if (!pFactsAlreadychecked.factsToAdd.insert(pFact).second)
    return false;
  auto it = pPreconditionToActions.find(pFact.name);
  if (it != pPreconditionToActions.end())
  {
    for (const auto& currActionId : it->second)
    {
      auto itAction = pDomain.actions().find(currActionId);
      if (itAction != pDomain.actions().end())
      {
        auto& action = itAction->second;
        std::map<std::string, std::string> parameters;
        for (const auto& currParam : action.parameters)
          parameters[currParam];
        if (_canFactsBecomeTrue(action.preconditions, pProblem) &&
            _lookForAPossibleEffect(parameters, action.effects, pEffectToLookFor, pProblem, pDomain, pFactsAlreadychecked))
        {
          bool actionIsAPossibleFollowUp = true;
          // fill parent parameters
          for (auto& currParentParam : pParentParameters)
          {
            if (currParentParam.second.empty())
            {
              //for (const auto& currFact : pFacts)
              {
                for (const auto& currActionPreconditionFact : action.preconditions.facts)
                {
                  currParentParam.second = /*currFact*/ pFact.tryToExtractParameterValueFromExemple(currParentParam.first, currActionPreconditionFact);
                  if (!currParentParam.second.empty())
                    break;
                }
                if (!currParentParam.second.empty())
                  break;
              }
              if (currParentParam.second.empty())
              {
                actionIsAPossibleFollowUp = false;
                break;
              }
            }
          }
          if (actionIsAPossibleFollowUp)
            return true;
        }
      }
    }
  }
  return false;
}



bool _lookForAPossibleEffect(std::map<std::string, std::string>& pParameters,
                             const SetOfFacts& pEffectsToCheck,
                             const Fact& pEffectToLookFor,
                             const Problem& pProblem,
                             const Domain& pDomain,
                             FactsAlreadychecked& pFactsAlreadychecked)
{
  if (_doesFactInFacts(pParameters, pEffectToLookFor, pEffectsToCheck.facts, false))
    return true;

  auto& preconditionToActions = pDomain.preconditionToActions();
  for (auto& currFact : pEffectsToCheck.facts)
    if (pProblem.facts().count(currFact) == 0)
      if (_lookForAPossibleExistingOrNotFact(currFact, pParameters, preconditionToActions, pEffectToLookFor,
                                             pProblem, pDomain, pFactsAlreadychecked))
        return true;
  auto& notPreconditionToActions = pDomain.notPreconditionToActions();
  for (auto& currFact : pEffectsToCheck.notFacts)
    if (pProblem.facts().count(currFact) > 0)
      if (_lookForAPossibleExistingOrNotFact(currFact, pParameters, notPreconditionToActions, pEffectToLookFor,
                                             pProblem, pDomain, pFactsAlreadychecked))
      return true;
  return false;
}

void _feedReachableFacts(Problem& pProblem,
                         const Fact& pFact,
                         const Domain& pDomain);

void _feedReachableFactsFromSetOfActions(Problem& pProblem,
                                         const std::set<ActionId>& pActions,
                                         const Domain& pDomain)
{
  for (const auto& currAction : pActions)
  {
    auto itAction = pDomain.actions().find(currAction);
    if (itAction != pDomain.actions().end())
    {
      auto& action = itAction->second;
      if (_canFactsBecomeTrue(action.preconditions, pProblem))
      {
        std::set<Fact> reachableFactsToAdd;
        std::vector<Fact> reachableFactsToAddWithAnyValues;
        _getTheFactsToAddFromTheActionEffects(reachableFactsToAdd, reachableFactsToAddWithAnyValues, action, pProblem.facts(), pProblem.reachableFacts());
        std::set<Fact> removableFactsToAdd;
        _getTheFactsToRemoveFromTheActionEffects(removableFactsToAdd, action, pProblem.facts(), pProblem.removableFacts());
        if (!reachableFactsToAdd.empty() || !reachableFactsToAddWithAnyValues.empty() || !removableFactsToAdd.empty())
        {
          pProblem.addReachableFacts(reachableFactsToAdd);
          pProblem.addReachableFactsWithAnyValues(reachableFactsToAddWithAnyValues);
          pProblem.addRemovableFacts(removableFactsToAdd);
          for (const auto& currNewFact : reachableFactsToAdd)
            _feedReachableFacts(pProblem, currNewFact, pDomain);
          for (const auto& currNewFact : removableFactsToAdd)
            _feedReachableFacts(pProblem, currNewFact, pDomain);
        }
      }
    }
  }
}


void _feedReachableFacts(Problem& pProblem,
                         const Fact& pFact,
                         const Domain& pDomain)
{
  auto itPrecToActions = pDomain.preconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.preconditionToActions().end())
    _feedReachableFactsFromSetOfActions(pProblem, itPrecToActions->second, pDomain);
}


bool _nextStepOfTheProblemForAGoalAndSetOfActions(PotentialNextAction& pCurrentResult,
                                                  const std::set<ActionId>& pActions,
                                                  const Fact& pGoal,
                                                  const Problem& pProblem,
                                                  const Domain& pDomain,
                                                  const Historical* pGlobalHistorical)
{
  PotentialNextAction newPotNextAction;
  for (const auto& currAction : pActions)
  {
    auto itAction = pDomain.actions().find(currAction);
    if (itAction != pDomain.actions().end())
    {
      auto& action = itAction->second;
      FactsAlreadychecked factsAlreadychecked;
      auto newPotRes = PotentialNextAction(currAction, action);
      if (_areFactsTrue(newPotRes.parameters, action.preconditions, pProblem) &&
          _lookForAPossibleEffect(newPotRes.parameters, action.effects, pGoal, pProblem, pDomain, factsAlreadychecked))
      {
        if (newPotRes.isMoreImportantThan(newPotNextAction, pProblem, pGlobalHistorical))
        {
          assert(newPotRes.actionPtr != nullptr);
          newPotNextAction = newPotRes;
          if (newPotRes.actionPtr->shouldBeDoneAsapWithoutHistoryCheck)
            break;
        }
      }
    }
  }

  if (newPotNextAction.isMoreImportantThan(pCurrentResult, pProblem, pGlobalHistorical))
  {
    assert(newPotNextAction.actionPtr != nullptr);
    pCurrentResult = newPotNextAction;
    if (newPotNextAction.actionPtr->shouldBeDoneAsapWithoutHistoryCheck)
      return true;
  }
  return false;
}


ActionId _nextStepOfTheProblemForAGoal(
    std::map<std::string, std::string>& pParameters,
    const Fact& pGoal,
    const Problem& pProblem,
    const Domain& pDomain,
    const Historical* pGlobalHistorical)
{
  PotentialNextAction res;
  for (const auto& currFact : pProblem.factNamesToNbOfFactOccurences())
  {
    auto itPrecToActions = pDomain.preconditionToActions().find(currFact.first);
    if (itPrecToActions != pDomain.preconditionToActions().end() &&
        _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pProblem,
                                                     pDomain, pGlobalHistorical))
    {
      pParameters = std::move(res.parameters);
      return res.actionId;
    }
  }
  for (const auto& currFact : pProblem.variablesToValue())
  {
    auto itPrecToActions = pDomain.preconditionToActionsExps().find(currFact.first);
    if (itPrecToActions != pDomain.preconditionToActionsExps().end() &&
        _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pProblem,
                                                     pDomain, pGlobalHistorical))
    {
      pParameters = std::move(res.parameters);
      return res.actionId;
    }
  }
  auto& actionsWithoutPrecondition = pDomain.actionsWithoutPrecondition();
  _nextStepOfTheProblemForAGoalAndSetOfActions(res, actionsWithoutPrecondition, pGoal, pProblem,
                                               pDomain, pGlobalHistorical);
  pParameters = std::move(res.parameters);
  return res.actionId;
}

}


bool SetOfFacts::containsFact(const Fact& pFact) const
{
  if (facts.count(pFact) > 0 || notFacts.count(pFact) > 0)
    return true;
  for (auto& currExp : exps)
    for (auto& currElt : currExp.elts)
      if (currElt.type == ExpressionElementType::FACT && currElt.value == pFact.toStr())
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
      if (currElt.type == ExpressionElementType::FACT && currElt.value == pOldFact.toStr())
        currElt.value = pNewFact.toStr();
}



std::list<std::pair<std::string, std::string>> SetOfFacts::toFactsStrs() const
{
  std::list<std::pair<std::string, std::string>> res;
  for (const auto& currFact : facts)
    res.emplace_back(currFact.toStr(), currFact.toStr());
  for (const auto& currNotFact : notFacts)
    res.emplace_back(currNotFact.toStr(), "!" + currNotFact.toStr());
  return res;
}


std::list<std::string> SetOfFacts::toStrs() const
{
  std::list<std::string> res;
  for (const auto& currFact : facts)
    res.emplace_back(currFact.toStr());
  for (const auto& currNotFact : notFacts)
    res.emplace_back("!" + currNotFact.toStr());
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
                               char pSeparator)
{
  std::vector<cp::Fact> vect;
  _splitFacts(vect, pStr, pSeparator);
  SetOfFacts res;

  for (auto& currFact : vect)
  {
    if (currFact.name.empty())
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

    if (!currFact.parameters.empty() ||
        (currFact.name[0] != '+' && currFact.name[0] != '$'))
    {
      if (currFact.name[0] == '!')
      {
        currFact.name = currFact.name.substr(1, currFact.name.size() - 1);
        res.notFacts.insert(currFact);
      }
      else
      {
        res.facts.insert(currFact);
      }
      continue;
    }

    auto currStr = currFact.toStr();
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
    res.exps.emplace_back(std::move(exp));
  }
  return res;
}



Domain::Domain(const std::map<ActionId, Action>& pActions)
{
  for (const auto& currAction : pActions)
  {
    if (currAction.second.preconditions == currAction.second.effects ||
        currAction.second.effects.empty())
      continue;
    _actions.emplace(currAction.first, currAction.second);
    for (const auto& currPrecondition : currAction.second.preconditions.facts)
      _preconditionToActions[currPrecondition.name].insert(currAction.first);
    for (const auto& currPrecondition : currAction.second.preconditions.notFacts)
      _notPreconditionToActions[currPrecondition.name].insert(currAction.first);

    for (auto& currExp : currAction.second.preconditions.exps)
      for (auto& currElt : currExp.elts)
        if (currElt.type == ExpressionElementType::FACT)
          _preconditionToActionsExps[currElt.value].insert(currAction.first);
    if (currAction.second.preconditions.facts.empty() && currAction.second.preconditions.exps.empty())
      _actionsWithoutPrecondition.insert(currAction.first);
  }
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


Problem::Problem(const Problem& pOther)
 : historical(pOther.historical),
   onVariablesToValueChanged(),
   onFactsChanged(),
   onGoalsChanged(),
   _goals(pOther._goals),
   _variablesToValue(pOther._variablesToValue),
   _facts(pOther._facts),
   _factNamesToNbOfFactOccurences(pOther._factNamesToNbOfFactOccurences),
   _reachableFacts(pOther._reachableFacts),
   _reachableFactsWithAnyValues(pOther._reachableFactsWithAnyValues),
   _removableFacts(pOther._removableFacts),
   _needToAddReachableFacts(pOther._needToAddReachableFacts)
{
}


std::string Problem::getCurrentGoal() const
{
  return _goals.empty() ? "" : _goals.front().toStr();
}

void Problem::addVariablesToValue(const std::map<std::string, std::string>& pVariablesToValue)
{
  if (!pVariablesToValue.empty())
    for (const auto& currFactToVal : pVariablesToValue)
      _variablesToValue[currFactToVal.first] = currFactToVal.second;
  onVariablesToValueChanged(_variablesToValue);
}

bool Problem::addFact(const Fact& pFact)
{
  return addFacts(std::vector<Fact>{pFact});
}

template<typename FACTS>
bool Problem::addFacts(const FACTS& pFacts)
{
  bool res = _addFactsWithoutFactNotification(pFacts);
  if (res)
    onFactsChanged(_facts);
  return res;
}

bool Problem::removeFact(const Fact& pFact)
{
  return removeFacts(std::vector<Fact>{pFact});
}

template<typename FACTS>
bool Problem::removeFacts(const FACTS& pFacts)
{
  bool res = _removeFactsWithoutFactNotification(pFacts);
  if (res)
    onFactsChanged(_facts);
  return res;
}

template bool Problem::addFacts<std::set<Fact>>(const std::set<Fact>&);
template bool Problem::addFacts<std::vector<Fact>>(const std::vector<Fact>&);


void Problem::_addFactNameRef(const std::string& pFactName)
{
  auto itFactName = _factNamesToNbOfFactOccurences.find(pFactName);
  if (itFactName == _factNamesToNbOfFactOccurences.end())
    _factNamesToNbOfFactOccurences[pFactName] = 1;
  else
    ++itFactName->second;
}

template<typename FACTS>
bool Problem::_addFactsWithoutFactNotification(const FACTS& pFacts)
{
  bool res = false;
  for (const auto& currFact : pFacts)
  {
    if (_facts.count(currFact) > 0)
      continue;
    res = true;
    _facts.insert(currFact);
    _addFactNameRef(currFact.name);

    auto itReachable = _reachableFacts.find(currFact);
    if (itReachable != _reachableFacts.end())
      _reachableFacts.erase(itReachable);
    else
      _clearRechableAndRemovableFacts();
  }
  return res;
}


template<typename FACTS>
bool Problem::_removeFactsWithoutFactNotification(const FACTS& pFacts)
{
  bool res = false;
  for (const auto& currFact : pFacts)
  {
    auto it = _facts.find(currFact);
    if (it == _facts.end())
      continue;
    res = true;
    _facts.erase(it);
    {
      auto itFactName = _factNamesToNbOfFactOccurences.find(currFact.name);
      if (itFactName == _factNamesToNbOfFactOccurences.end())
        assert(false);
      else if (itFactName->second == 1)
        _factNamesToNbOfFactOccurences.erase(itFactName);
      else
        --itFactName->second;
    }
    _clearRechableAndRemovableFacts();
  }
  return res;
}


template bool Problem::_addFactsWithoutFactNotification<std::set<Fact>>(const std::set<Fact>&);
template bool Problem::_addFactsWithoutFactNotification<std::vector<Fact>>(const std::vector<Fact>&);


void Problem::_clearRechableAndRemovableFacts()
{
  _needToAddReachableFacts = true;
  _reachableFacts.clear();
  _reachableFactsWithAnyValues.clear();
  _removableFacts.clear();
}

bool Problem::modifyFacts(const SetOfFacts& pSetOfFacts)
{
  bool factsChanged = _addFactsWithoutFactNotification(pSetOfFacts.facts);
  factsChanged = _removeFactsWithoutFactNotification(pSetOfFacts.notFacts) || factsChanged;
  bool variablesToValueChanged = false;
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
          _incrementStr(_variablesToValue[it->value]);
          variablesToValueChanged = true;
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
            _variablesToValue[factToSet] = it->value;
            variablesToValueChanged = true;
          }
        }
      }
    }
  }
  if (factsChanged)
    onFactsChanged(_facts);
  if (variablesToValueChanged)
    onVariablesToValueChanged(_variablesToValue);
  return factsChanged;
}


void Problem::setFacts(const std::set<Fact>& pFacts)
{
  if (_facts != pFacts)
  {
    _facts = pFacts;
    _factNamesToNbOfFactOccurences.clear();
    for (const auto& currFact : pFacts)
      _addFactNameRef(currFact.name);
    _clearRechableAndRemovableFacts();
    onFactsChanged(_facts);
  }
}

void Problem::addReachableFacts(const std::set<Fact>& pFacts)
{
  _reachableFacts.insert(pFacts.begin(), pFacts.end());
}

void Problem::addReachableFactsWithAnyValues(const std::vector<Fact>& pFacts)
{
  _reachableFactsWithAnyValues.insert(pFacts.begin(), pFacts.end());
}

void Problem::addRemovableFacts(const std::set<Fact>& pFacts)
{
  _removableFacts.insert(pFacts.begin(), pFacts.end());
}


void Problem::iterateOnGoalAndRemoveNonPersistent(
    const std::function<bool(const Goal&)>& pManageGoal)
{
  for (auto itGoal = _goals.begin(); itGoal != _goals.end(); )
  {
    if (pManageGoal(*itGoal))
      return;
    if (itGoal->isPersistent())
    {
      ++itGoal;
    }
    else
    {
      itGoal = _goals.erase(itGoal);
      onGoalsChanged(_goals);
    }
  }
}

void Problem::setGoals(const std::vector<Goal>& pGoals)
{
  if (_goals != pGoals)
  {
    _goals = pGoals;
    onGoalsChanged(_goals);
  }
}

void Problem::addGoals(const std::vector<Goal>& pGoals)
{
  if (pGoals.empty())
    return;
  _goals.insert(_goals.begin(), pGoals.begin(), pGoals.end());
  onGoalsChanged(_goals);
}

void Problem::pushBackGoal(const Goal& pGoal)
{
  _goals.push_back(pGoal);
  onGoalsChanged(_goals);
}


void Problem::notifyActionDone(const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters,
    const SetOfFacts& pEffect,
    const std::vector<Goal>* pGoalsToAdd)
{
    historical.notifyActionDone(pActionId);
    if (pParameters.empty())
    {
      modifyFacts(pEffect);
    }
    else
    {
      cp::SetOfFacts effect;
      effect.notFacts = pEffect.notFacts;
      effect.exps = pEffect.exps;
      for (auto& currFact : pEffect.facts)
      {
        auto fact = currFact;
        fact.fillParameters(pParameters);
        effect.facts.insert(std::move(fact));
      }
      modifyFacts(effect);
    }
    if (pGoalsToAdd != nullptr && !pGoalsToAdd->empty())
      addGoals(*pGoalsToAdd);
}




void replaceVariables(std::string& pStr,
                      const Problem& pProblem)
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

      auto& variablesToValue = pProblem.variablesToValue();
      auto it = variablesToValue.find(varName);
      if (it != variablesToValue.end())
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


std::vector<cp::Fact> factsFromString(const std::string& pStr,
                                      char pSeparator)
{
  std::vector<cp::Fact> res;
  _splitFacts(res, pStr, pSeparator);
  return res;
}


void fillReachableFacts(Problem& pProblem,
                        const Domain& pDomain)
{
  if (!pProblem.needToAddReachableFacts())
    return;
  for (const auto& currFact : pProblem.facts())
  {
    if (pProblem.reachableFacts().count(currFact) == 0)
      _feedReachableFacts(pProblem, currFact, pDomain);
  }
  _feedReachableFactsFromSetOfActions(pProblem, pDomain.actionsWithoutPrecondition(), pDomain);
  pProblem.noNeedToAddReachableFacts();
}


bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                  const Problem& pProblem)
{
  std::map<std::string, std::string> parameters;
  return _areFactsTrue(parameters, pSetOfFacts, pProblem);
}


ActionId lookForAnActionToDo(
    std::map<std::string, std::string>& pParameters,
    Problem& pProblem,
    const Domain& pDomain,
    const Historical* pGlobalHistorical)
{
  fillReachableFacts(pProblem, pDomain);

  ActionId res;
  auto tryToFindAnActionTowardGoal = [&](const Goal& pGoal){
    auto& facts = pProblem.facts();
    auto* goalConditionFactPtr = pGoal.conditionFactPtr();
    if (goalConditionFactPtr == nullptr ||
        facts.count(*goalConditionFactPtr) > 0)
    {
      auto& goalFact = pGoal.fact();
      if (facts.count(goalFact) == 0)
      {
        res = _nextStepOfTheProblemForAGoal(pParameters, goalFact, pProblem,
                                            pDomain, pGlobalHistorical);
        return !res.empty();
      }
    }
    return false;
  };

  pProblem.iterateOnGoalAndRemoveNonPersistent(tryToFindAnActionTowardGoal);
  return res;
}


std::string printActionIdWithParameters(
    const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters)
{
  std::string res = pActionId;
  if (!pParameters.empty())
  {
    res += "(";
    bool firstIeration = true;
    for (const auto& currParam : pParameters)
    {
      if (firstIeration)
        firstIeration = false;
      else
        res += ", ";
      res += currParam.first + " -> " + currParam.second;
    }
    res += ")";
  }
  return res;
}


std::list<ActionId> solve(Problem& pProblem,
                          const Domain& pDomain,
                          Historical* pGlobalHistorical)
{
  std::list<std::string> res;
  while (!pProblem.goals().empty())
  {
    std::map<std::string, std::string> parameters;
    auto actionToDo = lookForAnActionToDo(parameters, pProblem, pDomain, pGlobalHistorical);
    if (actionToDo.empty())
      break;
    res.emplace_back(printActionIdWithParameters(actionToDo, parameters));

    auto itAction = pDomain.actions().find(actionToDo);
    if (itAction != pDomain.actions().end())
    {
      if (pGlobalHistorical != nullptr)
        pGlobalHistorical->notifyActionDone(actionToDo);
      pProblem.notifyActionDone(actionToDo, parameters, itAction->second.effects, nullptr);
    }
  }
  return res;
}


} // !cp
