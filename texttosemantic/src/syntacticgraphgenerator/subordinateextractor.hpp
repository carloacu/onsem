#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_SUBORDINATEEXTRACTOR_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_SUBORDINATEEXTRACTOR_HPP

#include <list>
#include <memory>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/texttosemantic/type/enum/chunktype.hpp>
#include <onsem/texttosemantic/type/debug/synthanalendingstepfordebug.hpp>

namespace onsem
{
namespace linguistics
{
class LinguisticDictionary;
class SemanticFrameDictionary;
class AlgorithmSetForALanguage;
struct ChunkLink;
struct Chunk;


class SubordinateExtractor
{
public:
  explicit SubordinateExtractor
  (const AlgorithmSetForALanguage& pConfiguration);

  bool process
  (std::list<ChunkLink>& pFirstChildren,
   const SynthAnalEndingStepForDebug& pEndingStep) const;

private:
  struct SubordonatingPossibility
  {
    SubordonatingPossibility
    (std::list<ChunkLink>::reverse_iterator pSubSep,
     std::list<ChunkLink>::reverse_iterator pSubPhrase)
      : subSep(pSubSep),
        subPhrase(pSubPhrase)
    {
    }

    std::list<ChunkLink>::reverse_iterator subSep;
    std::list<ChunkLink>::reverse_iterator subPhrase;
  };
  const AlgorithmSetForALanguage& fConf;
  const SemanticFrameDictionary& fSemFrameDict;
  const LinguisticDictionary& fLingDico;

  void xResolveBadObjectRequests(std::list<ChunkLink>& pChunkLinks) const;

  void xModifyTheVerbsOfAChunk(std::shared_ptr<Chunk>& pChunk,
                               ChunkLinkType pFatherChunkLinkType,
                               ChunkType pPrevChunkType) const;
  void xModifyTheVerbsForStatementChildren(std::list<ChunkLink>& pChunkList) const;
  void xModifyTheVerbsForList(std::list<ChunkLink>& pChunkList,
                              ChunkLinkType pFatherChunkLinkType) const;

  void xLinkComplementaryNominalChunks
  (std::list<ChunkLink>& pChunkLinks) const;
  void xLinkComplementaryNominalChunksSeparatredByCommas
  (std::list<ChunkLink>& pChunkLinks) const;

  void xLinkSubordonateVerbs
  (std::list<ChunkLink>& pChunkLinks) const;

  void xLinkSubordinatesThatBeginWithAPreposition
  (std::list<ChunkLink>& pChunkLinks) const;

  void xLinkSubjectInfinitiveVerbs
  (std::list<ChunkLink>& pChunkLinks) const;

  void xLinkSubordonateOfCondition
  (std::list<ChunkLink>& pChunkLinks) const;

  void xLinkSubordinatesAtTheBeginning(std::list<ChunkLink>& pChunkLinks) const;

  void xLinkElseSubordonates
  (std::list<ChunkLink>& pChunkLinks) const;

  void xLinkSubordonateThatAreBeforeVerbs
  (std::list<ChunkLink>& pChunkLinks) const;

  bool xTryToLinkASubordonateVerb
  (std::list<ChunkLink>::iterator pChunkSep,
   std::list<ChunkLink>::iterator& pIt,
   std::list<ChunkLink>& pSyntTree) const;

  bool xAddOneOrAListofPastParticipleVerbs
  (std::list<ChunkLink>& pSyntTree,
   std::list<ChunkLink>::iterator& pIt,
   std::list<ChunkLink>::iterator pPrev,
   Chunk& pNewPotentialRootChunk) const;

  static void xAddASubChunk
  (std::list<ChunkLink>& pSyntTree,
   Chunk& pMotherCunk,
   std::list<ChunkLink>::iterator pPrev,
   std::list<ChunkLink>::iterator pIt,
   ChunkLinkType pType,
   SemanticLanguageEnum pLanguage);

  void xLinkSubjectAndAuxiliaryToVerbsInsideAList
  (std::list<ChunkLink>& pChunkLinks) const;

  void xComparisonsFinder
  (std::list<ChunkLink>& pChunkLinks) const;
};

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_SUBORDINATEEXTRACTOR_HPP
