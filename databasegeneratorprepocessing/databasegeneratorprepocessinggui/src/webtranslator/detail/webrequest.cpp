#include "webrequest.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>


namespace onsem
{


void flushrequestInFile
(const std::string& pInLang,
 const std::string& pOutLang,
 std::size_t pIdFile,
 const std::string& pContent)
{
  std::stringstream ssoutFileName;
  ssoutFileName << "request_words_" << pInLang << "_" << pOutLang << pIdFile << ".php";
  std::ofstream outfile(ssoutFileName.str().c_str());


  outfile << "<head>" << std::endl;
  outfile << "  <meta charset=\"utf-8\" />" << std::endl;
  outfile << "</head>" << std::endl;
  outfile << "<body>" << std::endl;

  outfile << "  <div id=\"google_translate_element\"></div><script type=\"text/javascript\">" << std::endl;
  outfile << "  function googleTranslateElementInit() {" << std::endl;
  outfile << "    new google.translate.TranslateElement({pageLanguage: '" << pInLang
          << "', includedLanguages: '" << pOutLang
          << "', layout: google.translate.TranslateElement.InlineLayout.SIMPLE}, 'google_translate_element');" << std::endl;
  outfile << "  }" << std::endl;
  outfile << "  </script><script type=\"text/javascript\" src=\"//translate.google.com/translate_a/element.js?cb=googleTranslateElementInit\"></script>" << std::endl;
  outfile << std::endl << std::endl << std::endl;
  outfile << pContent;
  outfile << "</body>" << std::endl;
  outfile.close();
}



void processOneTranslationRequest
(SemanticLanguageEnum pInLangEnum,
 const std::string& pInLang,
 const std::string& pOutLang,
 const LingdbTree& pLingDbTree)
{
  std::cout << "load database " <<  pInLang << "..." << std::endl;

  LinguisticIntermediaryDatabase lingDb;
  lingDb.setLanguage(semanticLanguageEnum_toLegacyStr(pInLangEnum));
  lingDb.load(pLingDbTree.getDynamicDatabasesFolder() + "/" +
              lingDb.getLanguage()->toStr() + "." +
              pLingDbTree.getExtDynDatabase());

  std::cout << "write file(s) " <<  pInLang << "..." << std::endl;


  std::stringstream ss;
  std::size_t idLine = 0;
  std::size_t idFile = 0;
  LingdbMeaning* meaning = lingDb.getFPAlloc().first<LingdbMeaning>();
  while (meaning != nullptr)
  {
    PartOfSpeech partOfSpeech = meaning->getPartOfSpeech();
    if (partOfSpeech != PartOfSpeech::INTERSPACE &&
        partOfSpeech != PartOfSpeech::LINKBETWEENWORDS &&
        partOfSpeech != PartOfSpeech::PUNCTUATION &&
        partOfSpeech != PartOfSpeech::AUX)
    {
      ss << "  <div gram=\"" << partOfSpeech_toStr(partOfSpeech)
              << "\" lemma=\"" << meaning->getLemma()->getWord()
              << "\">";
      if (partOfSpeech == PartOfSpeech::NOUN)
      {
        if (pInLangEnum == SemanticLanguageEnum::FRENCH)
        {
          ss << "l'" << meaning->getLemma()->getWord(true);
        }
        else
        {
          ss << "the " << meaning->getLemma()->getWord(true);
        }
      }
      else if (partOfSpeech == PartOfSpeech::VERB)
      {
        if (pInLangEnum == SemanticLanguageEnum::FRENCH)
        {
          ss << meaning->getLemma()->getWord(true);
        }
        else
        {
          ss << "to " << meaning->getLemma()->getWord(true) << ".";
        }
      }
      else
      {
        ss << meaning->getLemma()->getWord(true);
      }

      ss << "</div>" << std::endl;

      ++idLine;
      if (idLine >= 140000)
      {
        flushrequestInFile(pInLang, pOutLang, ++idFile, ss.str());
        ss.str("");
        idLine = 0;
      }
    }
    meaning = lingDb.getFPAlloc().next<LingdbMeaning>(meaning);
  }

  const std::string content = ss.str();
  if (!content.empty())
  {
    flushrequestInFile(pInLang, pOutLang, ++idFile, content);
  }
}



void process_webrequest(const std::string& pInputResourcesDir)
{
  LingdbTree lingDbTree(pInputResourcesDir);

  processOneTranslationRequest(SemanticLanguageEnum::FRENCH,
                               "fr", "en", lingDbTree);
  processOneTranslationRequest(SemanticLanguageEnum::ENGLISH,
                               "en", "fr", lingDbTree);
}



} // End of namespace onsem
