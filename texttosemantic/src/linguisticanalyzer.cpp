#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include "tokenizer/tokenizer.hpp"
#include "tokenizer/partofspeechfilterer.hpp"
#include "syntacticgraphgenerator/interjectinalchunker.hpp"
#include "syntacticgraphgenerator/verbalchunker.hpp"
#include "syntacticgraphgenerator/nominalchunker.hpp"
#include "syntacticgraphgenerator/chunkslinker.hpp"
#include "syntacticgraphgenerator/verbaltomoninalchunkslinker.hpp"
#include "syntacticgraphgenerator/errordetector.hpp"
#include "syntacticgraphgenerator/entityrecognizer.hpp"
#include "syntacticgraphgenerator/subordinateextractor.hpp"
#include "syntacticgraphgenerator/interjectionsadder.hpp"
#include "syntacticgraphgenerator/notunderstoodadder.hpp"
#include "syntacticgraphgenerator/listextractor.hpp"
#include "syntacticgraphgenerator/incompletelistsresolver.hpp"
#include "tosemantic/syntacticgraphtosemantic.hpp"
#include "tool/chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{

/**
 * @brief Construct a synctatic tree from a token list with partOfSpeech already filtered.
 * @param pTokensTree List of tokens.
 * @param pFirstChilds Resulting syntactic tree.
 * @param pLangConfig Algorithms to use for language of the text.
 * @param pSpecLingDb Databases of language of the text.
 * @param pIsRootLevel Yes I we restart for the entire text, False if it's only localized to a subordinate.
 * @param pTokenizerEndingStep For debug only, it indicates where to stop the partOfSpeech filter step. (in case we need to restart from here)
 * @param pEndingStep For debug only, it indicates where to stop the syntactic tree construction.
 */
void _constructSyntacticTree(TokensTree& pTokensTree,
                             std::list<ChunkLink>& pFirstChildren,
                             const AlgorithmSetForALanguage& pLangConfig,
                             const SpecificLinguisticDatabase& pSpecLingDb,
                             bool pIsRootLevel,
                             const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                             const SynthAnalEndingStepForDebug& pEndingStep);



struct SubordonateSyntGraphWorkingContext
{
  SubordonateSyntGraphWorkingContext
  (Chunk* pChunk,
   TokensTree& pTokenTree)
    : chunk(pChunk),
      tokenTree(pTokenTree),
      firstChildren()
  {
  }

  SubordonateSyntGraphWorkingContext
  (const SubordonateSyntGraphWorkingContext& pOther)
    : chunk(pOther.chunk),
      tokenTree(pOther.tokenTree),
      firstChildren(pOther.firstChildren)
  {
  }

  Chunk* chunk;
  TokensTree& tokenTree;
  std::list<ChunkLink> firstChildren;

private:
  SubordonateSyntGraphWorkingContext& operator=
  (const SubordonateSyntGraphWorkingContext& pOther);
};





void _iterateOnParenthesis
(std::list<SubordonateSyntGraphWorkingContext>& pSubordonatesOfChunks,
 std::set<const Chunk*>& pPrevChunks,
 const AlgorithmSetForALanguage& pLangConfig,
 std::list<ChunkLink>& pChunkLinks)
{
  for (ChunkLink& currChunkLink : pChunkLinks)
  {
    // check if we already look at this chunk
    if (!currChunkLink.chunk->hasOnlyOneReference)
    {
      if (pPrevChunks.find(&*currChunkLink.chunk) != pPrevChunks.end())
        continue;
      pPrevChunks.insert(&*currChunkLink.chunk);
    }

    // look inside the current chunk
    for (auto itTok = currChunkLink.chunk->tokRange.getItBegin();
         itTok != currChunkLink.chunk->tokRange.getItEnd(); ++itTok)
    {
      if (itTok->subTokens != nullptr &&
          !itTok->subTokens->tokens.empty() &&
          itTok->subTokens->tokens[0].str == "(")
        pSubordonatesOfChunks.emplace_back(&*currChunkLink.chunk, *itTok->subTokens);
    }

    // iterate on the children
    _iterateOnParenthesis(pSubordonatesOfChunks, pPrevChunks, pLangConfig,
                           currChunkLink.chunk->children);
  }
}


void _launchGrammRules(TokensTree& pTokensTree,
                       std::list<ChunkLink>& pFirstChildren,
                       const AlgorithmSetForALanguage& pLangConfig,
                       const SpecificLinguisticDatabase& pSpecLingDb,
                       bool pIsRootLevel,
                       const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                       const SynthAnalEndingStepForDebug& pEndingStep)
{
  partOfSpeechFilterer::process(pTokensTree.tokens, pSpecLingDb, pEndingStep.tokenizerEndingStep,
                                pEndingStep.nbOfDebugRoundsForTokens, pIsRootLevel);

  _constructSyntacticTree(pTokensTree, pFirstChildren, pLangConfig, pSpecLingDb,
                          pIsRootLevel, pSpellingMistakeTypesPossible, pEndingStep);
}


void _fillPassiveInformation(std::list<ChunkLink>& pChunkLinks,
                             const AlgorithmSetForALanguage& pLangConfig)
{
  const auto& lingDico = pLangConfig.getLingDico();
  const auto& flsChecker = pLangConfig.getFlsChecker();
  for (ChunkLink& currChunkLink : pChunkLinks)
  {
    Chunk& currChunk = *currChunkLink.chunk;
    if (chunkTypeIsVerbal(currChunk.type))
      currChunk.isPassive = isChunkAtPassiveForm(currChunk, lingDico, flsChecker);
  }
}


void _constructSyntacticTree(TokensTree& pTokensTree,
                             std::list<ChunkLink>& pFirstChildren,
                             const AlgorithmSetForALanguage& pLangConfig,
                             const SpecificLinguisticDatabase& pSpecLingDb,
                             bool pIsRootLevel,
                             const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                             const SynthAnalEndingStepForDebug& pEndingStep)
{
  const ErrorDetector& errDetector = pLangConfig.getErrorDetector();
  const InflectionsChecker& inflChecker = pLangConfig.getFlsChecker();

  // Verb chunker
  pLangConfig.getVerbChunker().process(pTokensTree, pFirstChildren);
  pLangConfig.getIntChunker().process(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::VERBALCHUNKER))
  { return; }

  // Chunker
  pLangConfig.getChunker().process(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::NOMINALCHUNKER))
  { return; }

  // Verbal to nominal chunks linker
  if (pLangConfig.getLanguageType() == SemanticLanguageEnum::ENGLISH)
    pLangConfig.getVerbToNounLinker().extractEnglishSubjectOf(pFirstChildren);
  pLangConfig.getVerbToNounLinker().process(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::VERBALTONOMINALCHUNKSLINKER))
  { return; }

  if (pIsRootLevel &&
      pLangConfig.getLanguageType() == SemanticLanguageEnum::ENGLISH &&
      errDetector.tryToConvertNounToImperativeVerbs(pFirstChildren))
  {
    SynthAnalEndingStepForDebug subEndingStep = pEndingStep;
    --subEndingStep.nbOfDebugRoundsForSynthAnalysis;
    _launchGrammRules(pTokensTree, pFirstChildren, pLangConfig, pSpecLingDb,
                      pIsRootLevel, pSpellingMistakeTypesPossible, subEndingStep);
    return;
  }
  _fillPassiveInformation(pFirstChildren, pLangConfig);

  // Entity recognizer
  pLangConfig.getEntityRecognizer().process(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::ENTITYRECOGNIZER))
  { return; }

  // Subordonates extractor
  bool subExtFinished = pLangConfig.getSubordonatesExtractor().process(pFirstChildren, pEndingStep);
  if (!subExtFinished || pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR))
  { return; }


  // resolve incomplete lists
  resolveIncompleteLists(pFirstChildren, pSpecLingDb.lingDico);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::RESOLVEINCOMPLETELISTS))
  { return; }


  if (pIsRootLevel)
  {
    // Error detector
    CarryOnFrom carryOnFrom = errDetector.falseGramPossibilitiesRemoved(pFirstChildren);
    switch (carryOnFrom)
    {
    case CarryOnFrom::PARTOFSPEECH_FILTERS:
    {
      SynthAnalEndingStepForDebug subEndingStep = pEndingStep;
      --subEndingStep.nbOfDebugRoundsForSynthAnalysis;
      _launchGrammRules(pTokensTree, pFirstChildren, pLangConfig, pSpecLingDb,
                        pIsRootLevel, pSpellingMistakeTypesPossible, subEndingStep);
      return;
    }
    case CarryOnFrom::SYNTACTIC_TREE:
    {
      SynthAnalEndingStepForDebug subEndingStep = pEndingStep;
      --subEndingStep.nbOfDebugRoundsForSynthAnalysis;
      _constructSyntacticTree(pTokensTree, pFirstChildren, pLangConfig, pSpecLingDb,
                              pIsRootLevel, pSpellingMistakeTypesPossible, subEndingStep);
      return;
    }
    case CarryOnFrom::HERE:
      break;
    }
  }
  if (pLangConfig.getLingDico().getLanguage() == SemanticLanguageEnum::FRENCH)
  {
    errDetector.frFixOfVerbalChunks(pFirstChildren);
  }
  errDetector.addYesOrNoRequestForVerbsBeforeInterrogationPunctuation(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::ERRORDETECTOR))
  { return; }

  if (pIsRootLevel)
  {
    // Linker
    pLangConfig.getLinker().process(pFirstChildren);
  }
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::LINKER))
  { return; }


  // Add interjection chunks
  addInterjections(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::INTERJECTIONS_ADDER))
  { return; }


  // Tag some chunk link has "not understood"
  bool isNotUnderstood = false;
  if (pIsRootLevel &&
      addNotUnderstood(pFirstChildren, isNotUnderstood, pSpellingMistakeTypesPossible,
                       inflChecker, pSpecLingDb.lingDico))
  {
    SynthAnalEndingStepForDebug subEndingStep = pEndingStep;
    --subEndingStep.nbOfDebugRoundsForSynthAnalysis;
    _launchGrammRules(pTokensTree, pFirstChildren, pLangConfig, pSpecLingDb,
                      pIsRootLevel, pSpellingMistakeTypesPossible, subEndingStep);
    return;
  }


  // Consider the texts in parenthesis
  {
    std::list<SubordonateSyntGraphWorkingContext> subordonatesOfChunks;
    std::set<const Chunk*> prevChunks;
    _iterateOnParenthesis(subordonatesOfChunks, prevChunks, pLangConfig, pFirstChildren);
    for (auto it = subordonatesOfChunks.begin(); it != subordonatesOfChunks.end(); ++it)
    {
      _launchGrammRules(it->tokenTree, it->firstChildren, pLangConfig,
                        pSpecLingDb, false, pSpellingMistakeTypesPossible, pEndingStep);
      auto& currChunk = *it->chunk;
      auto _chunckCanBeLinkedToASubject = [&](const Chunk& pChunk)
      {
        return pChunk.type == ChunkType::VERB_CHUNK &&
            canLinkVerbToASubject(whereToLinkTheSubject(pChunk), currChunk,
                                  inflChecker, true);
      };

      while (it->firstChildren.begin() != it->firstChildren.end())
      {
        std::list<ChunkLink>::iterator itChunkLink = it->firstChildren.begin();
        if (itChunkLink->chunk->type != ChunkType::SEPARATOR_CHUNK)
        {
          if (currChunk.type != ChunkType::VERB_CHUNK &&
              recInListConst(_chunckCanBeLinkedToASubject, *itChunkLink->chunk))
          {
            itChunkLink->type = ChunkLinkType::SUBJECT_OF;
          }
          else
          {
            itChunkLink->type = ChunkLinkType::COMPLEMENT;
          }
          currChunk.children.push_back(*itChunkLink);
        }
        it->firstChildren.erase(itChunkLink);
      }
    }
  }
}



void tokenizationAndSyntacticalAnalysis
(SyntacticGraph& pSyntGraph,
 const std::string& pInputSentence,
 const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
 const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr)
{
  // 1. Tokenisation
  tokenizeText(pSyntGraph.tokensTree, pSyntGraph.langConfig, pInputSentence,
               pCmdGrdExtractorPtr);

  // 2. Syntactic analysis
  SynthAnalEndingStepForDebug debugStruct;
  syntacticAnalysis(pSyntGraph, pSpellingMistakeTypesPossible, debugStruct);
}



void tokenizeText
(TokensTree& pTokensTree,
 const AlgorithmSetForALanguage& pLangConfig,
 const std::string& pInputSentence,
 const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr)
{
  tokenizer::tokenize(pTokensTree.tokens, pInputSentence,
                      pLangConfig.getSpecifcLingDb().lingDico,
                      pLangConfig.lingDb.commonSpecificLingDb().lingDico,
                      pCmdGrdExtractorPtr);
}


void syntacticAnalysis
(SyntacticGraph& pSyntGraph,
 const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
 const SynthAnalEndingStepForDebug& pEndingStep)
{
  _launchGrammRules(pSyntGraph.tokensTree, pSyntGraph.firstChildren,
                    pSyntGraph.langConfig, pSyntGraph.langConfig.getSpecifcLingDb(),
                    true, pSpellingMistakeTypesPossible, pEndingStep);
}



UniqueSemanticExpression convertToSemExp
(const SyntacticGraph& pSyntGraph,
 const TextProcessingContext& pLocutionContext,
 const SemanticTimeGrounding& pTimeGrd,
 std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout)
{
  SyntacticGraphToSemantic semTranslator(pSyntGraph.langConfig);
  return semTranslator.process(pSyntGraph, pLocutionContext, pTimeGrd,
                               std::move(pAgentWeAreTalkingAbout));
}


void extractProperNounsThatDoesntHaveAnyOtherGrammaticalTypes
(std::set<std::string>& pNewProperNouns,
 const std::string& pInputSentence,
 SemanticLanguageEnum pLanguage,
 const LinguisticDatabase& pLingDb)
{
  std::shared_ptr<ResourceGroundingExtractor> cmdGrdExtractorPtr;
  const LinguisticDictionary& commonLingDico = pLingDb.commonSpecificLingDb().lingDico;
  std::vector<Token> tokens;
  tokenizer::tokenize(tokens, pInputSentence, commonLingDico, commonLingDico, cmdGrdExtractorPtr);

  const LinguisticDictionary& lingDico = pLingDb.langToSpec[pLanguage].lingDico;
  // extract the proper nouns
  for (const auto& currToken : tokens)
  {
    const auto& inflWordFrom = currToken.inflWords.front();
    if (currToken.inflWords.size() == 1 &&
        inflWordFrom.word.partOfSpeech == PartOfSpeech::PROPER_NOUN &&
        inflWordFrom.inflections().type == InflectionType::EMPTY &&
        beginWithUppercase(currToken.str))
    {
      // consider only the proper nouns that cannot have another part of speech
      std::vector<Token> tokensOfTheProperNoun;
      tokenizer::tokenize(tokensOfTheProperNoun, currToken.str, lingDico, commonLingDico, cmdGrdExtractorPtr);
      if (!tokensOfTheProperNoun.empty() &&
          tokensOfTheProperNoun.front().inflWords.size() == 1)
        pNewProperNouns.insert(currToken.str);
    }
  }
}


void extractProperNouns
(std::vector<std::string>& pProperNouns,
 const std::string& pInputSentence,
 SemanticLanguageEnum pLanguage,
 const LinguisticDatabase& pLingDb)
{
  std::shared_ptr<ResourceGroundingExtractor> cmdGrdExtractorPtr;
  const auto& specLingDb = pLingDb.langToSpec[pLanguage];
  const auto& commonLingDico = pLingDb.commonSpecificLingDb().lingDico;
  const auto& lingDico = specLingDb.lingDico;
  std::vector<Token> tokens;
  tokenizer::tokenize(tokens, pInputSentence, lingDico, commonLingDico, cmdGrdExtractorPtr);
  partOfSpeechFilterer::process(tokens, specLingDb, tokenizerDefaultEndingStep, 1, true);

  // extract the proper nouns
  for (const auto& currToken : tokens)
    if (!currToken.inflWords.empty() &&
        currToken.inflWords.front().word.partOfSpeech == PartOfSpeech::PROPER_NOUN /* &&
        beginWithUppercase(currToken.str) */)
      pProperNouns.emplace_back(currToken.str);
}


bool isAProperNoun
(const std::string& pStr,
 const LinguisticDatabase& pLingDb)
{
  const auto& commonLingDico = pLingDb.commonSpecificLingDb().lingDico;
  std::list<InflectedWord> inflections;
  commonLingDico.getGramPossibilities(inflections, pStr, 0, pStr.size());
  for (const auto& currInfl : inflections)
    if (currInfl.word.partOfSpeech == PartOfSpeech::PROPER_NOUN)
      return true;
  return false;
}



} // End of namespace linguistics
} // End of namespace onsem
