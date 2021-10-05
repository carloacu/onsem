#include <onsem/lingdbeditor/allingdbmeaningtowords.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <onsem/lingdbeditor/allingdbwordforms.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/allingdbflexions.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>


namespace onsem
{

void ALLingdbMeaningToWords::findWordsConjugaisons
(std::map<const ALLingdbMeaning*, VerbConjugaison>& pVerbConjugaison,
 std::map<const ALLingdbMeaning*, NounAdjConjugaison>& pNounConjugaison,
 const LinguisticIntermediaryDatabase& pLingDatabase) const
{
  // iterate over all the words
  ALLingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    // iterator over all the wordForms
    const ForwardPtrList<ALLingdbWordForms>* wf = currNode->getWordForms();
    while (wf != nullptr)
    {
      // if it's a verb
      ALLingdbMeaning* currMeaning = wf->elt->getMeaning();
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
