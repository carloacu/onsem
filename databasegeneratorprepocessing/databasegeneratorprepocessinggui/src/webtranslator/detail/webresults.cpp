#include "webresults.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <QFile>
#include <QDomDocument>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/savers/traductionwriter.hpp>


namespace onsem
{


bool loadXmlWebTranslations
(QDomDocument& pDatabaseXml,
 const std::string& pFilename)
{
  QFile file(pFilename.c_str());
  if (!file.open(QIODevice::ReadOnly))
  {
    return false;
  }
  if (!pDatabaseXml.setContent(&file))
  {
    file.close();
    return false;
  }
  file.close();
  return true;
}

void process_oneWebresultsFile
(const std::string& pFilename,
 std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTraductions,
 std::map<LingdbMeaning*, LingdbSaverOutLinks>& pReverseTraductions,
 const LinguisticIntermediaryDatabase& pInLingDb,
 const LinguisticIntermediaryDatabase& pOutLingDb)
{
  QDomDocument databaseXml;
  if (!loadXmlWebTranslations(databaseXml, pFilename))
  {
    throw std::runtime_error("Could not open to read only: \"" + pFilename + "\".");
  }

  QDomElement currDbChild = databaseXml.firstChild().toElement().firstChild().toElement();
  while (!currDbChild.isNull())
  {
    QString tagName = currDbChild.tagName();

    if (tagName == "div")
    {
      PartOfSpeech gram =
          partOfSpeech_fromStr(currDbChild.attribute("gram", "").toUtf8().constData());
      std::string lemma = currDbChild.attribute("lemma", "").toUtf8().constData();
      LingdbMeaning* inMeaning = pInLingDb.getMeaning(lemma, gram);

      if (inMeaning != nullptr)
      {
        std::istringstream tradWords(currDbChild.text().toUtf8().constData());
        do
        {
            std::string subWord;
            tradWords >> subWord;
            if (subWord.empty())
            {
              break;
            }
            if (subWord == "le" || subWord == "la" || subWord == "the")
            {
              continue;
            }
            if (subWord[subWord.size() - 1] == '.')
            {
              subWord = subWord.substr(0, subWord.size() - 1);
            }
            if (subWord.compare(0, 2, "l'") == 0 && subWord.size() > 2)
            {
              subWord = subWord.substr(2, subWord.size() - 2);
            }

            LingdbDynamicTrieNode* trieNode = pOutLingDb.getPointerToEndOfWord(subWord);
            if (trieNode != nullptr)
            {
              LingdbMeaning* outMeaning = trieNode->getMeaning(gram);
              if (outMeaning != nullptr)
              {
                pTraductions[inMeaning].linkedWithInMeaningGram.insert(outMeaning);
                pReverseTraductions[outMeaning].linkedWithInMeaningGram.insert(inMeaning);
              }
            }
        }
        while (tradWords);
      }
    }
    else
    {
      throw std::runtime_error("Unknown tag name: \"" + std::string(tagName.toUtf8().constData()) +
                               "\" (in file: \"" + pFilename + "\")");
    }

    currDbChild = currDbChild.nextSiblingElement();
  }
}


void process_webresults(const LingdbTree& pLingDbTree,
                        const std::string& pTmpFolder,
                        const std::string& pShareDbFolder)
{
  const std::string inFolder = pShareDbFolder + "/databasegeneratorpreprocessing/webtranslator/in";

  std::cout << "load en words" << std::endl;
  LinguisticIntermediaryDatabase lingDbEn;
  lingDbEn.setLanguage(semanticLanguageEnum_toLegacyStr(SemanticLanguageEnum::ENGLISH));
  lingDbEn.load(pLingDbTree.getDynamicDatabasesFolder() + "/" +
                lingDbEn.getLanguage()->toStr() + "." +
                pLingDbTree.getExtDynDatabase());
  std::cout << "load fr words" << std::endl;
  LinguisticIntermediaryDatabase lingDbFr;
  lingDbFr.setLanguage(semanticLanguageEnum_toLegacyStr(SemanticLanguageEnum::FRENCH));
  lingDbFr.load(pLingDbTree.getDynamicDatabasesFolder() + "/" +
                lingDbFr.getLanguage()->toStr() + "." +
                pLingDbTree.getExtDynDatabase());


  std::map<LingdbMeaning*, LingdbSaverOutLinks> enToFrTraductions;
  std::map<LingdbMeaning*, LingdbSaverOutLinks> enToFrTraductions_reverse;
  std::map<LingdbMeaning*, LingdbSaverOutLinks> frToEnTraductions;
  std::map<LingdbMeaning*, LingdbSaverOutLinks> frToEnTraductions_reverse;


  std::cout << "read: googletranslator_words_en_fr.php" << std::endl;
  process_oneWebresultsFile(inFolder + "/googletranslator_words_en_fr.php",
                            enToFrTraductions, frToEnTraductions_reverse, lingDbEn, lingDbFr);

  std::cout << "read: googletranslator_words_fr_en.php" << std::endl;
  process_oneWebresultsFile(inFolder + "/googletranslator_words_fr_en.php",
                            frToEnTraductions, enToFrTraductions_reverse, lingDbFr, lingDbEn);

  std::cout << "write results" << std::endl;
  auto resultFolderPath = pTmpFolder + "/webtranslator";
  std::filesystem::create_directory(resultFolderPath);

  LingdbSaverTraductionWriter tradWriter;
  {
    const std::string outFilename = resultFolderPath + "/web_en_to_fr.wlks";
    std::cout << "write: " << outFilename << std::endl;
    tradWriter.writeTraductionsForOneWiki(outFilename, enToFrTraductions);
  }
  {
    const std::string outFilename = resultFolderPath + "/web_fr_to_en.wlks";
    std::cout << "write: " << outFilename << std::endl;
    tradWriter.writeTraductionsForOneWiki(outFilename, frToEnTraductions);
  }
  {
    const std::string outFilename = resultFolderPath + "/web_en_to_fr_fromReverse.wlks";
    std::cout << "write: " << outFilename << std::endl;
    tradWriter.writeTraductionsForOneWiki_fromReverseTraductions(outFilename, enToFrTraductions_reverse);
  }
  {
    const std::string outFilename = resultFolderPath + "/web_fr_to_en_fromReverse.wlks";
    std::cout << "write: " << outFilename << std::endl;
    tradWriter.writeTraductionsForOneWiki_fromReverseTraductions(outFilename, frToEnTraductions_reverse);
  }
}


} // End of namespace onsem
