#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_CHUNKSLINKER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_CHUNKSLINKER_HPP

#include <list>
#include <vector>
#include <memory>


namespace onsem
{
namespace linguistics
{
class LinguisticDictionary;
class AlgorithmSetForALanguage;
struct ChunkLink;
struct Chunk;
struct InflectedWord;


class ChunksLinker
{
public:
  explicit ChunksLinker(const AlgorithmSetForALanguage& pConfiguration);

  void process(std::list<ChunkLink>& pFirstChildren) const;

  bool canSubstituteAChunkByAnother(
      const ChunkLink& pPronounChkLk,
      const Chunk& pRefChunk,
      bool pRefChunkIsAtRoot) const;


private:
  struct IterToChkLink
  {
    IterToChkLink
    (std::list<ChunkLink>& pList,
     std::list<ChunkLink>::iterator pIt)
      : list(pList),
        it(pIt)
    {
    }

    std::list<ChunkLink>& list;
    std::list<ChunkLink>::iterator it;
  };
  struct PrevChunkLinkStruct
  {
    PrevChunkLinkStruct()
      : chLk(),
        sepToDelete()
    {
    }

    PrevChunkLinkStruct(PrevChunkLinkStruct&& pOther)
      : chLk(std::move(pOther.chLk)),
        sepToDelete(std::move(pOther.sepToDelete))
    {
    }

    PrevChunkLinkStruct& operator=(PrevChunkLinkStruct&& pOther)
    {
      chLk = std::move(pOther.chLk);
      sepToDelete = std::move(pOther.sepToDelete);
      return *this;
    }

    PrevChunkLinkStruct(const PrevChunkLinkStruct&) = delete;
    PrevChunkLinkStruct& operator=(const PrevChunkLinkStruct&) = delete;

    void reset()
    {
      chLk.reset();
      sepToDelete.reset();
    }

    std::unique_ptr<IterToChkLink> chLk;
    std::unique_ptr<IterToChkLink> sepToDelete;
  };
  struct History
  {
    History()
      : prevNominal(2),
        prevSubject(prevNominal.size()),
        prevDO(prevNominal.size())
    {
      _resetSubjectAndDO();
    }

    void reset()
    {
      prevNominal = std::vector<PrevChunkLinkStruct>(2);
      prevSubject = std::vector<ChunkLink*>(prevNominal.size());
      prevDO = std::vector<ChunkLink*>(prevNominal.size());
      _resetSubjectAndDO();
    }

    std::vector<PrevChunkLinkStruct> prevNominal;
    std::vector<ChunkLink*> prevSubject;
    std::vector<ChunkLink*> prevDO;

  private:
    void _resetSubjectAndDO()
    {
      for (std::size_t i = 0; i < prevNominal.size(); ++i)
      {
        prevSubject[i] = nullptr;
        prevDO[i] = nullptr;
      }
    }
  };
  const AlgorithmSetForALanguage& fConfiguration;
  const LinguisticDictionary& fLingDico;

  void _linkPronounToMorePreciseSubject(
      ChunkLink& pChunkLink,
      History& pHistory,
      std::list<Chunk*>& pStack,
      std::list<ChunkLink>& pFirstChildren) const;

  void _tryToLinkToAPrevWord(
      ChunkLink& pPronounChkLk,
      History& pHistory,
      std::list<Chunk*>& pStack) const;

  void _saveTheLink(
      ChunkLink*& pSaveLink,
      ChunkLink& pSubject) const;

  static void _linkPronounToReferentChunk(
      ChunkLink& pPronounToLink,
      ChunkLink& pReferentChunkLink);

  static void _newSentence(History& pHistory);
};

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_CHUNKSLINKER_HPP
