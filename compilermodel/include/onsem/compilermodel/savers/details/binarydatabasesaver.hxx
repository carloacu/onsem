#ifndef ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASESAVER_HXX
#define ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASESAVER_HXX

#include <onsem/compilermodel/savers/binarydatabasesaver.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <assert.h>


namespace onsem
{


template<typename T>
binarymasks::Ptr BinaryDatabaseSaver::xCreateRootNode
(T& pTreeCreationWorkState,
 LingdbDynamicTrieNode* pNode,
 const binarymasks::Ptr pBeginMemory, binarymasks::Ptr pEndMemory) const
{
  binarymasks::Ptr beginNode = pEndMemory;

  assert(pEndMemory.val % 4 == 0);

  // write parent node
  binarysaver::writeInThreeBytes(pEndMemory.pchar, 0);
  pEndMemory.val += 3;

  // Print number of letters
  binarysaver::writeChar(pEndMemory.pchar++, 0);

  // Print nb of meanings
  binarysaver::writeChar(pEndMemory.pchar++, 0);

  // Print nb of children
  char nbChildren = binarysaver::writeChar(pEndMemory.pchar++, pNode->nbSupBrother());

  pEndMemory = binarysaver::alignMemory(pEndMemory);

  // Memory allocation to write the children
  binarymasks::Ptr beginOfChildren = pEndMemory;
  pEndMemory.val += std::size_t(nbChildren * 4);
  assert(pEndMemory.val % 4 == 0);

  // Print each child
  return xIterateOnChildren(pTreeCreationWorkState,
                            binarysaver::alignedDecToSave(beginNode.val - pBeginMemory.val),
                            pNode, pBeginMemory, beginOfChildren, pEndMemory);
}


template<typename T>
binarymasks::Ptr BinaryDatabaseSaver::xIterateOnChildren
(T& pTreeCreationWorkState,
 int pParentPtr,
 LingdbDynamicTrieNode* pNode,
 const binarymasks::Ptr pBeginMemory,
 binarymasks::Ptr pChildPtr,
 binarymasks::Ptr pEndMemory) const
{
  while (pNode != nullptr)
  {
    // offset of the child node
    binarysaver::writeInThreeBytes(pChildPtr.pchar,
                     binarysaver::alignedDecToSave(pEndMemory.val - pBeginMemory.val));
    pChildPtr.val += 3;
    // first letter of the child nod
    binarysaver::writeChar(pChildPtr.pchar++, pNode->getLetter());
    pEndMemory = xAddANode(pTreeCreationWorkState, pParentPtr,
                           pNode, pBeginMemory, pEndMemory);
    pNode = pNode->getNextBrother();
  }
  return pEndMemory;
}


template<typename T>
binarymasks::Ptr BinaryDatabaseSaver::xAddANode
(T& pTreeCreationWorkState,
 int pParentPtr,
 LingdbDynamicTrieNode* pNode,
 const binarymasks::Ptr pBeginMemory,
 binarymasks::Ptr pEndMemory) const
{
  assert(pEndMemory.val % 4 == 0);
  int decNode = binarysaver::alignedDecToSave(pEndMemory.val - pBeginMemory.val);

  // write parent node
  binarysaver::writeInThreeBytes(pEndMemory.pchar, pParentPtr);
  pEndMemory.val += 3;

  // Print number of letters
  unsigned char nbLetters = binarysaver::writeChar(pEndMemory.pchar++, xMaxNbOfLettersInTheNode(pNode));

  // Print each letters
  for (char i = 0; i < nbLetters; ++i)
  {
    pNode = pNode->getFirstChild();
    binarysaver::writeChar(pEndMemory.pchar++, pNode->getLetter());
  }

  // Print nb of meanings
  std::list<std::pair<LingdbWordForms*, LingdbMeaning*> > wordFromsOrMeanings;
  pNode->getWordFormsAndMeanings(wordFromsOrMeanings);
  char nbMeanings = binarysaver::writeChar(pEndMemory.pchar++,
                                           static_cast<char>(wordFromsOrMeanings.size()));

  // Print nb of children
  char nbChildren = binarysaver::writeChar(pEndMemory.pchar++, pNode->getNbChildren());

  pEndMemory = binarysaver::alignMemory(pEndMemory);

  // Memory allocation to write the children
  binarymasks::Ptr beginOfChildren = pEndMemory;
  pEndMemory.val += std::size_t(nbChildren * 4);

  // Print node content
  if (nbMeanings > 0)
  {
    pEndMemory = pTreeCreationWorkState.printEndOfANode(pNode, decNode, pEndMemory);
  }
  assert(pEndMemory.val % 4 == 0);

  if (nbChildren > 0)
  {
    // Print each child
    return xIterateOnChildren(pTreeCreationWorkState, decNode,
                            pNode->getFirstChild(), pBeginMemory,
                            beginOfChildren, pEndMemory);
  }
  return pEndMemory;
}


} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASESAVER_HXX
