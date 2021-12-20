#include <onsem/chatbotplanner/chatbotplanner.hpp>
#include <iostream>
#include <assert.h>
#include "test_arithmeticevaluator.hpp"

using namespace onsem;

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

const std::string _action_presentation = "presentation";
const std::string _action_askQuestion1 = "ask_question_1";
const std::string _action_askQuestion2 = "ask_question_2";
const std::string _action_finisehdToAskQuestions = "finish_to_ask_questions";
const std::string _action_sayQuestionBilan = "say_question_bilan";
const std::string _action_greet = "greet";
const std::string _action_advertise = "advertise";
const std::string _action_checkIn = "check_in";
const std::string _action_joke = "joke";
const std::string _action_checkInWithQrCode = "check_in_with_qrcode";
const std::string _action_checkInWithPassword = "check_in_with_password";
const std::string _action_goodBoy = "good_boy";


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


std::string _solveStrConst(const cp::State& pState,
                           const std::map<std::string, cp::Action>& pActions,
                           cp::Historical* pGlobalHistorical = nullptr)
{
  auto state = pState;
  cp::Domain domain(pActions);
  return _listOfStrToStr(cp::solve(state, domain, pGlobalHistorical));
}

std::string _solveStr(cp::State& pState,
                      const std::map<std::string, cp::Action>& pActions,
                      cp::Historical* pGlobalHistorical = nullptr)
{
  cp::Domain domain(pActions);
  return _listOfStrToStr(cp::solve(pState, domain, pGlobalHistorical));
}


std::string _lookForAnActionToDoConst(const cp::State& pState,
                                      const cp::Domain& pDomain)
{
  auto state = pState;
  return cp::lookForAnActionToDo(state, pDomain);
}


void _test_setOfFactsFromStr()
{
  {
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr("a,b", ",");
    assert(sOfFacts.facts.count("a") == 1);
    assert(sOfFacts.facts.count("b") == 1);
  }
  {
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr(" a, b , c  ", ",");
    assert(sOfFacts.facts.count("a") == 1);
    assert(sOfFacts.facts.count("b") == 1);
    assert(sOfFacts.facts.count("c") == 1);
  }
}

void _noPreconditionGolalImmediatlyReached()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_goodBoy, cp::lookForAnActionToDo(state, domain));
  assert_true(!state.goals().empty());
  state.addFact(_fact_beHappy);
  assert_true(state.goals().empty());
}


void _noPlanWithALengthOf2()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(state, actions));
}


void _noPlanWithALengthOf3()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({_fact_greeted}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(state, actions));
}

void _2preconditions()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStr(state, actions));
}

void _2Goals()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(state, actions));
}

void _2UnrelatedGoals()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(state, actions));
}


void _impossibleGoal()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(state, actions));
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

  cp::State state;
  state.setGoals({_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setFacts({_fact_hasQrCode});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithQrCode + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setFacts({_fact_hasCheckInPasword});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(state, actions));
}

void _preconditionThatCannotBeSolved()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({_fact_hasQrCode}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithPassword, cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_true(cp::lookForAnActionToDo(state, domain).empty());
}


void _preferInContext()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({}, {_fact_checkedIn}, {_fact_hasQrCode}));
  actions.emplace(_action_checkInWithPassword, cp::Action({}, {_fact_checkedIn}, {_fact_hasCheckInPasword}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setFacts({_fact_hasQrCode});
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setFacts({_fact_hasCheckInPasword});
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  state.setFacts({});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setFacts({_fact_hasQrCode});
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setFacts({_fact_hasCheckInPasword});
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

}


void _preferWhenPreconditionAreCloserToTheRealFacts()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}, {_fact_beginOfConversation}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setFacts({_fact_beginOfConversation});
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(state, actions));
}


void _avoidToDo2TimesTheSameActionIfPossble()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}, {_fact_beginOfConversation}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(state, actions));

  state.setGoals({_fact_greeted});
  assert_eq(_action_greet, _solveStr(state, actions));

  state.setFacts({});
  state.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStr(state, actions));
}


void _takeHistoricalIntoAccount()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(state, actions, &state.historical));

  assert_eq(_action_presentation + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(state, actions, &state.historical));
}


void _goDoTheActionThatHaveTheMostPrerequisitValidated()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_advertise, cp::Action({}, {_fact_advertised}));
  actions.emplace(_action_checkIn, cp::Action({_fact_is_close}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_advertised, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::State state;
  state.setFacts({_fact_is_close});
  state.setGoals({_fact_beHappy});
  assert_eq(_action_checkIn, cp::lookForAnActionToDo(state, domain));
}


void _checkShouldBeDoneAsap()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}, {}, true));
  actions.emplace(_action_checkIn, cp::Action({_fact_is_close}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));

  cp::State state;
  state.setFacts({_fact_is_close});
  state.setGoals({_fact_beHappy});
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(state, actions));
}


void _checkNotInAPrecondition()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(cp::SetOfFacts({}, {_fact_checkedIn}), {_fact_greeted}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_greeted});
  assert_eq(_action_greet, _lookForAnActionToDoConst(state, domain));
  state.modifyFacts({_fact_checkedIn});
  assert_eq(std::string(), _lookForAnActionToDoConst(state, domain));
}


void _checkClearGoalsWhenItsAlreadySatisfied()
{
  cp::State state;
  state.setGoals({_fact_greeted, _fact_checkedIn});
  assert_eq<std::size_t>(2, state.goals().size());
  state.addFacts(std::vector<cp::Fact>{_fact_greeted, _fact_checkedIn});
  assert_eq<std::size_t>(0, state.goals().size());
  state.setFacts({});
  state.setGoals({_fact_greeted, _fact_checkedIn});
  assert_eq<std::size_t>(2, state.goals().size());
  state.addFacts(std::vector<cp::Fact>{_fact_checkedIn, _fact_greeted});
  assert_eq<std::size_t>(0, state.goals().size());
}


void _fromAndToStrOfSetOfFacts()
{
  std::string sep = "\n";
  auto setOfFacts = cp::SetOfFacts::fromStr("++${var-name}", sep);
  setOfFacts.rename("var-name", "var-new-name");
  assert_eq<std::string>("++${var-new-name}", setOfFacts.toStr(sep));
  setOfFacts = cp::SetOfFacts::fromStr("${var-name-to-check}=10", sep);
  setOfFacts.rename("var-name-to-check", "var-name-to-check-new");
  assert_eq<std::string>("${var-name-to-check-new}=10", setOfFacts.toStr(sep));
}


void _testIncrementOfVariables()
{
  std::map<std::string, cp::Action> actions;
  const cp::Action actionQ1({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", "\n"));
  const cp::Action actionFinishToActActions(cp::SetOfFacts::fromStr("${number-of-question}=${max-number-of-questions}", "\n"), {_fact_askAllTheQuestions}, {}, true);
  const cp::Action actionSayQuestionBilan({_fact_askAllTheQuestions}, {_fact_finishToAskQuestions});
  actions.emplace(_action_askQuestion1, actionQ1);
  actions.emplace(_action_askQuestion2, cp::Action({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", "\n")));
  actions.emplace(_action_finisehdToAskQuestions, actionFinishToActActions);
  actions.emplace(_action_sayQuestionBilan, actionSayQuestionBilan);
  cp::Domain domain(actions);

  std::string initFactsStr = "${number-of-question}=0\n${max-number-of-questions}=3";
  auto initFacts = cp::SetOfFacts::fromStr(initFactsStr, "\n");
  assert_eq(initFactsStr, initFacts.toStr("\n"));
  cp::State state;
  state.modifyFacts(initFacts);
  assert(cp::areFactsTrue(initFacts, state));
  assert(cp::areFactsTrue(actionQ1.preconditions, state));
  assert(!cp::areFactsTrue(actionFinishToActActions.preconditions, state));
  assert(!cp::areFactsTrue(actionSayQuestionBilan.preconditions, state));
  assert(cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+3", "\n"), state));
  assert(!cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+4", "\n"), state));
  assert(cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+4-1", "\n"), state));
  for (std::size_t i = 0; i < 3; ++i)
  {
    state.setGoals({_fact_finishToAskQuestions});
    auto actionToDo = cp::lookForAnActionToDo(state, domain);
    if (i == 0 || i == 2)
      assert_eq<std::string>(_action_askQuestion1, actionToDo);
    else
      assert_eq<std::string>(_action_askQuestion2, actionToDo);
    state.historical.notifyActionDone(actionToDo);
    auto itAction = domain.actions().find(actionToDo);
    assert(itAction != domain.actions().end());
    state.modifyFacts(itAction->second.effects);
    state.modifyFacts(cp::SetOfFacts({}, {_fact_askAllTheQuestions}));
  }
  assert(cp::areFactsTrue(actionQ1.preconditions, state));
  assert(cp::areFactsTrue(actionFinishToActActions.preconditions, state));
  assert(!cp::areFactsTrue(actionSayQuestionBilan.preconditions, state));
  state.setGoals({_fact_finishToAskQuestions});
  auto actionToDo = cp::lookForAnActionToDo(state, domain);
  assert_eq<std::string>(_action_finisehdToAskQuestions, actionToDo);
  state.historical.notifyActionDone(actionToDo);
  auto itAction = domain.actions().find(actionToDo);
  assert(itAction != domain.actions().end());
  state.modifyFacts(itAction->second.effects);
  assert_eq<std::string>(_action_sayQuestionBilan, cp::lookForAnActionToDo(state, domain));
  assert(cp::areFactsTrue(actionQ1.preconditions, state));
  assert(cp::areFactsTrue(actionFinishToActActions.preconditions, state));
  assert(cp::areFactsTrue(actionSayQuestionBilan.preconditions, state));
  state.modifyFacts(actionSayQuestionBilan.effects);
  assert(state.goals().empty());
}

void _precoditionEqualEffect()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({_fact_beHappy}, {_fact_beHappy}));
  cp::Domain domain(actions);
  assert_true(domain.actions().empty());

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_true(cp::lookForAnActionToDo(state, domain).empty());
}


void _circularDependencies()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({_fact_greeted}, {_fact_checkedIn}));
  actions.emplace("check-in-pwd", cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace("inverse-of-check-in-pwd", cp::Action({_fact_checkedIn}, {_fact_hasCheckInPasword}));
  cp::Domain domain(actions);

  cp::State state;
  state.setGoals({_fact_beHappy});
  assert_eq<std::string>("", _lookForAnActionToDoConst(state, domain));
}


void _triggerActionThatRemoveAFact()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_joke, cp::Action({_fact_beSad}, cp::SetOfFacts({}, {_fact_beSad})));
  actions.emplace(_action_goodBoy, cp::Action(cp::SetOfFacts({}, {_fact_beSad}), {_fact_beHappy}));

  cp::Historical historical;
  cp::State state;
  state.addFact(_fact_beSad);
  state.setGoals({_fact_beHappy});
  assert_eq(_action_joke + _sep +
            _action_goodBoy, _solveStrConst(state, actions, &historical));
}

}



int main(int argc, char *argv[])
{
  test_arithmeticEvaluator();
  _test_setOfFactsFromStr();
  _noPreconditionGolalImmediatlyReached();
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

  std::cout << "chatbot planner is ok !!!!" << std::endl;
  return 0;
}
