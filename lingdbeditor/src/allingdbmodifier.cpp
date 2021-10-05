#include <onsem/lingdbeditor/allingdbmodifier.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/allingdbflexions.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/lingdbeditor/allingdbwordforms.hpp>

namespace onsem
{


std::size_t ALLingdbModifier::findQueAddQu
(LinguisticIntermediaryDatabase& pLingDatabase) const
{
  std::size_t nbAddedWords = 0;
  ALLingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    std::string word = currNode->getWord();

    if (xEndWithQue(word))
    {
      std::string newWord = word.substr(0, word.size() - 1);
      const ForwardPtrList<ALLingdbWordForms>* wf = currNode->getWordForms();
      while (wf != nullptr)
      {
        pLingDatabase.addWordWithSpecificMeaning
            (newWord, *wf->elt->getMeaning(),
             std::vector<std::string>(), 4); // TODO: put same flexion infos that the original word
        wf = wf->next;
      }
    }
    else if (xEndWithQuApostrophe(word))
    {
      pLingDatabase.removeWord(word);
      currNode = pLingDatabase.getRoot()->advanceInTrie(word, false);
    }
    currNode = currNode->getNextWordNode();
  }
  return nbAddedWords;
}


void ALLingdbModifier::associateANewGramForAMeaning
(LinguisticIntermediaryDatabase& pLingDatabase,
 const std::string& pLemma,
 PartOfSpeech pRefPartOfSpeech,
 PartOfSpeech pNewPartOfSpeech) const
{
  // Get the reference meaning
  ALLingdbMeaning* refMeaning = pLingDatabase.getMeaning(pLemma, pRefPartOfSpeech);
  if (refMeaning == nullptr)
  {
    return;
  }

  // Associate the new meaning to the reference meaning
  ALLingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    ALLingdbWordForms* refWF = xGetWordFormThatAsASpecificMeaning(currNode->getWordForms(), refMeaning);
    if (refWF != nullptr)
    {
      ALLingdbWordForms* newWF = currNode->addWordForm(pLingDatabase, pLemma, pNewPartOfSpeech);
      newWF->copyFlexions(pLingDatabase, refWF->getFlexions());
      newWF->setFrequency(refWF->getFrequency());
    }
    currNode = currNode->getNextWordNode();
  }
}



std::size_t ALLingdbModifier::addToAtBeginOfVerbsForEnglish
(LinguisticIntermediaryDatabase& pLingDatabase) const
{
  std::list<ALLingdbWordForms*> imperativeVerbs;
  ALLingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    const ForwardPtrList<ALLingdbWordForms>* wf = currNode->getWordForms();
    while (wf != nullptr)
    {
      if (wf->elt->getMeaning()->getPartOfSpeech() == PartOfSpeech::VERB)
      {
        const ALLingdbFlexions* flexions = wf->elt->getFlexions();
        if (flexions != nullptr &&
            flexions->replaceInfinitiveByImperative())
        {
          imperativeVerbs.emplace_back(wf->elt);
        }
      }
      wf = wf->next;
    }
    currNode = currNode->getNextWordNode();
  }

  for (std::list<ALLingdbWordForms*>::iterator it = imperativeVerbs.begin();
       it != imperativeVerbs.end(); ++it)
  {
    std::string lemma = (*it)->getMeaning()->getLemma()->getWord();
    std::string infinitive = "to " + lemma;
    pLingDatabase.addWord(infinitive,
                          (*it)->getMeaning()->getLemma()->getWord(),
                          (*it)->getMeaning()->getPartOfSpeech(),
                          std::vector<std::string>(1, "W"), 4); // infinitive
  }

  return imperativeVerbs.size();
}




std::size_t ALLingdbModifier::delWordsWithACapitalLetter
(LinguisticIntermediaryDatabase& pLingDatabase) const
{
  std::size_t nbDeletedWords = 0;
  ALLingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    std::string word = currNode->getWord();
    if (xHasACapitalLetter(word))
    {
      pLingDatabase.removeWord(word);
      currNode = pLingDatabase.getRoot()->advanceInTrie(word, false);
      ++nbDeletedWords;
    }
    currNode = currNode->getNextWordNode();
  }
  return nbDeletedWords;
}


std::size_t ALLingdbModifier::delExprs
(LinguisticIntermediaryDatabase& pLingDatabase) const
{
  std::size_t nbDeletedWords = 0;
  ALLingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    if (currNode->isExpr())
    {
      std::string word = currNode->getWord();
      pLingDatabase.removeWord(word);
      currNode = pLingDatabase.getRoot()->advanceInTrie(word, false);
      ++nbDeletedWords;
    }
    currNode = currNode->getNextWordNode();
  }
  return nbDeletedWords;
}


std::size_t ALLingdbModifier::delAllWords
(LinguisticIntermediaryDatabase& pWords) const
{
  std::size_t nbDeletedWords = 0;
  ALLingdbDynamicTrieNode* currNode = pWords.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    std::string word = currNode->getWord();
    pWords.removeWord(word);
    currNode = pWords.getRoot()->advanceInTrie(word, false);
    currNode = currNode->getNextWordNode();
    ++nbDeletedWords;
  }
  return nbDeletedWords;
}


bool ALLingdbModifier::xHasACapitalLetter
(const std::string& pWord) const
{
  for (std::size_t i = 0; i < pWord.size(); ++i)
  {
    if (pWord[i] >= 'A' && pWord[i] <= 'Z')
    {
      return true;
    }
  }
  return false;
}


bool ALLingdbModifier::xEndWithQue
(const std::string& pWord) const
{
  if (pWord.size() < 4)
  {
    return false;
  }
  return pWord.compare(pWord.size() - 4, 4, " que") == 0;
}

bool ALLingdbModifier::xEndWithQuApostrophe
(const std::string& pWord) const
{
  if (pWord.size() < 4)
  {
    return false;
  }
  return pWord.compare(pWord.size() - 4, 4, " qu'") == 0;
}

ALLingdbWordForms* ALLingdbModifier::xGetWordFormThatAsASpecificMeaning
(const ForwardPtrList<ALLingdbWordForms>* pWf,
 ALLingdbMeaning* pMeaning) const
{
  while (pWf != nullptr)
  {
    if (pWf->elt->getMeaning() == pMeaning)
    {
      return pWf->elt;
    }
    pWf = pWf->next;
  }
  return nullptr;
}



} // End of namespace onsem
