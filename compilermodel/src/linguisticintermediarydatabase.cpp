#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <climits>
#include <onsem/compilermodel/savers/binarydatabasedicosaver.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbflexions.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/lingdbstring.hpp>
#include "concept/lingdblinktoaconcept.hpp"
#include "concept/lingdbconcept.hpp"

namespace onsem {

LinguisticIntermediaryDatabase::LinguisticIntermediaryDatabase()
    : fAlloc("Dynamic Trie Memory")
    , fRoot(nullptr)
    , fInfos(nullptr)
    , fMainWordToSimpleWordToAdd()
    , fConceptNameToPtr() {
    const unsigned char alignementMemory = 4;
    fAlloc.addANewLeaf<LingdbDynamicTrieNode>(
        "a node of the trie", alignementMemory, LingdbDynamicTrieNode::xGetPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbMeaning> >(
        "list of meanings", alignementMemory, ForwardPtrList<LingdbMeaning>::getPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbWordForms> >(
        "list of word forms", alignementMemory, ForwardPtrList<LingdbWordForms>::getPointers);
    fAlloc.addANewLeaf<LingdbWordForms>("a word form", alignementMemory, LingdbWordForms::xGetPointers);
    fAlloc.addANewLeaf<LingdbMeaning>("a meaning", alignementMemory, LingdbMeaning::xGetPointers);
    fAlloc.addANewLeaf<LingdbFlexions>("a flexion", alignementMemory, LingdbFlexions::xGetPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbAnimationsTag> >(
        "list of tags", alignementMemory, ForwardPtrList<LingdbAnimationsTag>::getPointers);

    fAlloc.addANewLeaf<LingdbQuestionWords>("question words", alignementMemory, LingdbQuestionWords::xGetPointers);
    fAlloc.addANewLeaf<LingdbQuestionWords::AQuestionWord>(
        "a question word", alignementMemory, LingdbQuestionWords::AQuestionWord::xGetPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbQuestionWords::AQuestionWord> >(
        "a question words list", alignementMemory, ForwardPtrList<LingdbQuestionWords::AQuestionWord>::getPointers);

    fAlloc.addANewLeaf<ForwardPtrList<std::pair<char, char> > >(
        "char pair list", alignementMemory, ForwardPtrList<std::pair<PartOfSpeech, PartOfSpeech> >::getPointers);
    fAlloc.addANewLeaf<std::pair<char, char> >("char pair", alignementMemory);

    fAlloc.addANewLeaf<char>("a character", alignementMemory);
    fAlloc.addANewLeaf<unsigned char>("a character not signed", alignementMemory);
    fAlloc.addANewLeaf<ForwardPtrList<char> >("char list", alignementMemory, ForwardPtrList<char>::getPointers);
    fAlloc.addANewLeaf<LingdbString>("a string", alignementMemory, LingdbString::xGetPointers);
    fAlloc.addANewLeaf<LingdbInfos>("database's infos", alignementMemory, LingdbInfos::getPointers);

    fAlloc.addANewLeaf<LingdbLinkToAConcept>(
        "a link to a concept", alignementMemory, LingdbLinkToAConcept::xGetPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbLinkToAConcept> >(
        "a list of all the links to a concept", alignementMemory, ForwardPtrList<LingdbLinkToAConcept>::getPointers);
    fAlloc.addANewLeaf<LingdbConcept>("a concept", alignementMemory, LingdbConcept::xGetPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbConcept> >(
        "a list of all the concepts", alignementMemory, ForwardPtrList<LingdbConcept>::getPointers);
    fAlloc.addANewLeaf<LingdbString>("a string in the concept composite", alignementMemory, LingdbString::xGetPointers);

    fAlloc.addANewLeaf<LingdbMultiMeaningsNode>(
        "a multi meanings node", alignementMemory, LingdbMultiMeaningsNode::getPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbDynamicTrieNode> >(
        "a list of dynamic trie nodes", alignementMemory, ForwardPtrList<LingdbDynamicTrieNode>::getPointers);
    fAlloc.addANewLeaf<LingdbNodeLinkedMeaning>(
        "a linked meaning", alignementMemory, LingdbNodeLinkedMeaning::getPointers);
    fAlloc.addANewLeaf<ForwardPtrList<LingdbNodeLinkedMeaning> >(
        "a list of linked meanings", alignementMemory, ForwardPtrList<LingdbNodeLinkedMeaning>::getPointers);

    xInitDatabase();
}

LinguisticIntermediaryDatabase::~LinguisticIntermediaryDatabase() {
    fConceptNameToPtr.clear();
    fAlloc.clear();
}

void LinguisticIntermediaryDatabase::load(const std::string& pFilename, float pCoef) {
    std::string errorMessage;
    fAlloc.deserialize(errorMessage, pFilename, pCoef);
    fRoot = fAlloc.first<LingdbDynamicTrieNode>();
    fInfos = fAlloc.first<LingdbInfos>();
    fConceptNameToPtr.clear();
    LingdbConcept* concept = fAlloc.first<LingdbConcept>();
    while (concept != nullptr) {
        fConceptNameToPtr.emplace(concept->getName()->toStr(), concept);
        concept = fAlloc.next<LingdbConcept>(concept);
    }
    if (!errorMessage.empty())
        throw std::runtime_error(errorMessage);
}

void LinguisticIntermediaryDatabase::xGetMainLemma(std::string& pMainLemma,
                                                   const std::string& pComposedWordsLemma) const {
    std::size_t posSep = pComposedWordsLemma.find_first_of('~');
    // if first separator between sub words is found
    if (posSep != std::string::npos && posSep > 0 && posSep + 1 < pComposedWordsLemma.size()) {
        pMainLemma = pComposedWordsLemma.substr(0, posSep);
    }
}

LingdbMeaning* LinguisticIntermediaryDatabase::getMeaning(const std::string& pLemma, PartOfSpeech pPartOfSpeech) const {
    LingdbDynamicTrieNode* wordNode = fRoot->advanceInTrieIfEndOfAWord(pLemma);
    if (wordNode != nullptr) {
        return wordNode->getMeaning(pPartOfSpeech);
    }

    // look in composed words
    std::string mainLemma;
    xGetMainLemma(mainLemma, pLemma);
    if (mainLemma.empty()) {
        return nullptr;
    }
    // get main lemma word node
    wordNode = fRoot->advanceInTrieIfEndOfAWord(mainLemma);
    if (wordNode == nullptr) {
        return nullptr;
    }
    // find the good multi meanings possibility
    const ForwardPtrList<LingdbDynamicTrieNode>* multiMeaningNodes = wordNode->getMultiMeaningsNodes();
    while (multiMeaningNodes != nullptr) {
        // if it has the good grammatical type
        LingdbMeaning* resMeaning = multiMeaningNodes->elt->getMeaning(pPartOfSpeech);
        if (resMeaning != nullptr) {
            // if the other sub words are the same
            LingdbMultiMeaningsNode* multiMeanings = multiMeaningNodes->elt->getDatasIfItsAMultiMeaningNode();
            if (multiMeanings->isStrEqualToListOfLemmes(pLemma, mainLemma.size() + 1)) {
                return resMeaning;
            }
        }
        multiMeaningNodes = multiMeaningNodes->next;
    }

    return nullptr;
}

LingdbConcept* LinguisticIntermediaryDatabase::getConcept(const std::string& pConceptName) const {
    auto it = fConceptNameToPtr.find(pConceptName);
    if (it != fConceptNameToPtr.end())
        return it->second;
    return nullptr;
}

LingdbConcept* LinguisticIntermediaryDatabase::addConcept(bool& pNewCptHasBeenInserted,
                                                          const std::string& pConceptName,
                                                          bool pAutoFill) {
    //  LingdbConcept* cpt = getConcept(pConceptName);
    LingdbConcept* cpt = nullptr;
    auto it = fConceptNameToPtr.find(pConceptName);
    if (it != fConceptNameToPtr.end())
        cpt = it->second;

    if (cpt == nullptr) {
        cpt = fAlloc.allocate<LingdbConcept>(1);
        cpt->xInit(fAlloc, pConceptName, pAutoFill);
        fConceptNameToPtr.emplace(pConceptName, cpt);
        pNewCptHasBeenInserted = true;
    } else {
        pNewCptHasBeenInserted = false;
    }
    return cpt;
}

void LinguisticIntermediaryDatabase::addWord(const std::string& pWord,
                                             const std::string& pLemma,
                                             PartOfSpeech pPartOfSpeech,
                                             const std::vector<std::string>& pFlexions,
                                             char pFrequency) {
    LingdbMeaning* meaning = getMeaning(pLemma, pPartOfSpeech);
    if (meaning != nullptr) {
        addWordWithSpecificMeaning(pWord, *meaning, pFlexions, pFrequency);
        return;
    }
    if (!fRoot) {
        return;
    }

    if (pWord != pLemma) {
        std::size_t posSepInLemme = pLemma.find('~');
        if (posSepInLemme != std::string::npos) {
            fMainWordToSimpleWordToAdd[pLemma.substr(0, posSepInLemme)].emplace_back(
                pWord, pLemma, pPartOfSpeech, pFlexions, pFrequency);
            return;
        }
    }

    LingdbDynamicTrieNode* wordNode = fRoot->xInsertWord(fAlloc, pWord, 0);
    LingdbWordForms* wordForm = wordNode->addWordForm(*this, pLemma, pPartOfSpeech);

    wordForm->xAddFlexions(fAlloc, pPartOfSpeech, pFlexions);
    wordForm->setFrequency(pFrequency);
}

void LinguisticIntermediaryDatabase::addWordWithSpecificMeaning(const std::string& pWord,
                                                                LingdbMeaning& pMeaning,
                                                                const std::vector<std::string>& pFlexions,
                                                                char pFrequency) {
    if (!fRoot) {
        return;
    }

    LingdbDynamicTrieNode* wordNode = fRoot->xInsertWord(fAlloc, pWord, 0);
    LingdbWordForms* wordForm = wordNode->addWordFormFromMeaning(*this, pMeaning);

    wordForm->xAddFlexions(fAlloc, pMeaning.getPartOfSpeech(), pFlexions);
    wordForm->setFrequency(pFrequency);
}

void LinguisticIntermediaryDatabase::addMultiMeaningsWord(
    LingdbMeaning* pRootMeaning,
    std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings,
    PartOfSpeech pPartOfSpeech) {
    if (!fRoot) {
        return;
    }

    LingdbDynamicTrieNode* simpleLemme = pRootMeaning->getLemma();
    LingdbDynamicTrieNode* newNode = simpleLemme->xAddMultiMeaningsNode(fAlloc, pRootMeaning, pLinkedMeanings);
    newNode->addWordFormFromMeaning(*this, *newNode->xAddMeaning(fAlloc, pPartOfSpeech));

    if (!fMainWordToSimpleWordToAdd.empty()) {
        std::string rootLemma = simpleLemme->getWord();
        std::map<std::string, std::list<WordToAdd> >::iterator itMainLemme = fMainWordToSimpleWordToAdd.find(rootLemma);
        if (itMainLemme != fMainWordToSimpleWordToAdd.end()) {
            std::string compeleteLemma = newNode->getWord();
            for (std::list<WordToAdd>::iterator itWordToAdds = itMainLemme->second.begin();
                 itWordToAdds != itMainLemme->second.end();
                 ++itWordToAdds) {
                if (itWordToAdds->lemma == compeleteLemma) {
                    addWord(itWordToAdds->word,
                            itWordToAdds->lemma,
                            itWordToAdds->partOfSpeech,
                            itWordToAdds->flexions,
                            itWordToAdds->frequency);
                    itMainLemme->second.erase(itWordToAdds);
                    if (itMainLemme->second.empty()) {
                        fMainWordToSimpleWordToAdd.erase(itMainLemme);
                    }
                    break;
                }
            }
        }
    }
}

void LinguisticIntermediaryDatabase::removeWord(const std::string& word) {
    LingdbDynamicTrieNode* node = getPointerToEndOfWord(word);
    if (node != nullptr) {
        node->xRemoveAllWordForms(fAlloc);
    }
}

void LinguisticIntermediaryDatabase::removeWordForm(const std::string& word, PartOfSpeech pPartOfSpeech) {
    {
        LingdbDynamicTrieNode* node = getPointerToEndOfWord(word);
        if (node != nullptr) {
            node->xRemoveWordForm(fAlloc, pPartOfSpeech);
            return;
        }
    }

    // look in composed words
    std::string mainLemma;
    xGetMainLemma(mainLemma, word);
    if (mainLemma.empty()) {
        return;
    }
    // get main lemma word node
    LingdbDynamicTrieNode* wordNode = fRoot->advanceInTrieIfEndOfAWord(mainLemma);
    if (wordNode == nullptr) {
        return;
    }
    const ForwardPtrList<LingdbDynamicTrieNode>* multiMeaningNodes = wordNode->getMultiMeaningsNodes();
    while (multiMeaningNodes != nullptr) {
        const ForwardPtrList<LingdbDynamicTrieNode>* nextMultiMeaningNodes = multiMeaningNodes->next;

        LingdbMeaning* resMeaning = multiMeaningNodes->elt->getMeaning(pPartOfSpeech);
        if (resMeaning != nullptr) {
            // if the other sub words are the same
            LingdbMultiMeaningsNode* multiMeanings = multiMeaningNodes->elt->getDatasIfItsAMultiMeaningNode();
            if (multiMeanings->isStrEqualToListOfLemmes(word, mainLemma.size() + 1)) {
                multiMeaningNodes->elt->xRemoveWordForm(fAlloc, pPartOfSpeech);
            }
        }

        multiMeaningNodes = nextMultiMeaningNodes;
    }
}

void LinguisticIntermediaryDatabase::getConceptToMeanings(
    std::map<const LingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings) const {
    LingdbMeaning* meaning = fAlloc.first<LingdbMeaning>();
    while (meaning != nullptr) {
        const ForwardPtrList<LingdbLinkToAConcept>* meaningLinkToConcepts = meaning->getLinkToConcepts();
        while (meaningLinkToConcepts != nullptr) {
            pConceptToMeanings[meaningLinkToConcepts->elt->getConcept()].insert(
                MeaningAndConfidence(meaning, meaningLinkToConcepts->elt->getRelatedToConcept()));
            meaningLinkToConcepts = meaningLinkToConcepts->next;
        }
        meaning = fAlloc.next<LingdbMeaning>(meaning);
    }
}

LingdbDynamicTrieNode* LinguisticIntermediaryDatabase::getPointerToEndOfWord(const std::string& word) const {
    if (!fRoot) {
        return nullptr;
    }
    return fRoot->advanceInTrieIfEndOfAWord(word);
}

LingdbQuestionWords* LinguisticIntermediaryDatabase::getQuestionWords() const {
    return fInfos->questionWords;
}

LingdbDynamicTrieNode* LinguisticIntermediaryDatabase::getPointerInTrie(const std::string& word) const {
    if (!fRoot) {
        return nullptr;
    }
    return fRoot->advanceInTrie(word, true);
}

void LinguisticIntermediaryDatabase::newQWords(LingdbQuestionWords* pQWords) {
    if (fInfos->questionWords != nullptr) {
        fInfos->questionWords->xDeallocate(fAlloc);
        fInfos->questionWords = nullptr;
    }
    fInfos->questionWords = pQWords;
}

LingdbDynamicTrieNode* LinguisticIntermediaryDatabase::findComposedWordFromString(const std::string& pStr) const {
    if (!fRoot) {
        return nullptr;
    }

    // get ending nodes of each words AND
    // find the root word
    std::list<LingdbDynamicTrieNode*> subWords;
    const ForwardPtrList<LingdbDynamicTrieNode>* multiMeaningNodes = nullptr;
    std::size_t afterLastSeparator = 0;
    std::size_t currSeparator = pStr.find_first_of(" '");
    while (currSeparator != std::string::npos) {
        if (afterLastSeparator != currSeparator
            && !xAddNewSubWord(
                multiMeaningNodes, subWords, pStr.substr(afterLastSeparator, currSeparator - afterLastSeparator))) {
            return nullptr;
        }
        afterLastSeparator = currSeparator + 1;
        currSeparator = pStr.find_first_of(" '", afterLastSeparator);
    }
    if (afterLastSeparator != pStr.size()
        && !xAddNewSubWord(
            multiMeaningNodes, subWords, pStr.substr(afterLastSeparator, pStr.size() - afterLastSeparator))) {
        return nullptr;
    }

    // for each sub meanings possibilities (from the root word)
    while (multiMeaningNodes != nullptr) {
        LingdbMultiMeaningsNode* compMeaning = multiMeaningNodes->elt->getDatasIfItsAMultiMeaningNode();
        assert(compMeaning != nullptr);

        // for each sub meanings
        const ForwardPtrList<LingdbNodeLinkedMeaning>* linkedMeanings = compMeaning->getLinkedMeanings();
        if (linkedMeanings->length() == subWords.size()) {
            while (linkedMeanings != nullptr) {
                bool subMeaningFound = false;
                for (std::list<LingdbDynamicTrieNode*>::iterator itSubWord = subWords.begin();
                     itSubWord != subWords.end();
                     ++itSubWord) {
                    if ((*itSubWord)->getWordFormFromMeaning(*linkedMeanings->elt->meaning) != nullptr) {
                        subMeaningFound = true;
                        break;
                    }
                }
                if (!subMeaningFound) {
                    break;
                }
                linkedMeanings = linkedMeanings->next;
            }

            // if we found all sub meanings
            if (linkedMeanings == nullptr && multiMeaningNodes->elt->getWordForms() != nullptr) {
                return multiMeaningNodes->elt;
            }
        }

        multiMeaningNodes = multiMeaningNodes->next;
    }

    return nullptr;
}

bool LinguisticIntermediaryDatabase::xAddNewSubWord(const ForwardPtrList<LingdbDynamicTrieNode>*& pMultiMeaningNodes,
                                                    std::list<LingdbDynamicTrieNode*>& pSubWords,
                                                    const std::string& pSubWord) const {
    LingdbDynamicTrieNode* endOfWord = fRoot->advanceInTrieIfEndOfAWord(pSubWord);
    if (endOfWord == nullptr || endOfWord->getWordForms() == nullptr) {
        return false;
    }
    if (pMultiMeaningNodes == nullptr) {
        pMultiMeaningNodes = endOfWord->getMultiMeaningsNodes();
        if (pMultiMeaningNodes == nullptr) {
            pSubWords.emplace_back(endOfWord);
        }
    } else {
        pSubWords.emplace_back(endOfWord);
    }
    return true;
}

void LinguisticIntermediaryDatabase::xInitDatabase() {
    // root of the trie
    fRoot = fAlloc.allocate<LingdbDynamicTrieNode>(1);
    fRoot->xInit(CHAR_MIN);

    // database infos
    fAlloc.reserve<LingdbInfos>(1);
    fInfos = fAlloc.allocate<LingdbInfos>(1);
    fInfos->init();
}

}    // End of namespace onsem
