#include <onsem/streamdatabaseaccessor/streamdatabaseaccessor.hpp>
#include <iostream>
#include <filesystem>

namespace onsem
{
namespace linguistics
{
namespace
{

void _loadConvForALanguage
(KeyToFStreams& pIStreams,
 SemanticLanguageEnum pLangEnum,
 const std::string& pPath,
 const std::string& pLocalPath)
{
  std::filesystem::directory_iterator itTreeConvsFolders(pPath);
  std::filesystem::directory_iterator endit;
  while (itTreeConvsFolders != endit)
  {
    const std::string absolutePath = itTreeConvsFolders->path().string();
    if (std::filesystem::is_directory(itTreeConvsFolders->path()))
    {
      _loadConvForALanguage(pIStreams, pLangEnum, absolutePath,
                            pLocalPath + "/" + itTreeConvsFolders->path().filename().string());
    }
    else if (!absolutePath.empty() &&
             absolutePath[absolutePath.size() - 1] != '~')
    {
      pIStreams.addConversionFile(pLangEnum,
                                  pLocalPath + "/" + itTreeConvsFolders->path().filename().string(),
                                  absolutePath);
    }
    ++itTreeConvsFolders;
  }
}

}



KeyToFStreams generateIStreams(const std::string& pLingDbPath,
                               const std::string& pDynamicDictionaryPath)
{
  KeyToFStreams iStreams;
  iStreams.addConceptFile(pLingDbPath + "/concepts.bdb");

  // /!\ The order of the folders is important
  static const std::vector<std::string> folders{"anyverb", "readonly", "manual"};
  for (const auto& currFolder : folders)
  {
    try
    {
      std::string wordModificationPath = pDynamicDictionaryPath + "/words/" + currFolder;
      addDynamicContentFromFolder(iStreams, wordModificationPath);
    }
    catch (const std::exception& e)
    {
      std::cerr << "Error on load readonly words dynamic database: " << e.what() << std::endl;
    }
  }

  for (std::size_t i = 0; i < semanticLanguageEnum_size; ++i)
  {
    auto language = semanticLanguageEnum_fromChar(static_cast<char>(i));
    if (language == SemanticLanguageEnum::OTHER)
      continue;

    auto languageFileName = semanticLanguageEnum_toLanguageFilenameStr(language);
    iStreams.addMainDicFile(language, pLingDbPath + "/" + languageFileName + "database.bdb");
    iStreams.addSynthesizerFile(language, pLingDbPath + "/" + languageFileName + "synthesizer.bdb");

    if (language != SemanticLanguageEnum::UNKNOWN)
    {
      for (std::size_t j = 0; j < semanticLanguageEnum_size; ++j)
      {
        auto secondLanguage = semanticLanguageEnum_fromChar(static_cast<char>(j));
        if (language != secondLanguage &&
            secondLanguage != SemanticLanguageEnum::OTHER &&
            secondLanguage != SemanticLanguageEnum::UNKNOWN)
        {
          auto filename = pLingDbPath + "/" +
              semanticLanguageEnum_toLegacyStr(language) + "_to_" +
              semanticLanguageEnum_toLegacyStr(secondLanguage) + ".bdb";
          iStreams.addFile(language, secondLanguage, filename);
        }
      }
    }
  }

  {
    auto treeConverterPth = pDynamicDictionaryPath + "/treeconversions";
    std::filesystem::directory_iterator itTreeConvsFolders(treeConverterPth);
    std::filesystem::directory_iterator endit;
    while (itTreeConvsFolders != endit)
    {
      if (std::filesystem::is_directory(itTreeConvsFolders->path()))
      {
        std::string filename = itTreeConvsFolders->path().filename().string();
        SemanticLanguageEnum langEnum =
            semanticLanguageTypeGroundingEnumFromStr(filename);
        _loadConvForALanguage(iStreams, langEnum,
                              itTreeConvsFolders->path().string(), filename);
      }
      ++itTreeConvsFolders;
    }
  }

  return iStreams;
}


void addDynamicContentFromFolder(KeyToFStreams& pStreams,
                                 const std::string& pFolderPath)
{
  std::filesystem::directory_iterator itFolder(pFolderPath);
  std::filesystem::directory_iterator endit;
  while (itFolder != endit)
  {
    const auto& currPath = itFolder->path();
    if (std::filesystem::is_directory(itFolder->path()))
    {
      addDynamicContentFromFolder(pStreams, currPath.string());
    }
    else
    {
      const std::string absolutePath = currPath.string();
      if (!absolutePath.empty() &&
          absolutePath[absolutePath.size() - 1] != '~')
        pStreams.addDynamicContentFile(absolutePath);
    }
    ++itFolder;
  }
}

} // End of namespace linguistics
} // End of namespace onsem
