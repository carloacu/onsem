#include "xmldatabaseloader.hpp"
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/databaserules/lingdbquestionwords.hpp>
#include "../concept/lingdbconcept.hpp"
#include "../lingdbanimationtag.hpp"

namespace onsem
{
// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)

namespace
{
void _loadXmlDirectory(const boost::filesystem::path& pFolderPath,
                       LinguisticIntermediaryDatabase& pLingDatabase,
                       const LingdbTree& pLingdbTree)
{
  boost::filesystem::directory_iterator itFolder(pFolderPath);
  boost::filesystem::directory_iterator endit;
  while (itFolder != endit)
  {
    const auto& currPath = itFolder->path();
    if (boost::filesystem::is_directory(itFolder->path()))
      _loadXmlDirectory(currPath, pLingDatabase, pLingdbTree);
    else
      XmlDatabaseLoader::merge(itFolder->path().string(), pLingDatabase, pLingdbTree);
    ++itFolder;
  }
}

}



int XmlDatabaseLoader::getVersion
(const std::string& pFilename)
{
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(pFilename, tree);

  auto versionChild = tree.get_child_optional("version");
  if (versionChild)
  {
    std::istringstream buffer(versionChild->get("<xmlattr>.val", "0"));
    unsigned int value;
    buffer >> value;
    return static_cast<int>(value);
  }
  return -1;
}


void XmlDatabaseLoader::merge
(const std::string& pFilename,
 LinguisticIntermediaryDatabase& pLingDatabase,
 const LingdbTree& pLingdbTree)
{
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(pFilename, tree);

  childLoop(tree, currSubTree, "linguisticdatabase")
  {
    const boost::property_tree::ptree& subTree = currSubTree.second;
    if (currSubTree.first == "word")
    {
      std::string word = subTree.get<std::string>("<xmlattr>.name", "");
      if (word.empty())
        continue;
      for(const auto& currWordTree : subTree)
      {
        if (currWordTree.first == "<xmlattr>")
          continue;
        std::string flStr = currWordTree.second.get<std::string>("<xmlattr>.flexions", "");
        std::vector<std::string> flexions;
        xSplit(flexions, flStr, ',');

        std::string frequencyStr = currWordTree.second.get<std::string>("<xmlattr>.frequency", "4");
        char frequencyChar = 4;
        try
        {
          frequencyChar = static_cast<char>(mystd::lexical_cast<int>(frequencyStr));
        }
        catch (const std::exception&)
        {
          throw std::runtime_error("\"" + frequencyStr + "\" is not a valid frequency");
        }
        pLingDatabase.addWord(word,
                              currWordTree.second.get<std::string>("<xmlattr>.lemme", word.c_str()),
                              partOfSpeech_fromStr(currWordTree.second.get<std::string>("<xmlattr>.gram", "")),
                              flexions, frequencyChar);
      }
    }
    else if (currSubTree.first == "words")
    {
      LingdbMeaning* meaningPtr = xExtractMeaning(subTree, pLingDatabase);
      if (meaningPtr != nullptr)
      {
        std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection> > linkedMeanings;
        for(const auto& currWordTree : subTree)
        {
          if (currWordTree.first == "<xmlattr>")
            continue;
          auto subTagName = currWordTree.first;
          if (subTagName == "nextMeaning" || subTagName == "prevMeaning" ||
              subTagName == "nextOrPrevMeaning")
          {
            LingdbMeaning* subMeaning = xExtractMeaning(currWordTree.second, pLingDatabase);
            if (subMeaning == nullptr)
            {
              throw std::runtime_error("Meaning not found, see previous error message for more infos");
            }
            if (subTagName == "nextMeaning")
            {
              linkedMeanings.emplace_back(subMeaning, LinkedMeaningDirection::FORWARD);
            }
            else if (subTagName == "prevMeaning")
            {
              linkedMeanings.emplace_back(subMeaning, LinkedMeaningDirection::BACKWARD);
            }
            else
            {
              linkedMeanings.emplace_back(subMeaning, LinkedMeaningDirection::BOTH);
            }
          }
          else if (subTagName == "wordForm")
          {
            std::string newGramStr = currWordTree.second.get<std::string>("<xmlattr>.gram", "");
            PartOfSpeech newGramEnum = partOfSpeech_fromStr(newGramStr);
            if (newGramEnum == PartOfSpeech::UNKNOWN)
            {
              throw std::runtime_error("Grammatical type unknown: \"" + newGramStr + "\".");
            }
            pLingDatabase.addMultiMeaningsWord(meaningPtr, linkedMeanings, newGramEnum);
          }
          else
          {
            throw std::runtime_error("Sub tag name: \"" + subTagName + "\" doesn't exist.");
          }
        }
      }
    }
    else if (currSubTree.first == "animationTag")
    {
      std::string tag = subTree.get<std::string>("<xmlattr>.name", "");
      if (tag.empty())
        throw std::runtime_error("Tag name is empty (in file: \"" + pFilename + "\").");
      LingdbAnimationsTag* animTag = pLingDatabase.addATag(tag);
      for(const auto& currAnimTree : subTree)
      {
        if (currAnimTree.first == "<xmlattr>")
          continue;
        const boost::property_tree::ptree& animAttrs = currAnimTree.second.get_child("<xmlattr>");
        if (currAnimTree.first == "linkedConcept")
        {
          std::string name = animAttrs.get<std::string>("name", "");
          if (name.empty())
          {
            throw std::runtime_error("Name of the concept not found (in file: \"" + pFilename + "\").");
          }
          LingdbConcept* conceptPtr = pLingDatabase.getConcept(name);
          if (conceptPtr == nullptr)
          {
            throw std::runtime_error("Concept not found: \"" + name + "\" (in file: \"" +
                                     pFilename + "\").");
          }
          char minValue = 1;
          std::string minValueStr = animAttrs.get<std::string>("minValue", "");
          if (!minValueStr.empty())
          {
            try
            {
              minValue = mystd::lexical_cast<char>(minValueStr);
            }
            catch (...)
            {
              minValue = 1;
            }
          }
          animTag->addConcept(pLingDatabase.xGetFPAlloc(), conceptPtr, minValue);
        }
        else if (currAnimTree.first == "linkedMeaning")
        {
          std::string lemma = animAttrs.get<std::string>("lemme", "");
          PartOfSpeech lemmaPartOfSpeech = partOfSpeech_fromStr(animAttrs.get<std::string>("gram", ""));
          char rela = static_cast<char>(mystd::lexical_cast<int>(animAttrs.get<std::string>("relationToConcept", "3")));
          LingdbMeaning* meaningPtr = pLingDatabase.getMeaning(lemma, lemmaPartOfSpeech);
          if (meaningPtr == nullptr)
          {
            if (pLingDatabase.doesWordExist(lemma))
            {
              std::cerr << "animation: add a new meaning to word: " << lemma << std::endl;
            }
            pLingDatabase.addWord(lemma, lemma, lemmaPartOfSpeech,std::vector<std::string>(), 4);
            meaningPtr = pLingDatabase.getMeaning(lemma, lemmaPartOfSpeech);
          }
          animTag->addMeaning(pLingDatabase.xGetFPAlloc(), meaningPtr, rela);
        }
      }
    }
    else if (currSubTree.first == "concept")
    {
      std::string conceptName = subTree.get<std::string>("<xmlattr>.name", "");
      LingdbConcept* concept = pLingDatabase.getConcept(conceptName);
      if (concept != nullptr)
      {
        for(const auto& currCptTree : subTree)
        {
          if (currCptTree.first == "<xmlattr>")
            continue;
          const boost::property_tree::ptree& cptAttrs = currCptTree.second.get_child("<xmlattr>");
          std::string lemma = cptAttrs.get<std::string>("lemme", "");
          if (lemma.empty())
            throw std::runtime_error("Meaning with an empty lemme in the \"" + conceptName +
                                     "\" concept (in file: \"" + pFilename + "\")");
          std::string inputGramStr = cptAttrs.get<std::string>("gram", "*");

          if (currCptTree.first == "linkedMeaning")
          {
            char relatedToConcept = mystd::lexical_cast<char>(cptAttrs.get<std::string>("relationToConcept", "0"));
            if (inputGramStr == "*")
            {
              LingdbDynamicTrieNode* pointerEndOfWord = pLingDatabase.getPointerToEndOfWord(lemma);
              if (pointerEndOfWord == nullptr)
              {
                std::cerr << "concept: add a new word with unknown grammatical type: " << lemma << std::endl;
                pLingDatabase.addWord(lemma, lemma, PartOfSpeech::UNKNOWN, std::vector<std::string>(), 4);
                pointerEndOfWord = pLingDatabase.getPointerToEndOfWord(lemma);
              }
              if (pointerEndOfWord != nullptr)
              {
                const ForwardPtrList<LingdbMeaning>* itMeanings = pointerEndOfWord->getMeaningsAtThisLemme();
                while (itMeanings != nullptr)
                {
                  itMeanings->elt->addLinkToConcept(pLingDatabase, concept, conceptName,
                                                    relatedToConcept, true);
                  itMeanings = itMeanings->next;
                }
              }
            }
            else
            {
              PartOfSpeech lemmaPartOfSpeech = partOfSpeech_fromStr(inputGramStr);
              LingdbMeaning* meaningPtr = pLingDatabase.getMeaning(lemma, lemmaPartOfSpeech);
              if (meaningPtr == nullptr)
              {
                if (pLingDatabase.doesWordExist(lemma))
                  std::cerr << "concept: add a new meaning to word: " << lemma << std::endl;
                else
                  std::cerr << "concept: add a new word: " << lemma << std::endl;
                pLingDatabase.addWord(lemma, lemma, lemmaPartOfSpeech, std::vector<std::string>(), 4);
                meaningPtr = pLingDatabase.getMeaning(lemma, lemmaPartOfSpeech);
              }
              meaningPtr->addLinkToConcept(pLingDatabase, concept, conceptName,
                                           relatedToConcept, true);
            }
          }
        }
      }
      else
      {
        throw std::runtime_error("concept: \"" + conceptName + "\" is not declared " +
                                 "(in file: \"" + pFilename + "\")");
      }
    }
    else if (currSubTree.first == "declareconcept")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      std::string conceptName = subTreeAttrs.get<std::string>("name", "");
      bool autoFill = subTreeAttrs.get<std::string>("autofill", "true") == "true";
      bool newCptHasBeenInserted = false;
      pLingDatabase.addConcept(newCptHasBeenInserted, conceptName, autoFill);
      if (newCptHasBeenInserted)
        pLingDatabase.addConcept(newCptHasBeenInserted, conceptName + "_*", autoFill);
      else
        std::cerr << "concept: \"" << conceptName << "\" is declared twice." << std::endl;
    }
    else if (currSubTree.first == "oppositeconcepts")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      std::string conceptName1 = subTreeAttrs.get<std::string>("concept1", "");
      LingdbConcept* cptPtr1 = pLingDatabase.getConcept(conceptName1);
      if (cptPtr1 == nullptr)
      {
        std::cerr << "the concept: \"" << conceptName1 << "\" doesn't exist (from opposite of concepts declaration)." << std::endl;
      }
      else
      {
        std::string conceptName2 = subTreeAttrs.get<std::string>("concept2", "");
        LingdbConcept* cptPtr2 = pLingDatabase.getConcept(conceptName2);
        if (cptPtr2 == nullptr)
        {
          std::cerr << "the concept: \"" << conceptName2 << "\" doesn't exist (from opposite of concepts declaration)." << std::endl;
        }
        else
        {
          cptPtr1->addAnOppositeConcept(pLingDatabase.xGetFPAlloc(), cptPtr2);
          cptPtr2->addAnOppositeConcept(pLingDatabase.xGetFPAlloc(), cptPtr1);
        }
      }
    }
    else if (currSubTree.first == "nearlyequalconcepts")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      std::string conceptName1 = subTreeAttrs.get<std::string>("concept1", "");
      LingdbConcept* cptPtr1 = pLingDatabase.getConcept(conceptName1);
      if (cptPtr1 == nullptr)
      {
        std::cerr << "the concept: \"" << conceptName1 << "\" doesn't exist (from nearly equal concepts declaration)." << std::endl;
      }
      else
      {
        std::string conceptName2 = subTreeAttrs.get<std::string>("concept2", "");
        LingdbConcept* cptPtr2 = pLingDatabase.getConcept(conceptName2);
        if (cptPtr2 == nullptr)
        {
          std::cerr << "the concept: \"" << conceptName2 << "\" doesn't exist (from nearly equal concepts declaration)." << std::endl;
        }
        else
        {
          cptPtr1->addANearlyEqualConcept(pLingDatabase.xGetFPAlloc(), cptPtr2);
          cptPtr2->addANearlyEqualConcept(pLingDatabase.xGetFPAlloc(), cptPtr1);
        }
      }
    }
    else if (currSubTree.first == "existingMeaning")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      std::string lemme = subTreeAttrs.get<std::string>("lemme", "");
      if (lemme.empty())
      {
        throw std::runtime_error("Meaning with an empty lemme (in file: \"" + pFilename + "\")");
      }
      std::string partOfSpeechStr = subTreeAttrs.get<std::string>("gram", "");
      if (partOfSpeechStr.empty())
      {
        throw std::runtime_error("Meaning with an empty grammatical info (in file: \"" + pFilename + "\")");
      }
      PartOfSpeech lemmaPartOfSpeech = partOfSpeech_fromStr(partOfSpeechStr);
      LingdbMeaning* meaningPtr = pLingDatabase.getMeaning(lemme, lemmaPartOfSpeech);
      if (meaningPtr == nullptr)
      {
        std::cerr << "Meaning not found: lemma: \"" << lemme << "\" gram: \""
                     << partOfSpeechStr << "\" (in file: \"" << pFilename << "\")." << std::endl;
      }
      else
      {
        for(const auto& currMeaningTree : subTree)
        {
          if (currMeaningTree.first == "<xmlattr>")
            continue;
          const boost::property_tree::ptree& meaningTreeAttrs = currMeaningTree.second.get_child("<xmlattr>");
          if (currMeaningTree.first == "contextInfo")
          {
            std::string contInfoStr = meaningTreeAttrs.get<std::string>("val", "");
            WordContextualInfos contInfo = wordContextualInfos_fromStr(contInfoStr);
            if (contInfo == WordContextualInfos::ENDOFENUM)
              throw std::runtime_error(contInfoStr + " is not a valid meaning contextual info (in file: \"" + pFilename + "\")");
            meaningPtr->pushBackContextInfo(pLingDatabase, contInfo);
          }
          else if (currMeaningTree.first == "removeContextInfo")
          {
            std::string contInfoStr = meaningTreeAttrs.get<std::string>("val", "");
            WordContextualInfos contInfo = wordContextualInfos_fromStr(contInfoStr);
            if (contInfo == WordContextualInfos::ENDOFENUM)
              throw std::runtime_error(contInfoStr + " is not a valid meaning contextual info (in file: \"" + pFilename + "\")");
            meaningPtr->removeContextInfo(pLingDatabase, contInfo);
          }
          else
          {
            throw std::runtime_error("\"" + currMeaningTree.first +
                                     "\" is an unknown child of \"existingMeaning\" (in file: \"" + pFilename + "\")");
          }
        }
      }
    }
    else if (currSubTree.first == "genericRule")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      std::string grName = subTreeAttrs.get<std::string>("name", "");
      if (grName == "questionWords")
      {
        LingdbQuestionWords* qWords = pLingDatabase.xGetFPAlloc().allocate<LingdbQuestionWords>(1);
        qWords->init();
        qWords->load(pLingDatabase.xGetFPAlloc(), subTree);
        pLingDatabase.newQWords(qWords);
      }
    }
    else if (currSubTree.first == "include")
    {
      std::string subFolder;
      pLingdbTree.getHoldingFolder(subFolder, pFilename);

      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      std::string folder = subTreeAttrs.get<std::string>("folder", "");
      if (folder != "")
      {
        boost::filesystem::path folderPath(subFolder + "/" + folder);
        _loadXmlDirectory(folderPath, pLingDatabase, pLingdbTree);
      }
      else
      {
        std::string file = subTreeAttrs.get<std::string>("file", "");
        if (file != "")
          merge(subFolder + "/" + file, pLingDatabase, pLingdbTree);
      }
    }
    else if (currSubTree.first == "separatorsBetweenWords")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      pLingDatabase.setSeparatorNeeded(subTreeAttrs.get<std::string>("val", "true") == "true");
    }
    else if (currSubTree.first == "language")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      pLingDatabase.setLanguage(subTreeAttrs.get<std::string>("val", ""));
    }
    else if (currSubTree.first == "version")
    {
      const boost::property_tree::ptree& subTreeAttrs = subTree.get_child("<xmlattr>");
      std::istringstream buffer(subTreeAttrs.get<std::string>("val", "0"));
      unsigned int value;
      buffer >> value;
      pLingDatabase.setVersion(value);
    }
    else if (currSubTree.first != "<xmlcomment>")
    {
      throw std::runtime_error("Unknown tag name: \"" + currSubTree.first +
                               "\" (in file: \"" + pFilename + "\")");
    }
  }
}



LingdbMeaning* XmlDatabaseLoader::xExtractMeaning
(const boost::property_tree::ptree& pTree,
 const LinguisticIntermediaryDatabase& pLingDatabase)
{
  const boost::property_tree::ptree& attrs = pTree.get_child("<xmlattr>");
  std::string lemma = attrs.get<std::string>("lemme", "");
  std::string partOfSpeechStr = attrs.get<std::string>("gram", "");
  PartOfSpeech lemmaPartOfSpeech = partOfSpeech_fromStr(partOfSpeechStr);
  LingdbMeaning* meaningPtr = pLingDatabase.getMeaning(lemma, lemmaPartOfSpeech);
  if (meaningPtr == nullptr)
    std::cerr << "Submeaning not exist, lemma: \"" << lemma
              << "\", gram: \"" << partOfSpeechStr << "\"" << std::endl;
  return meaningPtr;
}


/*
bool XmlDatabaseLoader::xLoad
(QDomDocument& pDatabaseXml,
 const boost::filesystem::path& pFilename)
{
  QFile file(QString::fromUtf8(pFilename.string().c_str()));
  if (!file.open(QIODevice::ReadOnly))
    return false;
  if (!pDatabaseXml.setContent(&file))
  {
    file.close();
    return false;
  }
  file.close();
  return true;
}
*/

void XmlDatabaseLoader::xSplit
(std::vector<std::string>& pItems,
 const std::string& pStr,
 char pDelim)
{
  std::size_t beginNewItems = 0;
  for (std::size_t i = beginNewItems; i < pStr.size(); ++i)
  {
    if (pStr[i] == '\\')
    {
        ++i;
        continue;
    }
    if (pStr[i] == pDelim)
    {
      if (i > beginNewItems)
      {
        pItems.emplace_back(pStr.substr(beginNewItems, i - beginNewItems));
      }
      beginNewItems = i + 1;
    }
  }
  if (pStr.size() > beginNewItems)
  {
    pItems.emplace_back(pStr.substr(beginNewItems, pStr.size() - beginNewItems));
  }
}


} // End of namespace onsem
