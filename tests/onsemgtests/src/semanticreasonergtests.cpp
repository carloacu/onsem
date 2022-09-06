#include "semanticreasonergtests.hpp"
#include <iostream>
#include <memory>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/binary/binarysaver.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexploader.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexpsaver.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpsimplifer.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semanticdebugger/semanticdebug.hpp>
#include <onsem/semanticdebugger/diagnosisprinter.hpp>
#include <onsem/tester/syntacticanalysisxmlloader.hpp>
#include <onsem/tester/syntacticanalysisxmlsaver.hpp>
#include <onsem/tester/sentencesloader.hpp>
#include "util/util.hpp"

using namespace onsem;
std::string SemanticReasonerGTests::lingDbPath = "";
std::string SemanticReasonerGTests::dynamicdictionaryPath = "";
std::string SemanticReasonerGTests::corpusPath = "";
std::string SemanticReasonerGTests::scenariosPath = "";
std::unique_ptr<linguistics::LinguisticDatabase> SemanticReasonerGTests::lingDbPtr;
const TextProcessingContext SemanticReasonerGTests::fromRobot = TextProcessingContext::getTextProcessingContextFromRobot(SemanticLanguageEnum::UNKNOWN);
const TextProcessingContext SemanticReasonerGTests::fromUser = TextProcessingContext::getTextProcessingContextToRobot(SemanticLanguageEnum::UNKNOWN);
std::string SemanticReasonerGTests::_corpusInputFolder = "";
std::string SemanticReasonerGTests::_corpusResultsFolder = "";


void checkALanguage(const std::string& pLang,
                    const linguistics::LinguisticDatabase& pLingDb,
                    const std::string& pCorpusResultsFolder)
{
  SemanticTimeGrounding::setAnHardCodedTimeElts(true, true);
  std::shared_ptr<syntacticAnalysisXmlLoader::DeserializedTextResults> diffResults
      (std::make_shared<syntacticAnalysisXmlLoader::DeserializedTextResults>());
  diffResults->whatNeedToChecked.input_reformulation = false;

  // if we can't compare
  syntacticAnalysisXmlSaver::compareResults(diffResults, pLang, pLingDb, pCorpusResultsFolder);
  bool changesFound = !diffResults->diffsInputSentences.empty();
  std::cout << diffResults->bilan << std::endl;
  EXPECT_FALSE(changesFound);
}


TEST_F(SemanticReasonerGTests, CheckCommonResults)
{ 
  checkALanguage(commonStr, *lingDbPtr, _corpusResultsFolder);
}

TEST_F(SemanticReasonerGTests, CheckFrenchResults)
{
  checkALanguage(frenchStr, *lingDbPtr, _corpusResultsFolder);
}

TEST_F(SemanticReasonerGTests, CheckEnglishResults)
{
  checkALanguage(englishStr, *lingDbPtr, _corpusResultsFolder);
}

TEST_F(SemanticReasonerGTests, CheckJapaneseResults)
{
  checkALanguage(japaneseStr, *lingDbPtr, _corpusResultsFolder);
}




TEST_F(SemanticReasonerGTests, diagnosisMethod)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  std::string res = diagnosisPrinter::diagnosis({"loadedDatabases"},
                                                       SemanticMemory(),
                                                       lingDb);
  std::cout << res << std::endl;
  ASSERT_FALSE(res.empty());
}


TEST_F(SemanticReasonerGTests, languageDetection)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  ASSERT_EQ(SemanticLanguageEnum::FRENCH,
            linguistics::getLanguage("je suis Paul", lingDb));
  ASSERT_EQ(SemanticLanguageEnum::FRENCH,
            linguistics::getLanguage("tu es cool", lingDb));
  ASSERT_EQ(SemanticLanguageEnum::FRENCH,
            linguistics::getLanguage("Dis salut tout le monde", lingDb));
  ASSERT_EQ(SemanticLanguageEnum::ENGLISH,
            linguistics::getLanguage("I am Paul", lingDb));
  ASSERT_EQ(SemanticLanguageEnum::ENGLISH,
            linguistics::getLanguage("bye-bye", lingDb));
  ASSERT_EQ(SemanticLanguageEnum::ENGLISH,
            linguistics::getLanguage("February 11th", lingDb));
  ASSERT_EQ(SemanticLanguageEnum::UNKNOWN,
            linguistics::getLanguage("adeadeade", lingDb));
}



TEST_F(SemanticReasonerGTests, synthesizeManyCommands)
{
  UniqueSemanticExpression semExp = []
  {
    auto listExp = mystd::make_unique<ListExpression>();
    auto textOfCommand = mystd::make_unique<GroundedExpression>
        (mystd::make_unique<SemanticResourceGrounding>("aMethodCall",
                                                                    SemanticLanguageEnum::UNKNOWN,
                                                                    "Service.method()"));
    listExp->elts.emplace_back(textOfCommand->clone());
    listExp->elts.emplace_back(textOfCommand->clone());
    return listExp;
  }();
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory userMem;

  std::string res;
  converter::semExpToText(res, std::move(semExp),
                                 TextProcessingContext::getTextProcessingContextFromRobot(SemanticLanguageEnum::ENGLISH),
                                 false, userMem, lingDb, nullptr);
  ASSERT_EQ("Service.method() Service.method()", res);
}


bool _doesTokenListEqualsInitalText(std::string& pErrorMessage,
                                    const std::string& pLanguageStr,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    const std::string& pCorpusInputFolder)
{
  SentencesLoader sentencesXml;
  sentencesXml.loadFolder(pCorpusInputFolder + "/" + pLanguageStr);
  const std::vector<std::string>& sentences = sentencesXml.getSentences();
  SemanticLanguageEnum langEnum = semanticLanguageTypeGroundingEnumFromStr(pLanguageStr);
  const std::set<SpellingMistakeType> spellingMistakeTypesPossible;

  for (const std::string& sent : sentences)
  {
    linguistics::SyntacticGraph syntGraph(pLingDb, langEnum);
    std::shared_ptr<ResourceGroundingExtractor> cmdGrdExtractorPtr;
    linguistics::tokenizationAndSyntacticalAnalysis(syntGraph, sent, spellingMistakeTypesPossible,
                                                    cmdGrdExtractorPtr);
    const std::string restitutedText = syntGraph.tokensTree.getText();
    if (sent != restitutedText)
    {
      pErrorMessage = "bad text restitution from \n\"" + sent + "\" to\n\"" + restitutedText + "\"";
      return false;
    }
  }
  return true;
}


TEST_F(SemanticReasonerGTests, CheckThatTheTokenListEqualsInitialText)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  std::string errorMessage;
  ASSERT_TRUE(_doesTokenListEqualsInitalText(errorMessage, frenchStr, lingDb, _corpusInputFolder))
      << errorMessage;
  ASSERT_TRUE(_doesTokenListEqualsInitalText(errorMessage, englishStr, lingDb, _corpusInputFolder))
      << errorMessage;
  ASSERT_TRUE(_doesTokenListEqualsInitalText(errorMessage, japaneseStr, lingDb, _corpusInputFolder))
      << errorMessage;
}


TEST_F(SemanticReasonerGTests, CheckFillParameters)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  {
    auto expToFill = textToSemExp("\\p_number=0\\", lingDb);
    IndexToSubNameToParameterValue params;
    params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("13", lingDb)));
    UniqueSemanticExpression resExp = expToFill->clone(&params);
    EXPECT_EQ("13", semExpToText(std::move(resExp), SemanticLanguageEnum::UNKNOWN, semMem, lingDb));
  }

  {
    auto expToFill = textToSemExp("\\p_number=0\\ arms", lingDb);
    IndexToSubNameToParameterValue params;
    params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("28", lingDb)));
    UniqueSemanticExpression resExp = expToFill->clone(&params);
    EXPECT_EQ("28 arms", semExpToText(std::move(resExp), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  }

  {
    auto expToFill = textToSemExp("You are \\p_number=0\\", lingDb);
    IndexToSubNameToParameterValue params;
    params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("32", lingDb)));
    UniqueSemanticExpression resExp = expToFill->clone(&params);
    EXPECT_EQ("I am 32.", semExpToText(resExp->clone(), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
    EXPECT_EQ("J'ai 32 ans.", semExpToText(std::move(resExp), SemanticLanguageEnum::FRENCH, semMem, lingDb));
  }

  {
    auto expToFill = textToSemExp("I see \\p_number=0\\ humans", lingDb);
    IndexToSubNameToParameterValue params;
    params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("0", lingDb)));
    UniqueSemanticExpression resExp = expToFill->clone(&params);
    EXPECT_EQ("You don't see any human.", semExpToText(std::move(resExp), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
  }

  {
    auto expToFill = textToSemExp("I see \\p_number=0\\ other humans", lingDb);
    IndexToSubNameToParameterValue params;
    params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("0", lingDb)));
    UniqueSemanticExpression resExp = expToFill->clone(&params);
    EXPECT_EQ("You don't see any other human.", semExpToText(resExp->clone(), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
    EXPECT_EQ("Tu ne vois aucun autre humain.", semExpToText(std::move(resExp), SemanticLanguageEnum::FRENCH, semMem, lingDb));
  }

  {
    auto expToFill = textToSemExp("I see \\p_number=0\\ other humans", lingDb);
    IndexToSubNameToParameterValue params;
    params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("1", lingDb)));
    UniqueSemanticExpression resExp = expToFill->clone(&params);
    EXPECT_EQ("You see another human.", semExpToText(resExp->clone(), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
    EXPECT_EQ("Tu vois un autre humain.", semExpToText(std::move(resExp), SemanticLanguageEnum::FRENCH, semMem, lingDb));
  }

  {
    auto expToFill = textToSemExp("I see \\p_number=0\\ other humans", lingDb);
    IndexToSubNameToParameterValue params;
    params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("2", lingDb)));
    UniqueSemanticExpression resExp = expToFill->clone(&params);
    EXPECT_EQ("You see 2 other humans.", semExpToText(resExp->clone(), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
    EXPECT_EQ("Tu vois 2 autres humains.", semExpToText(std::move(resExp), SemanticLanguageEnum::FRENCH, semMem, lingDb));
  }

  {
    auto expToFill = textToSemExp("If \\p_number=0\\ is equal to 1 then I see you else I don't see you.", lingDb);
    {
      IndexToSubNameToParameterValue params;
      params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("1", lingDb)));
      UniqueSemanticExpression resExp = expToFill->clone(&params);
      simplifier::solveConditionsInplace(resExp, semMem.memBloc, lingDb);
      EXPECT_EQ("You see me.", semExpToText(std::move(resExp), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
    }
    {
      IndexToSubNameToParameterValue params;
      params[0].emplace("", mystd::make_unique<UniqueSemanticExpression>(textToSemExp("0", lingDb)));
      UniqueSemanticExpression resExp = expToFill->clone(&params);
      simplifier::solveConditionsInplace(resExp, semMem.memBloc, lingDb);
      EXPECT_EQ("You don't see me.", semExpToText(std::move(resExp), SemanticLanguageEnum::ENGLISH, semMem, lingDb));
    }
  }
}


TEST_F(SemanticReasonerGTests, encodeDecodeASemanticExpressionInBinary)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
  genGrd->entityType = SemanticEntityType::ANIMAL;
  UniqueSemanticExpression semExp = mystd::make_unique<GroundedExpression>(std::move(genGrd));

  const std::size_t maxSize = 10000;
  binarymasks::Ptr mem = ::operator new(maxSize);
  EXPECT_EQ(mem.val, binarysaver::alignMemory(mem).val);
  binarymasks::Ptr beginPtr = mem;

  semexpsaver::writeSemExp(mem, *semExp, lingDb, nullptr);

  binarymasks::Ptr loaderPtr = beginPtr;
  auto readSemExp = semexploader::loadSemExp(loaderPtr.pcuchar, lingDb);
  ::operator delete(beginPtr.pchar);

  EXPECT_EQ(*semExp, *readSemExp);
}



