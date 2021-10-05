#ifndef ALBINARYDATABASEDICOSAVER_H
#define ALBINARYDATABASEDICOSAVER_H

#include <map>
#include <string>
#include <vector>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include "albinarydatabaseconceptssaver.hpp"


namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALCompositePoolAllocator;
class ALLingdbMeaning;
class ALLingdbDynamicTrieNode;
class ALLingdbAnimationsTag;
class ALLingdbConcept;
class ALLingdbFlexions;
struct WordLinkForConj;
class ALBinaryDatabaseConceptsSaver;
struct MeaningAndConfidence;


/// This class save a database to a binary static database file (for SyntaticAnalyzer).
class ALBinaryDatabaseDicoSaver : public ALBinaryDatabaseSaver
{
public:
  void save
  (std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const boost::filesystem::path& pFilenameDatabase,
   const boost::filesystem::path& pFilenameAnimationDatabase,
   const boost::filesystem::path& pFilenameSynthesizerDatabase,
   const LinguisticIntermediaryDatabase& pLingDatabase) const;

  void getLemme
  (int pMeaning) const;


private:
  struct TreeCreationWorkState : public ALBinaryDatabaseSaver
  {
    TreeCreationWorkState
    (std::map<ALLingdbDynamicTrieNode const*, int>& pNodesPtr,
     binarymasks::Ptr pBeginMeanings,
     const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
     const std::map<std::string, unsigned char>& pFlexionsPtr)
      : nodesPtr(pNodesPtr),
        beginMeanings(pBeginMeanings),
        meaningsPtr(pMeaningsPtr),
        flexionsPtr(pFlexionsPtr)
    {
    }

    binarymasks::Ptr printEndOfANode
    (ALLingdbDynamicTrieNode* pNode,
     int pDecNode,
     binarymasks::Ptr pEndMemory);

    std::map<ALLingdbDynamicTrieNode const*, int>& nodesPtr;
    binarymasks::Ptr beginMeanings;
    const std::map<const ALLingdbMeaning*, int>& meaningsPtr;
    const std::map<std::string, unsigned char>& flexionsPtr;
  };


  void xWriteNbWithSpaces
  (std::ofstream& pOutFile,
   std::size_t pNb) const;

  void xSaveAnimations
  (const boost::filesystem::path& pFilenameAnimDatabase,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDatabase,
   binarymasks::Ptr pMem) const;

  binarymasks::Ptr xAddAnimationsTags
  (int& pNbAnimTags,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   const ALCompositePoolAllocator& pFPAlloc,
   binarymasks::Ptr pEndMemory) const;

  void xWriteLemmeOfMeanings
  (binarymasks::Ptr pBeginMeaning,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   const std::map<ALLingdbDynamicTrieNode const*, int>& pNodesPtr) const;

  ALLingdbDynamicTrieNode* xGetMeaningLemma
  (const ALLingdbMeaning& pMeaning) const;

  void xWriteSynthesizerDb
  (std::size_t pMaxSize,
   binarymasks::Ptr pBeginMeaning,
   const std::map<const ALLingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   const std::map<ALLingdbDynamicTrieNode const*, int>& pNodesPtr,
   const boost::filesystem::path& pFilename,
   const LinguisticIntermediaryDatabase& pLingDatabase,
   SemanticLanguageEnum pLangType) const;

  binarymasks::Ptr xWriteVerbConj
  (binarymasks::Ptr pEndMemory,
   const std::map<ALLingdbDynamicTrieNode const*, int>& pNodesPtr,
   const std::vector<WordLinkForConj>& pTense,
   std::size_t pSize) const;

  binarymasks::Ptr xWriteNodeRef
  (binarymasks::Ptr pEndMemory,
   const std::map<ALLingdbDynamicTrieNode const*, int>& pNodesPtr,
   ALLingdbDynamicTrieNode const* pNode) const;


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
  (std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   std::map<const ALLingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const ALCompositePoolAllocator& pFPAlloc,
   const binarymasks::Ptr pBeginMemory, binarymasks::Ptr pEndMemory) const;

  binarymasks::Ptr xAddSomeFlexions
  (std::map<std::string, unsigned char>& pFlexionsPtr,
   const ALCompositePoolAllocator& pFPAlloc,
   const binarymasks::Ptr pBeginMemory, binarymasks::Ptr pEndMemory) const;



  binarymasks::Ptr xAddConceptsToMeanings
  (binarymasks::Ptr& pBeginMeaningsFromConcepts,
   const std::map<const ALLingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   binarymasks::Ptr pEndMemory) const;

  binarymasks::Ptr xWriteMeanings
  (const ALLingdbConcept& pConcept,
   const std::set<MeaningAndConfidence>& pMeaningsAndConf,
   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   binarymasks::Ptr pBegCptToMns,
   binarymasks::Ptr pBeginMeaningsFromConcepts,
   binarymasks::Ptr pEndMemory) const;

  static signed char* xGetLemme(signed char* pMeaning);

  template<typename TRULE>
  signed char* xWriteRuleWithMeanings
  (signed char* pEndMemory,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDatabase,
   TRULE* pRule,
   int pNbCharToPutZeroIfEmpty) const;

};

} // End of namespace onsem

#include "details/albinarydatabasedicosaver.hxx"

#endif // ALBINARYDATABASEDICOSAVER_H
