#include "adjbeforenounextractor.hpp"
#include <QtXml>
#include <fstream>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>

namespace onsem
{


AdjBeforeNounExtractor::AdjBeforeNounExtractor
(const CompositePoolAllocator& pAlloc)
  : fAdjStats()
{
  // init map of verb to auxiliaries stats
  LingdbMeaning* meaning = pAlloc.first<LingdbMeaning>();
  while (meaning != nullptr)
  {
    if (meaning->getPartOfSpeech() == PartOfSpeech::ADJECTIVE)
    {
      fAdjStats[meaning->getLemma()->getWord()];
    }
    meaning = pAlloc.next<LingdbMeaning>(meaning);
  }
}


AdjBeforeNounExtractor::AdjBeforeNounExtractor(const std::vector<AdjBeforeNounExtractor>& pOtherAdjBeforeNounExtractors)
  : fAdjStats()
{
  for (const auto& currAdjBeforeNounExtractor : pOtherAdjBeforeNounExtractors)
    for (const auto& currAdjStats : currAdjBeforeNounExtractor.fAdjStats)
      fAdjStats[currAdjStats.first].add(currAdjStats.second);
}


void AdjBeforeNounExtractor::processText
(const linguistics::TokensTree& pTokensTree)
{
  for (linguistics::TokCstIt itTok = pTokensTree.tokens.begin();
       itTok != pTokensTree.tokens.end(); ++itTok)
  {
    if (itTok->inflWords.size() == 1 &&
        itTok->inflWords.front().word.partOfSpeech == PartOfSpeech::NOUN)
    {
      linguistics::TokCstIt prevTok = getPrevToken(itTok,
                                                   pTokensTree.tokens.begin(),
                                                   pTokensTree.tokens.end());
      if (prevTok != pTokensTree.tokens.end())
      {
        xRefreshAdjPositionInfo(prevTok->inflWords, true);
      }
      linguistics::TokCstIt nextTok  = getNextToken(itTok,
                                                    pTokensTree.tokens.end());
      if (nextTok != pTokensTree.tokens.end())
      {
        xRefreshAdjPositionInfo(nextTok->inflWords, false);
      }
    }
  }
}


void AdjBeforeNounExtractor::xRefreshAdjPositionInfo
(const std::list<linguistics::InflectedWord>& pIGrams,
 bool pBeforeAfterNoun)
{
  for (const auto& currIGram : pIGrams)
  {
    if (currIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE)
    {
      std::map<std::string, AdjStats>::iterator itAdjStats =
          fAdjStats.find(currIGram.word.lemma);
      if (itAdjStats != fAdjStats.end())
      {
        if (pBeforeAfterNoun)
        {
          ++itAdjStats->second.nbBeforeNoun;
        }
        else
        {
          ++itAdjStats->second.nbAfterNoun;
        }
      }
    }
  }
}




void AdjBeforeNounExtractor::writeResults
(const std::string& pResultFilename) const
{
  std::cout << "Write results life: " << pResultFilename << std::endl;
  std::ofstream resultFile(pResultFilename.c_str());
  for (std::map<std::string, AdjStats>::const_iterator
       it = fAdjStats.begin(); it != fAdjStats.end(); ++it)
  {
    resultFile << "#" << it->first << "\t#"
               << it->second.nbBeforeNoun << "_"
               << it->second.nbAfterNoun << "_"
               << std::endl;
  }
  resultFile.close();
}


void AdjBeforeNounExtractor::writeXml
(const std::string& pResultFilename,
 const std::string& pResultFilenameXml)
{
  std::cout << "Write xml life: " << pResultFilenameXml << std::endl;
  std::ifstream resultFile(pResultFilename.c_str(), std::ifstream::in);
  if (!resultFile.is_open())
  {
    std::cerr << "Error: Can't open " << pResultFilename << " file !" << std::endl;
    return;
  }

  QDomDocument resultXml;
  QDomElement rootXml = resultXml.createElement("linguisticdatabase");
  resultXml.appendChild(rootXml);
  std::string line;
  while (getline(resultFile, line))
  {
    std::string lemma;
    std::vector<int> nbs(2);
    spitResultLine(lemma, nbs, line);
    int nbBeforeNoun = nbs[0];
    int nbAfterNoun = nbs[1];
    if (nbBeforeNoun > 50 &&
        nbBeforeNoun > nbAfterNoun)
    {
      QDomElement newAdjBefNoun = resultXml.createElement("existingMeaning");
      newAdjBefNoun.setAttribute("lemme", QString::fromUtf8(lemma.c_str()));
      newAdjBefNoun.setAttribute("gram",
                                 QString::fromUtf8(partOfSpeech_toStr(PartOfSpeech::ADJECTIVE).c_str()));
      QDomElement contextInfo = resultXml.createElement("contextInfo");
      contextInfo.setAttribute("val", "canBeBeforeNoun");
      newAdjBefNoun.appendChild(contextInfo);
      rootXml.appendChild(newAdjBefNoun);
    }
  }
  resultFile.close();

  QFile file(QString::fromUtf8(pResultFilenameXml.c_str()));
  if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    std::cerr << "Open the file for writing failed" << std::endl;
  }
  else
  {
    QString text = resultXml.toString();
    file.write(text.toUtf8());
    file.close();
  }
}



} // End of namespace onsem
