#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/type/reactionoptions.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "operators/operator_inform.hpp"
#include "triggers/triggers_add.hpp"
#include "triggers/triggers_match.hpp"


using namespace onsem;


namespace
{
const std::string _resourceLabel = "resLabel";

UniqueSemanticExpression _idToSemExp(const std::string& pId,
                                     SemanticLanguageEnum pLanguage)
{
  auto res = std::make_unique<MetadataExpression>(
        std::make_unique<GroundedExpression>(
          std::make_unique<SemanticResourceGrounding>(_resourceLabel, pLanguage, pId)));
  res->references.push_back(pId);
  return std::move(res);
}

std::shared_ptr<ExpressionWithLinks> _inform(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::list<std::string>& pReferences)
{
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  SemanticLanguageEnum::UNKNOWN);
  inContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{"hc_resource_html"});

  auto agentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>();
  return operator_inform_withTextProc(pText, pSemanticMemory, pLingDb, pReferences,
                                      nullptr, inContext);
}

}


TEST_F(SemanticReasonerGTests, test_bigMemory)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  auto language = SemanticLanguageEnum::FRENCH;

  const std::string textFilename = corpusPath + "/triggerAndTexts/triggersAndTextsCorpus.txt";
  std::ifstream triggersAndTextsCorpusFile(textFilename, std::ifstream::in);
  if (!triggersAndTextsCorpusFile.is_open())
    throw std::runtime_error("Can't open " + textFilename + " file !");

  std::map<std::string, std::string> triggersToReferenceOfAnswer;

  std::string currentLabel;
  std::string currentId;
  std::string currentText;

  auto fillCurrentInfos = [&]() {
    if (currentLabel == "trigger")
    {
      triggers_addToSemExpAnswer(currentText, _idToSemExp(currentId, language), semMem, lingDb, language);
      triggersToReferenceOfAnswer.emplace(currentText, currentId);
    }
    if (currentLabel == "inform" || currentLabel == "message")
      _inform(currentText, semMem, lingDb, {currentId});
  };

  std::string line;
  while (getline(triggersAndTextsCorpusFile, line))
  {
    if (line.empty())
      continue;
    if (line[0] == '#')
    {
      auto endOfLabel = line.find_first_of('#', 1);
      if (endOfLabel == std::string::npos)
      {
        std::cerr << "End of label not found for line: " << line << std::endl;
        continue;
      }

      auto label = line.substr(1, endOfLabel - 1);
      if (label == "trigger" || label == "inform" || label == "message")
      {
        auto beginOfId = endOfLabel + 1;
        auto endOfId = line.find_first_of('#', beginOfId);
        if (endOfId == std::string::npos)
        {
          std::cerr << "End of id not found for line: " << line << std::endl;
          continue;
        }
        auto id = line.substr(beginOfId, endOfId - beginOfId);

        fillCurrentInfos();

        currentLabel = label;
        currentId = id;
        auto beginOfText = endOfId + 1;
        currentText = line.substr(beginOfText, line.size() - beginOfText);
        continue;
      }
    }

    currentText += "\n" + line;
  }
  triggersAndTextsCorpusFile.close();
  fillCurrentInfos();

  std::cout << "nbOfKnowledges: " << semMem.memBloc.nbOfKnowledges() << std::endl;

  ReactionOptions reactionOptionsForTriggerTests;
  reactionOptionsForTriggerTests.canAnswerWithAllTheTriggers = true;
  for (const auto& currTrigger : triggersToReferenceOfAnswer)
  {
    auto answer = triggers_match(currTrigger.first, semMem, lingDb, language, &reactionOptionsForTriggerTests);
    auto refIt = std::find(answer.references.begin(), answer.references.end(), currTrigger.second);
    if (refIt == answer.references.end())
    {
      std::string refStrs = "[";
      for (auto& currRef : answer.references)
      {
        if (refStrs != "[")
          refStrs += ", ";
        refStrs += currRef;
      }
      refStrs += "]";
      std::cerr << "Error in trigger: \"" << currTrigger.first << "\" expected ref: \"" << currTrigger.second << "\" get refs: " << refStrs
                << " asnwer: \"" << answer.answer << "\"" << std::endl;
      EXPECT_TRUE(false);
    }
  }

  ReactionOptions reactionOptions;
  reactionOptions.canReactToANoun = true;
  reactionOptions.canSayOkToAnAffirmation = false;
  reactionOptions.reactWithTextAndResource = true;
  ONSEM_ANSWER_EQ("\\resLabel=#fr_FR#c-est-quoi-les-limbes\\",
                  operator_react("C'est quoi les limbes ?", semMem, lingDb, language, &reactionOptions));
}

