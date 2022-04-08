#include "semexpcreator.hpp"
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semexpsimplifer.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>


namespace onsem
{
namespace SemExpCreator
{

namespace
{

std::unique_ptr<GroundedExpression> _saySorry()
{
  return mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticConceptualGrounding>("sorry"));
}

std::unique_ptr<SemanticExpression> _copyAndReformateSemExpToPutItInAnAnswer
(const SemanticExpression& pSemExp)
{
  static const std::set<SemanticExpressionType> expressionTypesToSkip =
  {SemanticExpressionType::INTERPRETATION};
  return pSemExp.clone(nullptr, false, &expressionTypesToSkip);
}


UniqueSemanticExpression _addAnnotationsToSemExp
(UniqueSemanticExpression pSemExp,
 std::unique_ptr<AnnotatedExpression> pAnnExp,
 const std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer)
{
  if (!pAnnotationsOfTheAnswer.empty())
  {
    if (!pAnnExp)
      pAnnExp = mystd::make_unique<AnnotatedExpression>(std::move(pSemExp));
    for (const auto& currAnnChild : pAnnotationsOfTheAnswer)
      pAnnExp->annotations.emplace(currAnnChild.first, currAnnChild.second->clone());
  }

  if (pAnnExp)
    return std::move(pAnnExp);
  return pSemExp;
}


UniqueSemanticExpression _answerExpToSemExp(const AnswerExp& pAnswExp)
{
  auto res = pAnswExp.getGrdExp().clone();
  // remove the subordinate that are a sentence
  SemExpModifier::removeSpecificationsNotNecessaryForAnAnswer(*res);

  if (pAnswExp.equalitySemExp != nullptr)
    SemExpModifier::addChild(*res, GrammaticalType::SPECIFIER,
                             pAnswExp.equalitySemExp->clone(), ListExpressionType::UNRELATED);


  if (!pAnswExp.annotationsOfTheAnswer.empty())
  {
    std::unique_ptr<AnnotatedExpression> annExp;
    annExp = mystd::make_unique<AnnotatedExpression>(std::move(res));
    for (const auto& currAnnChild : pAnswExp.annotationsOfTheAnswer)
      annExp->annotations.emplace(currAnnChild.first, currAnnChild.second->clone());
    return std::move(annExp);
  }

  return _addAnnotationsToSemExp(std::move(res),
                                 std::unique_ptr<AnnotatedExpression>(),
                                 pAnswExp.annotationsOfTheAnswer);
}


mystd::unique_propagate_const<UniqueSemanticExpression> _copyListOfGrdExps
(const std::list<AnswerExp>& pGrdExps)
{
  std::size_t nbElts = pGrdExps.size();
  if (nbElts == 1)
  {
    return _answerExpToSemExp(pGrdExps.front());
  }
  else if (nbElts > 1)
  {
    auto listExp = mystd::make_unique<ListExpression>(ListExpressionType::AND);
    for (const auto& currChild : pGrdExps)
      listExp->elts.emplace_back(_answerExpToSemExp(currChild));
    return UniqueSemanticExpression(std::move(listExp));
  }
  return {};
}


std::unique_ptr<GroundedExpression> _reformulateQuestionWithListOfAnswers(
    std::map<SemanticRequestType, UniqueSemanticExpression>& pRequestToAnswers,
    const GroundedExpression& pGrdExpQuestion,
    const SemanticMemoryBlock& pMemBlock,
    const linguistics::LinguisticDatabase& pLingDb)
{
  auto rootGrdExp = pGrdExpQuestion.clone(nullptr, true);
  SemExpModifier::clearRequestList(*rootGrdExp);

  for (auto& currAnswer : pRequestToAnswers)
  {
    GrammaticalType gramTypeToAnswer = [](SemanticRequestType pRequest)
    {
      auto res = semanticRequestType_toSemGram(pRequest);
      if (res == GrammaticalType::UNKNOWN)
        return GrammaticalType::OBJECT;
      return res;
    }(currAnswer.first);
    bool needToReplaceGramTypeAnswer = true;
    if (currAnswer.first == SemanticRequestType::QUANTITY)
    {
      auto itChildFromQuestion = rootGrdExp->children.find(gramTypeToAnswer);
      if (itChildFromQuestion != rootGrdExp->children.end())
      {
        auto res = SemExpGetter::getNumberOfElements(*currAnswer.second);
        if (res)
        {
          SemExpModifier::setNumberFromSemExp(itChildFromQuestion->second, *res);
          needToReplaceGramTypeAnswer = false;
        }
      }
    }
    if (currAnswer.first != SemanticRequestType::TIME)
    {
      rootGrdExp->children.erase(GrammaticalType::TIME);
      rootGrdExp->children.erase(GrammaticalType::OCCURRENCE_RANK);
    }
    if (needToReplaceGramTypeAnswer)
      rootGrdExp->children[gramTypeToAnswer] = std::move(currAnswer.second);
  }

  for (auto& currChild : rootGrdExp->children)
    simplifier::processFromMemBlock(currChild.second, pMemBlock, pLingDb);
  return rootGrdExp;
}


std::unique_ptr<GroundedExpression> _sayThatWeKnow(
    const SemanticExpression& pObjectSemExp,
    bool pPolarity = true)
{
  auto rootGrdExp = sayIKnow(pPolarity);

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               _copyAndReformateSemExpToPutItInAnAnswer(pObjectSemExp));
  return rootGrdExp;
}


UniqueSemanticExpression _meSemExp()
{
  return mystd::make_unique<GroundedExpression>(SemanticAgentGrounding::getRobotAgentPtr());
}

std::unique_ptr<GroundedExpression> _statGrdExp(const std::string& pConcept)
{
  return mystd::make_unique<GroundedExpression>
      ([&pConcept]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace(pConcept, 4);
    return statementGrd;
  }());
}

std::unique_ptr<GroundedExpression> _whatIsGrdExp()
{
  return mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace(ConceptSet::conceptVerbEquality, 4);
    statementGrd->concepts.emplace("verb_equal_mean", 4);
    statementGrd->requests.set(SemanticRequestType::OBJECT);
    return statementGrd;
  }());
}

UniqueSemanticExpression _confirmSemExp(UniqueSemanticExpression pSemExp)
{
  auto yesGenGr = mystd::make_unique<SemanticGenericGrounding>();
  yesGenGr->concepts.emplace("accordance_agreement_yes", 4);
  return mystd::make_unique<FeedbackExpression>
      (mystd::make_unique<GroundedExpression>(std::move(yesGenGr)), std::move(pSemExp));
}

UniqueSemanticExpression _confirmWeAlreadyKnowIt(
    const GroundedExpression& pObjectGrdExp)
{
  return _confirmSemExp(_sayThatWeKnow(pObjectGrdExp));
}

}


std::unique_ptr<ListExpression> mergeInAList(UniqueSemanticExpression pSemExp1,
                                             UniqueSemanticExpression pSemExp2)
{
  auto res = mystd::make_unique<ListExpression>();
  res->elts.emplace_back(std::move(pSemExp1));
  res->elts.emplace_back(std::move(pSemExp2));
  return res;
}

std::unique_ptr<GroundedExpression> copyAndReformateGrdExpToPutItInAnAnswer
(const GroundedExpression& pGrdExp)
{
  static const std::set<SemanticExpressionType> expressionTypesToSkip =
  {SemanticExpressionType::INTERPRETATION};
  return pGrdExp.clone(nullptr, false, &expressionTypesToSkip);
}


std::unique_ptr<SemanticExpression> sayThat()
{
  return mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
    genGrd->coreference.emplace();
    genGrd->entityType = SemanticEntityType::THING;
    return genGrd;
  }());
}

std::unique_ptr<GroundedExpression> sayIKnow(bool pPolarity)
{
  // verb
  auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
  statementGrd->verbTense = SemanticVerbTense::PRESENT;
  if (!pPolarity)
    statementGrd->polarity = false;
  statementGrd->concepts.emplace("mentalState_know", 4);
  auto rootGrdExp =
      mystd::make_unique<GroundedExpression>(std::move(statementGrd));

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());
  return rootGrdExp;
}


std::unique_ptr<GroundedExpression> thereIsXStepsFor(
    int pNbOfSteps,
    UniqueSemanticExpression pPurposeSemExp)
{
  // verb
  auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
  statementGrd->verbTense = SemanticVerbTense::PRESENT;
  statementGrd->concepts.emplace("verb_generality", 4);
  auto rootGrdExp =
      mystd::make_unique<GroundedExpression>(std::move(statementGrd));

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, mystd::make_unique<GroundedExpression>([&]()
  {
    auto objGrounding = mystd::make_unique<SemanticGenericGrounding>();
    objGrounding->referenceType = SemanticReferenceType::INDEFINITE;
    objGrounding->concepts.emplace("step", 4);
    objGrounding->quantity.setNumber(pNbOfSteps);
    return objGrounding;
  }()));

  // purpose
  rootGrdExp->children.emplace(GrammaticalType::PURPOSE, std::move(pPurposeSemExp));

  return rootGrdExp;
}


std::unique_ptr<GroundedExpression> doYouWantMeToSayThemOneByOne(const SemanticAgentGrounding& pSubjectGrounding)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("verb_want", 4);
    statementGrd->requests.set(SemanticRequestType::YESORNO);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                               mystd::make_unique<GroundedExpression>
                               (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               [] {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PUNCTUALPRESENT;
    statementGrd->concepts.emplace("verb_action_say", 4);
    auto childGrdExp =
        mystd::make_unique<GroundedExpression>(std::move(statementGrd));

    // subject
    childGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

    // object
    childGrdExp->children.emplace(GrammaticalType::OBJECT, mystd::make_unique<GroundedExpression>([&]()
    {
      auto objGrounding = mystd::make_unique<SemanticGenericGrounding>();
      objGrounding->referenceType = SemanticReferenceType::DEFINITE;
      objGrounding->coreference.emplace();
      objGrounding->quantity.setPlural();
      return objGrounding;
    }()));

    // specifier
    childGrdExp->children.emplace(GrammaticalType::SPECIFIER, mystd::make_unique<GroundedExpression>([&]()
    {
      auto sepcGrounding = mystd::make_unique<SemanticGenericGrounding>();
      sepcGrounding->word = SemanticWord(SemanticLanguageEnum::FRENCH , "une par une", PartOfSpeech::ADVERB);
      return sepcGrounding;
    }()));

    return childGrdExp;
  }());
  return rootGrdExp;
}


std::unique_ptr<GroundedExpression> sayAndThenToContinue()
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PUNCTUALPRESENT;
    statementGrd->concepts.emplace("verb_action_say", 4);
    statementGrd->requests.set(SemanticRequestType::ACTION);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticTextGrounding>("et après")));

  // purpose
  rootGrdExp->children.emplace(GrammaticalType::PURPOSE,
                               mystd::make_unique<GroundedExpression>([] {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::UNKNOWN;
    statementGrd->concepts.emplace("verb_continue", 4);
    return statementGrd;
  }()));

  return rootGrdExp;
}



UniqueSemanticExpression formulateConditionToAction(
    const GroundedExpression& pCondition,
    const SemanticExpression& pStuffToDo,
    const SemanticExpression* pStuffToDoOtherwisePtr,
    bool pSemExpToDoIsAlwaysActive)
{
  auto condSemExp = mystd::make_unique<ConditionExpression>
      (pSemExpToDoIsAlwaysActive, false,
       pCondition.clone(), pStuffToDo.clone());
  if (pStuffToDoOtherwisePtr != nullptr)
    condSemExp->elseExp.emplace(pStuffToDoOtherwisePtr->clone());
  return std::move(condSemExp);
}


UniqueSemanticExpression saySemxExp1IsSemExp2(UniqueSemanticExpression pSemExpSubject,
                                              UniqueSemanticExpression pSemExpObject)
{
  auto rootGrdExp = _statGrdExp(ConceptSet::conceptVerbEquality);
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, std::move(pSemExpSubject));
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pSemExpObject));
  return std::move(rootGrdExp);
}

UniqueSemanticExpression sentenceFromTriple(UniqueSemanticExpression pSemExpSubject,
                                            const std::string& pVerbConcept,
                                            UniqueSemanticExpression pSemExpObject)
{
  auto rootGrdExp = _statGrdExp(pVerbConcept);
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, std::move(pSemExpSubject));
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pSemExpObject));
  return std::move(rootGrdExp);
}




UniqueSemanticExpression askWhatIs(const SemanticExpression& pSubjectSemExp)
{
  auto rootGrdExp = _whatIsGrdExp();
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, pSubjectSemExp.clone());
  return std::move(rootGrdExp);
}


UniqueSemanticExpression formulateActionDefinition(const GroundedExpression& pLabel,
                                                   const SemanticStatementGrounding& pEqualityStatement,
                                                   UniqueSemanticExpression pDefinition)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticStatementGrounding>(pEqualityStatement));
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, pLabel.clone());
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pDefinition));
  return std::move(rootGrdExp);
}

UniqueSemanticExpression whoSemExp()
{
  return mystd::make_unique<GroundedExpression>
      ([]()
 {
   auto anyHumanGrd = mystd::make_unique<SemanticGenericGrounding>();
   anyHumanGrd->entityType = SemanticEntityType::HUMAN;
   anyHumanGrd->concepts.emplace("agent", 4);
   return anyHumanGrd;
 }());
}

UniqueSemanticExpression askWhoIs(const SemanticExpression& pSubjecSemExp)
{
  auto rootGrdExp = _whatIsGrdExp();
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, pSubjecSemExp.clone());
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, whoSemExp());
  return std::move(rootGrdExp);
}


UniqueSemanticExpression getImperativeAssociateFrom(
    const GroundedExpression& pGrdExp)
{
  auto rootGrdExp = pGrdExp.clone();
  SemanticStatementGrounding* statementGrd = (*rootGrdExp)->getStatementGroundingPtr();
  if (statementGrd != nullptr)
  {
    statementGrd->verbGoal = VerbGoalEnum::NOTIFICATION;
    statementGrd->requests.set(SemanticRequestType::ACTION);
    statementGrd->verbTense = SemanticVerbTense::PUNCTUALPRESENT;
  }
  if (rootGrdExp->children.find(GrammaticalType::SUBJECT) == rootGrdExp->children.end())
  {
    rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());
  }
  return std::move(rootGrdExp);
}


UniqueSemanticExpression getFutureIndicativeFromInfinitive(
    const GroundedExpression& pGrdExp)
{
  auto rootGrdExp = pGrdExp.clone();
  SemanticStatementGrounding* statementGrd = (*rootGrdExp)->getStatementGroundingPtr();
  if (statementGrd != nullptr)
  {
    statementGrd->verbGoal = VerbGoalEnum::NOTIFICATION;
    statementGrd->verbTense = SemanticVerbTense::FUTURE;
  }
  auto itSubject = rootGrdExp->children.find(GrammaticalType::SUBJECT);
  if (itSubject == rootGrdExp->children.end() ||
      SemExpGetter::hasGenericConcept(&itSubject->second))
  {
    if (itSubject != rootGrdExp->children.end())
      rootGrdExp->children.erase(itSubject);
    rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());
  }
  return std::move(rootGrdExp);
}

UniqueSemanticExpression getIndicativeFromImperative(
    const GroundedExpression& pGrdExp)
{
  auto rootGrdExp = pGrdExp.clone();
  SemanticStatementGrounding* statementGrd = (*rootGrdExp)->getStatementGroundingPtr();
  if (statementGrd != nullptr)
    statementGrd->requests.clear();
  return std::move(rootGrdExp);
}


UniqueSemanticExpression getInfinitiveFromImperativeForm(
    const GroundedExpression& pGrdExp)
{
  auto rootGrdExp = pGrdExp.clone();
  SemanticStatementGrounding* statementGrd = (*rootGrdExp)->getStatementGroundingPtr();
  if (statementGrd != nullptr)
  {
    statementGrd->verbGoal = VerbGoalEnum::NOTIFICATION;
    statementGrd->requests.clear();
    statementGrd->verbTense = SemanticVerbTense::UNKNOWN;
  }
  rootGrdExp->children.erase(GrammaticalType::SUBJECT);
  return std::move(rootGrdExp);
}

UniqueSemanticExpression askIfTrue(const GroundedExpression& pOriginalGrdExp,
                                   const linguistics::LinguisticDatabase& pLingDb)
{
  UniqueSemanticExpression semExp = pOriginalGrdExp.clone();
  (*semExp->getGrdExpPtr())->getStatementGroundingPtr()->requests.add(SemanticRequestType::YESORNO);
  converter::splitPossibilitiesOfQuestions(semExp, pLingDb);
  return semExp;
}

UniqueSemanticExpression askDoYouWantMeToDoItNow(
    const SemanticAgentGrounding& pSubjectGrounding,
    const GroundedExpression& pActionGrdExp)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("verb_want", 4);
    statementGrd->requests.set(SemanticRequestType::YESORNO);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                               mystd::make_unique<GroundedExpression>
                               (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               [&pActionGrdExp]
  {
    auto subRootGrdExp = pActionGrdExp.clone();
    SemExpModifier::modifyVerbTense(*subRootGrdExp, SemanticVerbTense::UNKNOWN);
    SemExpModifier::clearRequestList(*subRootGrdExp);

    if (!SemExpGetter::hasChild(*subRootGrdExp, GrammaticalType::SUBJECT))
      subRootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

    if (!SemExpGetter::hasChild(*subRootGrdExp, GrammaticalType::TIME))
      subRootGrdExp->children.emplace(GrammaticalType::TIME,
                                      mystd::make_unique<GroundedExpression>
                                      (SemanticTimeGrounding::nowPtr()));

    return subRootGrdExp;
  }());
  return std::move(rootGrdExp);
}


UniqueSemanticExpression iWantThatYou(
    const std::string& pSubjectId,
    UniqueSemanticExpression pObject)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("verb_want", 4);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                               mystd::make_unique<GroundedExpression>
                               (mystd::make_unique<SemanticAgentGrounding>(pSubjectId)));

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pObject));
  return std::move(rootGrdExp);
}


UniqueSemanticExpression sayYesOrNo(bool pAnswerPolarity)
{
  return mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticConceptualGrounding>(pAnswerPolarity ? "accordance_agreement_true" : "accordance_disagreement_false"));
}


UniqueSemanticExpression formulateAge(
    UniqueSemanticExpression&& pSubject,
    std::unique_ptr<SemanticExpression> pAgeSemExp)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("predicate_hasAge", 4);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                               std::move(pSubject));
  // time
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               std::move(pAgeSemExp));
  return std::move(rootGrdExp);
}



UniqueSemanticExpression sayThatTheAssertionIsTrueOrFalse(
    const GroundedExpression& pGrdExp,
    bool pTrueOrFalse)
{
  if (pTrueOrFalse)
  {
    if (ConceptSet::haveAConcept(pGrdExp.grounding().concepts, "mentalState_know"))
      return _confirmSemExp(pGrdExp.clone());
    return _confirmWeAlreadyKnowIt(pGrdExp);
  }

  auto rootGrdExp = pGrdExp.clone();

  // invert verb polarity
  if (!pTrueOrFalse)
    SemExpModifier::invertPolarityFromGrdExp(*rootGrdExp);

  // interjection
  return mystd::make_unique<FeedbackExpression>(mystd::make_unique<GroundedExpression>([&pTrueOrFalse]()
  {
    auto noGrounding = mystd::make_unique<SemanticGenericGrounding>();
    noGrounding->concepts.emplace(pTrueOrFalse ? "accordance_agreement_yes" : "accordance_disagreement_no", 4);
    return noGrounding;
  }()), std::move(rootGrdExp));
}


std::unique_ptr<GroundedExpression> sayICan(bool pPolarity)
{
  auto rootGrdExp =
      mystd::make_unique<GroundedExpression>([pPolarity]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("verb_can", 4);
    statementGrd->polarity = pPolarity;
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

  return rootGrdExp;
}


UniqueSemanticExpression askDoYouWantToKnowHow(const SemanticAgentGrounding& pSubjectGrounding,
                                               UniqueSemanticExpression pActionSemExp)
{
  auto rootGrdExp =
      mystd::make_unique<GroundedExpression>([]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("verb_want", 4);
    statementGrd->requests.set(SemanticRequestType::YESORNO);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                                mystd::make_unique<GroundedExpression>
                                (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));

  rootGrdExp->children.emplace(GrammaticalType::OBJECT, [&]
  {
    auto knowGrdExp =
        mystd::make_unique<GroundedExpression>([]
    {
      auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
      statementGrd->verbTense = SemanticVerbTense::UNKNOWN;
      statementGrd->concepts.emplace("mentalState_know", 4);
      return statementGrd;
    }());

    knowGrdExp->children.emplace(GrammaticalType::OBJECT, [&]
    {
      auto annExp = mystd::make_unique<AnnotatedExpression>(
            mystd::make_unique<GroundedExpression>([]
      {
        auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
        statementGrd->coreference.emplace(CoreferenceDirectionEnum::ANNOTATION_SPECIFICATIONS);
        statementGrd->requests.set(SemanticRequestType::MANNER);
        return statementGrd;
      }()));
      annExp->annotations.emplace(GrammaticalType::SPECIFIER, [&]
      {
        auto teachGrdExp = mystd::make_unique<GroundedExpression>([&]
        {
          auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
          statementGrd->verbTense = SemanticVerbTense::PRESENT;
          statementGrd->concepts.emplace("verb_action_teach", 4);
          statementGrd->verbGoal = VerbGoalEnum::ABILITY;
          statementGrd->requests.set(SemanticRequestType::MANNER);
          return statementGrd;
        }());
        teachGrdExp->children.emplace(GrammaticalType::SUBJECT,
                                      mystd::make_unique<GroundedExpression>
                                      (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));
        teachGrdExp->children.emplace(GrammaticalType::RECEIVER, _meSemExp());
        teachGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pActionSemExp));
        return teachGrdExp;
      }());
      return annExp;
    }());

    return knowGrdExp;
  }());

  return std::move(rootGrdExp);
}


UniqueSemanticExpression askWhatIsYourName(const std::string& pSubjectId)
{
  auto rootGrdExp = _whatIsGrdExp();

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, [&]
  {
    auto nameGrdExp = mystd::make_unique<GroundedExpression>([]
    {
      auto nameGenGrd = mystd::make_unique<SemanticGenericGrounding>();
      nameGenGrd->entityType = SemanticEntityType::THING;
      nameGenGrd->concepts.emplace("name", 4);
      return nameGenGrd;
    }());
    nameGrdExp->children.emplace(GrammaticalType::OWNER,
                                 mystd::make_unique<GroundedExpression>
                                 (mystd::make_unique<SemanticAgentGrounding>(pSubjectId)));
    return nameGrdExp;
  }());
  return std::move(rootGrdExp);
}


void addButYouCanTeachMe(GroundedExpression& pRootGrdExp,
                         const SemanticAgentGrounding& pSubjectGrounding)
{
  pRootGrdExp.children.emplace(GrammaticalType::MITIGATION,
                               [&]
  {
    auto teachGrdExp =
        mystd::make_unique<GroundedExpression>([]()
    {
      // verb
      auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
      statementGrd->verbTense = SemanticVerbTense::PRESENT;
      statementGrd->verbGoal = VerbGoalEnum::ABILITY;
      statementGrd->concepts.emplace("verb_action_teach", 4);
      return statementGrd;
    }());

    // subject
    teachGrdExp->children.emplace(GrammaticalType::SUBJECT,
                                  mystd::make_unique<GroundedExpression>
                                  (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));

    // receiver
    teachGrdExp->children.emplace(GrammaticalType::RECEIVER, _meSemExp());
    return teachGrdExp;
  }());
}


UniqueSemanticExpression sayIThoughtThat(UniqueSemanticExpression pObjectSemExp)
{
  auto rootGrdExp =
      mystd::make_unique<GroundedExpression>([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PAST;
    statementGrd->concepts.emplace("verb_think", 4);
    return statementGrd;
  }());

  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(pObjectSemExp));
  return std::move(rootGrdExp);
}


UniqueSemanticExpression sayThatWeThoughtTheContrary()
{
  return sayIThoughtThat(mystd::make_unique<GroundedExpression>
                         ([]()
  {
    auto contraryGrounding = mystd::make_unique<SemanticGenericGrounding>();
    contraryGrounding->referenceType = SemanticReferenceType::DEFINITE;
    contraryGrounding->concepts.emplace("opposite", 4);
    return contraryGrounding;
  }()));
}


UniqueSemanticExpression formulateWeekDay(
    const std::string& pWeekDayConcept)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PAST;
    statementGrd->concepts.emplace(ConceptSet::conceptVerbEquality, 4);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                               mystd::make_unique<GroundedExpression>
                               (SemanticGenericGrounding::makeThingThatHasToBeCompletedFromContext()));

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               mystd::make_unique<GroundedExpression>([&pWeekDayConcept]()
  {
    auto objectGrd = mystd::make_unique<SemanticGenericGrounding>(SemanticReferenceType::INDEFINITE,
                                                                  SemanticEntityType::THING);
    objectGrd->concepts.emplace(pWeekDayConcept, 4);
    return objectGrd;
  }()));

  // interjection
  return mystd::make_unique<FeedbackExpression>(sayOk(), std::move(rootGrdExp));
}


UniqueSemanticExpression okIRemoveAllConditions()
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PUNCTUALPRESENT;
    statementGrd->concepts.emplace("verb_action_remove", 4);
    return statementGrd;
  }());
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               mystd::make_unique<GroundedExpression>([]()
  {
    auto objectGrd = mystd::make_unique<SemanticGenericGrounding>();
    objectGrd->quantity.type = SemanticQuantityType::MAXNUMBER;
    objectGrd->referenceType = SemanticReferenceType::DEFINITE;
    objectGrd->entityType = SemanticEntityType::THING;
    objectGrd->concepts.emplace("condition", 4);
    return objectGrd;
  }()));

  // interjection
  return mystd::make_unique<FeedbackExpression>(sayOk(), std::move(rootGrdExp));
}


UniqueSemanticExpression formulateHowWeKnowSomething(
    const SemanticExpression& pWhatWeKnow,
    const SemanticExpression& pHowWeKnowThat)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("mentalState_know", 4);
    return statementGrd;
  }());

  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               _copyAndReformateSemExpToPutItInAnAnswer(pWhatWeKnow));
  rootGrdExp->children.emplace(GrammaticalType::CAUSE, pHowWeKnowThat.clone());
  return std::move(rootGrdExp);
}


UniqueSemanticExpression forExampleSayToDoMeansToSayIDo(
    const SemanticAgentGrounding& pAuthor,
    const GroundedExpression& pActionGrdExp)
{
  return mystd::make_unique<FeedbackExpression>(
        mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticConceptualGrounding>("forExample")),
        [&]
  {
    auto rootGrdExp = mystd::make_unique<GroundedExpression>([]
    {
      auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
      statementGrd->verbTense = SemanticVerbTense::PRESENT;
      statementGrd->verbGoal = VerbGoalEnum::ABILITY;
      statementGrd->concepts.emplace("verb_action_say", 4);
      statementGrd->word = SemanticWord(SemanticLanguageEnum::ENGLISH, "tell", PartOfSpeech::VERB);
      return statementGrd;
    }());

    rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                                 mystd::make_unique<GroundedExpression>
                                 (mystd::make_unique<SemanticAgentGrounding>(pAuthor)));

    rootGrdExp->children.emplace(GrammaticalType::RECEIVER, _meSemExp());

    rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                                 [&]
    {
      auto meansGrdExp = _statGrdExp(ConceptSet::conceptVerbEquality);
      meansGrdExp->children.emplace(GrammaticalType::SUBJECT, pActionGrdExp.clone());
      meansGrdExp->children.emplace(GrammaticalType::OBJECT, [&]
      {
        auto sayGrdExp = mystd::make_unique<GroundedExpression>([&]
        {
          auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
          statementGrd->verbTense = SemanticVerbTense::UNKNOWN;
          statementGrd->concepts.emplace("verb_action_say", 4);
          return statementGrd;
        }());
        auto actionInPresent = pActionGrdExp.clone();
        SemExpModifier::modifyVerbTense(*actionInPresent, SemanticVerbTense::PUNCTUALPRESENT);
        actionInPresent->children.emplace(GrammaticalType::SUBJECT, _meSemExp());
        sayGrdExp->children.emplace(GrammaticalType::OBJECT, std::move(actionInPresent));
        return sayGrdExp;
      }());
      return meansGrdExp;
    }());
    return rootGrdExp;
  }());
}


std::unique_ptr<SemanticExpression> getSemExpThatSomebodyToldMeThat(
    const SemanticAgentGrounding& pAuthor)
{
  // verb
  auto grdExp =
      mystd::make_unique<GroundedExpression>([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PUNCTUALPAST;
    statementGrd->concepts.emplace("verb_action_say", 4);
    return statementGrd;
  }());

  // subject
  grdExp->children.emplace(GrammaticalType::SUBJECT,
                           mystd::make_unique<GroundedExpression>
                           (mystd::make_unique<SemanticAgentGrounding>(pAuthor)));

  // receiver
  grdExp->children.emplace(GrammaticalType::RECEIVER, _meSemExp());

  // object
  grdExp->children.emplace(GrammaticalType::OBJECT,
                           mystd::make_unique<GroundedExpression>
                           (SemanticGenericGrounding::makeThingThatHasToBeCompletedFromContext()));

  return std::move(grdExp);
}



std::unique_ptr<SemanticExpression> getSemExpOfEventValue(
    const std::string& pEventName,
    const std::string& pEventValue)
{
  // verb
  auto grdExp = _statGrdExp(ConceptSet::conceptVerbEquality);

  // subject
  grdExp->children.emplace(GrammaticalType::SUBJECT, [&pEventName]()
  {
    auto subjectGrdExp = mystd::make_unique<GroundedExpression>([]()
    {
      auto subjectGenGrd = mystd::make_unique<SemanticGenericGrounding>();
      subjectGenGrd->referenceType = SemanticReferenceType::DEFINITE;
      subjectGenGrd->entityType = SemanticEntityType::THING;
      subjectGenGrd->concepts.emplace("value", 4);
      return subjectGenGrd;
    }());

    // subject owner
    subjectGrdExp->children.emplace(GrammaticalType::OWNER,
                                    mystd::make_unique<GroundedExpression>
                                    (mystd::make_unique<SemanticTextGrounding>(pEventName)));
    return subjectGrdExp;
  }());

  // object
  grdExp->children.emplace(GrammaticalType::OBJECT,
                           mystd::make_unique<GroundedExpression>
                           (mystd::make_unique<SemanticTextGrounding>(pEventValue)));

  return std::move(grdExp);
}


std::unique_ptr<SemanticExpression> sayThatOpNotifyInformedMeThat()
{
  // verb
  auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
  statementGrd->verbTense = SemanticVerbTense::PUNCTUALPAST;
  statementGrd->concepts.emplace("verb_action_notify", 4);
  auto grdExp =
      mystd::make_unique<GroundedExpression>(std::move(statementGrd));

  // subject
  grdExp->children.emplace(GrammaticalType::SUBJECT,
                           mystd::make_unique<GroundedExpression>
                           (mystd::make_unique<SemanticTextGrounding>("operator_inform")));

  // receiver
  grdExp->children.emplace(GrammaticalType::RECEIVER, _meSemExp());

  // object
  grdExp->children.emplace(GrammaticalType::OBJECT,
                           mystd::make_unique<GroundedExpression>
                           (SemanticGenericGrounding::makeThingThatHasToBeCompletedFromContext()));
  return std::move(grdExp);
}



UniqueSemanticExpression sayThanksThatsCool(
    const SemanticAgentGrounding& pSubjectGrounding)
{
  // fill verb
  auto res = _statGrdExp(ConceptSet::conceptVerbEquality);

  // fill subject
  res->children.emplace(GrammaticalType::SUBJECT,
                        mystd::make_unique<GroundedExpression>
                        (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));

  // fill object
  {
    auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
    genGrd->concepts.emplace("sentiment_positive_kind", 4);
    res->children.emplace(GrammaticalType::OBJECT,
                          mystd::make_unique<GroundedExpression>(std::move(genGrd)));
  }

  return mystd::make_unique<FeedbackExpression>(sayThanks(), std::move(res));
}


UniqueSemanticExpression sayIAmHappyToHearThat()
{
  // fill verb
  auto res = _statGrdExp(ConceptSet::conceptVerbEquality);

  // fill subject
  res->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

  // fill object
  res->children.emplace(GrammaticalType::OBJECT,
  [] {
    auto objGenGrd = mystd::make_unique<SemanticGenericGrounding>();
    objGenGrd->word = SemanticWord(SemanticLanguageEnum::ENGLISH, "happy", PartOfSpeech::ADJECTIVE);
    auto objGrdExp = mystd::make_unique<GroundedExpression>(std::move(objGenGrd));
    objGrdExp->children.emplace(GrammaticalType::SPECIFIER,
                          []
    {
      auto subStatementGrd = mystd::make_unique<SemanticStatementGrounding>();
      subStatementGrd->concepts.emplace("perception_hear", 4);
      auto subRes = mystd::make_unique<GroundedExpression>(std::move(subStatementGrd));
      subRes->children.emplace(GrammaticalType::OBJECT, sayThat());
      return subRes;
    }());
    return objGrdExp;
  }());

  return std::move(res);
}


UniqueSemanticExpression itsMe()
{
  auto rootGrdExp = _statGrdExp(ConceptSet::conceptVerbEquality);
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, sayThat());
  rootGrdExp->children.emplace(GrammaticalType::OBJECT, _meSemExp());
  return std::move(rootGrdExp);
}


UniqueSemanticExpression itIsNotKind()
{
  // fill verb
  auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
  statementGrd->verbTense = SemanticVerbTense::PRESENT;
  statementGrd->concepts.emplace(ConceptSet::conceptVerbEquality, 4);
  statementGrd->polarity = false;
  auto res = mystd::make_unique<GroundedExpression>(std::move(statementGrd));

  // fill subject
  res->children.emplace(GrammaticalType::SUBJECT, sayThat());

  // fill object
  res->children.emplace(GrammaticalType::OBJECT, mystd::make_unique<GroundedExpression>
                        (mystd::make_unique<SemanticConceptualGrounding>("sentiment_positive_kind")));
  return std::move(res);
}


UniqueSemanticExpression itIsABadNews()
{
  // fill verb
  auto res = _statGrdExp(ConceptSet::conceptVerbEquality);

  // fill subject
  res->children.emplace(GrammaticalType::SUBJECT, sayThat());

  // fill object
  auto grdExpObject = mystd::make_unique<GroundedExpression>([]()
  {
    auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
    genGrd->referenceType = SemanticReferenceType::INDEFINITE;
    genGrd->concepts.emplace("news", 4);
    genGrd->entityType = SemanticEntityType::THING;
    return genGrd;
  }());
  grdExpObject->children.emplace(GrammaticalType::SPECIFIER, mystd::make_unique<GroundedExpression>
                                 (mystd::make_unique<SemanticConceptualGrounding>("sentiment_negative_bad")));
  res->children.emplace(GrammaticalType::OBJECT, std::move(grdExpObject));

  return std::move(res);
}


UniqueSemanticExpression niceYouLikeIt(UniqueSemanticExpression pNiceSemExp,
                                       const SemanticAgentGrounding& pSubjectGrounding)
{
  auto res = _statGrdExp("verb_like");
  res->children.emplace(GrammaticalType::SUBJECT,
                        mystd::make_unique<GroundedExpression>
                        (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));
  res->children.emplace(GrammaticalType::OBJECT, sayThat());
  return mystd::make_unique<FeedbackExpression>(std::move(pNiceSemExp), std::move(res));
}


UniqueSemanticExpression sorryIWillTryToImproveMyself()
{
  auto res = mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::FUTURE;
    statementGrd->concepts.emplace("verb_action_try", 4);
    return statementGrd;
  }());
  res->children.emplace(GrammaticalType::SUBJECT, _meSemExp());
  res->children.emplace(GrammaticalType::OBJECT, []
  {
    auto subRes = mystd::make_unique<GroundedExpression>
        ([]()
    {
      auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
      statementGrd->verbTense = SemanticVerbTense::UNKNOWN;
      statementGrd->concepts.emplace("verb_action_improve", 4);
      return statementGrd;
    }());
    subRes->children.emplace(GrammaticalType::OBJECT, _meSemExp());
    return subRes;
  }());
  return mystd::make_unique<FeedbackExpression>(_saySorry(), std::move(res));
}


UniqueSemanticExpression iAmSorryToHearThat()
{
  // fill verb
  auto res = _statGrdExp(ConceptSet::conceptVerbEquality);

  // fill subject
  res->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

  // fill object
  auto grdExpObject = _saySorry();
  auto hearStatement = mystd::make_unique<SemanticStatementGrounding>();
  hearStatement->concepts.emplace("perception_hear", 4);
  auto grdExpHear = mystd::make_unique<GroundedExpression>(std::move(hearStatement));
  grdExpHear->children.emplace(GrammaticalType::OBJECT, sayThat());
  grdExpObject->children.emplace(GrammaticalType::SPECIFIER, std::move(grdExpHear));
  res->children.emplace(GrammaticalType::OBJECT, std::move(grdExpObject));

  return std::move(res);
}


UniqueSemanticExpression getWhoIsSomebodyQuestion(
    const GroundedExpression& pPersonToAsk)
{
  // verb
  auto rootGrdExp = _whatIsGrdExp();

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT, pPersonToAsk.clone());

  // object
  SemExpModifier::addNewChildWithConcept(rootGrdExp->children, GrammaticalType::OBJECT, "agent");
  return std::move(rootGrdExp);
}



UniqueSemanticExpression generateNumberOfTimesAnswer(
    const GroundedExpression& pGrdExpQuestion,
    std::size_t pNbOfTimes)
{
  auto rootGrdExp = pGrdExpQuestion.clone();
  SemanticStatementGrounding* statGr = (*rootGrdExp)->getStatementGroundingPtr();
  if (statGr != nullptr)
    statGr->requests.clear();
  rootGrdExp->children.emplace(GrammaticalType::REPETITION,
                               mystd::make_unique<GroundedExpression>
                               ([pNbOfTimes]()
  {
    auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
    genGrd->concepts.emplace("times", 4);
    genGrd->quantity.setNumber(static_cast<int>(pNbOfTimes));
    genGrd->entityType = SemanticEntityType::THING;
    return genGrd;
  }()));

  return std::move(rootGrdExp);
}


std::unique_ptr<GroundedExpression> _invertSubjectAndObject(
    const GroundedExpression& pGrdExp)
{
  auto res = mystd::make_unique<GroundedExpression>(pGrdExp.cloneGrounding());
  for (const auto& currChild : pGrdExp.children)
    res->children.emplace(SemExpGetter::invertGrammaticalType(currChild.first), currChild.second->clone());
  return res;
}


mystd::unique_propagate_const<UniqueSemanticExpression> generateAnswer(
    std::map<SemanticRequestType, AllAnswerElts>& pAllAnswers,
    std::list<std::string>& pReferences,
    const GroundedExpression& pGrdExpQuestion,
    const SemanticRequests& pRequests,
    const SemanticMemoryBlock& pMemBlock,
    const linguistics::LinguisticDatabase& pLingDb)
{
  std::unique_ptr<GroundedExpression> invertedGrdExpQuestion =
      [&pGrdExpQuestion]
  {
    if (SemExpGetter::getMainRequestTypeFromGrdExp(pGrdExpQuestion) == SemanticRequestType::SUBJECT &&
        ConceptSet::haveAConcept(pGrdExpQuestion->concepts, ConceptSet::conceptVerbEquality))
      return _invertSubjectAndObject(pGrdExpQuestion);
    return std::unique_ptr<GroundedExpression>();
  }();
  const GroundedExpression& grdExpQuestion = invertedGrdExpQuestion ?
        *invertedGrdExpQuestion : pGrdExpQuestion;

  SemanticRequestType request = pRequests.firstOrNothing();

  if (pAllAnswers.empty())
    return mystd::unique_propagate_const<UniqueSemanticExpression>();

  std::map<SemanticRequestType, UniqueSemanticExpression> requestToAnswers;
  mystd::unique_propagate_const<UniqueSemanticExpression> res;
  for (auto& currAnsw : pAllAnswers)
  {
    AllAnswerElts& answElts = currAnsw.second;
    auto answerSemExpOpt = _copyListOfGrdExps(answElts.answersFromMemory);
    if (!answerSemExpOpt &&
        !answElts.answersGenerated.empty())
      answerSemExpOpt = answElts.answersGenerated.front().genSemExp->clone();

    if (!answerSemExpOpt)
    {
      if (request == SemanticRequestType::TIMES)
      {
        res.emplace(generateNumberOfTimesAnswer(grdExpQuestion, 0));
        answElts.getReferences(pReferences);
      }
    }
    else
    {
      requestToAnswers.emplace(currAnsw.first, std::move(*answerSemExpOpt));
    }
  }

  if (!res)
  {
    if (request == SemanticRequestType::YESORNO)
    {
      auto& answElts = pAllAnswers.begin()->second;
      if (!answElts.answersGenerated.empty())
      {
        AnswerExpGenerated& firstAnswerExp = answElts.answersGenerated.front();
        res.emplace(std::move(firstAnswerExp.genSemExp));
        answElts.getReferences(pReferences);
      }
    }
    else if (request == SemanticRequestType::TIMES)
    {
      auto& answElts = pAllAnswers.begin()->second;
      res.emplace(generateNumberOfTimesAnswer(grdExpQuestion, answElts.answersFromMemory.size()));
      answElts.getReferences(pReferences);
    }
    else if (request == SemanticRequestType::VERB)
    {
      auto it = requestToAnswers.find(request);
      if (it != requestToAnswers.end())
      {
        res = std::move(it->second);
        auto& answElts = pAllAnswers.begin()->second;
        answElts.getReferences(pReferences);
      }
    }
    else if ((request == SemanticRequestType::SUBJECT && !SemExpGetter::isPassive(grdExpQuestion)) ||
             (request == SemanticRequestType::OBJECT && SemExpGetter::isPassive(grdExpQuestion)))
    {
      bool resEmplaced = false;
      // For the cases that the verb is "be" and the subject has at least one child,
      // we produce a sentence instead of just the raw answer
      if (ConceptSet::haveAConcept(grdExpQuestion->concepts, ConceptSet::conceptVerbEquality))
      {
        auto itSubject = grdExpQuestion.children.find(GrammaticalType::SUBJECT);
        if (itSubject != grdExpQuestion.children.end() &&
            SemExpGetter::isDefinite(*itSubject->second))
        {
          auto itSubject = requestToAnswers.find(SemanticRequestType::SUBJECT);
          if (itSubject != requestToAnswers.end())
          {
            requestToAnswers.emplace(SemanticRequestType::OBJECT, std::move(itSubject->second));
            requestToAnswers.erase(itSubject);
          }
          res.emplace(_reformulateQuestionWithListOfAnswers(requestToAnswers, grdExpQuestion,
                                                            pMemBlock, pLingDb));
          resEmplaced = true;
        }
      }
      if (!resEmplaced)
      {
        auto nbOfReqToAswers = requestToAnswers.size();
        if (nbOfReqToAswers == 1)
        {
          res = std::move(requestToAnswers.begin()->second);
        }
        else if (nbOfReqToAswers > 1)
        {
          auto listExp = mystd::make_unique<ListExpression>();
          for (auto& currAnswer : requestToAnswers)
            listExp->elts.emplace_back(std::move(currAnswer.second));
          res = std::move(listExp);
        }
      }
      for (const auto& currAnswer : pAllAnswers)
        currAnswer.second.getReferences(pReferences);
    }
    else
    {
      res.emplace(_reformulateQuestionWithListOfAnswers(requestToAnswers, grdExpQuestion, pMemBlock, pLingDb));
      for (const auto& currAnswer : pAllAnswers)
        currAnswer.second.getReferences(pReferences);
    }
  }

  return res;
}



UniqueSemanticExpression sayThatWeDontKnowTheAnswer(
    const SemanticExpression& pSemExp)
{
  return _sayThatWeKnow(pSemExp, false);
}


UniqueSemanticExpression sayThatWeDontKnowAnInstanceOf(
    const SemanticExpression& pSemExp)
{
  auto grdExpPtr = _sayThatWeKnow(pSemExp);
  SemExpModifier::invertPolarityFromGrdExp(*grdExpPtr);
  return std::move(grdExpPtr);
}


UniqueSemanticExpression sayThatTheRobotCannotDoIt(
    const SemanticExpression& pSemExp)
{
  auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
  statementGrd->verbTense = SemanticVerbTense::PRESENT;
  statementGrd->polarity = false;
  statementGrd->concepts.emplace("verb_able", 4);
  auto rootGrdExp =
      mystd::make_unique<GroundedExpression>(std::move(statementGrd));

  SemExpModifier::addChild(*rootGrdExp, GrammaticalType::SUBJECT, _meSemExp());

  auto newSpecifSemExp = _copyAndReformateSemExpToPutItInAnAnswer(pSemExp);
  GroundedExpression* specGrdExp = newSpecifSemExp->getGrdExpPtr_SkipWrapperPtrs();
  if (specGrdExp != nullptr)
  {
    SemanticStatementGrounding* statGrd = (*specGrdExp)->getStatementGroundingPtr();
    if (statGrd != nullptr)
    {
      statGrd->verbGoal = VerbGoalEnum::NOTIFICATION;
      statGrd->verbTense = SemanticVerbTense::UNKNOWN;
      statGrd->requests.clear();
    }
    auto itSubjectSpec = specGrdExp->children.find(GrammaticalType::SUBJECT);
    if (itSubjectSpec != specGrdExp->children.end())
      specGrdExp->children.erase(itSubjectSpec);
  }
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               std::move(newSpecifSemExp));
  return std::move(rootGrdExp);
}


UniqueSemanticExpression sayThatTheRobotIsNotAbleToDoIt(
    const GroundedExpression& pGrdExp)
{
  auto grdExpPtr = pGrdExp.clone();
  auto* statGrdPtr = grdExpPtr->grounding().getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
  {
    auto& statGrd = *statGrdPtr;
    statGrd.verbTense = SemanticVerbTense::PRESENT;
    statGrd.polarity = false;
    statGrd.verbGoal = VerbGoalEnum::ABILITY;
    statGrd.requests.clear();
  }

  grdExpPtr->children.emplace(GrammaticalType::MITIGATION,
                              []() {
    auto mtgGrdExp = mystd::make_unique<GroundedExpression>
        ([](){
      auto knowStatGrd = mystd::make_unique<SemanticStatementGrounding>();
      knowStatGrd->verbTense = SemanticVerbTense::PRESENT;
      knowStatGrd->concepts.emplace("mentalState_know", 4);
      return knowStatGrd;
    }());

    SemExpModifier::addChild(*mtgGrdExp, GrammaticalType::SUBJECT, _meSemExp());

    mtgGrdExp->children.emplace(GrammaticalType::OBJECT,
                                 mystd::make_unique<GroundedExpression>
                                 ([]()
    {
      auto doStGr = mystd::make_unique<SemanticStatementGrounding>();
      doStGr->verbTense = SemanticVerbTense::UNKNOWN;
      doStGr->requests.add(SemanticRequestType::MANNER);
      doStGr->concepts.emplace("verb_action", 4);
      return doStGr;
    }()));

    return mtgGrdExp;
  }());

  return std::move(grdExpPtr);
}


std::unique_ptr<GroundedExpression> doYouWantMeToSayToTellYouHowTo(const SemanticAgentGrounding& pSubjectGrounding,
                                                                   const GroundedExpression& pGrdExp)
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>([]()
  {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace("verb_want", 4);
    statementGrd->requests.set(SemanticRequestType::YESORNO);
    return statementGrd;
  }());

  // subject
  rootGrdExp->children.emplace(GrammaticalType::SUBJECT,
                               mystd::make_unique<GroundedExpression>
                               (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));

  // object
  rootGrdExp->children.emplace(GrammaticalType::OBJECT,
                               [&] {
    // verb
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PUNCTUALPRESENT;
    statementGrd->concepts.emplace("verb_action_say", 4);
    auto childGrdExp =
        mystd::make_unique<GroundedExpression>(std::move(statementGrd));

    // subject
    childGrdExp->children.emplace(GrammaticalType::SUBJECT, _meSemExp());

    // receiver
    childGrdExp->children.emplace(GrammaticalType::RECEIVER,
                                  mystd::make_unique<GroundedExpression>
                                  (mystd::make_unique<SemanticAgentGrounding>(pSubjectGrounding)));

    // object
    childGrdExp->children.emplace(GrammaticalType::OBJECT, [&]() {
      auto grdExpPtr = pGrdExp.clone();
      auto* statGrdPtr = grdExpPtr->grounding().getStatementGroundingPtr();
      if (statGrdPtr != nullptr)
      {
        auto& statGrd = *statGrdPtr;
        statGrd.verbTense = SemanticVerbTense::UNKNOWN;
        statGrd.requests.clear();
        statGrd.requests.add(SemanticRequestType::MANNER);
      }

      grdExpPtr->children.erase(GrammaticalType::SUBJECT);

      return grdExpPtr;
    }());

    return childGrdExp;
  }());
  return rootGrdExp;
}

UniqueSemanticExpression askForPrecision(
    const GroundedExpression& pGrdExp,
    SemanticRequestType pRequestType)
{
  auto rootGrdExp = pGrdExp.clone();
  SemExpModifier::clearRequestList(*rootGrdExp);

  if (pRequestType == SemanticRequestType::OBJECT)
  {
    SemanticStatementGrounding* actionStruct = (*rootGrdExp)->getStatementGroundingPtr();
    if (actionStruct != nullptr)
      actionStruct->verbGoal = VerbGoalEnum::MANDATORY;
  }
  SemExpModifier::addRequest(*rootGrdExp, pRequestType);
  return std::move(rootGrdExp);
}


UniqueSemanticExpression sayWeWillDoIt_fromGrdExp(
    const GroundedExpression& pGrdExp)
{
  auto rootGrdExp = pGrdExp.clone();
  SemExpModifier::clearRequestList(*rootGrdExp);
  SemExpModifier::modifyVerbTense(*rootGrdExp, SemanticVerbTense::FUTURE);
  return mystd::make_unique<FeedbackExpression>(sayOk(), std::move(rootGrdExp));
}


UniqueSemanticExpression sayWeWillDoIt(
    const ConditionSpecification& pCondSpec)
{
  auto condExp = pCondSpec.convertToConditionExpression();
  ConditionExpression& rootCondExp = *condExp;

  SemExpModifier::clearRequestListOfSemExp(*rootCondExp.thenExp);
  SemExpModifier::modifyVerbTenseOfSemExp(*rootCondExp.thenExp,
                                          SemanticVerbTense::FUTURE);
  if (rootCondExp.elseExp)
  {
    SemExpModifier::clearRequestListOfSemExp(**rootCondExp.elseExp);
    SemExpModifier::modifyVerbTenseOfSemExp(**rootCondExp.elseExp,
                                            SemanticVerbTense::FUTURE);
  }
  return mystd::make_unique<FeedbackExpression>(sayOk(), std::move(condExp));
}


UniqueSemanticExpression confirmInformation(
    const GroundedExpression& pGrdExp)
{
  auto rootGrdExp = pGrdExp.clone();
  SemExpModifier::clearRequestList(*rootGrdExp);
  return mystd::make_unique<FeedbackExpression>(sayOk(), std::move(rootGrdExp));
}


UniqueSemanticExpression confirmInfoCondition(
    const ConditionSpecification& pCondSpec)
{
  auto condExp = pCondSpec.convertToConditionExpression();
  ConditionExpression& rootCondExp = *condExp;
  SemExpModifier::clearRequestListOfSemExp(*rootCondExp.thenExp);
  if (rootCondExp.elseExp)
    SemExpModifier::clearRequestListOfSemExp(**rootCondExp.elseExp);
  return mystd::make_unique<FeedbackExpression>(sayOk(), std::move(condExp));
}


UniqueSemanticExpression sayAlso()
{
  return mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticConceptualGrounding>("also"));
}

UniqueSemanticExpression sayThanks()
{
  return mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticConceptualGrounding>("sentiment_positive_thanks"));
}

UniqueSemanticExpression sayOk()
{
  return mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto okGrounding = mystd::make_unique<SemanticGenericGrounding>();
    okGrounding->word.language = SemanticLanguageEnum::ENGLISH;
    okGrounding->word.lemma = "ok";
    okGrounding->word.partOfSpeech = PartOfSpeech::ADVERB; // TODO: check that this meaning exists
    return okGrounding;
  }());
}

UniqueSemanticExpression sayTrue()
{
  auto trueGrounding = mystd::make_unique<SemanticGenericGrounding>();
  trueGrounding->word.language = SemanticLanguageEnum::ENGLISH;
  trueGrounding->word.lemma = "true";
  trueGrounding->word.partOfSpeech = PartOfSpeech::ADVERB; // TODO: check that this meaning exists
  trueGrounding->concepts.emplace("accordance_agreement_true", 4);
  return mystd::make_unique<GroundedExpression>(std::move(trueGrounding));
}

UniqueSemanticExpression sayFalse()
{
  auto falseGrounding = mystd::make_unique<SemanticGenericGrounding>();
  falseGrounding->word.language = SemanticLanguageEnum::ENGLISH;
  falseGrounding->word.lemma = "false";
  falseGrounding->word.partOfSpeech = PartOfSpeech::ADVERB; // TODO: check that this meaning exists
  falseGrounding->concepts.emplace("accordance_disagreement_false", 4);
  return mystd::make_unique<GroundedExpression>(std::move(falseGrounding));
}


UniqueSemanticExpression generateYesOrNo(bool pSamePolarity)
{
  return mystd::make_unique<GroundedExpression>([&pSamePolarity]{
    auto yesGenGr = mystd::make_unique<SemanticGenericGrounding>();
    yesGenGr->concepts.emplace(pSamePolarity ? "accordance_agreement_yes" : "accordance_disagreement_no", 4);
    return yesGenGr;
  }());
}


UniqueSemanticExpression generateYesOrNoAnswer(UniqueSemanticExpression pSemExp,
                                               bool pSamePolarity)
{
  return mystd::make_unique<FeedbackExpression>(generateYesOrNo(pSamePolarity), std::move(pSemExp));
}


UniqueSemanticExpression generateYesOrNoAnswerFromQuestion(const GroundedExpression& pGrdExpQuestion,
                                                           bool pSamePolarity,
                                                           const std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer)
{
  auto rootGrdExp = pGrdExpQuestion.clone();
  SemanticStatementGrounding* statGr = (*rootGrdExp)->getStatementGroundingPtr();
  if (statGr != nullptr)
  {
    statGr->requests.clear();
    if (!pSamePolarity)
      SemExpModifier::invertPolarityFromGrdExp(*rootGrdExp);
  }

  auto answerSemExp = generateYesOrNoAnswer(std::move(rootGrdExp), pSamePolarity);
  if (!pAnnotationsOfTheAnswer.empty())
    answerSemExp = _addAnnotationsToSemExp(std::move(answerSemExp),
                                           std::unique_ptr<AnnotatedExpression>(),
                                           pAnnotationsOfTheAnswer);
  return answerSemExp;
}


UniqueSemanticExpression generateYesOrNoAnswerFromMemory(
    UniqueSemanticExpression pSemExp,
    bool pSamePolarity,
    const std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer)
{
  SemExpModifier::clearRequestListOfSemExp(*pSemExp);
  auto answerSemExp = generateYesOrNoAnswer(std::move(pSemExp), pSamePolarity);
  if (!pAnnotationsOfTheAnswer.empty())
    answerSemExp = _addAnnotationsToSemExp(std::move(answerSemExp),
                                           std::unique_ptr<AnnotatedExpression>(),
                                           pAnnotationsOfTheAnswer);
  return answerSemExp;
}



} // End of namespace SemExpCreator
} // End of namespace onsem
