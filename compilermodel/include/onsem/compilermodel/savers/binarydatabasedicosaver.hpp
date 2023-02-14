#ifndef ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASEDICOSAVER_HPP
#define ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASEDICOSAVER_HPP

#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include "binarydatabaseconceptssaver.hpp"


namespace onsem
{
class LinguisticIntermediaryDatabase;
class CompositePoolAllocator;
class LingdbMeaning;
class LingdbDynamicTrieNode;
class LingdbAnimationsTag;
class LingdbConcept;
class LingdbFlexions;
struct WordLinkForConj;
struct MeaningAndConfidence;


/// This class save a database to a binary static database file (for SyntaticAnalyzer).
class BinaryDatabaseDicoSaver : public BinaryDatabaseSaver
{
public:
  void save
  (std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::filesystem::path &pFilenameDatabase,
   const std::filesystem::path &pFilenameAnimationDatabase,
   const std::filesystem::path &pFilenameSynthesizerDatabase,
   const LinguisticIntermediaryDatabase& pLingDatabase) const;

  void getLemme
  (int pMeaning) const;


private:
  struct TreeCreationWorkState : public BinaryDatabaseSaver
  {
    TreeCreationWorkState
    (std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr,
     binarymasks::Ptr pBeginMeanings,
     const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
     const std::map<std::string, unsigned char>& pFlexionsPtr)
      : nodesPtr(pNodesPtr),
        beginMeanings(pBeginMeanings),
        meaningsPtr(pMeaningsPtr),
        flexionsPtr(pFlexionsPtr)
    {
    }

    binarymasks::Ptr printEndOfANode
    (LingdbDynamicTrieNode* pNode,
     int pDecNode,
     binarymasks::Ptr pEndMemory);

    std::map<LingdbDynamicTrieNode const*, int>& nodesPtr;
    binarymasks::Ptr beginMeanings;
    const std::map<const LingdbMeaning*, int>& meaningsPtr;
    const std::map<std::string, unsigned char>& flexionsPtr;
  };


  void xWriteNbWithSpaces
  (std::ofstream& pOutFile,
   std::size_t pNb) const;

  void xSaveAnimations
  (const std::filesystem::path &pFilenameAnimDatabase,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDatabase,
   binarymasks::Ptr pMem) const;

  binarymasks::Ptr xAddAnimationsTags
  (int& pNbAnimTags,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   const CompositePoolAllocator& pFPAlloc,
   binarymasks::Ptr pEndMemory) const;

  void xWriteLemmeOfMeanings
  (binarymasks::Ptr pBeginMeaning,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr) const;

  LingdbDynamicTrieNode* xGetMeaningLemma
  (const LingdbMeaning& pMeaning) const;

  void xWriteSynthesizerDb
  (std::size_t pMaxSize,
   binarymasks::Ptr pBeginMeaning,
   const std::map<const LingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr,
   const std::filesystem::path &pFilename,
   const LinguisticIntermediaryDatabase& pLingDatabase,
   SemanticLanguageEnum pLangType) const;

  binarymasks::Ptr xWriteVerbConj
  (binarymasks::Ptr pEndMemory,
   const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr,
   const std::vector<WordLinkForConj>& pTense,
   std::size_t pSize) const;

  binarymasks::Ptr xWriteNodeRef
  (binarymasks::Ptr pEndMemory,
   const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr,
   LingdbDynamicTrieNode const* pNode) const;


  /**
   * @brief Add the meanings into the memory block that will be saved in the binary file.
   * @param[out] pMeaningsPtr Save where we have saved each meanings.
   * @param pConceptsPtr Where each concept has been already saved.
   * @param pFPAlloc The allocator.
   * @param pBeginMemory Pointer at the begin of the memory block.
   * @param pEndMemory Current pointer position.
   * @return Pointer at the end of the meanings.
   */
  binarymasks::Ptr xAddMeanings
  (std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   std::map<const LingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const CompositePoolAllocator& pFPAlloc,
   const binarymasks::Ptr pBeginMemory, binarymasks::Ptr pEndMemory) const;

  binarymasks::Ptr xAddSomeFlexions
  (std::map<std::string, unsigned char>& pFlexionsPtr,
   const CompositePoolAllocator& pFPAlloc,
   const binarymasks::Ptr pBeginMemory, binarymasks::Ptr pEndMemory) const;



  binarymasks::Ptr xAddConceptsToMeanings
  (binarymasks::Ptr& pBeginMeaningsFromConcepts,
   const std::map<const LingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   binarymasks::Ptr pEndMemory) const;

  binarymasks::Ptr xWriteMeanings
  (const LingdbConcept& pConcept,
   const std::set<MeaningAndConfidence>& pMeaningsAndConf,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   binarymasks::Ptr pBegCptToMns,
   binarymasks::Ptr pBeginMeaningsFromConcepts,
   binarymasks::Ptr pEndMemory) const;

  static signed char* xGetLemme(signed char* pMeaning);

  template<typename TRULE>
  signed char* xWriteRuleWithMeanings
  (signed char* pEndMemory,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDatabase,
   TRULE* pRule,
   int pNbCharToPutZeroIfEmpty) const;

};

} // End of namespace onsem

#include "details/binarydatabasedicosaver.hxx"

#endif // ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASEDICOSAVER_HPP
