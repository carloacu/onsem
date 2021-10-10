#ifndef ONSEM_COMPILERMODEL_LINGDBDYNAMICTRIENODE_HXX
#define ONSEM_COMPILERMODEL_LINGDBDYNAMICTRIENODE_HXX

#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <assert.h>


namespace onsem
{


inline LingdbDynamicTrieNode* LingdbDynamicTrieNode::getFirstChild
() const
{
  return fFirstChild;
}


inline LingdbDynamicTrieNode* LingdbDynamicTrieNode::getNextBrother
() const
{
  return fNextBrother;
}

inline LingdbDynamicTrieNode* LingdbDynamicTrieNode::getFather
() const
{
  return fFather;
}



inline unsigned char LingdbDynamicTrieNode::getNbChildren
() const
{
  if (fFirstChild == nullptr)
  {
    return 0;
  }
  return fFirstChild->nbSupBrother();
}


inline char LingdbDynamicTrieNode::getLetter
() const
{
  return fLetter;
}


inline const ForwardPtrList<LingdbWordForms>* LingdbDynamicTrieNode::getWordForms
() const
{
  return fWordForms;
}

inline const ForwardPtrList<LingdbMeaning>* LingdbDynamicTrieNode::getMeaningsAtThisLemme
() const
{
  return fMeaningsAtThisLemme;
}

inline ForwardPtrList<LingdbDynamicTrieNode>* LingdbDynamicTrieNode::getMultiMeaningsNodes
() const
{
  return fMultiMeaningsNodes;
}

inline LingdbMultiMeaningsNode* LingdbDynamicTrieNode::getDatasIfItsAMultiMeaningNode
() const
{
  return fDatasIfItsAMultiMeaningNode;
}



inline void LingdbDynamicTrieNode::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbDynamicTrieNode*>
                 (pVar)->fDatasIfItsAMultiMeaningNode);
  pRes.emplace_back(&reinterpret_cast<LingdbDynamicTrieNode*>
                 (pVar)->fMeaningsAtThisLemme);
  pRes.emplace_back(&reinterpret_cast<LingdbDynamicTrieNode*>
                 (pVar)->fWordForms);
  pRes.emplace_back(&reinterpret_cast<LingdbDynamicTrieNode*>
                 (pVar)->fFather);
  pRes.emplace_back(&reinterpret_cast<LingdbDynamicTrieNode*>
                 (pVar)->fFirstChild);
  pRes.emplace_back(&reinterpret_cast<LingdbDynamicTrieNode*>
                 (pVar)->fNextBrother);
  pRes.emplace_back(&reinterpret_cast<LingdbDynamicTrieNode*>
                 (pVar)->fMultiMeaningsNodes);
}


} // End of namespace onsem



#endif // ONSEM_COMPILERMODEL_LINGDBDYNAMICTRIENODE_HXX
