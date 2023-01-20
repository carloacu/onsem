#ifndef ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASECONCEPTSSAVER_HPP
#define ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASECONCEPTSSAVER_HPP

#include <filesystem>
#include <map>
#include "binarydatabasesaver.hpp"


namespace onsem
{
class LingdbConcept;

struct ConceptsBinMem
{
  ConceptsBinMem()
    : id(0),
      alignedBegNode(0)
  {
  }

  ConceptsBinMem
  (unsigned int pId,
   unsigned int pAlignedBegNode)
    : id(pId),
      alignedBegNode(pAlignedBegNode)
  {
  }

  unsigned int id;
  unsigned int alignedBegNode;
};



/// This class save a database to a binary static database file (for SyntaticAnalyzer).
class BinaryDatabaseConceptsSaver : public BinaryDatabaseSaver
{
public:

  void saveConceptsDb
  (std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
   const std::map<std::string, LingdbConcept*>& pCptStrToCptStruct,
   const std::filesystem::path& pFilename) const;


private:
  struct TreeConceptsWorkState : public BinaryDatabaseSaver
  {
    TreeConceptsWorkState
    (std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
     const std::map<std::string, LingdbConcept*>& pCptStrToCptStruct)
      : conceptsOffsets(pConceptsOffsets),
        nextId(1),
        cptStrToCptStruct(pCptStrToCptStruct),
        cptStrToOppCpts(),
        cptStrToEquCpts()
    {
    }

    binarymasks::Ptr printEndOfANode
    (LingdbDynamicTrieNode* pNode,
     int pDecNode,
     binarymasks::Ptr pEndMemory);

    std::map<std::string, ConceptsBinMem>& conceptsOffsets;
    // an id for each concepts it's useful because
    // in another db we have array contiguous in memory of: conceptIds -> main word
    unsigned int nextId;
    const std::map<std::string, LingdbConcept*>& cptStrToCptStruct;
    std::map<std::string, std::list<binarymasks::Ptr> > cptStrToOppCpts;
    std::map<std::string, std::list<binarymasks::Ptr> > cptStrToEquCpts;
  };
};


} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASECONCEPTSSAVER_HPP
