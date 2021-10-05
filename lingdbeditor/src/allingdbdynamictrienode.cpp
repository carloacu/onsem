#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <assert.h>
#include <list>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/allingdbwordforms.hpp>


namespace onsem
{

ALLingdbDynamicTrieNode::ALLingdbDynamicTrieNode(char pLetter)
  : fEndOfWord(false),
    fLetter(pLetter),
    fDatasIfItsAMultiMeaningNode(nullptr),
    fMeaningsAtThisLemme(nullptr),
    fWordForms(nullptr),
    fFather(nullptr),
    fFirstChild(nullptr),
    fNextBrother(nullptr),
    fMultiMeaningsNodes(nullptr)
{
}

void ALLingdbDynamicTrieNode::xInit
(char pLetter)
{
  fEndOfWord = false;
  fLetter = pLetter;
  fDatasIfItsAMultiMeaningNode = nullptr;
  fMeaningsAtThisLemme = nullptr;
  fWordForms = nullptr;
  fFather = nullptr;
  fFirstChild = nullptr;
  fNextBrother = nullptr;
  fMultiMeaningsNodes = nullptr;
}


void ALLingdbDynamicTrieNode::putAWordFormAtTheTopOfTheList
(PartOfSpeech pGram,
 const std::string& pLemme)
{
  if (fWordForms == nullptr)
  {
    return;
  }
  ForwardPtrList<ALLingdbWordForms>* prev = nullptr;
  ForwardPtrList<ALLingdbWordForms>* curr = fWordForms;
  while (curr != nullptr)
  {
    if (curr->elt->getMeaning()->getPartOfSpeech() == pGram &&
        (pLemme.empty() || curr->elt->getMeaning()->getLemma()->getWord() == pLemme))
    {
      if (prev != nullptr)
      {
        prev->next = curr->next;
        curr->next = fWordForms;
        fWordForms = curr;
      }
      return;
    }
    prev = curr;
    curr = curr->next;
  }

  // if lemme exist but not found,
  // we focus only on the grammatical specification
  if (!pLemme.empty())
  {
    putAWordFormAtTheTopOfTheList(pGram, "");
  }
}



ALLingdbMeaning* ALLingdbDynamicTrieNode::getMeaning
(PartOfSpeech pPartOfSpeech) const
{
  ForwardPtrList<ALLingdbMeaning>* itMeaning = fMeaningsAtThisLemme;
  while (itMeaning != nullptr)
  {
    if (itMeaning->elt->getPartOfSpeech() == pPartOfSpeech)
    {
      return itMeaning->elt;
    }
    itMeaning = itMeaning->next;
  }
  return nullptr;
}


ALLingdbWordForms* ALLingdbDynamicTrieNode::getWordForm
(const std::string& pLemme,
 PartOfSpeech pPartOfSpeech) const
{
  ForwardPtrList<ALLingdbWordForms>* itWordForm = fWordForms;
  while (itWordForm != nullptr)
  {
    if (itWordForm->elt->getMeaning()->getPartOfSpeech() == pPartOfSpeech &&
        itWordForm->elt->getMeaning()->getLemma()->getWord() == pLemme)
    {
      return itWordForm->elt;
    }
    itWordForm = itWordForm->next;
  }
  return nullptr;
}


ALLingdbWordForms* ALLingdbDynamicTrieNode::getWordFormFromMeaning
(ALLingdbMeaning& pMeaning) const
{
  ForwardPtrList<ALLingdbWordForms>* itWordForm = fWordForms;
  while (itWordForm != nullptr)
  {
    if (itWordForm->elt->getMeaning() == &pMeaning)
    {
      return itWordForm->elt;
    }
    itWordForm = itWordForm->next;
  }
  return nullptr;
}



unsigned char ALLingdbDynamicTrieNode::nbMeaningsAtThisLemme
() const
{
  if (fMeaningsAtThisLemme == nullptr)
  {
    return 0;
  }
  return fMeaningsAtThisLemme->length();
}

unsigned char ALLingdbDynamicTrieNode::nbWordForms
() const
{
  if (fWordForms == nullptr)
  {
    return 0;
  }
  return fWordForms->length();
}


void ALLingdbDynamicTrieNode::getWordFormsAndMeanings
(std::list<std::pair<ALLingdbWordForms*, ALLingdbMeaning*> >& pWordFromsOrMeanings) const
{
  std::set<ALLingdbMeaning*> meaningsAlreadyAdded;
  ForwardPtrList<ALLingdbWordForms>* itWordForm = fWordForms;
  while (itWordForm != nullptr)
  {
    meaningsAlreadyAdded.insert(itWordForm->elt->getMeaning());
    pWordFromsOrMeanings.emplace_back
        (std::pair<ALLingdbWordForms*, ALLingdbMeaning*>(itWordForm->elt, nullptr));
    itWordForm = itWordForm->next;
  }

  ForwardPtrList<ALLingdbMeaning>* itMeaning = fMeaningsAtThisLemme;
  while (itMeaning != nullptr)
  {
    if (meaningsAlreadyAdded.find(itMeaning->elt) == meaningsAlreadyAdded.end())
    {
      pWordFromsOrMeanings.emplace_back
          (std::pair<ALLingdbWordForms*, ALLingdbMeaning*>(nullptr, itMeaning->elt));
    }
    itMeaning = itMeaning->next;
  }
}


ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::xAddMultiMeaningsNode
(ALCompositePoolAllocator& pAlloc,
 ALLingdbMeaning* pRootMeaning,
 std::list<std::pair<ALLingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings)
{
  ALLingdbDynamicTrieNode* newNode = pAlloc.allocate<ALLingdbDynamicTrieNode>(1);
  newNode->xInit(0);
  newNode->fFather = this;
  newNode->fEndOfWord = true;
  newNode->fDatasIfItsAMultiMeaningNode = pAlloc.allocate<ALLingdbMultiMeaningsNode>(1);
  newNode->fDatasIfItsAMultiMeaningNode->xInit(pAlloc, pRootMeaning, pLinkedMeanings);

  ForwardPtrList<ALLingdbDynamicTrieNode>* newNodeList =
      pAlloc.allocate<ForwardPtrList<ALLingdbDynamicTrieNode> >(1);
  newNodeList->init(newNode);
  newNodeList->next = fMultiMeaningsNodes;
  fMultiMeaningsNodes = newNodeList;
  return newNode;
}


ALLingdbMeaning* ALLingdbDynamicTrieNode::xAddMeaning
(ALCompositePoolAllocator& pFPAlloc,
 PartOfSpeech pPartOfSpeech)
{
  ALLingdbMeaning* meaning = getMeaning(pPartOfSpeech);
  if (meaning == nullptr)
  {
    meaning = pFPAlloc.allocate<ALLingdbMeaning>(1);
    meaning->xInit(this, static_cast<char>(pPartOfSpeech));
    ForwardPtrList<ALLingdbMeaning>* newMeaningList =
        pFPAlloc.allocate<ForwardPtrList<ALLingdbMeaning> >(1);
    newMeaningList->init(meaning);
    newMeaningList->next = fMeaningsAtThisLemme;
    fMeaningsAtThisLemme = newMeaningList;
  }
  return meaning;
}


ALLingdbWordForms* ALLingdbDynamicTrieNode::addWordForm
(LinguisticIntermediaryDatabase& pLingDatabase,
 const std::string& pLemma,
 PartOfSpeech pPartOfSpeech,
 bool pAtFront)
{
  if (!fEndOfWord)
  {
    return nullptr;
  }
  ALLingdbWordForms* wordForm = getWordForm(pLemma, pPartOfSpeech);
  if (wordForm == nullptr)
  {
    ALCompositePoolAllocator& alloc = pLingDatabase.xGetFPAlloc();
    return xAddWordFormFromMeaning
        (alloc,
         *pLingDatabase.getRoot()->xInsertWord(alloc, pLemma, 0)->xAddMeaning(alloc, pPartOfSpeech),
         pAtFront);
  }
  return wordForm;
}


ALLingdbWordForms* ALLingdbDynamicTrieNode::addWordFormFromMeaning
(LinguisticIntermediaryDatabase& pLingDatabase,
 ALLingdbMeaning& pMeaning,
 bool pAtFront)
{
  if (!fEndOfWord)
  {
    return nullptr;
  }
  ALLingdbWordForms* wordForm = getWordFormFromMeaning(pMeaning);
  if (wordForm == nullptr)
  {
    return xAddWordFormFromMeaning(pLingDatabase.xGetFPAlloc(),
                                   pMeaning, pAtFront);
  }
  return wordForm;
}


ALLingdbWordForms* ALLingdbDynamicTrieNode::xAddWordFormFromMeaning
(ALCompositePoolAllocator& pAlloc,
 ALLingdbMeaning& pMeaning,
 bool pAtFront)
{
  ALLingdbWordForms* wordForm = pAlloc.allocate<ALLingdbWordForms>(1);
  wordForm->xInit(&pMeaning);
  ForwardPtrList<ALLingdbWordForms>* newWordFormList =
      pAlloc.allocate<ForwardPtrList<ALLingdbWordForms> >(1);
  newWordFormList->init(wordForm);
  if (pAtFront)
  {
    newWordFormList->next = fWordForms;
    fWordForms = newWordFormList;
  }
  else
  {
    newWordFormList->next = nullptr;
    if (fWordForms != nullptr)
    {
      fWordForms->back()->next = newWordFormList;
    }
    else
    {
      fWordForms = newWordFormList;
    }
  }
  return wordForm;
}




ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::xInsertWord
(ALCompositePoolAllocator& pFPAlloc,
 const std::string& pWord,
 std::size_t pOffset)
{
  // if osset is out of the word, we have nothing to do
  if (pOffset >= pWord.size())
  {
    assert(false);
    return nullptr;
  }

  assert(pWord[pOffset] >= fLetter);
  // our node == the next letter that we have to insert
  if (pWord[pOffset] == fLetter)
  {
    // if this is the end of the word
    if (pOffset == pWord.size() - 1)
    {
      fEndOfWord = true;
      return this;
    }
    else // skip this node and call the first child to continue the insertion
    {
      // if we have to insert a new node before the first child
      if (fFirstChild == nullptr || fFirstChild->getLetter() > pWord[pOffset + 1])
      {
        // add a new node
        ALLingdbDynamicTrieNode* SecondChild = fFirstChild;
        fFirstChild = pFPAlloc.allocate<ALLingdbDynamicTrieNode>(1);
        fFirstChild->xInit(pWord[pOffset + 1]);
        fFirstChild->fFather = this;
        fFirstChild->fNextBrother = SecondChild;
      }
      // call the first child to continue the insertion
      return fFirstChild->xInsertWord(pFPAlloc, pWord, pOffset + 1);
    }
  }
  else // the letter to insert has to be after the current node
  {
    // there is no brother OR the insertion has to be before the next brother
    if (fNextBrother == nullptr || pWord[pOffset] < fNextBrother->getLetter())
    {
      // add a new node
      ALLingdbDynamicTrieNode* secondNextBrother = fNextBrother;
      fNextBrother = pFPAlloc.allocate<ALLingdbDynamicTrieNode>(1);
      fNextBrother->xInit(pWord[pOffset]);
      fNextBrother->fFather = fFather;
      fNextBrother->fNextBrother = secondNextBrother;
    }
    // call the next brother to continue the insertion
    return fNextBrother->xInsertWord(pFPAlloc, pWord, pOffset);
  }
}


void ALLingdbDynamicTrieNode::xRemoveMeaning
(ALCompositePoolAllocator& pFPAlloc,
 PartOfSpeech pPartOfSpeech)
{
  ForwardPtrList<ALLingdbMeaning>* prev = nullptr;
  ForwardPtrList<ALLingdbMeaning>* meaning = fMeaningsAtThisLemme;
  while (meaning != nullptr)
  {
    if (meaning->elt->getPartOfSpeech() == pPartOfSpeech)
    {
      if (prev == nullptr)
      {
        fMeaningsAtThisLemme = meaning->next;
      }
      else
      {
        prev->next = meaning->next;
      }
      pFPAlloc.deallocate<ForwardPtrList<ALLingdbMeaning> >(meaning);
      break;
    }
    prev = meaning;
    meaning = meaning->next;
  }

  xRemoveWordIfNoLongerAWord(pFPAlloc);
}


void ALLingdbDynamicTrieNode::xRemoveAllWordForms
(ALCompositePoolAllocator& pAlloc)
{
  assert(fEndOfWord);
  if (fWordForms != nullptr)
  {
    fWordForms->clearComposedElts(pAlloc);
    fWordForms = nullptr;
  }
  xRemoveWordIfNoLongerAWord(pAlloc);
}



void ALLingdbDynamicTrieNode::xRemoveWordForm
(ALCompositePoolAllocator& pAlloc,
 PartOfSpeech pPartOfSpeech)
{
  assert(fEndOfWord);
  ForwardPtrList<ALLingdbWordForms>* prev = nullptr;
  ForwardPtrList<ALLingdbWordForms>* curr = fWordForms;
  while (curr != nullptr)
  {
    if (curr->elt->getMeaning()->getPartOfSpeech() == pPartOfSpeech)
    {
      if (prev == nullptr)
        ForwardPtrList<ALLingdbWordForms>::clearNextComposedElt
            (fWordForms, pAlloc);
      else
        ForwardPtrList<ALLingdbWordForms>::clearNextComposedElt
            (prev->next, pAlloc);
      break;
    }
    prev = curr;
    curr = curr->next;
  }

  xRemoveWordIfNoLongerAWord(pAlloc);
}


ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::advanceInTrieIfEndOfAWord
(const std::string& word) const
{
  ALLingdbDynamicTrieNode* node = advanceInTrie(word, true);
  if (node == nullptr || node->fEndOfWord == false)
  {
    return nullptr;
  }
  return node;
}



ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::advanceInTrie
(const std::string& word, bool pUntilEndOfWord) const
{
  std::size_t off = 0;
  ALLingdbDynamicTrieNode const* prevNode = this;
  ALLingdbDynamicTrieNode const* node = prevNode->xGetBrother(word[off]);
  while (off < word.size() && node != nullptr)
  {
    prevNode = node;
    node = node->xGetChild(word[++off]);
  }
  if (!pUntilEndOfWord || off == word.size())
  {
    return const_cast<ALLingdbDynamicTrieNode*>(prevNode);
  }
  return nullptr;
}


void ALLingdbDynamicTrieNode::xRemoveWordIfNoLongerAWord
(ALCompositePoolAllocator& pFPAlloc)
{
  if (fEndOfWord && fMeaningsAtThisLemme == nullptr && fWordForms == nullptr &&
      fMultiMeaningsNodes == nullptr)
  {
    fEndOfWord = false;
    xTryToDeallocateNodes(pFPAlloc);
  }
}

void ALLingdbDynamicTrieNode::xTryToDeallocateNodes
(ALCompositePoolAllocator& pFPAlloc)
{
  if (fEndOfWord || fFirstChild != nullptr ||
      pFPAlloc.first<ALLingdbDynamicTrieNode>() == this)
  {
    return;
  }
  ALLingdbDynamicTrieNode* currNode;
  if (fFather == nullptr)
  {
    currNode = pFPAlloc.first<ALLingdbDynamicTrieNode>();
  }
  else
  {
    currNode = fFather->fFirstChild;
    if (currNode == this)
    {
      fFather->fFirstChild = fNextBrother;
      fFather->xTryToDeallocateNodes(pFPAlloc);
      pFPAlloc.deallocate<ALLingdbDynamicTrieNode>(this);
      return;
    }
  }
  while (currNode->fNextBrother != this &&
         currNode->fNextBrother != nullptr) // Because it's not found if we remove a composed word
  {
    currNode = currNode->fNextBrother;
  }

  if (currNode->fNextBrother != nullptr)
  {
    currNode->fNextBrother = fNextBrother;
  }
  else
  {
    // if we are here, this means we deleting a multi words meanings
    ForwardPtrList<ALLingdbDynamicTrieNode>* prevMultiMeaningsNodes = nullptr;
    ForwardPtrList<ALLingdbDynamicTrieNode>* multiMeaningsNodes = fFather->fMultiMeaningsNodes;
    while (multiMeaningsNodes != nullptr)
    {
      if (multiMeaningsNodes->elt == this)
      {
        if (prevMultiMeaningsNodes == nullptr)
        {
          currNode->fMultiMeaningsNodes = multiMeaningsNodes->next;
        }
        else
        {
          prevMultiMeaningsNodes->next = multiMeaningsNodes->next;
        }
        pFPAlloc.deallocate<ForwardPtrList<ALLingdbDynamicTrieNode> >(multiMeaningsNodes);
        break;
      }
      prevMultiMeaningsNodes = multiMeaningsNodes;
      multiMeaningsNodes = multiMeaningsNodes->next;
    }
    fFather->xRemoveWordIfNoLongerAWord(pFPAlloc);
  }

  pFPAlloc.deallocate<ALLingdbDynamicTrieNode>(this);
}



ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::getNextWordNode
() const
{
  if (fFirstChild != nullptr)
  {
    return fFirstChild->xGetWordNode();
  }

  ALLingdbDynamicTrieNode const* currNode = this;
  do
  {
    if (currNode->fNextBrother != nullptr)
    {
      return currNode->fNextBrother->xGetWordNode();
    }
    currNode = currNode->fFather;
  } while (currNode != nullptr);

  return nullptr;
}



ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::xGetWordNode
() const
{
  if (fEndOfWord)
  {
    return const_cast<ALLingdbDynamicTrieNode*>(this);
  }
  return getNextWordNode();
}

ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::xGetChild
(char pLetter) const
{
  if (fFirstChild == nullptr)
  {
    return nullptr;
  }
  return fFirstChild->xGetBrother(pLetter);
}


ALLingdbDynamicTrieNode* ALLingdbDynamicTrieNode::xGetBrother
(char pLetter) const
{
  ALLingdbDynamicTrieNode const* children = this;
  while (children != nullptr)
  {
    if (children->fLetter > pLetter)
    {
      return nullptr;
    }
    else if (children->fLetter == pLetter)
    {
      return const_cast<ALLingdbDynamicTrieNode*>(children);
    }
    children = children->fNextBrother;
  }
  return nullptr;
}



unsigned char ALLingdbDynamicTrieNode::nbSupBrother
() const
{
  char res = 0;
  ALLingdbDynamicTrieNode const* children = this;
  while (children != nullptr)
  {
    ++res;
    children = children->getNextBrother();
  }
  return res;
}



bool ALLingdbDynamicTrieNode::isExpr() const
{
  if (canBeASeparator())
  {
    return false;
  }
  ALLingdbDynamicTrieNode const* node = this;
  while (node != nullptr)
  {
    if (node->getLetter() == ' ')
    {
      return true;
    }
    node = node->fFather;
  }
  return false;
}


bool ALLingdbDynamicTrieNode::canBeASeparator() const
{
  ForwardPtrList<ALLingdbWordForms>* itWordForm = fWordForms;
  while (itWordForm != nullptr)
  {
    if (!partOfSpeech_isAWord(itWordForm->elt->getMeaning()->getPartOfSpeech()))
      return true;
    itWordForm = itWordForm->next;
  }
  return false;
}



std::string ALLingdbDynamicTrieNode::getWord
(bool pNaturalLangPrint) const
{
  std::list<char> resList;
  ALLingdbDynamicTrieNode const* node = this;

  // if it's a multi meanings node we skip it
  std::string beforeRootLemme;
  std::string afterRootLemme;
  if (fDatasIfItsAMultiMeaningNode != nullptr)
  {
    const ForwardPtrList<ALLingdbNodeLinkedMeaning>*
        lksMeanings = fDatasIfItsAMultiMeaningNode->getLinkedMeanings();
    if (pNaturalLangPrint)
    {
      while (lksMeanings != nullptr)
      {
        if (lksMeanings->elt->direction ==
            static_cast<char>(LinkedMeaningDirection::FORWARD))
        {
          afterRootLemme += " " + lksMeanings->elt->meaning->getLemma()->getWord();
        }
        else
        {
          beforeRootLemme = lksMeanings->elt->meaning->getLemma()->getWord() + " " + beforeRootLemme;
        }
        lksMeanings = lksMeanings->next;
      }
    }
    else
    {
      while (lksMeanings != nullptr)
      {
        afterRootLemme += "~" + lksMeanings->elt->meaning->getLemma()->getWord();
        lksMeanings = lksMeanings->next;
      }
    }
    node = node->fFather;
  }

  while (node != nullptr)
  {
    resList.push_front(node->getLetter());
    node = node->fFather;
  }
  std::string word(resList.size(), ' ');
  std::size_t count = 0;
  for (std::list<char>::iterator it = resList.begin(); it != resList.end(); ++it)
  {
    word[count++] = *it;
  }

  if (!beforeRootLemme.empty() || !afterRootLemme.empty())
  {
    return beforeRootLemme + word + afterRootLemme;
  }
  return word;
}




} // End of namespace onsem
