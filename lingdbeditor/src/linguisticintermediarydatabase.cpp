#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <climits>
#include <onsem/lingdbeditor/savers/albinarydatabasedicosaver.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <onsem/lingdbeditor/allingdbflexions.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/allingdbwordforms.hpp>
#include "allingdbanimationtag.hpp"
#include <onsem/lingdbeditor/allingdbstring.hpp>
#include "concept/allingdblinktoaconcept.hpp"
#include "concept/allingdbconcept.hpp"


namespace onsem
{


LinguisticIntermediaryDatabase::LinguisticIntermediaryDatabase()
  : fAlloc("Dynamic Trie Memory"),
    fRoot(nullptr),
    fInfos(nullptr),
    fMainWordToSimpleWordToAdd(),
    fConceptNameToPtr()
{
  const unsigned char alignementMemory = 4;
  fAlloc.addANewLeaf<ALLingdbDynamicTrieNode>("a node of the trie", alignementMemory, ALLingdbDynamicTrieNode::xGetPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbMeaning> >("list of meanings", alignementMemory, ForwardPtrList<ALLingdbMeaning>::getPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbWordForms> >("list of word forms", alignementMemory, ForwardPtrList<ALLingdbWordForms>::getPointers);
  fAlloc.addANewLeaf<ALLingdbWordForms>("a word form", alignementMemory, ALLingdbWordForms::xGetPointers);
  fAlloc.addANewLeaf<ALLingdbMeaning>("a meaning", alignementMemory, ALLingdbMeaning::xGetPointers);
  fAlloc.addANewLeaf<ALLingdbFlexions>("a flexion", alignementMemory, ALLingdbFlexions::xGetPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbAnimationsTag> >("list of tags", alignementMemory, ForwardPtrList<ALLingdbAnimationsTag>::getPointers);
  fAlloc.addANewLeaf<ALLingdbAnimationsTag>("a tag", alignementMemory, ALLingdbAnimationsTag::xGetPointers);
  fAlloc.addANewLeaf<ForwardPtrList<PonderatedMeaning>>("a list of meanings with a ponderated value", alignementMemory, ForwardPtrList<PonderatedMeaning>::getPointers);
  fAlloc.addANewLeaf<PonderatedMeaning>("a meaning with a ponderated value", alignementMemory, PonderatedMeaning::xGetPointers);

  fAlloc.addANewLeaf<ALLingdbQuestionWords>("question words", alignementMemory, ALLingdbQuestionWords::xGetPointers);
  fAlloc.addANewLeaf<ALLingdbQuestionWords::ALAQuestionWord>("a question word", alignementMemory, ALLingdbQuestionWords::ALAQuestionWord::xGetPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbQuestionWords::ALAQuestionWord> >("a question words list", alignementMemory, ForwardPtrList<ALLingdbQuestionWords::ALAQuestionWord>::getPointers);

  fAlloc.addANewLeaf<ForwardPtrList<std::pair<char, char> > >("char pair list", alignementMemory, ForwardPtrList<std::pair<PartOfSpeech, PartOfSpeech> >::getPointers);
  fAlloc.addANewLeaf<std::pair<char, char> >("char pair", alignementMemory);

  fAlloc.addANewLeaf<char>("a character", alignementMemory);
  fAlloc.addANewLeaf<unsigned char>("a character not signed", alignementMemory);
  fAlloc.addANewLeaf<ForwardPtrList<char> >("char list", alignementMemory, ForwardPtrList<char>::getPointers);
  fAlloc.addANewLeaf<ALLingdbString>("a string", alignementMemory, ALLingdbString::xGetPointers);
  fAlloc.addANewLeaf<ALLingdbInfos>("database's infos", alignementMemory, ALLingdbInfos::getPointers);

  fAlloc.addANewLeaf<ALLingdbLinkToAConcept>("a link to a concept", alignementMemory, ALLingdbLinkToAConcept::xGetPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbLinkToAConcept> >("a list of all the links to a concept", alignementMemory, ForwardPtrList<ALLingdbLinkToAConcept>::getPointers);
  fAlloc.addANewLeaf<ALLingdbConcept>("a concept", alignementMemory, ALLingdbConcept::xGetPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbConcept> >("a list of all the concepts", alignementMemory, ForwardPtrList<ALLingdbConcept>::getPointers);
  fAlloc.addANewLeaf<ALLingdbString>("a string in the concept composite", alignementMemory, ALLingdbString::xGetPointers);

  fAlloc.addANewLeaf<ALLingdbMultiMeaningsNode>("a multi meanings node", alignementMemory, ALLingdbMultiMeaningsNode::getPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbDynamicTrieNode> >("a list of dynamic trie nodes", alignementMemory, ForwardPtrList<ALLingdbDynamicTrieNode>::getPointers);
  fAlloc.addANewLeaf<ALLingdbNodeLinkedMeaning>("a linked meaning", alignementMemory, ALLingdbNodeLinkedMeaning::getPointers);
  fAlloc.addANewLeaf<ForwardPtrList<ALLingdbNodeLinkedMeaning> >("a list of linked meanings", alignementMemory, ForwardPtrList<ALLingdbNodeLinkedMeaning>::getPointers);

  xInitDatabase();
}


LinguisticIntermediaryDatabase::~LinguisticIntermediaryDatabase()
{
  fConceptNameToPtr.clear();
  fAlloc.clear();
}


void LinguisticIntermediaryDatabase::load
(const boost::filesystem::path& pFilename, float pCoef)
{ 
  std::string errorMessage;
  fAlloc.deserialize(errorMessage,
                     pFilename, pCoef);
  fRoot = fAlloc.first<ALLingdbDynamicTrieNode>();
  fInfos = fAlloc.first<ALLingdbInfos>();
  fConceptNameToPtr.clear();
  ALLingdbConcept* concept = fAlloc.first<ALLingdbConcept>();
  while (concept != nullptr)
  {
    fConceptNameToPtr.emplace(concept->getName()->toStr(), concept);
    concept = fAlloc.next<ALLingdbConcept>(concept);
  }
  if (!errorMessage.empty())
    throw std::runtime_error(errorMessage);
}


void LinguisticIntermediaryDatabase::xGetMainLemma
(std::string& pMainLemma,
 const std::string& pComposedWordsLemma) const
{
  std::size_t posSep = pComposedWordsLemma.find_first_of('~');
  // if first separator between sub words is found
  if (posSep != std::string::npos && posSep > 0 &&
      posSep + 1 < pComposedWordsLemma.size())
  {
    pMainLemma = pComposedWordsLemma.substr(0, posSep);
  }
}


ALLingdbMeaning* LinguisticIntermediaryDatabase::getMeaning
(const std::string& pLemma,
 PartOfSpeech pPartOfSpeech) const
{
  ALLingdbDynamicTrieNode* wordNode
      = fRoot->advanceInTrieIfEndOfAWord(pLemma);
  if (wordNode != nullptr)
  {
    return wordNode->getMeaning(pPartOfSpeech);
  }

  // look in composed words
  std::string mainLemma;
  xGetMainLemma(mainLemma, pLemma);
  if (mainLemma.empty())
  {
    return nullptr;
  }
  // get main lemma word node
  wordNode = fRoot->advanceInTrieIfEndOfAWord(mainLemma);
  if (wordNode == nullptr)
  {
    return nullptr;
  }
  // find the good multi meanings possibility
  const ForwardPtrList<ALLingdbDynamicTrieNode>*
      multiMeaningNodes = wordNode->getMultiMeaningsNodes();
  while (multiMeaningNodes != nullptr)
  {
    // if it has the good grammatical type
    ALLingdbMeaning* resMeaning = multiMeaningNodes->elt->getMeaning(pPartOfSpeech);
    if (resMeaning !=  nullptr)
    {
      // if the other sub words are the same
      ALLingdbMultiMeaningsNode* multiMeanings = multiMeaningNodes->elt->getDatasIfItsAMultiMeaningNode();
      if (multiMeanings->isStrEqualToListOfLemmes(pLemma, mainLemma.size() + 1))
      {
        return resMeaning;
      }
    }
    multiMeaningNodes = multiMeaningNodes->next;
  }

  return nullptr;
}


ALLingdbConcept* LinguisticIntermediaryDatabase::getConcept
(const std::string& pConceptName) const
{
  auto it = fConceptNameToPtr.find(pConceptName);
  if (it != fConceptNameToPtr.end())
    return it->second;
  return nullptr;
}


ALLingdbConcept* LinguisticIntermediaryDatabase::addConcept
(bool& pNewCptHasBeenInserted,
 const std::string& pConceptName,
 bool pAutoFill)
{
//  ALLingdbConcept* cpt = getConcept(pConceptName);
  ALLingdbConcept* cpt = nullptr;
  auto it = fConceptNameToPtr.find(pConceptName);
  if (it != fConceptNameToPtr.end())
    cpt = it->second;

  if (cpt == nullptr)
  {
    cpt = fAlloc.allocate<ALLingdbConcept>(1);
    cpt->xInit(fAlloc, pConceptName, pAutoFill);
    fConceptNameToPtr.emplace(pConceptName, cpt);
    pNewCptHasBeenInserted = true;
  }
  else
  {
    pNewCptHasBeenInserted = false;
  }
  return cpt;
}


void LinguisticIntermediaryDatabase::removeAllTags()
{
  ALLingdbAnimationsTag* currTag = fAlloc.first<ALLingdbAnimationsTag>();
  while (currTag != nullptr)
  {
    currTag->xDeallocate(fAlloc);
    currTag = fAlloc.next<ALLingdbAnimationsTag>(currTag);
  }
}

void LinguisticIntermediaryDatabase::addWord
(const std::string& pWord,
 const std::string& pLemma,
 PartOfSpeech pPartOfSpeech,
 const std::vector<std::string>& pFlexions,
 char pFrequency)
{
  ALLingdbMeaning* meaning = getMeaning(pLemma, pPartOfSpeech);
  if (meaning != nullptr)
  {
    addWordWithSpecificMeaning(pWord, *meaning, pFlexions, pFrequency);
    return;
  }
  if (!fRoot)
  {
    return;
  }

  if (pWord != pLemma)
  {
    std::size_t posSepInLemme = pLemma.find('~');
    if (posSepInLemme != std::string::npos)
    {
      fMainWordToSimpleWordToAdd
          [pLemma.substr(0, posSepInLemme)].emplace_back(pWord, pLemma, pPartOfSpeech,
                                                         pFlexions, pFrequency);
      return;
    }
  }

  ALLingdbDynamicTrieNode* wordNode = fRoot->xInsertWord(fAlloc, pWord, 0);
  ALLingdbWordForms* wordForm = wordNode->addWordForm(*this, pLemma, pPartOfSpeech);

  wordForm->xAddFlexions(fAlloc, pPartOfSpeech, pFlexions);
  wordForm->setFrequency(pFrequency);
}


void LinguisticIntermediaryDatabase::addWordWithSpecificMeaning
(const std::string& pWord,
 ALLingdbMeaning& pMeaning,
 const std::vector<std::string>& pFlexions,
 char pFrequency)
{
  if (!fRoot)
  {
    return;
  }

  ALLingdbDynamicTrieNode* wordNode = fRoot->xInsertWord(fAlloc, pWord, 0);
  ALLingdbWordForms* wordForm = wordNode->addWordFormFromMeaning(*this, pMeaning);

  wordForm->xAddFlexions(fAlloc, pMeaning.getPartOfSpeech(), pFlexions);
  wordForm->setFrequency(pFrequency);
}


void LinguisticIntermediaryDatabase::addMultiMeaningsWord
(ALLingdbMeaning* pRootMeaning,
 std::list<std::pair<ALLingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings,
 PartOfSpeech pPartOfSpeech)
{
  if (!fRoot)
  {
    return;
  }

  ALLingdbDynamicTrieNode* simpleLemme = pRootMeaning->getLemma();
  ALLingdbDynamicTrieNode* newNode = simpleLemme->xAddMultiMeaningsNode
      (fAlloc, pRootMeaning, pLinkedMeanings);
  newNode->addWordFormFromMeaning(*this, *newNode->xAddMeaning(fAlloc, pPartOfSpeech));

  if (!fMainWordToSimpleWordToAdd.empty())
  {
    std::string rootLemma = simpleLemme->getWord();
    std::map<std::string, std::list<WordToAdd> >::iterator
        itMainLemme = fMainWordToSimpleWordToAdd.find(rootLemma);
    if (itMainLemme != fMainWordToSimpleWordToAdd.end())
    {
      std::string compeleteLemma = newNode->getWord();
      for (std::list<WordToAdd>::iterator itWordToAdds = itMainLemme->second.begin();
           itWordToAdds != itMainLemme->second.end(); ++itWordToAdds)
      {
        if (itWordToAdds->lemma == compeleteLemma)
        {
          addWord(itWordToAdds->word, itWordToAdds->lemma,
                  itWordToAdds->partOfSpeech,
                  itWordToAdds->flexions, itWordToAdds->frequency);
          itMainLemme->second.erase(itWordToAdds);
          if (itMainLemme->second.empty())
          {
            fMainWordToSimpleWordToAdd.erase(itMainLemme);
          }
          break;
        }
      }
    }
  }
}


ALLingdbAnimationsTag* LinguisticIntermediaryDatabase::addATag
(const std::string& pTag)
{
  if (pTag.empty())
  {
    return nullptr;
  }

  // Check if the tag don't already exist in the memory
  ALLingdbAnimationsTag* newTag = nullptr;
  ALLingdbAnimationsTag* tagsInMemory = fAlloc.first<ALLingdbAnimationsTag>();
  while (tagsInMemory != nullptr)
  {
    if (tagsInMemory->getTag()->toStr() == pTag)
    {
      newTag = tagsInMemory;
      break;
    }
    tagsInMemory = fAlloc.next<ALLingdbAnimationsTag>(tagsInMemory);
  }

  // Add a new tag in memory if necessary
  if (newTag == nullptr)
  {
    newTag = fAlloc.allocate<ALLingdbAnimationsTag>(1);
    newTag->xInit(fAlloc, pTag);
  }
  return newTag;
}



void LinguisticIntermediaryDatabase::removeWord
(const std::string& word)
{
  ALLingdbDynamicTrieNode* node = getPointerToEndOfWord(word);
  if (node != nullptr)
  {
    node->xRemoveAllWordForms(fAlloc);
  }
}

void LinguisticIntermediaryDatabase::removeWordForm
(const std::string& word,
 PartOfSpeech pPartOfSpeech)
{
  {
    ALLingdbDynamicTrieNode* node = getPointerToEndOfWord(word);
    if (node != nullptr)
    {
      node->xRemoveWordForm(fAlloc, pPartOfSpeech);
      return;
    }
  }

  // look in composed words
  std::string mainLemma;
  xGetMainLemma(mainLemma, word);
  if (mainLemma.empty())
  {
    return;
  }
  // get main lemma word node
  ALLingdbDynamicTrieNode* wordNode =
      fRoot->advanceInTrieIfEndOfAWord(mainLemma);
  if (wordNode == nullptr)
  {
    return;
  }
  const ForwardPtrList<ALLingdbDynamicTrieNode>*
      multiMeaningNodes = wordNode->getMultiMeaningsNodes();
  while (multiMeaningNodes != nullptr)
  {
    const ForwardPtrList<ALLingdbDynamicTrieNode>* nextMultiMeaningNodes =
        multiMeaningNodes->next;

    ALLingdbMeaning* resMeaning = multiMeaningNodes->elt->getMeaning(pPartOfSpeech);
    if (resMeaning !=  nullptr)
    {
      // if the other sub words are the same
      ALLingdbMultiMeaningsNode* multiMeanings = multiMeaningNodes->elt->getDatasIfItsAMultiMeaningNode();
      if (multiMeanings->isStrEqualToListOfLemmes(word, mainLemma.size() + 1))
      {
        multiMeaningNodes->elt->xRemoveWordForm(fAlloc, pPartOfSpeech);
      }
    }

    multiMeaningNodes = nextMultiMeaningNodes;
  }
}



void LinguisticIntermediaryDatabase::getConceptToMeanings
(std::map<const ALLingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings) const
{
  ALLingdbMeaning* meaning = fAlloc.first<ALLingdbMeaning>();
  while (meaning != nullptr)
  {
    const ForwardPtrList<ALLingdbLinkToAConcept>* meaningLinkToConcepts = meaning->getLinkToConcepts();
    while (meaningLinkToConcepts != nullptr)
    {
      pConceptToMeanings[meaningLinkToConcepts->elt->getConcept()].insert
          (MeaningAndConfidence(meaning, meaningLinkToConcepts->elt->getRelatedToConcept()));
      meaningLinkToConcepts = meaningLinkToConcepts->next;
    }
    meaning = fAlloc.next<ALLingdbMeaning>(meaning);
  }
}



ALLingdbDynamicTrieNode* LinguisticIntermediaryDatabase::getPointerToEndOfWord
(const std::string& word) const
{
  if (!fRoot)
  {
    return nullptr;
  }
  return fRoot->advanceInTrieIfEndOfAWord(word);
}




ALLingdbQuestionWords* LinguisticIntermediaryDatabase::getQuestionWords
() const
{
  return fInfos->questionWords;
}

ALLingdbDynamicTrieNode* LinguisticIntermediaryDatabase::getPointerInTrie
(const std::string& word) const
{
  if (!fRoot)
  {
    return nullptr;
  }
  return fRoot->advanceInTrie(word, true);
}


void LinguisticIntermediaryDatabase::newQWords
(ALLingdbQuestionWords* pQWords)
{
  if (fInfos->questionWords != nullptr)
  {
    fInfos->questionWords->xDeallocate(fAlloc);
    fInfos->questionWords = nullptr;
  }
  fInfos->questionWords = pQWords;
}



ALLingdbDynamicTrieNode* LinguisticIntermediaryDatabase::findComposedWordFromString
(const std::string& pStr) const
{
  if (!fRoot)
  {
    return nullptr;
  }

  // get ending nodes of each words AND
  // find the root word
  std::list<ALLingdbDynamicTrieNode*> subWords;
  const ForwardPtrList<ALLingdbDynamicTrieNode>* multiMeaningNodes = nullptr;
  std::size_t afterLastSeparator = 0;
  std::size_t currSeparator = pStr.find_first_of(" '");
  while (currSeparator != std::string::npos)
  {
    if (afterLastSeparator != currSeparator &&
        !xAddNewSubWord(multiMeaningNodes, subWords,
                       pStr.substr(afterLastSeparator, currSeparator - afterLastSeparator)))
    {
      return nullptr;
    }
    afterLastSeparator = currSeparator + 1;
    currSeparator = pStr.find_first_of(" '", afterLastSeparator);
  }
  if (afterLastSeparator != pStr.size() &&
      !xAddNewSubWord(multiMeaningNodes, subWords,
                     pStr.substr(afterLastSeparator, pStr.size() - afterLastSeparator)))
  {
    return nullptr;
  }


  // for each sub meanings possibilities (from the root word)
  while (multiMeaningNodes != nullptr)
  {
    ALLingdbMultiMeaningsNode* compMeaning = multiMeaningNodes->elt->getDatasIfItsAMultiMeaningNode();
    assert(compMeaning != nullptr);

    // for each sub meanings
    const ForwardPtrList<ALLingdbNodeLinkedMeaning>* linkedMeanings = compMeaning->getLinkedMeanings();
    if (linkedMeanings->length() == subWords.size())
    {
      while (linkedMeanings != nullptr)
      {
        bool subMeaningFound = false;
        for (std::list<ALLingdbDynamicTrieNode*>::iterator itSubWord = subWords.begin();
             itSubWord != subWords.end(); ++itSubWord)
        {
          if ((*itSubWord)->getWordFormFromMeaning(*linkedMeanings->elt->meaning) != nullptr)
          {
            subMeaningFound = true;
            break;
          }
        }
        if (!subMeaningFound)
        {
          break;
        }
        linkedMeanings = linkedMeanings->next;
      }

      // if we found all sub meanings
      if (linkedMeanings == nullptr &&
          multiMeaningNodes->elt->getWordForms() != nullptr)
      {
        return multiMeaningNodes->elt;
      }
    }

    multiMeaningNodes = multiMeaningNodes->next;
  }

  return nullptr;
}



bool LinguisticIntermediaryDatabase::xAddNewSubWord
(const ForwardPtrList<ALLingdbDynamicTrieNode>*& pMultiMeaningNodes,
 std::list<ALLingdbDynamicTrieNode*>& pSubWords,
 const std::string& pSubWord) const
{
  ALLingdbDynamicTrieNode* endOfWord = fRoot->advanceInTrieIfEndOfAWord(pSubWord);
  if (endOfWord == nullptr ||
      endOfWord->getWordForms() == nullptr)
  {
    return false;
  }
  if (pMultiMeaningNodes == nullptr)
  {
    pMultiMeaningNodes = endOfWord->getMultiMeaningsNodes();
    if (pMultiMeaningNodes == nullptr)
    {
      pSubWords.emplace_back(endOfWord);
    }
  }
  else
  {
    pSubWords.emplace_back(endOfWord);
  }
  return true;
}



void LinguisticIntermediaryDatabase::xInitDatabase()
{
  // root of the trie
  fRoot = fAlloc.allocate<ALLingdbDynamicTrieNode>(1);
  fRoot->xInit(CHAR_MIN);

  // database infos
  fAlloc.reserve<ALLingdbInfos>(1);
  fInfos = fAlloc.allocate<ALLingdbInfos>(1);
  fInfos->init();
}



} // End of namespace onsem

