#include <onsem/compilermodel/lingdbmeaningtowords.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbflexions.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>


namespace onsem
{

void LingdbMeaningToWords::findWordsConjugaisons
(std::map<const LingdbMeaning*, VerbConjugaison>& pVerbConjugaison,
 std::map<const LingdbMeaning*, NounAdjConjugaison>& pNounConjugaison,
 const LinguisticIntermediaryDatabase& pLingDatabase) const
{
  // iterate over all the words
  LingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    // iterator over all the wordForms
    const ForwardPtrList<LingdbWordForms>* wf = currNode->getWordForms();
    while (wf != nullptr)
    {
      // if it's a verb
      LingdbMeaning* currMeaning = wf->elt->getMeaning();
      PartOfSpeech currGram = currMeaning->getPartOfSpeech();
      if (currGram == PartOfSpeech::VERB || currGram == PartOfSpeech::AUX)
      {
        auto itWordConjugaison = pVerbConjugaison.find(currMeaning);
        if (itWordConjugaison == pVerbConjugaison.end())
        {
          pVerbConjugaison[currMeaning];
          itWordConjugaison = pVerbConjugaison.find(currMeaning);
        }
        if (wf->elt->getFlexions() != nullptr)
        {
          wf->elt->getFlexions()->fillVerbConjugaison
              (itWordConjugaison->second, currNode, wf->elt->getFrequency());
        }
      }
      else if (partOfSpeech_isNominal(currGram) || currGram == PartOfSpeech::ADJECTIVE)
      {
        auto itNounConjugaison = pNounConjugaison.find(currMeaning);
        if (itNounConjugaison == pNounConjugaison.end())
        {
          pNounConjugaison[currMeaning];
          itNounConjugaison = pNounConjugaison.find(currMeaning);
        }
        if (wf->elt->getFlexions() != nullptr)
        {
          wf->elt->getFlexions()->fillNounConjugaison
              (itNounConjugaison->second, currNode, wf->elt->getFrequency());
        }
        else
        {
          itNounConjugaison->second.neutralSingular.newNodeCandidate(currNode, wf->elt->getFrequency());
        }
      }

      wf = wf->next;
    }

    currNode = currNode->getNextWordNode();
  }
}



} // End of namespace onsem
