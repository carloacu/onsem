#include <contextualplanner/contextualplanner.hpp>
#include <iostream>
#include <assert.h>
#include "test_arithmeticevaluator.hpp"


namespace
{
const std::string _sep = ", ";

const std::string _fact_advertised = "advertised";
const std::string _fact_beginOfConversation = "begin_of_conversation";
const std::string _fact_presented = "presented";
const std::string _fact_greeted = "greeted";
const std::string _fact_is_close = "is_close";
const std::string _fact_hasQrCode = "has_qrcode";
const std::string _fact_hasCheckInPasword = "has_check_in_password";
const std::string _fact_checkedIn = "checked_in";
const std::string _fact_beSad = "be_sad";
const std::string _fact_beHappy = "be_happy";
const std::string _fact_askAllTheQuestions = "ask_all_the_questions";
const std::string _fact_finishToAskQuestions = "finished_to_ask_questions";
const std::string _fact_engagedWithUser = "engaged_with_user";
const std::string _fact_userSatisfied = "user_satisfied";
const std::string _fact_robotLearntABehavior = "robot_learnt_a_behavior";

const std::string _action_presentation = "presentation";
const std::string _action_askQuestion1 = "ask_question_1";
const std::string _action_askQuestion2 = "ask_question_2";
const std::string _action_finisehdToAskQuestions = "finish_to_ask_questions";
const std::string _action_sayQuestionBilan = "say_question_bilan";
const std::string _action_greet = "greet";
const std::string _action_advertise = "advertise";
const std::string _action_checkIn = "check_in";
const std::string _action_joke = "joke";
const std::string _action_checkInWithRealPerson = "check_in_with_real_person";
const std::string _action_checkInWithQrCode = "check_in_with_qrcode";
const std::string _action_checkInWithPassword = "check_in_with_password";
const std::string _action_goodBoy = "good_boy";
const std::string _action_navigate = "navigate";
const std::string _action_welcome = "welcome";

template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}

template <typename TYPE>
void assert_true(const TYPE& pValue)
{
  if (!pValue)
    assert(false);
}


std::string _listOfStrToStr(const std::list<std::string>& pStrs)
{
  auto size = pStrs.size();
  if (size == 1)
    return pStrs.front();
  std::string res;
  bool firstIteration = true;
  for (const auto& currStr : pStrs)
  {
    if (firstIteration)
      firstIteration = false;
    else
      res += _sep;
    res += currStr;
  }
  return res;
}


std::string _solveStrConst(const cp::Problem& pProblem,
                           const std::map<std::string, cp::Action>& pActions,
                           cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  cp::Domain domain(pActions);
  return _listOfStrToStr(cp::solve(problem, domain, pGlobalHistorical));
}

std::string _solveStr(cp::Problem& pProblem,
                      const std::map<std::string, cp::Action>& pActions,
                      cp::Historical* pGlobalHistorical = nullptr)
{
  cp::Domain domain(pActions);
  return _listOfStrToStr(cp::solve(pProblem, domain, pGlobalHistorical));
}


std::string _lookForAnActionToDo(cp::Problem& pProblem,
                                 const cp::Domain& pDomain)
{
  std::map<std::string, std::string> parameters;
  auto actionId = cp::lookForAnActionToDo(parameters, pProblem, pDomain);
  return cp::printActionIdWithParameters(actionId, parameters);
}


std::string _lookForAnActionToDoConst(const cp::Problem& pProblem,
                                      const cp::Domain& pDomain)
{
  auto problem = pProblem;
  std::map<std::string, std::string> parameters;
  auto actionId = cp::lookForAnActionToDo(parameters, problem, pDomain);
  return cp::printActionIdWithParameters(actionId, parameters);
}


void _test_setOfFactsFromStr()
{
  {
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr("a,b", ',');
    assert(sOfFacts.facts.count(cp::Fact("a")) == 1);
    assert(sOfFacts.facts.count(cp::Fact("b")) == 1);
  }
  {
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr(" a, b=ok , c(r)=t , d(b=ok, c(r)=t)=val , e ", ',');
    assert(sOfFacts.facts.count(cp::Fact("a")) == 1);
    cp::Fact bFact;
    bFact.name = "b";
    bFact.value = "ok";
    assert(sOfFacts.facts.count(bFact) == 1);
    cp::Fact cFact;
    cFact.name = "c";
    cFact.parameters.emplace_back(cp::Fact("r"));
    cFact.value = "t";
    assert(sOfFacts.facts.count(cFact) == 1);
    cp::Fact dFact;
    dFact.name = "d";
    dFact.parameters.emplace_back(bFact);
    dFact.parameters.emplace_back(cFact);
    dFact.value = "val";
    assert(sOfFacts.facts.count(dFact) == 1);
    assert(sOfFacts.facts.count(cp::Fact("e")) == 1);
    assert(sOfFacts.facts.count(cp::Fact("f")) == 0);
  }
}

void _noPreconditionGoalImmediatlyReached()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_goodBoy, _lookForAnActionToDo(problem, domain));
  assert_true(!problem.goals().empty());
  problem.addFact(_fact_beHappy);
}


void _noPlanWithALengthOf2()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _noPlanWithALengthOf3()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({_fact_greeted}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2preconditions()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2Goals()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2UnrelatedGoals()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _impossibleGoal()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _privigelizeTheActionsThatHaveManyPreconditions()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({_fact_hasQrCode}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithPassword, cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasQrCode});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithQrCode + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasCheckInPasword});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}

void _preconditionThatCannotBeSolved()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({_fact_hasQrCode}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithPassword, cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_true(_lookForAnActionToDo(problem, domain).empty());
}


void _preferInContext()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({}, {_fact_checkedIn}, {_fact_hasQrCode}));
  actions.emplace(_action_checkInWithPassword, cp::Action({}, {_fact_checkedIn}, {_fact_hasCheckInPasword}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasQrCode});
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasCheckInPasword});
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  problem.setFacts({});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasQrCode});
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasCheckInPasword});
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  auto dontHaveAQrCode = cp::SetOfFacts::fromStr("!" + _fact_hasQrCode, ',');
  actions.emplace(_action_checkInWithRealPerson, cp::Action({}, {_fact_checkedIn}, dontHaveAQrCode));
  problem.setFacts({});
  assert_eq(_action_checkInWithRealPerson + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}


void _preferWhenPreconditionAreCloserToTheRealFacts()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}, {_fact_beginOfConversation}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_beginOfConversation});
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _avoidToDo2TimesTheSameActionIfPossble()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}, {_fact_beginOfConversation}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setGoals({_fact_greeted});
  assert_eq(_action_greet, _solveStr(problem, actions));

  problem.setFacts({});
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _takeHistoricalIntoAccount()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, &problem.historical));

  assert_eq(_action_presentation + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, &problem.historical));
}


void _goDoTheActionThatHaveTheMostPrerequisitValidated()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_advertise, cp::Action({}, {_fact_advertised}));
  actions.emplace(_action_checkIn, cp::Action({_fact_is_close}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_advertised, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setFacts({_fact_is_close});
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDo(problem, domain));
}


void _checkShouldBeDoneAsap()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}, {}, true));
  actions.emplace(_action_checkIn, cp::Action({_fact_is_close}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));

  cp::Problem problem;
  problem.setFacts({_fact_is_close});
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _checkNotInAPrecondition()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(cp::SetOfFacts({}, {_fact_checkedIn}), {_fact_greeted}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_greeted});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  problem.modifyFacts({_fact_checkedIn});
  assert_eq(std::string(), _lookForAnActionToDoConst(problem, domain));
}


void _checkClearGoalsWhenItsAlreadySatisfied()
{
  std::map<std::string, cp::Action> actions;
  cp::Domain domain(actions);
  cp::Problem problem;
  problem.setFacts({_fact_greeted});
  problem.setGoals({_fact_greeted});
  assert_eq<std::size_t>(1, problem.goals().size());
  _lookForAnActionToDo(problem, domain);
  assert_eq<std::size_t>(0, problem.goals().size());
}


void _fromAndToStrOfSetOfFacts()
{
  char sep = '\n';
  auto setOfFacts = cp::SetOfFacts::fromStr("++${var-name}", sep);
  setOfFacts.rename(cp::Fact::fromStr("var-name"), cp::Fact::fromStr("var-new-name"));
  assert_eq<std::string>("++${var-new-name}", setOfFacts.toStr("\n"));
  setOfFacts = cp::SetOfFacts::fromStr("${var-name-to-check}=10", sep);
  setOfFacts.rename(cp::Fact::fromStr("var-name-to-check"), cp::Fact::fromStr("var-name-to-check-new"));
  assert_eq<std::string>("${var-name-to-check-new}=10", setOfFacts.toStr("\n"));
}


void _testIncrementOfVariables()
{
  std::map<std::string, cp::Action> actions;
  const cp::Action actionQ1({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", '\n'));
  const cp::Action actionFinishToActActions(cp::SetOfFacts::fromStr("${number-of-question}=${max-number-of-questions}", '\n'), {_fact_askAllTheQuestions}, {}, true);
  const cp::Action actionSayQuestionBilan({_fact_askAllTheQuestions}, {_fact_finishToAskQuestions});
  actions.emplace(_action_askQuestion1, actionQ1);
  actions.emplace(_action_askQuestion2, cp::Action({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", '\n')));
  actions.emplace(_action_finisehdToAskQuestions, actionFinishToActActions);
  actions.emplace(_action_sayQuestionBilan, actionSayQuestionBilan);
  cp::Domain domain(actions);

  std::string initFactsStr = "${number-of-question}=0\n${max-number-of-questions}=3";
  auto initFacts = cp::SetOfFacts::fromStr(initFactsStr, '\n');
  assert_eq(initFactsStr, initFacts.toStr("\n"));
  cp::Problem problem;
  problem.modifyFacts(initFacts);
  assert(cp::areFactsTrue(initFacts, problem));
  assert(cp::areFactsTrue(actionQ1.preconditions, problem));
  assert(!cp::areFactsTrue(actionFinishToActActions.preconditions, problem));
  assert(!cp::areFactsTrue(actionSayQuestionBilan.preconditions, problem));
  assert(cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+3", '\n'), problem));
  assert(!cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+4", '\n'), problem));
  assert(cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+4-1", '\n'), problem));
  for (std::size_t i = 0; i < 3; ++i)
  {
    problem.setGoals({_fact_finishToAskQuestions});
    auto actionToDo = _lookForAnActionToDo(problem, domain);
    if (i == 0 || i == 2)
      assert_eq<std::string>(_action_askQuestion1, actionToDo);
    else
      assert_eq<std::string>(_action_askQuestion2, actionToDo);
    problem.historical.notifyActionDone(actionToDo);
    auto itAction = domain.actions().find(actionToDo);
    assert(itAction != domain.actions().end());
    problem.modifyFacts(itAction->second.effects);
    problem.modifyFacts(cp::SetOfFacts({}, {_fact_askAllTheQuestions}));
  }
  assert(cp::areFactsTrue(actionQ1.preconditions, problem));
  assert(cp::areFactsTrue(actionFinishToActActions.preconditions, problem));
  assert(!cp::areFactsTrue(actionSayQuestionBilan.preconditions, problem));
  problem.setGoals({_fact_finishToAskQuestions});
  auto actionToDo = _lookForAnActionToDo(problem, domain);
  assert_eq<std::string>(_action_finisehdToAskQuestions, actionToDo);
  problem.historical.notifyActionDone(actionToDo);
  auto itAction = domain.actions().find(actionToDo);
  assert(itAction != domain.actions().end());
  problem.modifyFacts(itAction->second.effects);
  assert_eq<std::string>(_action_sayQuestionBilan, _lookForAnActionToDo(problem, domain));
  assert(cp::areFactsTrue(actionQ1.preconditions, problem));
  assert(cp::areFactsTrue(actionFinishToActActions.preconditions, problem));
  assert(cp::areFactsTrue(actionSayQuestionBilan.preconditions, problem));
  problem.modifyFacts(actionSayQuestionBilan.effects);
}

void _precoditionEqualEffect()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({_fact_beHappy}, {_fact_beHappy}));
  cp::Domain domain(actions);
  assert_true(domain.actions().empty());

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_true(_lookForAnActionToDo(problem, domain).empty());
}


void _circularDependencies()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({_fact_greeted}, {_fact_checkedIn}));
  actions.emplace("check-in-pwd", cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace("inverse-of-check-in-pwd", cp::Action({_fact_checkedIn}, {_fact_hasCheckInPasword}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setGoals({_fact_beHappy});
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain));
}


void _triggerActionThatRemoveAFact()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_joke, cp::Action({_fact_beSad}, cp::SetOfFacts({}, {_fact_beSad})));
  actions.emplace(_action_goodBoy, cp::Action(cp::SetOfFacts({}, {_fact_beSad}), {_fact_beHappy}));

  cp::Historical historical;
  cp::Problem problem;
  problem.addFact(_fact_beSad);
  problem.setGoals({_fact_beHappy});
  assert_eq(_action_joke + _sep +
            _action_goodBoy, _solveStr(problem, actions, &historical));
}


void _actionWithConstantValue()
{
  std::map<std::string, cp::Action> actions;
  cp::Action navigate({}, {cp::Fact::fromStr("place=kitchen")});
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  problem.setGoals({cp::Goal("place=kitchen")});
  assert_eq(_action_navigate, _solveStr(problem, actions));
}


void _actionWithParameterizedValue()
{
  std::map<std::string, cp::Action> actions;
  cp::Action navigate({}, {cp::Fact::fromStr("place=target")});
  navigate.parameters.emplace_back("target");
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  problem.setGoals({cp::Goal("place=kitchen")});
  assert_eq(_action_navigate + "(target -> kitchen)", _solveStr(problem, actions));
}


void _actionWithParameterizedParameter()
{
  std::map<std::string, cp::Action> actions;
  cp::Action joke({}, {cp::Fact::fromStr("isHappy(human)")});
  joke.parameters.emplace_back("human");
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  problem.setGoals({cp::Goal("isHappy(1)")});
  assert_eq(_action_joke + "(human -> 1)", _solveStr(problem, actions));
}


void _actionWithParametersInPreconditionsAndEffects()
{
  std::map<std::string, cp::Action> actions;
  cp::Action joke({cp::Fact::fromStr("isEngaged(human)")}, {cp::Fact::fromStr("isHappy(human)")});
  joke.parameters.emplace_back("human");
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  problem.addFact(cp::Fact::fromStr("isEngaged(1)"));
  problem.setGoals({cp::Goal("isHappy(1)")});
  assert_eq(_action_joke + "(human -> 1)", _solveStr(problem, actions));
}


void _actionWithParametersInPreconditionsAndEffectsWithoutSolution()
{
  std::map<std::string, cp::Action> actions;
  cp::Action joke({cp::Fact::fromStr("isEngaged(human)")}, {cp::Fact::fromStr("isHappy(human)")});
  joke.parameters.emplace_back("human");
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  problem.addFact(cp::Fact::fromStr("isEngaged(2)"));
  problem.setGoals({cp::Goal("isHappy(1)")});
  assert_eq<std::string>("", _solveStr(problem, actions));
}

void _actionWithParametersInsideThePath()
{
  std::map<std::string, cp::Action> actions;
  cp::Action navigateAction({}, {cp::Fact::fromStr("place=target")});
  navigateAction.parameters.emplace_back("target");
  actions.emplace(_action_navigate, navigateAction);

  actions.emplace(_action_welcome, cp::Action({cp::Fact::fromStr("place=entrance")}, {cp::Fact::fromStr("welcomePeople")}));

  cp::Problem problem;
  problem.setGoals({cp::Goal("welcomePeople")});
  assert_eq<std::string>(_action_navigate + "(target -> entrance)" + _sep +
                         _action_welcome, _solveStr(problem, actions));
}


void _testPersistGoal()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_welcome, cp::Action({}, {cp::Fact::fromStr("welcomePeople")}));

  cp::Problem problem;
  problem.setGoals({cp::Goal("welcomePeople")});
  assert_eq<std::size_t>(1, problem.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goals().size());

  problem = cp::Problem();
  problem.setGoals({cp::Goal("persist(welcomePeople)")});
  assert_eq<std::size_t>(1, problem.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goals().size());
}


void _testImplyGoal()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));

  cp::Problem problem;
  problem.setGoals({cp::Goal("persist(imply(" + _fact_greeted + ", " + _fact_checkedIn + "))")});
  assert_eq<std::string>("", _solveStr(problem, actions));
  problem.addFact(_fact_greeted);
  assert_eq<std::string>(_action_checkIn, _solveStr(problem, actions));
}


void _checkPreviousBugAboutSelectingAnInappropriateAction()
{
  std::map<std::string, cp::Action> actions;
  auto removeLearntBehavior = cp::SetOfFacts::fromStr("!" + _fact_robotLearntABehavior, ',');
  actions.emplace(_action_askQuestion1, cp::Action({_fact_engagedWithUser}, {_fact_userSatisfied}, removeLearntBehavior));
  actions.emplace(_action_checkIn, cp::Action({}, cp::SetOfFacts::fromStr("!" + _fact_robotLearntABehavior + ", " + _fact_advertised, ',')));
  cp::Domain domain(actions);

  cp::Historical historical;
  cp::Problem problem;
  problem.setFacts({_fact_engagedWithUser});
  problem.setGoals({"persist(" + _fact_userSatisfied + ")"});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
  problem.removeFact(_fact_userSatisfied);
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


void _dontLinkActionWithPreferredInContext()
{
  std::map<std::string, cp::Action> actions;
  auto removeLearntBehavior = cp::SetOfFacts::fromStr("!" + _fact_robotLearntABehavior, ',');
  actions.emplace(_action_askQuestion1, cp::Action({}, {_fact_userSatisfied}, {_fact_checkedIn}));
  actions.emplace(_action_checkIn, cp::Action({_fact_engagedWithUser}, {_fact_checkedIn}));
  cp::Domain domain(actions);

  cp::Historical historical;
  cp::Problem problem;
  problem.setFacts({_fact_engagedWithUser});
  problem.setGoals({_fact_userSatisfied});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


}



int main(int argc, char *argv[])
{
  test_arithmeticEvaluator();
  _test_setOfFactsFromStr();
  _noPreconditionGoalImmediatlyReached();
  _noPlanWithALengthOf2();
  _noPlanWithALengthOf3();
  _2preconditions();
  _2Goals();
  _2UnrelatedGoals();
  _impossibleGoal();
  _privigelizeTheActionsThatHaveManyPreconditions();
  _preconditionThatCannotBeSolved();
  _preferInContext();
  _preferWhenPreconditionAreCloserToTheRealFacts();
  _avoidToDo2TimesTheSameActionIfPossble();
  _takeHistoricalIntoAccount();
  _goDoTheActionThatHaveTheMostPrerequisitValidated();
  _checkShouldBeDoneAsap();
  _checkNotInAPrecondition();
  _checkClearGoalsWhenItsAlreadySatisfied();
  _fromAndToStrOfSetOfFacts();
  _testIncrementOfVariables();
  _precoditionEqualEffect();
  _circularDependencies();
  _triggerActionThatRemoveAFact();
  _actionWithConstantValue();
  _actionWithParameterizedValue();
  _actionWithParameterizedParameter();
  _actionWithParametersInPreconditionsAndEffects();
  _actionWithParametersInPreconditionsAndEffectsWithoutSolution();
  _actionWithParametersInsideThePath();
  _testPersistGoal();
  _testImplyGoal();
  _checkPreviousBugAboutSelectingAnInappropriateAction();
  _dontLinkActionWithPreferredInContext();

  std::cout << "chatbot planner is ok !!!!" << std::endl;
  return 0;
}
