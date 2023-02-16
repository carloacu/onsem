#include <onsem/texttosemantic/dbtype/linguisticdatabase/binarydatabasessizeprinter.hpp>
#include <sstream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/treeconverter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/statictranslationdictionary.hpp>
#include <onsem/common/utility/sizeprinterinbytes.hpp>


namespace onsem
{
namespace binaryDbSizePrinter
{


void _printSize(std::list<std::pair<std::size_t, std::string> >& dbsSizePrints,
                std::size_t& pTotalSize,
                const std::string& pDbName,
                bool pIsLoaded,
                const std::string& pErrorMsg,
                std::size_t pSize)
{
  std::stringstream ss;
  ss << pDbName;
  if (!pErrorMsg.empty())
  {
    ss << pErrorMsg;
  }
  else if (!pIsLoaded)
  {
    ss << "NOT_LOADED";
  }
  else
  {
    prettyPrintSizeNbInBytes(ss, pSize);
    pTotalSize += pSize;
  }

  auto itWhereToAdd = dbsSizePrints.begin();
  while (itWhereToAdd != dbsSizePrints.end() &&
         itWhereToAdd->first > pSize)
  {
    ++itWhereToAdd;
  }
  dbsSizePrints.insert(itWhereToAdd,
                       std::pair<std::size_t, std::string>
                       (pSize, ss.str()));
}


void _printBinDbSize(std::list<std::pair<std::size_t, std::string> >& dbsSizePrints,
                     std::size_t& pTotalSize,
                     const std::string& pDbName,
                     const VirtualSemBinaryDatabase& pBinDb)
{
  bool isLoaded = false;
  std::string errorMsg;
  std::size_t dbSize = pBinDb.getSize(errorMsg, isLoaded);
  _printSize(dbsSizePrints, pTotalSize, pDbName,
             isLoaded, errorMsg, dbSize);
}


void printSize(std::string& pText,
               const linguistics::LinguisticDatabase& pLingDb)
{
  std::size_t totalSize = 0;
  std::list<std::pair<std::size_t, std::string> > dbsSizePrints;

  {
    bool isLoaded = false;
    std::string errorMsg;
    std::size_t dbSize = pLingDb.conceptSet.getStaticConceptsSize(errorMsg, isLoaded);
    _printSize(dbsSizePrints, totalSize, "conceptsdatabase\t\t",
               isLoaded, errorMsg, dbSize);
  }

  {
    bool isLoaded = false;
    std::string errorMsg;
    std::size_t dbSize = pLingDb.treeConverter.getSize(errorMsg, isLoaded);
    _printSize(dbsSizePrints, totalSize, "conversionsdatabase\t\t",
               isLoaded, errorMsg, dbSize);
  }


  std::list<SemanticLanguageEnum> dbLanguages;
  dbLanguages.emplace_back(SemanticLanguageEnum::FRENCH);
  dbLanguages.emplace_back(SemanticLanguageEnum::ENGLISH);
  dbLanguages.emplace_back(SemanticLanguageEnum::JAPANESE);
  dbLanguages.emplace_back(SemanticLanguageEnum::UNKNOWN);

  for (SemanticLanguageEnum currLang : dbLanguages)
  {
    _printBinDbSize
        (dbsSizePrints, totalSize,
         semanticLanguageEnum_toLanguageFilenameStr(currLang) + "database\t\t",
        pLingDb.langToSpec[currLang].lingDico.statDb);

    _printBinDbSize
        (dbsSizePrints, totalSize,
         semanticLanguageEnum_toLanguageFilenameStr(currLang) + "synthesizer\t\t",
        pLingDb.langToSpec[currLang].synthDico.statDb);

    if (currLang != SemanticLanguageEnum::UNKNOWN)
    {
      for (SemanticLanguageEnum currLang2 : dbLanguages)
      {
        if (currLang2 != currLang &&
            currLang2 != SemanticLanguageEnum::UNKNOWN)
        {
          const StaticTranslationDictionary* staticTranslationDictionaryPtr =
              pLingDb.transDict.getStaticTranslationDictionaryPtr(currLang, currLang2);
          if (staticTranslationDictionaryPtr != nullptr)
            _printBinDbSize
                (dbsSizePrints, totalSize,
                 semanticLanguageEnum_toLanguageFilenameStr(currLang) + "_to_" +
                 semanticLanguageEnum_toLanguageFilenameStr(currLang2) + "\t\t",
                 *staticTranslationDictionaryPtr);
        }
      }
    }
  }


  std::stringstream ss;
  for (std::list<std::pair<std::size_t, std::string> >::iterator
       itDbsSize = dbsSizePrints.begin(); itDbsSize != dbsSizePrints.end(); ++itDbsSize)
  {
    ss << itDbsSize->second << "\n";
  }
  ss << "\n=================================================\n\n";
  ss << "Total used space:\t";
  prettyPrintSizeNbInBytes(ss, totalSize);
  ss << "\n";

  pText += ss.str();
}


} // End of namespace binaryDbSizePrinter
} // End of namespace onsem
