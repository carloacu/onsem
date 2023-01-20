#include "helpers.hpp"
#include <QDomDocument>
#include <QFile>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>


namespace onsem
{

FrGrammarBookExtHelpers::FrGrammarBookExtHelpers
(const linguistics::StaticLinguisticDictionary& pBinDico,
 const LingdbTree& pLingDbTree)
  : fBinDico(pBinDico),
    fSePronCompWord(),
    fQuelqueChoseStr("quelquechose"),
    fQuelquePartStr("quelquepart"),
    fQuelquUnStr("quelqu'un"),
    fPreps(),
    fPrepToChunkLinkType()
{
  fSePronCompWord.language = SemanticLanguageEnum::FRENCH;
  fSePronCompWord.lemma = "se";
  fSePronCompWord.partOfSpeech = PartOfSpeech::PRONOUN_COMPLEMENT;

  LinguisticIntermediaryDatabase lingDb;
  lingDb.setLanguage(semanticLanguageEnum_toLegacyStr(SemanticLanguageEnum::FRENCH));
  lingDb.load(pLingDbTree.getDynamicDatabasesFolder() + "/" +
              lingDb.getLanguage()->toStr() + "." +
              pLingDbTree.getExtDynDatabase());

  const CompositePoolAllocator& alloc = lingDb.getFPAlloc();
  LingdbMeaning* meaning = alloc.first<LingdbMeaning>();
  while (meaning != nullptr)
  {
    if (meaning->getPartOfSpeech() == PartOfSpeech::PREPOSITION)
    {
      fPreps[StrSortedBySize(meaning->getLemma()->getWord())];
    }
    meaning = alloc.next<LingdbMeaning>(meaning);
  }

  fPrepToChunkLinkType["à"] = linguistics::ChunkLinkType::INDIRECTOBJECT;
  fPrepToChunkLinkType["par"] = linguistics::ChunkLinkType::MANNER;
}


void FrGrammarBookExtHelpers::removeAllSpacesAndSplit
(std::list<std::string>& pRes,
 const std::string& pStr)
{
  std::string newStr(pStr.size(), ' ');
  std::size_t newSize = 0;
  std::size_t nbBlanckInARow = 0;
  for (std::size_t i = 0; i < pStr.size(); ++i)
  {
    if (pStr[i] != ' ')
    {
      newStr[newSize] = pStr[i];
      ++newSize;
      nbBlanckInARow = 0;
    }
    else
    {
      ++nbBlanckInARow;
      if (nbBlanckInARow > 5)
      {
        if (newSize > 0)
        {
          newStr.resize(newSize);
          pRes.push_back(newStr);
          newStr.resize(pStr.size());
          newSize = 0;
        }
        if (nbBlanckInARow > 50)
        {
          break;
        }
      }
    }
  }

  if (newSize > 0)
  {
    newStr.resize(newSize);
    pRes.push_back(newStr);
  }
}

void FrGrammarBookExtHelpers::removeAfterDoublePoint
(std::string& pStr)
{
  std::size_t doublePointPos = pStr.find_first_of(':');
  if (doublePointPos != std::string::npos)
  {
    pStr = pStr.substr(0, doublePointPos);
  }
}


void FrGrammarBookExtHelpers::removeBackSlashRAtEnd
(std::string& pStr)
{
  if (pStr[pStr.size() - 1] == '\r')
  {
    pStr = pStr.substr(0, pStr.size() - 1);
  }
}


void FrGrammarBookExtHelpers::toLowerCase
(std::string& pStr)
{
  for (std::size_t i = 0; i < pStr.size(); ++i)
  {
    if (pStr[i] >= 'A' && pStr[i] <= 'Z')
    {
      pStr[i] = static_cast<char>(pStr[i] + 'a' - 'A');
    }
    else if (pStr.compare(i, 2, "É") == 0)
    {
      pStr.replace(i, 2, "é");
    }
    else
    {
      std::string capLetter = "Ê";
      if (pStr.compare(i, capLetter.size(), capLetter) == 0)
      {
        pStr.replace(i, capLetter.size(), "ê");
      }
    }
  }
}

void FrGrammarBookExtHelpers::replaceSomeCharsToBlanck
(std::string& pStr)
{
  for (std::size_t i = 0; i < pStr.size(); ++i)
  {
    if (pStr[i] == '|' || pStr[i] == '\\')
    {
      pStr[i] = ' ';
    }
  }
}


void FrGrammarBookExtHelpers::removeArrowAtBeginning
(std::string& pStr)
{
  pStr = pStr.substr(arrowStr.size(), pStr.size() - arrowStr.size());
}


bool FrGrammarBookExtHelpers::removeBlanksAtBeginning
(std::string& pStr,
 std::size_t pMaxNbToRemove)
{
  std::size_t firstChar = pStr.find_first_not_of(' ');
  if (firstChar > pMaxNbToRemove || firstChar == std::string::npos)
  {
    return false;
  }
  pStr = pStr.substr(firstChar, pStr.size() - firstChar);
  return true;
}

bool FrGrammarBookExtHelpers::findVerb
(std::list<std::string>& pVerbs,
 const std::string& pLine) const
{
  bool res = false;
  std::size_t parenthesisPos = pLine.find_first_of('(');
  if (parenthesisPos != std::string::npos)
  {
    if (pLine.compare(parenthesisPos, 3, "(ou") == 0)
    {
      res |= xTryToAddVerb(pVerbs, pLine.substr(0, parenthesisPos), false);
      parenthesisPos += 3;
      std::size_t endParenth = pLine.find_first_of(')', parenthesisPos);
      if (endParenth != std::string::npos)
      {
        res |= xTryToAddVerb(pVerbs, pLine.substr(parenthesisPos, endParenth - parenthesisPos),
                             false);
      }
    }
    else
    {
      res |= xTryToAddVerb(pVerbs, pLine.substr(0, parenthesisPos), true);
    }
  }
  else
  {
    res |= xTryToAddVerb(pVerbs, pLine.substr(0, parenthesisPos), false);
  }
  return res;
}



void FrGrammarBookExtHelpers::extractPrepsToObjectType
(std::set<PrepToObjectType>& pPrepsToObjectType,
 const std::string& pStr) const
{
  std::list<std::string> currPreps;
  std::size_t strPos = 0;

  while (strPos != std::string::npos)
  {
    if (xTryToDetectAnyObjectType(pPrepsToObjectType, currPreps,
                                  strPos, pStr) ||
        xTryToDetectAPrep(currPreps, strPos, pStr))
    {
      continue;
    }
    if (pStr[strPos] == '(' || pStr[strPos] == ')' ||
        pStr[strPos] == '/')
    {
      ++strPos;
      continue;
    }
    break;
  }
}


void FrGrammarBookExtHelpers::removeVerbsThatOccur2Times
(std::list<VerbToPreps>& pVerbToPreps)
{
  if (pVerbToPreps.size() < 4)
  {
    return;
  }
  std::list<VerbToPreps>::iterator itPrev = pVerbToPreps.begin();
  std::list<VerbToPreps>::iterator itPrevPrev = ++pVerbToPreps.begin();
  for (std::list<VerbToPreps>::iterator it = ++(++pVerbToPreps.begin());
       it != pVerbToPreps.end(); ++it)
  {
    if (it->currVerbs.front() == itPrev->currVerbs.front())
    {
      itPrevPrev->prepsToObjectType.insert(itPrev->prepsToObjectType.begin(),
                                           itPrev->prepsToObjectType.end());
      pVerbToPreps.erase(itPrev);
    }
    else
    {
      itPrevPrev = itPrev;
    }
    itPrev = it;
  }
}

void FrGrammarBookExtHelpers::writeMostFrequentAfterPrep
(std::ofstream& pInfosFile,
 const std::list<FrGrammarBookExtHelpers::VerbToPreps>& pVbToPrepsList)
{
  std::map<std::string, std::map<FrGrammarBookObjectType, int> > prepsToStats;
  for (std::list<FrGrammarBookExtHelpers::VerbToPreps>::const_iterator
       it = pVbToPrepsList.begin(); it != pVbToPrepsList.end(); ++it)
  {
    for (std::set<FrGrammarBookExtHelpers::PrepToObjectType>::const_iterator
         itPrep = it->prepsToObjectType.begin();
         itPrep != it->prepsToObjectType.end(); ++itPrep)
    {
      ++prepsToStats[itPrep->prep][itPrep->objType];
    }
  }
  for (std::map<std::string, std::map<FrGrammarBookObjectType, int> >::const_iterator
       it = prepsToStats.begin(); it != prepsToStats.end(); ++it)
  {
    xWritePrepStats(pInfosFile, it->first, it->second);
  }
}


void FrGrammarBookExtHelpers::writeVerbsPrepsXmlFile
(const std::string& pVerbsPrepsXmlFilename,
 const std::list<FrGrammarBookExtHelpers::VerbToPreps>& pVbToPrepsList) const
{
  std::list<std::string> prepsToPrintAlways;
  prepsToPrintAlways.emplace_back("à");

  QDomDocument resultXml;
  QDomElement rootXml = resultXml.createElement("dictionary_modification");
  rootXml.setAttribute("language", QString::fromUtf8(semanticLanguageEnum_toLegacyStr(SemanticLanguageEnum::FRENCH).c_str()));
  resultXml.appendChild(rootXml);
  for (std::list<FrGrammarBookExtHelpers::VerbToPreps>::const_iterator
       itVbToPrep = pVbToPrepsList.begin(); itVbToPrep != pVbToPrepsList.end(); ++itVbToPrep)
  {
    for (std::list<std::string>::const_iterator itVb = itVbToPrep->currVerbs.begin();
         itVb != itVbToPrep->currVerbs.end(); ++itVb)
    {
      QDomElement newVerb = resultXml.createElement("word");
      newVerb.setAttribute("lemma", QString::fromUtf8(itVb->c_str()));
      newVerb.setAttribute("pos",
                           QString::fromUtf8(partOfSpeech_toStr(PartOfSpeech::VERB).c_str()));
      for (std::set<FrGrammarBookExtHelpers::PrepToObjectType>::const_iterator
           itPrep = itVbToPrep->prepsToObjectType.begin();
           itPrep != itVbToPrep->prepsToObjectType.end(); ++itPrep)
      {
        if (!itPrep->prep.empty() && itPrep->objType != OBJECTTYPE_AGENT &&
            xHas(prepsToPrintAlways, itPrep->prep) &&
            itPrep->objType == OBJECTTYPE_OBJECT)
        {
          auto itPrepToObjType = fPrepToChunkLinkType.find(itPrep->prep);
          if (itPrepToObjType != fPrepToChunkLinkType.end() &&
              itPrepToObjType->second == linguistics::ChunkLinkType::INDIRECTOBJECT)
          {
            QDomElement childElement = resultXml.createElement("include_child_list_template");
            childElement.setAttribute("name", "à-coi");
            newVerb.appendChild(childElement);
          }
        }
      }
      if (!newVerb.childNodes().isEmpty())
      {
        rootXml.appendChild(newVerb);
      }
    }
  }

  QFile file(QString::fromUtf8(pVerbsPrepsXmlFilename.c_str()));
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

bool FrGrammarBookExtHelpers::xTryToAddVerb
(std::list<std::string>& pVerbs,
 const std::string& pLine,
 bool pIsAPronominalVerb) const
{
  std::list<linguistics::InflectedWord> infosGram;
  fBinDico.getGramPossibilities(infosGram, pLine, 0, pLine.size());
  for (const linguistics::InflectedWord& currIGram : infosGram)
  {
    if (currIGram.word.partOfSpeech == PartOfSpeech::VERB)
    {
      if (pIsAPronominalVerb)
      {
        for (const linguistics::LingWordsGroup& metaMean: currIGram.infos.metaMeanings)
        {
          for (const auto& linkMean : metaMean.linkedMeanings)
          {
            if (*linkMean.first == fSePronCompWord)
            {
              pVerbs.emplace_back(pLine + "~se");
              return true;
            }
          }
        }
        return true;
      }
      pVerbs.push_back(pLine);
      return true;
    }
  }
  return false;
}


void FrGrammarBookExtHelpers::xWritePrepStats
(std::ofstream& pInfosFile,
 const std::string& pPrep,
 const std::map<FrGrammarBookObjectType, int>& pStats)
{
  std::map<FrGrammarBookObjectType, int>::const_iterator itBest = pStats.end();
  for (std::map<FrGrammarBookObjectType, int>::const_iterator
       itObjType = pStats.begin(); itObjType != pStats.end(); ++itObjType)
  {
    if (itObjType->second > 0 &&
        (itBest == pStats.end() || itObjType->second >= itBest->second))
    {
      itBest = itObjType;
    }
  }
  if (itBest != pStats.end())
  {
    if (!pPrep.empty())
    {
      fPreps[StrSortedBySize(pPrep)] = itBest->first;
    }
    pInfosFile << ">" << pPrep << ":"
               << FrGrammarBookObjectType_toStr[itBest->first]
               << "|\t";
    for (std::map<FrGrammarBookObjectType, int>::const_iterator
         itObjType = pStats.begin(); itObjType != pStats.end(); ++itObjType)
    {
      pInfosFile << " " << FrGrammarBookObjectType_toStr[itObjType->first]
                 << "(" << itObjType->second << ")";
    }
    pInfosFile << std::endl;
  }
}


bool FrGrammarBookExtHelpers::xTryToDetectAnyObjectType
(std::set<PrepToObjectType>& pPrepsToObjectType,
 std::list<std::string>& pCurrPreps,
 std::size_t& pStrPos,
 const std::string& pStr) const
{
  return xTryToDetectObjectType(pPrepsToObjectType, pCurrPreps, pStrPos, pStr,
                                fQuelqueChoseStr, OBJECTTYPE_OBJECT) ||
      xTryToDetectObjectType(pPrepsToObjectType, pCurrPreps, pStrPos, pStr,
                             fQuelquePartStr, OBJECTTYPE_LOCATION) ||
      xTryToDetectObjectType(pPrepsToObjectType, pCurrPreps, pStrPos, pStr,
                             fQuelquUnStr, OBJECTTYPE_AGENT);
}


bool FrGrammarBookExtHelpers::xTryToDetectObjectType
(std::set<PrepToObjectType>& pPrepsToObjectType,
 std::list<std::string>& pCurrPreps,
 std::size_t& pStrPos,
 const std::string& pStr,
 const std::string& pObjectStr,
 FrGrammarBookObjectType pObjectType) const
{
  if (pStr.compare(pStrPos, pObjectStr.size(), pObjectStr) == 0)
  {
    for (std::list<std::string>::const_iterator itPrep = pCurrPreps.begin();
         itPrep != pCurrPreps.end(); ++itPrep)
    {
      pPrepsToObjectType.insert(PrepToObjectType(*itPrep, pObjectType));
      if (pObjectType == OBJECTTYPE_AGENT)
      {
        pPrepsToObjectType.insert(PrepToObjectType(*itPrep, OBJECTTYPE_OBJECT));
      }
    }
    pStrPos += pObjectStr.size();
    bool insertedSomething = !pCurrPreps.empty();
    if (pStrPos < pStr.size())
    {
      if (pStr[pStrPos] == '/')
      {
        ++pStrPos;
        insertedSomething |= xTryToDetectAnyObjectType(pPrepsToObjectType, pCurrPreps, pStrPos, pStr);
      }
      else if (pStr[pStrPos] == '(')
      {
        ++pStrPos;
        insertedSomething |= xTryToDetectPrepsToAnObjectType
            (pPrepsToObjectType, pStrPos, pStr, pObjectType);
      }
    }

    if (!insertedSomething)
    {
      pPrepsToObjectType.insert(PrepToObjectType("", pObjectType));
      if (pObjectType == OBJECTTYPE_AGENT)
      {
        pPrepsToObjectType.insert(PrepToObjectType("", OBJECTTYPE_OBJECT));
      }
    }
    pCurrPreps.clear();
    return true;
  }
  return false;
}


bool FrGrammarBookExtHelpers::xTryToDetectPrepsToAnObjectType
(std::set<PrepToObjectType>& pPrepsToObjectType,
 std::size_t& pStrPos,
 const std::string& pStr,
 FrGrammarBookObjectType pObjectType) const
{
  bool res = false;
  for (; pStrPos < pStr.size(); ++pStrPos)
  {
    if (pStr[pStrPos] == ')')
    {
      break;
    }
    for (std::map<StrSortedBySize, std::optional<FrGrammarBookObjectType> >::const_iterator
         itPrep = fPreps.begin(); itPrep != fPreps.end(); ++itPrep)
    {
      if (pStr.compare(pStrPos, itPrep->first.str.size(), itPrep->first.str) == 0)
      {
        pPrepsToObjectType.insert(PrepToObjectType(itPrep->first.str, pObjectType));
        pStrPos += itPrep->first.str.size();
        res = true;
      }
    }
  }
  return res;
}


bool FrGrammarBookExtHelpers::xTryToDetectAPrep
(std::list<std::string>& pCurrPreps,
 std::size_t& pStrPos,
 const std::string& pStr) const
{
  for (auto itPrep = fPreps.begin(); itPrep != fPreps.end(); ++itPrep)
  {
    if (pStr.compare(pStrPos, itPrep->first.str.size(), itPrep->first.str) == 0)
    {
      pCurrPreps.push_back(itPrep->first.str);
      pStrPos += itPrep->first.str.size();

      if (pStrPos < pStr.size() &&
          pStr[pStrPos] == '/')
      {
        ++pStrPos;
        xTryToDetectAPrep(pCurrPreps, pStrPos, pStr);
      }
      return true;
    }
  }
  return false;
}


bool FrGrammarBookExtHelpers::xHas
(const std::list<std::string>& pList,
 const std::string& pStr)
{
  for (const auto& currElt : pList)
    if (currElt == pStr)
      return true;
  return false;
}


} // End of namespace onsem
