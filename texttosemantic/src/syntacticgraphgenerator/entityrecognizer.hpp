#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_ENTITYRECOGNIZER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_ENTITYRECOGNIZER_HPP

#include <list>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>


namespace onsem
{
namespace linguistics
{
struct Token;
class StaticLinguisticDictionary;
class AlgorithmSetForALanguage;
struct ChunkLinkWorkingZone;
struct Chunk;


class EntityRecognizer
{
public:
  explicit EntityRecognizer(const AlgorithmSetForALanguage& pConfiguration);

  void process(std::list<ChunkLink>& pChunkList) const;

  ChunkLinkType findNatureOfAChunkLink
  (ChunkLink& pChunkLink,
   Chunk* pFirstVerbChunk,
   bool pFromEnglishPossessive = false) const;

  void addSubordonatesToAVerb
  (Chunk& pVerbRoot,
   ChunkLinkWorkingZone& pWorkingZone) const;


  /**
    * @brief Get the appropriate chunk link from a context.
    * @param pIGramVerbBefore Mother verb of the main word.
    * @param pIGramMainWord The main word for which we search for the chunk link.
    * @param pChunkType The chunk type holding the main word.
    * @param pNextToken Token after the main word.
    * @return The corresponding chunk link. It can not be found and then be unset.
    */
  mystd::optional<ChunkLinkType> getAppropriateChunkLink
  (InflectedWord* pVerbInflectedWord,
   mystd::optional<const SemanticWord*>& pIntroductingWord,
   const InflectedWord* pPrepInflWordPtr,
   ChunkType pChunkType,
   const ConstTokenIterator* pNextToken) const;

  static bool isTheBeginOfAPartitive(const Chunk& pChunk);

  bool pronounCompIsReflexiveOfASubject(const InflectedWord& pSubjectInflWord,
                                        const InflectedWord& pPronounCompInflWord) const;



private:
  const AlgorithmSetForALanguage& fConfiguration;
  const SpecificLinguisticDatabase& fSpecLingDb;
  const LinguisticDictionary& fLingDico;

  ChunkLinkType xFindNatureOfANominalGroup
  (ChunkLink& pChunkLink,
   Chunk* pFirstVerbChunk,
   bool pFromEnglishPossessive) const;

  ChunkLinkType xGetChkLinkTypeFromChunkLink
  (const ChunkLink& pChunkLink,
   Chunk* pFirstVerbChunk) const;

  ChunkLinkType xVerbFollowedBy(const StaticLinguisticMeaning& pVerbMeaningMeaning) const;

  void xAddComplementsOfVerbFromBegin
  (Chunk& pRootVerb,
   ChunkLink* pSubjectChunkLink,
   Chunk& pCurrentChunk,
   Chunk& pChunkToSplit,
   TokIt pItBeforeBegin,
   TokIt pItEnd) const;

  void xAddComplementsOfVerbFromEnd
  (Chunk& pRootVerb,
   ChunkLink* pSubjectChunkLink,
   Chunk& pCurrentChunk,
   Chunk& pChunkToSplit,
   TokIt pItBegin,
   TokIt pItEnd,
   ChunkLinkType pVerbCanBeFollowedBy) const;

  void xLinkPronounComplementBeforeVerb
  (Chunk& pRootVerb,
   ChunkLink* pSubjectChunkLink,
   Chunk& pCurrentChunk,
   Chunk& pChunkToSplit,
   TokIt pPronounComplement,
   ChunkLinkType pVerbCanBeFollowedBy) const;

  void xLinkPronounComplementAfterVerb
  (Chunk& pRootVerb,
   ChunkLink* pSubjectChunkLink,
   Chunk& pCurrentChunk,
   Chunk& pChunkToSplit,
   TokIt pPronounComplement) const;
};

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_ENTITYRECOGNIZER_HPP
