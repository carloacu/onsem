#include <onsem/lingdbeditor/savers/traductionwriter.hpp>
#include <boost/filesystem/fstream.hpp>

namespace onsem
{

void LingdbSaverTraductionWriter::writeTranslations
(const boost::filesystem::path& pFilnename,
 const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads1,
 const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads2,
 const LinguisticIntermediaryDatabase& pLingDatabase) const
{
  int nbMeaningsWithTrad = 0;
  int nbMeanings = 0;
  boost::filesystem::ofstream outfile(pFilnename);

  std::map<const ALLingdbMeaning*, std::set<MeaningAndConfidence> > allTrads;
  const ALCompositePoolAllocator& alloc = pLingDatabase.getFPAlloc();
  ALLingdbMeaning* meaning = alloc.first<ALLingdbMeaning>();
  while (meaning != nullptr)
  {
    const LingdbSaverOutLinks* outLk1 = xGetOutLkForAMeaning(pTrads1, meaning);
    const LingdbSaverOutLinks* outLk2 = xGetOutLkForAMeaning(pTrads2, meaning);
    ++nbMeanings;
    if (outLk1 != nullptr || outLk2 != nullptr)
    {
      ++nbMeaningsWithTrad;

      std::set<MeaningAndConfidence>& tradWithConfidences = allTrads[meaning];
      if (outLk1 != nullptr)
      {
        xGetTradsWithConfidence(tradWithConfidences,
                                meaning->getPartOfSpeech(), *outLk1);
        if (outLk2 != nullptr)
        {
          std::set<MeaningAndConfidence> tradWithConfidences2;
          xGetTradsWithConfidence(tradWithConfidences2,
                                  meaning->getPartOfSpeech(), *outLk2);
          xMerge2Confidences_withSecondConfLessImportant(tradWithConfidences, tradWithConfidences2);
        }
      }
      else
      {
        xGetTradsWithConfidence(tradWithConfidences,
                                meaning->getPartOfSpeech(), *outLk2);
      }

      xWriteMeaning(outfile, meaning);
      outfile << ":";
      xWriteTrads(outfile, ",", tradWithConfidences);
      outfile << std::endl;
    }

    meaning = alloc.next<ALLingdbMeaning>(meaning);
  }

  if (nbMeanings == 0)
  {
    std::cerr << "no meanings found!" << std::endl;
    return;
  }

  std::cout << "before add words: nbMeaningsWithTrad / nbMeanings: " <<
               nbMeaningsWithTrad << " / " << nbMeanings << " = "
            << static_cast<float>(nbMeaningsWithTrad) / static_cast<float>(nbMeanings) << std::endl;


  std::map<const ALLingdbMeaning*, std::set<MeaningAndConfidence> > newTrads;
  ALLingdbDynamicTrieNode* currNode = pLingDatabase.getRoot()->getNextWordNode();
  while (currNode != nullptr)
  {
    std::set<const std::set<MeaningAndConfidence>*> tradsOfNode;
    std::set<ALLingdbMeaning*> meaningsWithNoTrad;
    const ForwardPtrList<ALLingdbWordForms>* wfs = currNode->getWordForms();
    while (wfs != nullptr)
    {
      std::map<const ALLingdbMeaning*, std::set<MeaningAndConfidence> >::const_iterator itTrad = allTrads.find(wfs->elt->getMeaning());
      if (itTrad != allTrads.end())
      {
        tradsOfNode.insert(&itTrad->second);
      }
      else
      {
        meaningsWithNoTrad.insert(wfs->elt->getMeaning());
      }
      wfs = wfs->next;
    }

    if (!tradsOfNode.empty() && !meaningsWithNoTrad.empty())
    {
      for (std::set<ALLingdbMeaning*>::iterator itMWNT = meaningsWithNoTrad.begin();
           itMWNT != meaningsWithNoTrad.end(); ++itMWNT)
      {
         for (std::set<const std::set<MeaningAndConfidence>*>::iterator itTON = tradsOfNode.begin();
              itTON != tradsOfNode.end(); ++itTON)
         {
           newTrads[*itMWNT].insert((*itTON)->begin(), (*itTON)->end());
         }
      }
    }
    currNode = currNode->getNextWordNode();
  }


  while (!newTrads.empty())
  {
    std::map<const ALLingdbMeaning*, std::set<MeaningAndConfidence> >::iterator itNewTrad = newTrads.begin();
    ++nbMeaningsWithTrad;

    xWriteMeaning(outfile, itNewTrad->first);
    outfile << ":";
    xWriteTrads(outfile, ",auto,", itNewTrad->second);
    outfile << std::endl;

    newTrads.erase(itNewTrad);
  }


  std::cout << "after add words: nbMeaningsWithTrad / nbMeanings: " <<
               nbMeaningsWithTrad << " / " << nbMeanings << " = "
            << static_cast<float>(nbMeaningsWithTrad) / static_cast<float>(nbMeanings) << std::endl;

  outfile.close();
}




void LingdbSaverTraductionWriter::writeTraductionsForOneWiki
(const std::string& pFilnename,
 const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads)
{
  std::ofstream outfile(pFilnename.c_str());

  for (std::map<ALLingdbMeaning*, LingdbSaverOutLinks>::const_iterator
       itTrad = pTrads.begin(); itTrad != pTrads.end(); ++itTrad)
  {
    std::set<MeaningAndConfidence> tradWithConfidences;
    xGetTradsWithConfidence(tradWithConfidences,
                            itTrad->first->getPartOfSpeech(), itTrad->second);


    xWriteMeaning(outfile, itTrad->first);
    outfile << ":";
    xWriteTrads(outfile, ",", tradWithConfidences);
    outfile << std::endl;
  }

  outfile.close();
}


void LingdbSaverTraductionWriter::writeTraductionsForOneWiki_fromReverseTraductions
(const std::string& pFilnename,
 const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads)
{
  std::ofstream outfile(pFilnename.c_str());

  for (std::map<ALLingdbMeaning*, LingdbSaverOutLinks>::const_iterator
       itTrad = pTrads.begin(); itTrad != pTrads.end(); ++itTrad)
  {
    std::set<MeaningAndConfidence> tradWithConfidences;
    for (std::set<ALLingdbMeaning*>::const_iterator itLWM =  itTrad->second.linkedWithInMeaningGram.begin();
         itLWM !=  itTrad->second.linkedWithInMeaningGram.end(); ++itLWM)
    {
      tradWithConfidences.insert(MeaningAndConfidence(*itLWM, 2));
    }

    for (std::set<ALLingdbMeaning*>::const_iterator itNLWM =  itTrad->second.notLinkedWithInMeaningGram.begin();
         itNLWM !=  itTrad->second.notLinkedWithInMeaningGram.end(); ++itNLWM)
    {
      if (xGetMeaningOfTradWC(tradWithConfidences, *itNLWM) == tradWithConfidences.end())
      {
        tradWithConfidences.insert(MeaningAndConfidence(*itNLWM, 1));
      }
    }

    xWriteMeaning(outfile, itTrad->first);
    outfile << ":";
    xWriteTrads(outfile, ",", tradWithConfidences);
    outfile << std::endl;
  }

  outfile.close();
}



void LingdbSaverTraductionWriter::writeSummaryTraductions
(const std::string& pFilnename,
 const std::map<ALLingdbMeaning*, std::set<MeaningAndConfidence> >& pTrads)
{
  std::ofstream outfile(pFilnename.c_str());

  for (std::map<ALLingdbMeaning*, std::set<MeaningAndConfidence> >::const_iterator
       itTrad = pTrads.begin(); itTrad != pTrads.end(); ++itTrad)
  {
    xWriteMeaning(outfile, itTrad->first);
    outfile << ":";
    xWriteTrads(outfile, ",", itTrad->second);
    outfile << std::endl;
  }

  outfile.close();
}




void LingdbSaverTraductionWriter::xMerge2Confidences_withSecondConfLessImportant
(std::set<MeaningAndConfidence>& pTradWithConfidences,
 const std::set<MeaningAndConfidence>& pTradWithConfidences2) const
{
  for (std::set<MeaningAndConfidence>::const_iterator itTrad2 = pTradWithConfidences2.begin();
       itTrad2 != pTradWithConfidences2.end(); ++itTrad2)
  {
    std::set<MeaningAndConfidence>::const_iterator itTrad1 = xGetMeaningOfTradWC
        (pTradWithConfidences, itTrad2->meaning);
    if (itTrad1 == pTradWithConfidences.end())
    {
      pTradWithConfidences.insert(*itTrad2);
    }
    else
    {
      char newConfidence = static_cast<char>(itTrad1->confidence + itTrad2->confidence);
      if (itTrad2->confidence > 0)
      {
        newConfidence = static_cast<char>(newConfidence - 1);
      }
      pTradWithConfidences.erase(itTrad1);
      pTradWithConfidences.insert(MeaningAndConfidence(itTrad2->meaning, newConfidence));
    }
  }
}



const LingdbSaverOutLinks* LingdbSaverTraductionWriter::xGetOutLkForAMeaning
(const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads,
 ALLingdbMeaning* pMeaning) const
{
  std::map<ALLingdbMeaning*, LingdbSaverOutLinks>::const_iterator itTrad = pTrads.find(pMeaning);
  if (itTrad != pTrads.end())
  {
    return &itTrad->second;
  }
  return nullptr;
}




void LingdbSaverTraductionWriter::xGetTradsWithConfidence
(std::set<MeaningAndConfidence>& pTradWithConfidences,
 PartOfSpeech pInPartOfSpeech,
 const LingdbSaverOutLinks& pOutLinks) const
{
  for (std::set<ALLingdbMeaning*>::const_iterator itLWM = pOutLinks.linkedWithInMeaningGram.begin();
       itLWM != pOutLinks.linkedWithInMeaningGram.end(); ++itLWM)
  {
    pTradWithConfidences.insert(MeaningAndConfidence
                                (*itLWM, (*itLWM)->getPartOfSpeech() == pInPartOfSpeech ? 5 : 4));
  }

  for (std::set<ALLingdbMeaning*>::const_iterator itNLWM = pOutLinks.notLinkedWithInMeaningGram.begin();
       itNLWM != pOutLinks.notLinkedWithInMeaningGram.end(); ++itNLWM)
  {
    if (xGetMeaningOfTradWC(pTradWithConfidences, *itNLWM) == pTradWithConfidences.end())
    {
      pTradWithConfidences.insert(MeaningAndConfidence
                                  (*itNLWM, (*itNLWM)->getPartOfSpeech() == pInPartOfSpeech ? 3 : 2));
    }
  }
}


std::set<MeaningAndConfidence>::const_iterator LingdbSaverTraductionWriter::xGetMeaningOfTradWC
(const std::set<MeaningAndConfidence>& pTradWithConfidences,
 const ALLingdbMeaning* pMeaning) const
{
  for (std::set<MeaningAndConfidence>::const_iterator it = pTradWithConfidences.begin();
       it != pTradWithConfidences.end(); ++it)
  {
    if (it->meaning == pMeaning)
    {
      return it;
    }
  }
  return pTradWithConfidences.end();
}



void LingdbSaverTraductionWriter::xWriteTrads
(std::ofstream& pOutfile,
 const std::string& pSep,
 const std::set<MeaningAndConfidence>& pTradWithConfidences) const
{
  for (std::set<MeaningAndConfidence>::const_iterator itTrad = pTradWithConfidences.begin();
       itTrad != pTradWithConfidences.end(); ++itTrad)
  {
    xWriteMeaningWithConfidence(pOutfile, itTrad->meaning, pSep,
                                itTrad->confidence);
  }
}


void LingdbSaverTraductionWriter::xWriteMeaning
(std::ofstream& pOutfile,
 const ALLingdbMeaning* pMeaning) const
{
  pOutfile << "<";
  xWriteStr(pOutfile, pMeaning->getLemma()->getWord());
  pOutfile  << "," << partOfSpeech_toStr(pMeaning->getPartOfSpeech()) << ">";
}

void LingdbSaverTraductionWriter::xWriteMeaningWithConfidence
(std::ofstream& pOutfile,
 const ALLingdbMeaning* pMeaning,
 const std::string& pSep,
 char pConfidence) const
{
  pOutfile << "<";
  xWriteStr(pOutfile, pMeaning->getLemma()->getWord());
  pOutfile  << "," << partOfSpeech_toStr(pMeaning->getPartOfSpeech())
            << pSep << static_cast<int>(pConfidence) << ">";
}


void LingdbSaverTraductionWriter::xWriteStr
(std::ofstream& pOutfile,
 const std::string& pStr) const
{
  std::size_t begSubStr = 0;
  std::size_t specCharact = pStr.find_first_of("\\,>", begSubStr);
  while (specCharact != std::string::npos)
  {
    if (specCharact > begSubStr)
    {
      pOutfile << pStr.substr(begSubStr, specCharact - begSubStr);
    }
    pOutfile << "\\" << pStr[specCharact];
    begSubStr = specCharact + 1;
    specCharact = pStr.find_first_of("\\,>", begSubStr);
  }

  if (pStr.size() > begSubStr)
  {
    pOutfile << pStr.substr(begSubStr, pStr.size() - begSubStr);
  }
}

} // End of namespace onsem

