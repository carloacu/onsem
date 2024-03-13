#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <assert.h>
#include <list>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>

namespace onsem {

LingdbDynamicTrieNode::LingdbDynamicTrieNode(char pLetter)
    : fEndOfWord(false)
    , fLetter(pLetter)
    , fDatasIfItsAMultiMeaningNode(nullptr)
    , fMeaningsAtThisLemme(nullptr)
    , fWordForms(nullptr)
    , fFather(nullptr)
    , fFirstChild(nullptr)
    , fNextBrother(nullptr)
    , fMultiMeaningsNodes(nullptr) {}

void LingdbDynamicTrieNode::xInit(char pLetter) {
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

void LingdbDynamicTrieNode::putAWordFormAtTheTopOfTheList(PartOfSpeech pGram, const std::string& pLemme) {
    if (fWordForms == nullptr) {
        return;
    }
    ForwardPtrList<LingdbWordForms>* prev = nullptr;
    ForwardPtrList<LingdbWordForms>* curr = fWordForms;
    while (curr != nullptr) {
        if (curr->elt->getMeaning()->getPartOfSpeech() == pGram
            && (pLemme.empty() || curr->elt->getMeaning()->getLemma()->getWord() == pLemme)) {
            if (prev != nullptr) {
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
    if (!pLemme.empty()) {
        putAWordFormAtTheTopOfTheList(pGram, "");
    }
}

LingdbMeaning* LingdbDynamicTrieNode::getMeaning(PartOfSpeech pPartOfSpeech) const {
    ForwardPtrList<LingdbMeaning>* itMeaning = fMeaningsAtThisLemme;
    while (itMeaning != nullptr) {
        if (itMeaning->elt->getPartOfSpeech() == pPartOfSpeech) {
            return itMeaning->elt;
        }
        itMeaning = itMeaning->next;
    }
    return nullptr;
}

LingdbWordForms* LingdbDynamicTrieNode::getWordForm(const std::string& pLemme, PartOfSpeech pPartOfSpeech) const {
    ForwardPtrList<LingdbWordForms>* itWordForm = fWordForms;
    while (itWordForm != nullptr) {
        if (itWordForm->elt->getMeaning()->getPartOfSpeech() == pPartOfSpeech
            && itWordForm->elt->getMeaning()->getLemma()->getWord() == pLemme) {
            return itWordForm->elt;
        }
        itWordForm = itWordForm->next;
    }
    return nullptr;
}

LingdbWordForms* LingdbDynamicTrieNode::getWordFormFromMeaning(LingdbMeaning& pMeaning) const {
    ForwardPtrList<LingdbWordForms>* itWordForm = fWordForms;
    while (itWordForm != nullptr) {
        if (itWordForm->elt->getMeaning() == &pMeaning) {
            return itWordForm->elt;
        }
        itWordForm = itWordForm->next;
    }
    return nullptr;
}

unsigned char LingdbDynamicTrieNode::nbMeaningsAtThisLemme() const {
    if (fMeaningsAtThisLemme == nullptr) {
        return 0;
    }
    return fMeaningsAtThisLemme->length();
}

unsigned char LingdbDynamicTrieNode::nbWordForms() const {
    if (fWordForms == nullptr) {
        return 0;
    }
    return fWordForms->length();
}

void LingdbDynamicTrieNode::getWordFormsAndMeanings(
    std::list<std::pair<LingdbWordForms*, LingdbMeaning*> >& pWordFromsOrMeanings) const {
    std::set<LingdbMeaning*> meaningsAlreadyAdded;
    ForwardPtrList<LingdbWordForms>* itWordForm = fWordForms;
    while (itWordForm != nullptr) {
        meaningsAlreadyAdded.insert(itWordForm->elt->getMeaning());
        pWordFromsOrMeanings.emplace_back(std::pair<LingdbWordForms*, LingdbMeaning*>(itWordForm->elt, nullptr));
        itWordForm = itWordForm->next;
    }

    ForwardPtrList<LingdbMeaning>* itMeaning = fMeaningsAtThisLemme;
    while (itMeaning != nullptr) {
        if (meaningsAlreadyAdded.find(itMeaning->elt) == meaningsAlreadyAdded.end()) {
            pWordFromsOrMeanings.emplace_back(std::pair<LingdbWordForms*, LingdbMeaning*>(nullptr, itMeaning->elt));
        }
        itMeaning = itMeaning->next;
    }
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::xAddMultiMeaningsNode(
    CompositePoolAllocator& pAlloc,
    LingdbMeaning* pRootMeaning,
    std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings) {
    LingdbDynamicTrieNode* newNode = pAlloc.allocate<LingdbDynamicTrieNode>(1);
    newNode->xInit(0);
    newNode->fFather = this;
    newNode->fEndOfWord = true;
    newNode->fDatasIfItsAMultiMeaningNode = pAlloc.allocate<LingdbMultiMeaningsNode>(1);
    newNode->fDatasIfItsAMultiMeaningNode->xInit(pAlloc, pRootMeaning, pLinkedMeanings);

    ForwardPtrList<LingdbDynamicTrieNode>* newNodeList = pAlloc.allocate<ForwardPtrList<LingdbDynamicTrieNode> >(1);
    newNodeList->init(newNode);
    newNodeList->next = fMultiMeaningsNodes;
    fMultiMeaningsNodes = newNodeList;
    return newNode;
}

LingdbMeaning* LingdbDynamicTrieNode::xAddMeaning(CompositePoolAllocator& pFPAlloc, PartOfSpeech pPartOfSpeech) {
    LingdbMeaning* meaning = getMeaning(pPartOfSpeech);
    if (meaning == nullptr) {
        meaning = pFPAlloc.allocate<LingdbMeaning>(1);
        meaning->xInit(this, static_cast<char>(pPartOfSpeech));
        ForwardPtrList<LingdbMeaning>* newMeaningList = pFPAlloc.allocate<ForwardPtrList<LingdbMeaning> >(1);
        newMeaningList->init(meaning);
        newMeaningList->next = fMeaningsAtThisLemme;
        fMeaningsAtThisLemme = newMeaningList;
    }
    return meaning;
}

LingdbWordForms* LingdbDynamicTrieNode::addWordForm(LinguisticIntermediaryDatabase& pLingDatabase,
                                                    const std::string& pLemma,
                                                    PartOfSpeech pPartOfSpeech,
                                                    bool pAtFront) {
    if (!fEndOfWord) {
        return nullptr;
    }
    LingdbWordForms* wordForm = getWordForm(pLemma, pPartOfSpeech);
    if (wordForm == nullptr) {
        CompositePoolAllocator& alloc = pLingDatabase.xGetFPAlloc();
        return xAddWordFormFromMeaning(
            alloc,
            *pLingDatabase.getRoot()->xInsertWord(alloc, pLemma, 0)->xAddMeaning(alloc, pPartOfSpeech),
            pAtFront);
    }
    return wordForm;
}

LingdbWordForms* LingdbDynamicTrieNode::addWordFormFromMeaning(LinguisticIntermediaryDatabase& pLingDatabase,
                                                               LingdbMeaning& pMeaning,
                                                               bool pAtFront) {
    if (!fEndOfWord) {
        return nullptr;
    }
    LingdbWordForms* wordForm = getWordFormFromMeaning(pMeaning);
    if (wordForm == nullptr) {
        return xAddWordFormFromMeaning(pLingDatabase.xGetFPAlloc(), pMeaning, pAtFront);
    }
    return wordForm;
}

LingdbWordForms* LingdbDynamicTrieNode::xAddWordFormFromMeaning(CompositePoolAllocator& pAlloc,
                                                                LingdbMeaning& pMeaning,
                                                                bool pAtFront) {
    LingdbWordForms* wordForm = pAlloc.allocate<LingdbWordForms>(1);
    wordForm->xInit(&pMeaning);
    ForwardPtrList<LingdbWordForms>* newWordFormList = pAlloc.allocate<ForwardPtrList<LingdbWordForms> >(1);
    newWordFormList->init(wordForm);
    if (pAtFront) {
        newWordFormList->next = fWordForms;
        fWordForms = newWordFormList;
    } else {
        newWordFormList->next = nullptr;
        if (fWordForms != nullptr) {
            fWordForms->back()->next = newWordFormList;
        } else {
            fWordForms = newWordFormList;
        }
    }
    return wordForm;
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::xInsertWord(CompositePoolAllocator& pFPAlloc,
                                                          const std::string& pWord,
                                                          std::size_t pOffset) {
    // if osset is out of the word, we have nothing to do
    if (pOffset >= pWord.size()) {
        assert(false);
        return nullptr;
    }

    assert(pWord[pOffset] >= fLetter);
    // our node == the next letter that we have to insert
    if (pWord[pOffset] == fLetter) {
        // if this is the end of the word
        if (pOffset == pWord.size() - 1) {
            fEndOfWord = true;
            return this;
        } else    // skip this node and call the first child to continue the insertion
        {
            // if we have to insert a new node before the first child
            if (fFirstChild == nullptr || fFirstChild->getLetter() > pWord[pOffset + 1]) {
                // add a new node
                LingdbDynamicTrieNode* SecondChild = fFirstChild;
                fFirstChild = pFPAlloc.allocate<LingdbDynamicTrieNode>(1);
                fFirstChild->xInit(pWord[pOffset + 1]);
                fFirstChild->fFather = this;
                fFirstChild->fNextBrother = SecondChild;
            }
            // call the first child to continue the insertion
            return fFirstChild->xInsertWord(pFPAlloc, pWord, pOffset + 1);
        }
    } else    // the letter to insert has to be after the current node
    {
        // there is no brother OR the insertion has to be before the next brother
        if (fNextBrother == nullptr || pWord[pOffset] < fNextBrother->getLetter()) {
            // add a new node
            LingdbDynamicTrieNode* secondNextBrother = fNextBrother;
            fNextBrother = pFPAlloc.allocate<LingdbDynamicTrieNode>(1);
            fNextBrother->xInit(pWord[pOffset]);
            fNextBrother->fFather = fFather;
            fNextBrother->fNextBrother = secondNextBrother;
        }
        // call the next brother to continue the insertion
        return fNextBrother->xInsertWord(pFPAlloc, pWord, pOffset);
    }
}

void LingdbDynamicTrieNode::xRemoveMeaning(CompositePoolAllocator& pFPAlloc, PartOfSpeech pPartOfSpeech) {
    ForwardPtrList<LingdbMeaning>* prev = nullptr;
    ForwardPtrList<LingdbMeaning>* meaning = fMeaningsAtThisLemme;
    while (meaning != nullptr) {
        if (meaning->elt->getPartOfSpeech() == pPartOfSpeech) {
            if (prev == nullptr) {
                fMeaningsAtThisLemme = meaning->next;
            } else {
                prev->next = meaning->next;
            }
            pFPAlloc.deallocate<ForwardPtrList<LingdbMeaning> >(meaning);
            break;
        }
        prev = meaning;
        meaning = meaning->next;
    }

    xRemoveWordIfNoLongerAWord(pFPAlloc);
}

void LingdbDynamicTrieNode::xRemoveAllWordForms(CompositePoolAllocator& pAlloc) {
    assert(fEndOfWord);
    if (fWordForms != nullptr) {
        fWordForms->clearComposedElts(pAlloc);
        fWordForms = nullptr;
    }
    xRemoveWordIfNoLongerAWord(pAlloc);
}

void LingdbDynamicTrieNode::xRemoveWordForm(CompositePoolAllocator& pAlloc, PartOfSpeech pPartOfSpeech) {
    assert(fEndOfWord);
    ForwardPtrList<LingdbWordForms>* prev = nullptr;
    ForwardPtrList<LingdbWordForms>* curr = fWordForms;
    while (curr != nullptr) {
        if (curr->elt->getMeaning()->getPartOfSpeech() == pPartOfSpeech) {
            if (prev == nullptr)
                ForwardPtrList<LingdbWordForms>::clearNextComposedElt(fWordForms, pAlloc);
            else
                ForwardPtrList<LingdbWordForms>::clearNextComposedElt(prev->next, pAlloc);
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    xRemoveWordIfNoLongerAWord(pAlloc);
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::advanceInTrieIfEndOfAWord(const std::string& word) const {
    LingdbDynamicTrieNode* node = advanceInTrie(word, true);
    if (node == nullptr || node->fEndOfWord == false) {
        return nullptr;
    }
    return node;
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::advanceInTrie(const std::string& word, bool pUntilEndOfWord) const {
    std::size_t off = 0;
    LingdbDynamicTrieNode const* prevNode = this;
    LingdbDynamicTrieNode const* node = prevNode->xGetBrother(word[off]);
    while (off < word.size() && node != nullptr) {
        prevNode = node;
        node = node->xGetChild(word[++off]);
    }
    if (!pUntilEndOfWord || off == word.size()) {
        return const_cast<LingdbDynamicTrieNode*>(prevNode);
    }
    return nullptr;
}

void LingdbDynamicTrieNode::xRemoveWordIfNoLongerAWord(CompositePoolAllocator& pFPAlloc) {
    if (fEndOfWord && fMeaningsAtThisLemme == nullptr && fWordForms == nullptr && fMultiMeaningsNodes == nullptr) {
        fEndOfWord = false;
        xTryToDeallocateNodes(pFPAlloc);
    }
}

void LingdbDynamicTrieNode::xTryToDeallocateNodes(CompositePoolAllocator& pFPAlloc) {
    if (fEndOfWord || fFirstChild != nullptr || pFPAlloc.first<LingdbDynamicTrieNode>() == this) {
        return;
    }
    LingdbDynamicTrieNode* currNode;
    if (fFather == nullptr) {
        currNode = pFPAlloc.first<LingdbDynamicTrieNode>();
    } else {
        currNode = fFather->fFirstChild;
        if (currNode == this) {
            fFather->fFirstChild = fNextBrother;
            fFather->xTryToDeallocateNodes(pFPAlloc);
            pFPAlloc.deallocate<LingdbDynamicTrieNode>(this);
            return;
        }
    }
    while (currNode->fNextBrother != this
           && currNode->fNextBrother != nullptr)    // Because it's not found if we remove a composed word
    {
        currNode = currNode->fNextBrother;
    }

    if (currNode->fNextBrother != nullptr) {
        currNode->fNextBrother = fNextBrother;
    } else {
        // if we are here, this means we deleting a multi words meanings
        ForwardPtrList<LingdbDynamicTrieNode>* prevMultiMeaningsNodes = nullptr;
        ForwardPtrList<LingdbDynamicTrieNode>* multiMeaningsNodes = fFather->fMultiMeaningsNodes;
        while (multiMeaningsNodes != nullptr) {
            if (multiMeaningsNodes->elt == this) {
                if (prevMultiMeaningsNodes == nullptr) {
                    fFather->fMultiMeaningsNodes = multiMeaningsNodes->next;
                } else {
                    prevMultiMeaningsNodes->next = multiMeaningsNodes->next;
                }
                pFPAlloc.deallocate<ForwardPtrList<LingdbDynamicTrieNode> >(multiMeaningsNodes);
                break;
            }
            prevMultiMeaningsNodes = multiMeaningsNodes;
            multiMeaningsNodes = multiMeaningsNodes->next;
        }
        fFather->xRemoveWordIfNoLongerAWord(pFPAlloc);
    }

    pFPAlloc.deallocate<LingdbDynamicTrieNode>(this);
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::getNextWordNode() const {
    if (fFirstChild != nullptr) {
        return fFirstChild->xGetWordNode();
    }

    LingdbDynamicTrieNode const* currNode = this;
    do {
        if (currNode->fNextBrother != nullptr) {
            return currNode->fNextBrother->xGetWordNode();
        }
        currNode = currNode->fFather;
    } while (currNode != nullptr);

    return nullptr;
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::xGetWordNode() const {
    if (fEndOfWord) {
        return const_cast<LingdbDynamicTrieNode*>(this);
    }
    return getNextWordNode();
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::xGetChild(char pLetter) const {
    if (fFirstChild == nullptr) {
        return nullptr;
    }
    return fFirstChild->xGetBrother(pLetter);
}

LingdbDynamicTrieNode* LingdbDynamicTrieNode::xGetBrother(char pLetter) const {
    LingdbDynamicTrieNode const* children = this;
    while (children != nullptr) {
        if (children->fLetter > pLetter) {
            return nullptr;
        } else if (children->fLetter == pLetter) {
            return const_cast<LingdbDynamicTrieNode*>(children);
        }
        children = children->fNextBrother;
    }
    return nullptr;
}

unsigned char LingdbDynamicTrieNode::nbSupBrother() const {
    char res = 0;
    LingdbDynamicTrieNode const* children = this;
    while (children != nullptr) {
        ++res;
        children = children->getNextBrother();
    }
    return res;
}

bool LingdbDynamicTrieNode::isExpr() const {
    if (canBeASeparator()) {
        return false;
    }
    LingdbDynamicTrieNode const* node = this;
    while (node != nullptr) {
        if (node->getLetter() == ' ') {
            return true;
        }
        node = node->fFather;
    }
    return false;
}

bool LingdbDynamicTrieNode::canBeASeparator() const {
    ForwardPtrList<LingdbWordForms>* itWordForm = fWordForms;
    while (itWordForm != nullptr) {
        if (!partOfSpeech_isAWord(itWordForm->elt->getMeaning()->getPartOfSpeech()))
            return true;
        itWordForm = itWordForm->next;
    }
    return false;
}

std::string LingdbDynamicTrieNode::getWord(bool pNaturalLangPrint) const {
    std::list<char> resList;
    LingdbDynamicTrieNode const* node = this;

    // if it's a multi meanings node we skip it
    std::string beforeRootLemme;
    std::string afterRootLemme;
    if (fDatasIfItsAMultiMeaningNode != nullptr) {
        const ForwardPtrList<LingdbNodeLinkedMeaning>* lksMeanings = fDatasIfItsAMultiMeaningNode->getLinkedMeanings();
        if (pNaturalLangPrint) {
            while (lksMeanings != nullptr) {
                if (lksMeanings->elt->direction == static_cast<char>(LinkedMeaningDirection::FORWARD)) {
                    afterRootLemme += " " + lksMeanings->elt->meaning->getLemma()->getWord();
                } else {
                    beforeRootLemme = lksMeanings->elt->meaning->getLemma()->getWord() + " " + beforeRootLemme;
                }
                lksMeanings = lksMeanings->next;
            }
        } else {
            while (lksMeanings != nullptr) {
                afterRootLemme += "~" + lksMeanings->elt->meaning->getLemma()->getWord();
                lksMeanings = lksMeanings->next;
            }
        }
        node = node->fFather;
    }

    while (node != nullptr) {
        resList.push_front(node->getLetter());
        node = node->fFather;
    }
    std::string word(resList.size(), ' ');
    std::size_t count = 0;
    for (std::list<char>::iterator it = resList.begin(); it != resList.end(); ++it) {
        word[count++] = *it;
    }

    if (!beforeRootLemme.empty() || !afterRootLemme.empty()) {
        return beforeRootLemme + word + afterRootLemme;
    }
    return word;
}

}    // End of namespace onsem
