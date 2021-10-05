#include <onsem/lingdbeditor/savers/albinarydatabaseconceptssaver.hpp>
#include <boost/filesystem/fstream.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include "../concept/allingdbconcept.hpp"



namespace onsem
{


void ALBinaryDatabaseConceptsSaver::saveConceptsDb
(std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
 const std::map<std::string, ALLingdbConcept*>& pCptStrToCptStruct,
 const boost::filesystem::path& pFilename) const
{
  const unsigned int version = 10;
  boost::filesystem::ofstream outfile(pFilename,
                                      boost::filesystem::ofstream::binary);

  // fill a lingdb with all the concepts names
  LinguisticIntermediaryDatabase conceptWords;
  TreeConceptsWorkState treeConceptsWS(pConceptsOffsets, pCptStrToCptStruct);
  for (const auto& currCpt : pCptStrToCptStruct)
    conceptWords.addWord(currCpt.first, currCpt.first, PartOfSpeech::NOUN,
                         std::vector<std::string>(), 4);

  // allocate the memory (that will be flushed into the file)
  std::size_t maxSize = conceptWords.getFPAlloc().getOccupatedSize();
  binarymasks::Ptr mem = ::operator new(maxSize);
  binarymasks::Ptr endMemory = mem;
  assert(endMemory.val == binarysaver::alignMemory(endMemory).val);

  // write the formalism id
  binarysaver::writeInt(endMemory.pchar, fFormalism);
  endMemory.val += sizeof(int);

  // write if the version of the database
  binarysaver::writeInt(endMemory.pchar, version);
  endMemory.val += sizeof(int);

  // put a place where we will write the size of the file
  binarymasks::Ptr sizeConcepts = endMemory;
  binarysaver::writeInt(endMemory.pchar, 0);
  endMemory.val += sizeof(int);

  // write all the concepts
  binarymasks::Ptr beginConcepts = endMemory;
  endMemory = xCreateRootNode(treeConceptsWS,
                              conceptWords.getFPAlloc().first<ALLingdbDynamicTrieNode>(),
                              endMemory, endMemory);

  auto writeConceptsPtrs = [this, &treeConceptsWS](const std::map<std::string, std::list<binarymasks::Ptr>>& pCptStrToCpts)
  {
    for (const auto& currEquCpts : pCptStrToCpts)
    {
      auto itCptLink = treeConceptsWS.conceptsOffsets.find(currEquCpts.first);
      assert(itCptLink != treeConceptsWS.conceptsOffsets.end());
      for (const binarymasks::Ptr& currSpaceToWriteTheCpt : currEquCpts.second)
        binarysaver::writeInThreeBytes(currSpaceToWriteTheCpt.pchar, itCptLink->second.alignedBegNode);
    }
  };

  // write the opposite of concepts
  writeConceptsPtrs(treeConceptsWS.cptStrToOppCpts);

  // write the nearly equal concepts
  writeConceptsPtrs(treeConceptsWS.cptStrToEquCpts);

  // write the size of the file
  binarysaver::writeInt(sizeConcepts.pchar, binarysaver::alignedDecToSave(endMemory.val - beginConcepts.val));

  // Flush all the memory into the file
  outfile.write(reinterpret_cast<const char*>(mem.ptr),
                endMemory.val - mem.val);

  outfile.close();
  ::operator delete(mem.ptr);
}



binarymasks::Ptr ALBinaryDatabaseConceptsSaver::TreeConceptsWorkState::printEndOfANode
(ALLingdbDynamicTrieNode* pNode,
 int pDecNode,
 binarymasks::Ptr pEndMemory)
{
  pEndMemory = binarysaver::alignMemory(pEndMemory);
  std::string conceptStr = pNode->getWord();
  conceptsOffsets[conceptStr] = ConceptsBinMem(nextId, pDecNode);
  // write id of the concept
  binarysaver::writeInt(pEndMemory.pchar, nextId);
  pEndMemory.val += sizeof(unsigned int);
  ++nextId;

  // get concept object
  auto itToStruct = cptStrToCptStruct.find(conceptStr);
  assert(itToStruct != cptStrToCptStruct.end());
  const ALLingdbConcept* cptStruct = itToStruct->second;

  // write number of opposite concepts
  const ForwardPtrList<ALLingdbConcept>* oppCpts =
      cptStruct->getOppositeConcepts();
  if (oppCpts != nullptr)
    binarysaver::writeChar(pEndMemory.pchar++, oppCpts->length());
  else
    binarysaver::writeChar(pEndMemory.pchar++, 0);

  // write number of nearly equal concepts
  const ForwardPtrList<ALLingdbConcept>* equCpts =
      cptStruct->getNearlyEqualConcepts();
  if (equCpts != nullptr)
    binarysaver::writeChar(pEndMemory.pchar++, equCpts->length());
  else
    binarysaver::writeChar(pEndMemory.pchar++, 0);

  pEndMemory = binarysaver::alignMemory(pEndMemory);


  // add space to write the opposite concepts
  while (oppCpts != nullptr)
  {
    cptStrToOppCpts[oppCpts->elt->getName()->toStr()].push_back(pEndMemory);
    binarysaver::writeInt(pEndMemory.pchar, 0);
    pEndMemory.val += sizeof(int);
    oppCpts = oppCpts->next;
  }

  // add space to write the nearly equal concepts
  while (equCpts != nullptr)
  {
    cptStrToEquCpts[equCpts->elt->getName()->toStr()].push_back(pEndMemory);
    binarysaver::writeInt(pEndMemory.pchar, 0);
    pEndMemory.val += sizeof(int);
    equCpts = equCpts->next;
  }

  return pEndMemory;
}


} // End of namespace onsem
