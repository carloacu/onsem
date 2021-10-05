#ifndef ALLINGDBDYNAMICTRIENODE_HXX
#define ALLINGDBDYNAMICTRIENODE_HXX

#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <assert.h>


namespace onsem
{


inline ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::getFirstChild
() const
{
  return fFirstChild;
}


inline ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::getNextBrother
() const
{
  return fNextBrother;
}

inline ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::getFather
() const
{
  return fFather;
}



inline unsigned char ALLingdbDynamicTrieNode::getNbChildren
() const
{
  if (fFirstChild == nullptr)
  {
    return 0;
  }
  return fFirstChild->nbSupBrother();
}


inline char ALLingdbDynamicTrieNode::getLetter
() const
{
  return fLetter;
}


inline const ForwardPtrList<ALLingdbWordForms>* ALLingdbDynamicTrieNode::getWordForms
() const
{
  return fWordForms;
}

inline const ForwardPtrList<ALLingdbMeaning>* ALLingdbDynamicTrieNode::getMeaningsAtThisLemme
() const
{
  return fMeaningsAtThisLemme;
}

inline ForwardPtrList<ALLingdbDynamicTrieNode>* ALLingdbDynamicTrieNode::getMultiMeaningsNodes
() const
{
  return fMultiMeaningsNodes;
}

inline ALLingdbMultiMeaningsNode* ALLingdbDynamicTrieNode::getDatasIfItsAMultiMeaningNode
() const
{
  return fDatasIfItsAMultiMeaningNode;
}



inline void ALLingdbDynamicTrieNode::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbDynamicTrieNode*>
                 (pVar)->fDatasIfItsAMultiMeaningNode);
  pRes.emplace_back(&reinterpret_cast<ALLingdbDynamicTrieNode*>
                 (pVar)->fMeaningsAtThisLemme);
  pRes.emplace_back(&reinterpret_cast<ALLingdbDynamicTrieNode*>
                 (pVar)->fWordForms);
  pRes.emplace_back(&reinterpret_cast<ALLingdbDynamicTrieNode*>
                 (pVar)->fFather);
  pRes.emplace_back(&reinterpret_cast<ALLingdbDynamicTrieNode*>
                 (pVar)->fFirstChild);
  pRes.emplace_back(&reinterpret_cast<ALLingdbDynamicTrieNode*>
                 (pVar)->fNextBrother);
  pRes.emplace_back(&reinterpret_cast<ALLingdbDynamicTrieNode*>
                 (pVar)->fMultiMeaningsNodes);
}


} // End of namespace onsem



#endif // ALLINGDBDYNAMICTRIENODE_HXX
