#include <onsem/tester/syntacticanalysisxmlsaver.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <onsem/semanticdebugger/printer/semexplinestostr.hpp>
#include <onsem/semanticdebugger/textananlyzedebugger.hpp>
#include <onsem/semanticdebugger/timechecker.hpp>


namespace onsem
{
namespace syntacticAnalysisXmlSaver
{
// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)

namespace
{

void _getTextToResult(std::shared_ptr<syntacticAnalysisXmlLoader::SemanticAnalysisResult>& pResults,
                      const std::string& pText,
                      const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                      const linguistics::LinguisticDatabase& pLingDb,
                      const SemanticAnalysisDebugOptions& pDebugOptions,
                      SemanticLanguageEnum pLanguageType)
{
  pResults = std::make_shared<syntacticAnalysisXmlLoader::SemanticAnalysisResult>(pLingDb, pLanguageType);
  pResults->semAnal.inputText = pText;
  linguistics::TextAnalyzeDebugger::fillSemAnalResult
      (pResults->semAnal, pResults->semAnalHighLevelResults, pSpellingMistakeTypesPossible, pDebugOptions);
  // save ending token iterators
  pResults->saveEndOfLists();
}


void _saveIGramPossibilities(boost::property_tree::ptree& pTree,
                             const std::string& pXmlTagName,
                             const linguistics::Token& pCurrToken)
{
  boost::property_tree::ptree& inflsWordsTree = pTree.add_child(pXmlTagName, {});
  for (const linguistics::InflectedWord& currIGram : pCurrToken.inflWords)
  {
    auto& currInflWordTree = inflsWordsTree.add_child("infosGram", {});
    auto& currInflWordAttributes = currInflWordTree.add_child("<xmlattr>", {});
    currInflWordAttributes.put("lemma", currIGram.word.lemma);
    currInflWordAttributes.put("partOfSpeech", partOfSpeech_toStr(currIGram.word.partOfSpeech));
    std::stringstream ssInfls;
    currIGram.inflections().concisePrint(ssInfls);
    std::string inflections = ssInfls.str();
    if (!inflections.empty())
      currInflWordAttributes.put("inflections", inflections);
  }
}


void _saveTheConcepts(boost::property_tree::ptree& pTree,
                      const linguistics::Token& pCurrToken)
{
  boost::property_tree::ptree& conceptsTree = pTree.add_child("concepts", {});
  const auto& concepts = pCurrToken.inflWords.front().infos.concepts;
  for (const auto& currConcept : concepts)
  {
    auto& currCptTree = conceptsTree.add_child("concept", {});
    auto& currCptAttributes = currCptTree.add_child("<xmlattr>", {});
    currCptAttributes.put("str", currConcept.first);
    currCptAttributes.put("coef", currConcept.second);
  }
}

void _saveText(boost::property_tree::ptree& pTree,
               const syntacticAnalysisXmlLoader::SemanticAnalysisResult& pTextAnalRes)
{
  boost::property_tree::ptree& textTree = pTree.add_child("text", {});
  textTree.put("<xmlattr>.str", pTextAnalRes.semAnal.inputText);

  // tokens
  {
    boost::property_tree::ptree& tokensTree = textTree.add_child("tokens", {});
    std::list<boost::property_tree::ptree*> tokensPile;
    boost::property_tree::ptree* lastToken = nullptr;
    for (linguistics::ConstTokenIterator itToken = pTextAnalRes.semAnal.syntGraph.tokensTree.beginToken();
         !itToken.atEnd(); ++itToken)
    {
      // update the token pile
      if (itToken.getOffset() > tokensPile.size())
      {
        assert(lastToken != nullptr);
        tokensPile.push_back(&lastToken->add_child("tokens", {}));
      }
      else if (itToken.getOffset() < tokensPile.size())
      {
        tokensPile.pop_back();
      }

      boost::property_tree::ptree* currTokenTreePtr = nullptr;
      // link to the good father
      if (itToken.getOffset() == 0)
      {
        currTokenTreePtr = &tokensTree.add_child("token", {});
      }
      else
      {
        assert(!tokensPile.empty());
        currTokenTreePtr = &tokensPile.back()->add_child("token", {});
      }
      boost::property_tree::ptree& currTokenTree = *currTokenTreePtr;

      const linguistics::Token& currToken = itToken.getToken();
      boost::property_tree::ptree& tokenAttributes = currTokenTree.add_child("<xmlattr>", {});
      tokenAttributes.put("str", currToken.str);
      tokenAttributes.put("tokenPos", currToken.tokenPos.toStr());
      _saveIGramPossibilities(currTokenTree,
                              "contextualInfosGramList", currToken);
      _saveTheConcepts(currTokenTree, currToken);

      // save the last token
      lastToken = &currTokenTree;
    }
  }

  const auto& semAnalHighLevelResults = pTextAnalRes.semAnalHighLevelResults;
  // initialGramPossibilities
  {
    auto& initialGramPossibilitiesTree = textTree.add_child("initialGramPossibilities", {});
    for (const auto& currToken : semAnalHighLevelResults.initialGramPossibilities)
    {
      bool newToken  = true;
      for (const auto& currGramPoss : currToken)
      {
        auto& gramPossTree = initialGramPossibilitiesTree.add_child("gramPoss", {});
        if (newToken)
        {
          gramPossTree.put("<xmlattr>.newTok", true);
          newToken = false;
        }
        gramPossTree.put("<xmlattr>.str", currGramPoss);
      }
    }
  }

  // syntactic graph
  textTree.put("syntGraph.<xmlattr>.str", semAnalHighLevelResults.syntGraphStr);

  textTree.put("semExps.<xmlattr>.str", semAnalHighLevelResults.semExpStr);
  textTree.put("sentiments.<xmlattr>.str", semAnalHighLevelResults.sentimentsInfos);
  textTree.put("completeness.<xmlattr>.bool", semAnalHighLevelResults.completeness);
  textTree.put("reformulations.<xmlattr>.str", semAnalHighLevelResults.reformulations);
}

}


void compareResults(std::shared_ptr<syntacticAnalysisXmlLoader::DeserializedTextResults>& pDiffResults,
                    const std::string& pLanguageStr,
                    const linguistics::LinguisticDatabase& pLingDb,
                    const std::string& pCorpusResultsFolder,
                    const std::map<std::string, std::string>* pEquivalencesPtr,
                    std::string* pPerformancesPtr)
{
  SemanticLanguageEnum langType = semanticLanguageTypeGroundingEnumFromStr(pLanguageStr);

  auto oldSaveFilename = pCorpusResultsFolder + "/" + pLanguageStr + "_syntacticanalysis_results.xml";

  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(oldSaveFilename, tree);
  std::unique_ptr<TimeChecker> timeChecker;

  childLoop(tree, currSubTree, "syntacticAnalysisResults")
  {
    auto oldResult = std::make_shared<syntacticAnalysisXmlLoader::SemanticAnalysisResult>(pLingDb, langType);
    syntacticAnalysisXmlLoader::loadOneText(*oldResult, currSubTree.second);

    SemanticAnalysisDebugOptions debugOptions;
    debugOptions.outputFormat = PrintSemExpDiffsOutPutFormat::HTML;
    if (pPerformancesPtr != nullptr)
      debugOptions.timeChecker = mystd::make_unique<TimeChecker>();

    std::shared_ptr<syntacticAnalysisXmlLoader::SemanticAnalysisResult> newResult;
    _getTextToResult(newResult, oldResult->semAnal.inputText,
                     _spellingMistakeTypesPossibleForDebugOnTextComparisons,
                     pLingDb, debugOptions, langType);
    syntacticAnalysisXmlLoader::compareOneTextResults(*pDiffResults, oldResult, *newResult,
                                                      pEquivalencesPtr);
    if (pPerformancesPtr != nullptr)
    {
      if (timeChecker)
        timeChecker->concatenate(*debugOptions.timeChecker);
      else
        timeChecker = std::move(debugOptions.timeChecker);
    }
  }
  syntacticAnalysisXmlLoader::writeBilan(*pDiffResults);
  if (pPerformancesPtr != nullptr)
    timeChecker->printBilanOfTimeSlots(*pPerformancesPtr);
}



void save(const std::string& pFilename,
          SemanticLanguageEnum pLanguageType,
          const std::vector<std::string>& pInputSentences,
          const linguistics::LinguisticDatabase& pLingDb)
{
  boost::property_tree::ptree tree;
  boost::property_tree::ptree& resultsTree = tree.add_child("syntacticAnalysisResults", {});

  for (std::size_t i = 0; i < pInputSentences.size(); ++i)
  {
    std::shared_ptr<syntacticAnalysisXmlLoader::SemanticAnalysisResult> result;
    SemanticAnalysisDebugOptions debugOptions;
    debugOptions.outputFormat = PrintSemExpDiffsOutPutFormat::HTML;
    _getTextToResult(result, pInputSentences[i],
                     _spellingMistakeTypesPossibleForDebugOnTextComparisons,
                     pLingDb, debugOptions, pLanguageType);
    _saveText(resultsTree, *result);
  }

  boost::property_tree::write_xml(pFilename, tree, std::locale(),
                                  boost::property_tree::xml_writer_make_settings<std::string>(' ', 2));
}


} // End of namespace syntacticAnalysisXmlSaver
} // End of namespace onsem
