#ifndef ONSEM_TESTER_SYNTACTICANALYSISXMLLOADER_HPP
#define ONSEM_TESTER_SYNTACTICANALYSISXMLLOADER_HPP

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/semanticdebugger/syntacticgraphresult.hpp>
#include <onsem/semanticdebugger/semanticdebug.hpp>
#include "api.hpp"

namespace onsem
{
namespace syntacticAnalysisXmlLoader
{

struct ONSEMTESTER_API WhatPartAreDifferent
{
  WhatPartAreDifferent()
    : tokens(false),
      tagsGram(false),
      tokConcepts(false),
      syntaticGraph(false),
      semExps(false),
      sentimentsInfos(false),
      completeness(false),
      reformulations(false),
      input_reformulation(false)
  {
  }

  void setAllTrue()
  {
    tokens = true;
    tagsGram = true;
    tokConcepts = true;
    syntaticGraph = true;
    semExps = true;
    sentimentsInfos = true;
    completeness = true;
    reformulations = true;
    input_reformulation = true;
  }

  bool areDifferent() const
  { return tokens || tagsGram || tokConcepts || syntaticGraph ||
        semExps || sentimentsInfos || completeness ||
        reformulations || input_reformulation; }

  bool tokens;
  bool tagsGram;
  bool tokConcepts;
  bool syntaticGraph;
  bool semExps;
  bool sentimentsInfos;
  bool completeness;
  bool reformulations;
  bool input_reformulation;
};


struct ONSEMTESTER_API SemanticAnalysisResult
{
  SemanticAnalysisResult(const linguistics::LinguisticDatabase& pLingDb,
                         SemanticLanguageEnum pLanguage)
    : semAnal(pLingDb, pLanguage),
      semAnalHighLevelResults(),
      idsToTokList(),
      idsToToken(),
      endingTokensToId()
  {
  }

  void saveEndOfLists()
  {
    int nextEndingId = -1;
    xRecSaveEndOfLists(nextEndingId, semAnal.syntGraph.tokensTree);
  }

  std::string getTokenId
  (const std::vector<linguistics::Token>& pTokenVect,
   linguistics::TokCstIt pItToken) const
  {
    for (auto endingTokIt = endingTokensToId.begin();
         endingTokIt != endingTokensToId.end(); ++endingTokIt)
      if (tokListHaveSameInflectedFormThan(pTokenVect, endingTokIt->first) &&
          pItToken == endingTokIt->first.end())
        return endingTokIt->second;

    assert(pItToken != pTokenVect.end());
    return pItToken->tokenPos.toStr();
  }

  SyntacticGraphResult semAnal;
  SemanticAnalysisHighLevelResults semAnalHighLevelResults;
  std::map<std::string, std::vector<linguistics::Token>*> idsToTokList;
  std::map<std::string, mystd::optional<std::size_t>> idsToToken;
  std::list<std::pair<std::vector<linguistics::Token>&, std::string> > endingTokensToId;

private:
  void xRecSaveEndOfLists
  (int& pNextEndingId,
   linguistics::TokensTree& pTokensTree)
  {
    std::stringstream ssId;
    ssId << pNextEndingId--;
    std::string idEndToken = ssId.str();
    endingTokensToId.emplace_back(pTokensTree.tokens, idEndToken);
    idsToTokList[idEndToken] = &pTokensTree.tokens;
    idsToToken[idEndToken];
    for (auto& currTok : pTokensTree.tokens)
      if (currTok.subTokens)
        xRecSaveEndOfLists(pNextEndingId, *currTok.subTokens);
  }
};


struct ONSEMTESTER_API DeserializedCurrWorkingStruct
{
  DeserializedCurrWorkingStruct() = default;

  DeserializedCurrWorkingStruct(const DeserializedCurrWorkingStruct&) = delete;
  DeserializedCurrWorkingStruct& operator=(const DeserializedCurrWorkingStruct&) = delete;

  SemanticAnalysisResult* currDesText = nullptr;
  linguistics::TokensTree* currFinalTokensTree = nullptr;
  linguistics::Token* currToken = nullptr;
};


struct ONSEMTESTER_API DeserializedTextResults
{
  DeserializedTextResults()
  {
    whatNeedToChecked.setAllTrue();
  }

  DeserializedTextResults(const DeserializedTextResults&) = delete;
  DeserializedTextResults& operator=(const DeserializedTextResults&) = delete;

  std::string bilan{};
  std::size_t nbOfTexts = 0;
  std::vector<std::shared_ptr<SemanticAnalysisResult>> oldResultThatDiffers{};
  std::vector<std::string> diffsInputSentences{};
  std::vector<WhatPartAreDifferent> whatsDiff{};
  WhatPartAreDifferent whatNeedToChecked{};
};


ONSEMTESTER_API
void loadOneText(SemanticAnalysisResult& pTextOutput,
                 const boost::property_tree::ptree& pTree);

ONSEMTESTER_API
void compareOneTextResults(DeserializedTextResults& pDiffResults,
                           std::shared_ptr<SemanticAnalysisResult>& pOldResult,
                           const SemanticAnalysisResult& pNewResult,
                           const std::map<std::string, std::string>* pEquivalencesPtr);

ONSEMTESTER_API
void writeBilan(DeserializedTextResults& pDiffResults);



} // End of namespace syntacticAnalysisXmlLoader
} // End of namespace onsem

#endif // ONSEM_TESTER_SYNTACTICANALYSISXMLLOADER_HPP
