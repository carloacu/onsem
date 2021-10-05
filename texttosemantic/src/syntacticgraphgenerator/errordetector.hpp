#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_ERRORDETECTOR_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_ERRORDETECTOR_HPP

#include <list>
#include <vector>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>

namespace onsem
{
namespace linguistics
{
struct ChunkLinkIter;
class SemanticFrameDictionary;
class StaticLinguisticDictionary;
class AlgorithmSetForALanguage;
struct TokensTree;
struct Token;

enum class CarryOnFrom
{
  PARTOFSPEECH_FILTERS,
  SYNTACTIC_TREE,
  HERE
};


class ErrorDetector
{
public:
  explicit ErrorDetector(const AlgorithmSetForALanguage& pConfiguration);

  bool tryToConvertNounToImperativeVerbs
  (std::list<ChunkLink>& pSyntTree) const;

  CarryOnFrom falseGramPossibilitiesRemoved
  (std::list<ChunkLink>& pSyntTree) const;

  void frFixOfVerbalChunks
  (std::list<ChunkLink>& pSyntTree) const;

  static void addYesOrNoRequestForVerbsBeforeInterrogationPunctuation
  (std::list<ChunkLink>& pSyntTree);

private:
  const AlgorithmSetForALanguage& fConfiguration;
  const SemanticFrameDictionary& fSemFrameDict;
  const LinguisticDictionary& fLingDico;
  const std::vector<PartOfSpeech> fPossNewHeadGram;

  CarryOnFrom xCheckThatNominalGroupHaveAValidHead
  (std::list<ChunkLink>::iterator pItChLk) const;

  CarryOnFrom xRemoveInvalidPronouns(Chunk& pChunk) const;
  CarryOnFrom xCorrectFalsePrepositionalChunk(std::list<ChunkLink>::iterator pItChLk) const;

  bool xSolveVerbThatShouldHaveAnAuxiliary
  (Chunk& pVerbChunk) const;

  bool xSolveVerbThatHaveASubjectThatBeginsWithAPrep
  (Chunk& pVerbChunk) const;

  bool xTryToCorrectVerbsWithoutSubject
  (ChunkLinkIter& pChkLkIter) const;

  CarryOnFrom xRemoveSubordinatingConjonctionUnliked
  (ChunkLink& pItChLk,
   ChunkLink* pNextItChLk) const;

  void xPutRepetitionChildToTheFatherNode(Chunk& pChunk) const;

  CarryOnFrom xSolveBadVerbChunks(ChunkLinkIter& pChkLkIter,
                                  ChunkLinkType pParentChkLk,
                                  ChunkType pPreviousChunkType) const;

  CarryOnFrom xSolveConjunctionUnlinked(std::list<ChunkLink>::iterator pItChLk) const;

  void xBreakLinkedTokens(Token& pToken) const;

  bool xTryToConvertNounToImperativeVerbsForAChunk(Chunk& pChunk) const;
};


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_ERRORDETECTOR_HPP
