#include <onsem/tester/syntacticanalysisxmlloader.hpp>
#include <iostream>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/semanticdebugger/aretextsequivalent.hpp>

namespace onsem
{
namespace syntacticAnalysisXmlLoader
{

// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)

namespace
{
void _loadGramPossibility(std::list<std::list<std::string>>& pInitialGramPossibilities,
                          const boost::property_tree::ptree& pTree)
{
  const auto& attributes = pTree.get_child("<xmlattr>");
  if (attributes.get_optional<bool>("newTok"))
    pInitialGramPossibilities.emplace_back();
  assert(!pInitialGramPossibilities.empty());
  pInitialGramPossibilities.back().emplace_back(attributes.get<std::string>("str"));
}


void _updateDiffFromTokensTree(WhatPartAreDifferent& pCurrDiffs,
                               const WhatPartAreDifferent& pWhatNeedToChecked,
                               const linguistics::TokensTree& pOldTokensTree,
                               const linguistics::TokensTree& pNewTokensTree,
                               bool pCheckConcepts)
{
  for (linguistics::ConstTokenIterator itOldTok = pOldTokensTree.beginToken(),
       itNewTok = pNewTokensTree.beginToken();
       !itOldTok.atEnd() || !itNewTok.atEnd(); ++itOldTok, ++itNewTok)
  {
    const linguistics::Token& oldTok = itOldTok.getToken();
    const linguistics::Token& newTok = itNewTok.getToken();
    if (oldTok.str != newTok.str ||
        oldTok.inflWords.size() != newTok.inflWords.size())
    {
      if (pWhatNeedToChecked.tokens)
      {
        pCurrDiffs.tokens = true;
      }
    }
    else
    {
      for (auto itOldIgram = oldTok.inflWords.begin(),
           itNewIgram = newTok.inflWords.begin(); itOldIgram != oldTok.inflWords.end();
           ++itOldIgram, ++itNewIgram)
      {
        if (pWhatNeedToChecked.tagsGram &&
            (itOldIgram->word.partOfSpeech != itNewIgram->word.partOfSpeech ||
             itOldIgram->word.lemma != itNewIgram->word.lemma))
        {
          pCurrDiffs.tagsGram = true;
        }
      }
      if (pCheckConcepts &&
          oldTok.inflWords.front().infos.concepts != newTok.inflWords.front().infos.concepts)
        pCurrDiffs.tokConcepts = true;
    }
  }
}

}



void compareOneTextResults(DeserializedTextResults& pDiffResults,
                           std::shared_ptr<SemanticAnalysisResult>& pOldResultPtr,
                           const SemanticAnalysisResult& pNewTextResult,
                           const std::map<std::string, std::string>* pEquivalencesPtr)
{
  ++pDiffResults.nbOfTexts;
  WhatPartAreDifferent currDiffs;
  const SyntacticGraphResult& oldResult = pOldResultPtr->semAnal;
  const SemanticAnalysisHighLevelResults& oldHighLevelResult = pOldResultPtr->semAnalHighLevelResults;
  const SyntacticGraphResult& newResult = pNewTextResult.semAnal;
  const SemanticAnalysisHighLevelResults& newHighLevelResult = pNewTextResult.semAnalHighLevelResults;

  // check tokens and tagsGram
  if (oldHighLevelResult.initialGramPossibilities.size() !=
      newHighLevelResult.initialGramPossibilities.size() ||
      oldResult.syntGraph.tokensTree.size() !=
      newResult.syntGraph.tokensTree.size())
  {
    if (pDiffResults.whatNeedToChecked.tokens)
      currDiffs.tokens = true;
  }
  else
  {
    _updateDiffFromTokensTree(currDiffs, pDiffResults.whatNeedToChecked,
                              oldResult.syntGraph.tokensTree,
                              newResult.syntGraph.tokensTree,
                              pDiffResults.whatNeedToChecked.tokConcepts);
  }

  // check syntaticGraph
  if (pDiffResults.whatNeedToChecked.syntaticGraph &&
      oldHighLevelResult.syntGraphStr != newHighLevelResult.syntGraphStr)
    currDiffs.syntaticGraph = true;

  // check semExps
  if (pDiffResults.whatNeedToChecked.semExps &&
      oldHighLevelResult.semExpStr != newHighLevelResult.semExpStr)
  {
    currDiffs.semExps = true;
  }

  // check sentiments
  if (pDiffResults.whatNeedToChecked.sentimentsInfos &&
      oldHighLevelResult.sentimentsInfos != newHighLevelResult.sentimentsInfos)
  {
    currDiffs.sentimentsInfos = true;
  }

  // check completeness
  if (pDiffResults.whatNeedToChecked.completeness &&
      oldHighLevelResult.completeness != newHighLevelResult.completeness)
  {
    currDiffs.completeness = true;
  }

  // check reformations
  if (pDiffResults.whatNeedToChecked.reformulations &&
      oldHighLevelResult.reformulations != newHighLevelResult.reformulations)
  {
    currDiffs.reformulations = true;
  }

  // check input / reformation
  if (pDiffResults.whatNeedToChecked.input_reformulation &&
      !areTextEquivalent(oldResult.inputText, newHighLevelResult.reformulationInputLanguage, pEquivalencesPtr))
  {
    currDiffs.input_reformulation = true;
  }

  if (currDiffs.areDifferent())
  {
    pDiffResults.oldResultThatDiffers.push_back(pOldResultPtr);
    pDiffResults.diffsInputSentences.push_back(oldResult.inputText);
    pDiffResults.whatsDiff.push_back(currDiffs);
  }
}


void writeBilan(DeserializedTextResults& pDiffResults)
{
  std::size_t nbOfDiffs = 0;
  std::size_t tokens = 0;
  std::size_t tagsGram = 0;
  std::size_t conceptsByTokens = 0;
  std::size_t syntaticGraph = 0;
  std::size_t semExps = 0;
  std::size_t sentInfos = 0;
  std::size_t completeness = 0;
  std::size_t reformulations = 0;
  std::size_t input_reformulation = 0;

  for (std::size_t i = 0; i < pDiffResults.whatsDiff.size(); ++i)
  {
    bool aDiff = false;
    if (pDiffResults.whatsDiff[i].tokens) { ++tokens; aDiff = true; }
    if (pDiffResults.whatsDiff[i].tagsGram) { ++tagsGram; aDiff = true; }
    if (pDiffResults.whatsDiff[i].tokConcepts) { ++conceptsByTokens; aDiff = true; }
    if (pDiffResults.whatsDiff[i].syntaticGraph) { ++syntaticGraph; aDiff = true; }
    if (pDiffResults.whatsDiff[i].semExps) { ++semExps; aDiff = true; }
    if (pDiffResults.whatsDiff[i].sentimentsInfos) { ++sentInfos; aDiff = true; }
    if (pDiffResults.whatsDiff[i].completeness) { ++completeness; aDiff = true; }
    if (pDiffResults.whatsDiff[i].reformulations) { ++reformulations; aDiff = true; }
    if (pDiffResults.whatsDiff[i].input_reformulation) { ++input_reformulation; aDiff = true; }
    if (aDiff) { ++nbOfDiffs; }
  }

  std::stringstream ss;
  if (pDiffResults.whatNeedToChecked.tokens)
  {
    ss << "Nb of different token lists:\t\t" << tokens << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different token lists:\t\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.tagsGram)
  {
    ss << "Nb of different grammatical tags:\t" << tagsGram << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different grammatical tags:\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.tokConcepts)
  {
    ss << "Nb of different concepts:\t\t" << conceptsByTokens << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different concepts:\t\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.syntaticGraph)
  {
    ss << "Nb of different syntactic graphs:\t" << syntaticGraph << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different syntactic graphs:\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.semExps)
  {
    ss << "Nb of different semantic expressions:\t" << semExps << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different semantic expressions:\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.sentimentsInfos)
  {
    ss << "Nb of different sentiments:\t\t" << sentInfos << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different sentiments:\t\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.completeness)
  {
    ss << "Nb of different completeness:\t\t" << completeness << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different completeness:\t\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.reformulations)
  {
    ss << "Nb of different reformulations:\t" << reformulations << " / "
       << pDiffResults.nbOfTexts << "\n";
  }
  else
  {
    ss << "Nb of different reformulations:\tdisabled\n";
  }

  if (pDiffResults.whatNeedToChecked.input_reformulation)
  {
    std::size_t nbOfSuccess = pDiffResults.nbOfTexts - input_reformulation;
    std::size_t percentage = (nbOfSuccess * 100) / pDiffResults.nbOfTexts;
    ss << "Reformulation as input:\t\t" << input_reformulation << " / "
       << pDiffResults.nbOfTexts << " (" << percentage <<  "% of success)\n\n";
  }
  else
  {
    ss << "Reformulation as input:\t\tdisabled\n\n";
  }

  ss << "===================================================================\n\n";
  ss << "Nb of different texts:\t\t" << nbOfDiffs << " / "
     << pDiffResults.nbOfTexts;
  pDiffResults.bilan = ss.str();
}


void xLoadOneIGram(DeserializedCurrWorkingStruct& pWkStruct,
                   const boost::property_tree::ptree& pTree)
{
  const boost::property_tree::ptree& attributes = pTree.get_child("<xmlattr>");
  pWkStruct.currToken->inflWords.emplace_back();
  linguistics::InflectedWord& inflWord = pWkStruct.currToken->inflWords.back();
  inflWord.word.partOfSpeech = partOfSpeech_fromStr(attributes.get<std::string>("partOfSpeech"));
  inflWord.word.lemma = attributes.get<std::string>("lemma");
  auto inflectionsPot = attributes.get_optional<std::string>("inflections");
  if (inflectionsPot)
  {
    InflectionType inflType = inflectionType_fromPartOfSpeech(inflWord.word.partOfSpeech);
    inflWord.moveInflections(Inflections::create(inflType, *inflectionsPot));
  }
}


void xLoadOneConcept(DeserializedCurrWorkingStruct& pWkStruct,
                     const boost::property_tree::ptree& pTree)
{
  const boost::property_tree::ptree& attributes = pTree.get_child("<xmlattr>");
  pWkStruct.currToken->inflWords.front().infos.concepts
      [attributes.get<std::string>("str")] =
      attributes.get<char>("coef");
}


void xLoadOneToken(DeserializedCurrWorkingStruct& pWkStruct,
                   const boost::property_tree::ptree& pTree)
{
  const boost::property_tree::ptree& attributes = pTree.get_child("<xmlattr>");
  const std::string tokStr = attributes.get<std::string>("str");
  const std::string tokenPosStr = attributes.get<std::string>("tokenPos");

  linguistics::TokenPos tokenPos;
  tokenPos.fromStr(tokenPosStr);

  // final IGram
  pWkStruct.currFinalTokensTree->tokens.emplace_back(tokStr);
  pWkStruct.currToken = &pWkStruct.currFinalTokensTree->tokens.back();
  pWkStruct.currToken->tokenPos = tokenPos;
  pWkStruct.currDesText->idsToTokList[tokenPosStr] =
      &pWkStruct.currFinalTokensTree->tokens;
  std::size_t idToken = pWkStruct.currFinalTokensTree->tokens.size() - 1;
  pWkStruct.currDesText->idsToToken[tokenPosStr].emplace(idToken);
  {
    childLoop(pTree, currSubTree, "contextualInfosGramList")
        xLoadOneIGram(pWkStruct, currSubTree.second);
  }

  // concepts list
  {
    childLoop(pTree, currSubTree, "concepts")
        xLoadOneConcept(pWkStruct, currSubTree.second);
  }

  // sub tokens
  auto optChildTokens = pTree.get_child_optional("tokens");
  if (optChildTokens)
  {
    pWkStruct.currFinalTokensTree->tokens.back().subTokens =
        mystd::make_unique<linguistics::TokensTree>();

    linguistics::TokensTree* finalTokensTreeSAVE = pWkStruct.currFinalTokensTree;

    pWkStruct.currFinalTokensTree =
        &*pWkStruct.currFinalTokensTree->tokens.back().subTokens;
    for (const auto& currTokenTree : *optChildTokens)
      xLoadOneToken(pWkStruct, currTokenTree.second);

    pWkStruct.currFinalTokensTree = finalTokensTreeSAVE;
  }
}



void loadOneText(SemanticAnalysisResult& pTextOutput,
                 const boost::property_tree::ptree& pTree)
{
  DeserializedCurrWorkingStruct wkStruct;
  wkStruct.currDesText = &pTextOutput;
  wkStruct.currDesText->semAnal.inputText = pTree.get<std::string>("<xmlattr>.str");
  //wkStruct.currInitTokensTree = &wkStruct.currDesText->semAnal.initialGramPossibilities;
  wkStruct.currFinalTokensTree = &wkStruct.currDesText->semAnal.syntGraph.tokensTree;

  // tokens
  {
    childLoop(pTree, currSubTree, "tokens")
      xLoadOneToken(wkStruct, currSubTree.second);
    wkStruct.currDesText->saveEndOfLists();
  }

  auto& semAnalHighLevelResults = wkStruct.currDesText->semAnalHighLevelResults;
  {
    childLoop(pTree, currSubTree, "initialGramPossibilities")
      _loadGramPossibility(semAnalHighLevelResults.initialGramPossibilities, currSubTree.second);
  }

  // synthGraph
  semAnalHighLevelResults.syntGraphStr = pTree.get<std::string>("syntGraph.<xmlattr>.str");

  semAnalHighLevelResults.semExpStr = pTree.get<std::string>("semExps.<xmlattr>.str");
  semAnalHighLevelResults.sentimentsInfos = pTree.get<std::string>("sentiments.<xmlattr>.str");
  semAnalHighLevelResults.completeness = pTree.get<bool>("completeness.<xmlattr>.bool");
  semAnalHighLevelResults.reformulations = pTree.get<std::string>("reformulations.<xmlattr>.str");
}


} // End of namespace syntacticAnalysisXmlLoader
} // End of namespace onsem
