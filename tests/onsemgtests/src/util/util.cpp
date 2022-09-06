#include "util.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/semanticbehaviordefinition.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentenceid.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/serialization.hpp>
#include <onsem/semantictotext/executor/textexecutor.hpp>
#include <onsem/tester/resourcelabelfortests.hpp>


namespace onsem
{

UniqueSemanticExpression textToSemExp
(const std::string& pText,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage)
{
  return converter::textToSemExp(pText,
                                 TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                       SemanticAgentGrounding::me,
                                                       pLanguage),
                                 pLingDb);
}


UniqueSemanticExpression textToSemExpFromRobot
(const std::string& pText,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage)
{
  return converter::textToSemExp(pText,
                                 TextProcessingContext(SemanticAgentGrounding::me,
                                                       SemanticAgentGrounding::currentUser,
                                                       pLanguage),
                                 pLingDb);
}


UniqueSemanticExpression textToContextualSemExp
(const std::string& pText,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage)
{
  return converter::textToContextualSemExp(pText,
                                           TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                                 SemanticAgentGrounding::me,
                                                                 pLanguage),
                                           SemanticSourceEnum::ASR, pLingDb);
}

std::string semExpToText
(UniqueSemanticExpression pSemExp,
 SemanticLanguageEnum pLanguage,
 const SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb)
{
  std::string res;
  converter::semExpToText(res, std::move(pSemExp),
                          TextProcessingContext(SemanticAgentGrounding::me,
                                                SemanticAgentGrounding::currentUser,
                                                pLanguage),
                          false, pSemanticMemory, pLingDb, nullptr);
  return res;
}


std::string semExpToTextExectionResult
(UniqueSemanticExpression pSemExp,
 SemanticLanguageEnum pLanguage,
 SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb)
{
  std::string resStr;
  DefaultExecutorLogger logger(resStr);
  TextExecutor textExec(pSemanticMemory, pLingDb, logger);
  TextProcessingContext outContext(SemanticAgentGrounding::me,
                                   SemanticAgentGrounding::currentUser,
                                   pLanguage);
  outContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{resourceLabelForTests_cmd, resourceLabelForTests_url});
  auto execContext = std::make_shared<ExecutorContext>(outContext);
  textExec.runSemExp(std::move(pSemExp), execContext);
  return resStr;
}


std::string reformulate
(const std::string& pText,
 const SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage)
{
  auto semExp = textToSemExp(pText, pLingDb, pLanguage);
  return semExpToText(std::move(semExp), pLanguage, pSemanticMemory, pLingDb);
}



void compareWithRef(const std::string& pExpectedAnswerStr,
                    const std::list<std::string>& pReferences,
                    const DetailedReactionAnswer& pResult)
{
  EXPECT_EQ(pExpectedAnswerStr, pResult.answer);
  EXPECT_EQ(pReferences, pResult.references);
}

void copyMemory(SemanticMemory& pNewSemMem,
                const SemanticMemory& pNewSemMemToCopy,
                const linguistics::LinguisticDatabase& pLingDb)
{
  boost::property_tree::ptree propTree;
  serialization::saveSemMemory(propTree, pNewSemMemToCopy);
  serialization::loadSemMemory(propTree, pNewSemMem, pLingDb);
}


UniqueSemanticExpression invertPolarity(UniqueSemanticExpression pUSemExp)
{
  SemExpModifier::invertPolarity(*pUSemExp);
  return pUSemExp;
}


UniqueSemanticExpression removeChild(UniqueSemanticExpression pUSemExp,
                                     GrammaticalType pChildType)
{
  auto* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    SemExpModifier::removeChild(*grdExpPtr, pChildType);
  return pUSemExp;
}



TrackSemMemoryNotifications::TrackSemMemoryNotifications(SemanticMemory& pSemMem,
                                                           const linguistics::LinguisticDatabase& pLingDb)
  : _semMem(pSemMem),
    _lingDb(pLingDb),
    _infActionChangedConnection(),
    _infActions(),
    _conditionToActionChangedConnection(),
    _conditionToActions()
{
  _infActionChangedConnection =
      _semMem.memBloc.infActionChanged.connectUnsafe([this](const std::map<intSemId, const SemanticMemorySentence*>& pInfActions)
  {
    std::list<SemanticBehaviorDefinition> actionDefinitions;
    SemanticMemoryBlock::extractActions(actionDefinitions, pInfActions);

    std::set<std::string> actionDefinitionsStr;
    for (const auto& currAction : actionDefinitions)
    {
      auto actionSemExp = [&]() -> UniqueSemanticExpression
      {
        if (currAction.composition->isEmpty())
          return currAction.label->clone();
        auto rootGrdExp = mystd::make_unique<GroundedExpression>
            ([]()
        {
          auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
          statementGrd->verbTense = SemanticVerbTense::PRESENT;
          statementGrd->concepts["verb_equal_mean"] = 4;
          return statementGrd;
        }());
        rootGrdExp->children.emplace(GrammaticalType::SUBJECT, currAction.label->clone());
        rootGrdExp->children.emplace(GrammaticalType::OBJECT, currAction.composition->clone());
        return std::move(rootGrdExp);
      }();
      actionDefinitionsStr.insert(
            semExpToText(std::move(actionSemExp), _semMem.defaultLanguage,
                         _semMem, _lingDb));
    }

    _infActions.clear();
    bool firstIteration = true;
    for (const auto& currActionStr : actionDefinitionsStr)
    {
      if (!firstIteration)
        _infActions += "\n";
      else
        firstIteration = false;
      _infActions += currActionStr;
    }
  });

  _conditionToActionChangedConnection =
      _semMem.memBloc.conditionToActionChanged.connectUnsafe([this](const std::set<const SemanticMemorySentence*>& pCondToActions)
  {
    std::list<UniqueSemanticExpression> condToActionsSemExp;
    SemanticMemoryBlock::extractConditionToActions(condToActionsSemExp, pCondToActions);

    std::set<std::string> condToActionsStr;
    for (const auto& currCondToActionsSemExp : condToActionsSemExp)
      condToActionsStr.insert(
            semExpToText(currCondToActionsSemExp->clone(), _semMem.defaultLanguage,
                         _semMem, _lingDb));

    _conditionToActions.clear();
    bool firstIteration = true;
    for (const auto& currCondToActionsStr : condToActionsStr)
    {
      if (!firstIteration)
        _conditionToActions += "\n";
      else
        firstIteration = false;
      _conditionToActions += currCondToActionsStr;
    }
  });
}


TrackSemMemoryNotifications::~TrackSemMemoryNotifications()
{
  _semMem.memBloc.infActionChanged.disconnectUnsafe(_infActionChangedConnection);
  _semMem.memBloc.conditionToActionChanged.disconnectUnsafe(_conditionToActionChangedConnection);
}



} // End of namespace onsem
