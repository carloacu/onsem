#include <onsem/tester/reactOnTexts.hpp>
#include <onsem/tester/resourcelabelfortests.hpp>
#include <iostream>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/executor/textexecutor.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semanticdebugger/loaddbpediatxtmemory.hpp>

namespace onsem
{
namespace
{
bool _test_oneAnswerValue(const std::string& pInput,
                          const std::string& pExpectedReaction,
                          ContextualAnnotation pReactionType,
                          SemanticMemory& pSemanticMemory,
                          const linguistics::LinguisticDatabase& pLingDb,
                          std::size_t pLine)
{
  DetailedReactionAnswer detAnswer = operator_react(pInput, pSemanticMemory, pLingDb);
  if (detAnswer.answer != pExpectedReaction)
  {
    std::cerr << __FILE__ << ":" << pLine << ": Failure" << std::endl;
    std::cerr << "Value of: " << pInput << std::endl;
    std::cerr << "  Actual: " << detAnswer.answer << std::endl;
    std::cerr << "Expected: " << pExpectedReaction << std::endl;
    return false;
  }
  if (detAnswer.reactionType != pReactionType)
  {
    std::cerr << __FILE__ << ":" << pLine << ": Failure type of reaction" << std::endl;
    std::cerr << "  Actual: " << contextualAnnotation_toStr(detAnswer.reactionType) << std::endl;
    std::cerr << "Expected: " << contextualAnnotation_toStr(pReactionType) << std::endl;
    return false;
  }
  return true;
}
}


DetailedReactionAnswer reactionToAnswer(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                                        SemanticMemory& pSemanticMemory,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        SemanticLanguageEnum pLanguage)
{
  DetailedReactionAnswer res;
  if (!pReaction)
    return res;
  SemExpGetter::extractReferences(res.references, **pReaction);
  res.reactionType = SemExpGetter::extractContextualAnnotation(**pReaction);

  TextProcessingContext outContext(SemanticAgentGrounding::me,
                                   SemanticAgentGrounding::currentUser,
                                   pLanguage);
  auto execContext = std::make_shared<ExecutorContext>(outContext);
  DefaultExecutorLogger logger(res.answer);
  TextExecutor textExec(pSemanticMemory, pLingDb, logger);
  textExec.runSemExp(std::move(*pReaction), execContext);
  return res;
}


DetailedReactionAnswer operator_react_fromSemExp(UniqueSemanticExpression pSemExp,
                                                 SemanticMemory& pSemanticMemory,
                                                 const linguistics::LinguisticDatabase& pLingDb,
                                                 SemanticLanguageEnum pTextLanguage,
                                                 const ReactionOptions* pReactionOptions)
{
  if (pTextLanguage == SemanticLanguageEnum::UNKNOWN)
    pTextLanguage = pSemanticMemory.defaultLanguage;
  mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
  memoryOperation::react(reaction, pSemanticMemory, std::move(pSemExp), pLingDb,
                         pReactionOptions);
  return reactionToAnswer(reaction, pSemanticMemory, pLingDb, pTextLanguage);
}


DetailedReactionAnswer operator_react(const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pTextLanguage,
    const ReactionOptions* pReactionOptions)
{
  SemanticLanguageEnum textLanguage = pTextLanguage == SemanticLanguageEnum::UNKNOWN ?
      linguistics::getLanguage(pText, pLingDb) : pTextLanguage;
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  textLanguage);
  inContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{resourceLabelForTests_cmd, resourceLabelForTests_url});
  inContext.spellingMistakeTypesPossible.insert(SpellingMistakeType::CONJUGATION);
  auto semExp =
      converter::textToContextualSemExp(pText, inContext,
                                        SemanticSourceEnum::ASR, pLingDb);
  memoryOperation::mergeWithContext(semExp, pSemanticMemory, pLingDb);
  return operator_react_fromSemExp(std::move(semExp), pSemanticMemory, pLingDb,
                                   textLanguage, pReactionOptions);
}




bool test_knowTheNameOf(const std::string& pName,
                        SemanticMemory& pSemanticMemory,
                        const linguistics::LinguisticDatabase& pLingDb)
{
  bool res = true;
  res = _test_oneAnswerValue("quel est mon nom ?", "Ton nom est " + pName + ".", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue("mon nom est " + pName + " ?", "Oui, ton nom est " + pName + ".", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue("qui suis je", "Tu es " + pName + ".", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue(pName + " c'est mon nom ?", "Oui, " + pName + " est ton nom.", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue("comment je m'appelle", "Tu t'appelles " + pName + ".", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue("qui suis je", "Tu es " + pName + ".", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue("dis mon nom", pName, ContextualAnnotation::BEHAVIOR, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue("mon nom est Unautrenom ?", "Non, ton nom est " + pName + ".", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;

  res = _test_oneAnswerValue("what is my name", "Your name is " + pName + ".", ContextualAnnotation::ANSWER, pSemanticMemory, pLingDb, __LINE__) && res;
  res = _test_oneAnswerValue("say my name", pName, ContextualAnnotation::BEHAVIOR, pSemanticMemory, pLingDb, __LINE__) && res;
  return res;
}


void getResultOfAScenario(
    std::list<std::string>& pResult,
    const std::string& pFilename,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb)
{
  std::ifstream scenarioSpecFile(pFilename, std::ifstream::in);
  if (!scenarioSpecFile.is_open())
    throw std::runtime_error("Can't open " + pFilename + " file !");

  pSemanticMemory.clear();
  memoryOperation::defaultKnowledge(pSemanticMemory, pLingDb);
  std::string beforeLine;
  std::string line;
  while (getline(scenarioSpecFile, line))
  {
    if (line.empty())
      continue;
    char lastCharOfLine = line[line.size() - 1];
    if (lastCharOfLine != '"' && lastCharOfLine != '>' &&
        lastCharOfLine != ']')
    {
      beforeLine += line + "\n";
      continue;
    }
    if (!beforeLine.empty())
    {
      line = beforeLine + line;
      beforeLine = "";
    }

    const std::string inLabel = "in: \"";
    if (line.compare(0, inLabel.size(), inLabel) != 0 ||
        line.size() == inLabel.size() ||
        line[line.size() - 1] != '\"')
    {
      scenarioSpecFile.close();
      throw std::runtime_error("Wrong line: \"" + line + "\" for file: \"" +
                               pFilename + "\"!");
    }

    const std::string currText =
        line.substr(inLabel.size(), line.size() - inLabel.size() - 1);
    pResult.emplace_back(line);

    const auto reaction = operator_react(currText, pSemanticMemory, pLingDb);
    if (!reaction.answer.empty())
      pResult.emplace_back("out: " + reaction.toStr());
  }
  scenarioSpecFile.close();
}


void loadOneFileInSemanticMemory(std::size_t& pNbOfInforms,
                                 const std::string& pFilename,
                                 SemanticMemory& pSemanticMemory,
                                 linguistics::LinguisticDatabase& pLingDb,
                                 bool pAddReferences,
                                 const std::string* pPathToWriteTextReplaced)
{
  std::unique_ptr<std::ofstream> textReplacedFilePtr;
  if (pPathToWriteTextReplaced != nullptr)
  {
    const auto textReplacedFilename = *pPathToWriteTextReplaced + "/text_replaced.txt";
    std::cout << "textReplacedFilename: " << textReplacedFilename << std::endl;
    textReplacedFilePtr = mystd::make_unique<std::ofstream>(textReplacedFilename.c_str());
  }

  std::set<std::string> properNouns;
  loadDbPediaMemory(pNbOfInforms, properNouns, pSemanticMemory, pLingDb, pFilename, textReplacedFilePtr, pAddReferences);

  if (textReplacedFilePtr)
    textReplacedFilePtr->close();
  pLingDb.addProperNouns(properNouns);
}


} // End of namespace onsem
