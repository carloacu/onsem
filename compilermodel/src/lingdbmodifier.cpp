#include <onsem/compilermodel/lingdbmodifier.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbflexions.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>

namespace onsem
{


std::size_t LingdbModifier::findQueAddQu
(LinguisticIntermediaryDatabase& pLingDatabase) const
{
  std::size_t nbAddedWords = 0;
  LingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    std::string word = currNode->getWord();

    if (xEndWithQue(word))
    {
      std::string newWord = word.substr(0, word.size() - 1);
      const ForwardPtrList<LingdbWordForms>* wf = currNode->getWordForms();
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


void LingdbModifier::associateANewGramForAMeaning
(LinguisticIntermediaryDatabase& pLingDatabase,
 const std::string& pLemma,
 PartOfSpeech pRefPartOfSpeech,
 PartOfSpeech pNewPartOfSpeech) const
{
  // Get the reference meaning
  LingdbMeaning* refMeaning = pLingDatabase.getMeaning(pLemma, pRefPartOfSpeech);
  if (refMeaning == nullptr)
  {
    return;
  }

  // Associate the new meaning to the reference meaning
  LingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    LingdbWordForms* refWF = xGetWordFormThatAsASpecificMeaning(currNode->getWordForms(), refMeaning);
    if (refWF != nullptr)
    {
      LingdbWordForms* newWF = currNode->addWordForm(pLingDatabase, pLemma, pNewPartOfSpeech);
      newWF->copyFlexions(pLingDatabase, refWF->getFlexions());
      newWF->setFrequency(refWF->getFrequency());
    }
    currNode = currNode->getNextWordNode();
  }
}



std::size_t LingdbModifier::addToAtBeginOfVerbsForEnglish
(LinguisticIntermediaryDatabase& pLingDatabase) const
{
  std::list<LingdbWordForms*> imperativeVerbs;
  LingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    const ForwardPtrList<LingdbWordForms>* wf = currNode->getWordForms();
    while (wf != nullptr)
    {
      if (wf->elt->getMeaning()->getPartOfSpeech() == PartOfSpeech::VERB)
      {
        const LingdbFlexions* flexions = wf->elt->getFlexions();
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

  for (std::list<LingdbWordForms*>::iterator it = imperativeVerbs.begin();
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


std::size_t LingdbModifier::delExprs
(LinguisticIntermediaryDatabase& pLingDatabase) const
{
  std::size_t nbDeletedWords = 0;
  LingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
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


std::size_t LingdbModifier::delAllWords
(LinguisticIntermediaryDatabase& pWords) const
{
  std::size_t nbDeletedWords = 0;
  LingdbDynamicTrieNode* currNode = pWords.getRoot()->getNextWordNode();
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

bool LingdbModifier::xEndWithQue
(const std::string& pWord) const
{
  if (pWord.size() < 4)
  {
    return false;
  }
  return pWord.compare(pWord.size() - 4, 4, " que") == 0;
}

bool LingdbModifier::xEndWithQuApostrophe
(const std::string& pWord) const
{
  if (pWord.size() < 4)
  {
    return false;
  }
  return pWord.compare(pWord.size() - 4, 4, " qu'") == 0;
}

LingdbWordForms* LingdbModifier::xGetWordFormThatAsASpecificMeaning
(const ForwardPtrList<LingdbWordForms>* pWf,
 LingdbMeaning* pMeaning) const
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
