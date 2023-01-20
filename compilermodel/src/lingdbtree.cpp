#include <onsem/compilermodel/lingdbtree.hpp>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <onsem/compilermodel/savers/binarydatabaseconceptssaver.hpp>
#include <onsem/compilermodel/savers/binarydatabasedicosaver.hpp>
#include <onsem/compilermodel/savers/binarytradsaver.hpp>
#include <onsem/compilermodel/loaders/anydatabaseloader.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/loaders/wlksdatabaseloader.hpp>
#include "concept/lingdbconcept.hpp"

namespace onsem
{
static const std::string semanticdb_formalismVersion = "1.0.5";


namespace {

void _printAllSubPaths(std::ostream& pFStream,
                       const std::string& pRelativePath,
                       const std::string& pFolderPath)
{
  std::filesystem::directory_iterator itFolder(pFolderPath);
  std::filesystem::directory_iterator endit;
  while (itFolder != endit)
  {
    const auto& currPath = itFolder->path();
    const std::string absolutePath = currPath.string();
    std::string relativePath = pRelativePath;
    if (!relativePath.empty())
      relativePath += "/";
    relativePath += itFolder->path().filename().string();
    if (std::filesystem::is_directory(currPath))
    {
      _printAllSubPaths(pFStream, relativePath, absolutePath);
    }
    else
    {
      if (!absolutePath.empty() &&
          absolutePath[absolutePath.size() - 1] != '~')
        pFStream << relativePath << "\n";
    }
    ++itFolder;
  }
}
}


LingdbTree::LingdbTree(const std::string& pInputResourcesDir)
  : fInputResourcesDir(pInputResourcesDir),
    fDynamicDatabasesFolder(),
    fLanguages(),
    fIn32Bits(sizeof(std::size_t) == 4)
{
  fLanguages.push_back(SemanticLanguageEnum::UNKNOWN);
  fLanguages.push_back(SemanticLanguageEnum::FRENCH);
  fLanguages.push_back(SemanticLanguageEnum::ENGLISH);
  fLanguages.push_back(SemanticLanguageEnum::JAPANESE);
}




bool LingdbTree::xExtractVersionFromFile
(std::string& pFormalismVersion,
 std::string& pVersion,
 const std::string& pFilePath)
{
  std::ifstream versionFile(pFilePath, std::ifstream::in);
  if (!versionFile.is_open())
  {
    return false;
  }
  std::string line;
  getline(versionFile, line);
  {
    const std::string formalismVersionLabel = "version_formalism:";
    assert(line.size() > formalismVersionLabel.size());
    pFormalismVersion = line.substr(formalismVersionLabel.size(),
                                    line.size() - formalismVersionLabel.size());
  }

  getline(versionFile, line);
  {
    const std::string versionLabel = "version:";
    assert(line.size() > versionLabel.size());
    pVersion = line.substr(versionLabel.size(),
                           line.size() - versionLabel.size());
  }
  versionFile.close();
  return true;
}




void LingdbTree::clearAll() const
{
  xClearDirectory(fDynamicDatabasesFolder);
}


std::string LingdbTree::getDynDbFilenameForLanguage(const std::string& pLang) const
{
  return fDynamicDatabasesFolder + "/" + xGetDynDatabaseFilename(pLang);
}


void LingdbTree::update(const std::string& pSdkShareDir,
                          const std::string& pLoadDatabasesDir,
                          const std::string& pDynamicdictionaryPath,
                          bool pClearTmpFolder)
{
  std::string inputVersionFilePath = pLoadDatabasesDir + "/version.txt";
  std::string inputDbFormalismVersion;
  std::string inputDbVersion;
  if (!xExtractVersionFromFile(inputDbFormalismVersion,
                               inputDbVersion,
                               inputVersionFilePath))
  {
    throw std::runtime_error(inputVersionFilePath + " is missing!");
  }
  if (inputDbFormalismVersion != semanticdb_formalismVersion)
  {
    throw std::runtime_error("The tool semanticdbgenerator is too old! Please regenerate it!\n"
                             "If you are using qibuild the command to do is: \"qibuild make-host-tools\"");
  }

  std::string linguisticPath = pSdkShareDir + "/linguistic";
  xCreateDirectory(linguisticPath);
  std::string linguisticDatabasesPath = linguisticPath + "/databases";
  xCreateDirectory(linguisticDatabasesPath);
  std::string installedVersionFilePath = linguisticDatabasesPath + "/version.txt";
  fDynamicDatabasesFolder = linguisticPath + "/tmpdatabases";

  bool needToUpdateAllStatBinDico = false;
  {
    std::string installedDbFormalismVersion;
    std::string installedDbVersion;
    xExtractVersionFromFile(installedDbFormalismVersion,
                            installedDbVersion,
                            installedVersionFilePath);
    if (installedDbVersion != inputDbVersion)
    {
      needToUpdateAllStatBinDico = true;
    }
    /*
    else
    {
      for (std::size_t i = 0; i < fLanguages.size(); ++i)
      {
        std::string lang = semanticLanguageEnum_toLanguageFilenameStr(fLanguages[i].langGroundingsType);

        fLanguages[i].statDbVersion = linguistics::StaticLinguisticDictionary::getVersion
            (linguisticDatabasesPath / xGetStatDatabaseFilename(lang));
        if (fLanguages[i].statDbVersion == -1)
        {
          needToUpdateAllStatBinDico = true;
        }
      }
    }
    */
  }

  if (needToUpdateAllStatBinDico)
  {
    std::cout << "lingDbTree info: generate the semantic databases" << std::endl;

    xCreateDirectory(fDynamicDatabasesFolder);
    LinguisticIntermediaryDatabase currLingdb;
    AnyDatabaseLoader anyLoader;
    anyLoader.open(pLoadDatabasesDir + "/reloadall.rla", fInputResourcesDir,
                   currLingdb, *this);
    xTryToLoadDynamicDbs();
    xCopyFile(inputVersionFilePath, installedVersionFilePath);

    std::map<std::string, LingdbConcept*> cptStrToCptStruct;
    for (std::size_t i = 0; i < fLanguages.size(); ++i)
    {
      if (fLanguages[i].dynDbVersion == -1)
      {
        std::string lang = semanticLanguageEnum_toLanguageFilenameStr(fLanguages[i].langGroundingsType);
        std::cerr << "Error: The language " << lang << " has no databases associated" << std::endl;
        continue;
      }

      auto& fpAlloc = fLanguages[i].dynDb->getFPAlloc();
      LingdbConcept* conceptElt = fpAlloc.first<LingdbConcept>();
      while (conceptElt != nullptr)
      {
        std::string conceptStr = conceptElt->getName()->toStr();
        cptStrToCptStruct.emplace(conceptStr, conceptElt);
        conceptElt = fpAlloc.next<LingdbConcept>(conceptElt);
      }
    }

    std::map<std::string, ConceptsBinMem> conceptsOffsets;
    BinaryDatabaseConceptsSaver conceceptsDicoSaver;
    conceceptsDicoSaver.saveConceptsDb
        (conceptsOffsets, cptStrToCptStruct,
         linguisticDatabasesPath / std::filesystem::path("concepts." + getExtBinaryDatabase()));


    std::map<SemanticLanguageEnum, std::map<const LingdbMeaning*, int> > langToMeaningsPtr;
    BinaryDatabaseDicoSaver binDicoSaver;
    for (std::size_t i = 0; i < fLanguages.size(); ++i)
    {
      if (fLanguages[i].dynDbVersion == -1)
      {
        std::string lang = semanticLanguageEnum_toLanguageFilenameStr(fLanguages[i].langGroundingsType);
        std::cerr << "Error: The language " << lang << " has no databases associated" << std::endl;
        continue;
      }

      std::string lang = semanticLanguageEnum_toLanguageFilenameStr(fLanguages[i].langGroundingsType);
      binDicoSaver.save(langToMeaningsPtr[fLanguages[i].langGroundingsType], conceptsOffsets,
          linguisticDatabasesPath + "/" + xGetStatDatabaseFilename(lang),
          linguisticDatabasesPath / std::filesystem::path(lang + "animations." + getExtBinaryDatabase()),
          linguisticDatabasesPath / std::filesystem::path(lang + "synthesizer." + getExtBinaryDatabase()),
          *fLanguages[i].dynDb);
    }

    xGenerateTranslations(langToMeaningsPtr, linguisticDatabasesPath);
    if (pClearTmpFolder)
      xRemoveDirectory(fDynamicDatabasesFolder);

    // List the paths
    if (!pDynamicdictionaryPath.empty())
    {
      {
        std::ofstream wordsrelativePathsFile(linguisticPath + "/wordsrelativePaths.txt");
        std::string wordModificationPath = pDynamicdictionaryPath + "/words";
        // /!\ The order of the folders is important
        static const std::vector<std::string> folders{"anyverb", "readonly", "manual"};
        for (const auto& currFolder : folders)
        {
          try
          {
            _printAllSubPaths(wordsrelativePathsFile, currFolder, wordModificationPath + "/" + currFolder);
          }
          catch (const std::exception& e)
          {
            std::cerr << "Error on load readonly words dynamic database: " << e.what() << std::endl;
          }
        }
        wordsrelativePathsFile.close();
      }

      {
        std::ofstream treeConvertionsPathsFile(linguisticPath + "/treeConvertionsPaths.txt");

        auto treeConverterPath = pDynamicdictionaryPath + "/treeconversions";
        std::filesystem::directory_iterator itTreeConvsFolders(treeConverterPath);
        std::filesystem::directory_iterator endit;
        while (itTreeConvsFolders != endit)
        {
          if (std::filesystem::is_directory(itTreeConvsFolders->path()))
          {
            const std::string filename = itTreeConvsFolders->path().filename().string();
            SemanticLanguageEnum langEnum =
                semanticLanguageTypeGroundingEnumFromStr(filename);
            treeConvertionsPathsFile << "#" << semanticLanguageEnum_toLanguageFilenameStr(langEnum) << "\n";
            _printAllSubPaths(treeConvertionsPathsFile, filename, treeConverterPath + "/" + filename);
          }
          ++itTreeConvsFolders;
        }
        treeConvertionsPathsFile.close();
      }
    }
  }
}


bool LingdbTree::xTryToLoadDynamicDbs()
{
  for (std::size_t i = 0; i < fLanguages.size(); ++i)
  {
    std::string lang = semanticLanguageEnum_toLanguageFilenameStr(fLanguages[i].langGroundingsType);

    fLanguages[i].dynDb = std::make_unique<LinguisticIntermediaryDatabase>();
    try
    {
      fLanguages[i].dynDb->load(getDynDbFilenameForLanguage(lang));
      fLanguages[i].dynDbVersion = fLanguages[i].dynDb->getVersion();
    }
    catch (const std::exception&)
    {
      fLanguages[i].dynDbVersion = -1;
    }

    if (fLanguages[i].dynDbVersion == -1)
    {
      return false;
    }
  }
  return true;
}



void LingdbTree::xGenerateTranslations
(const std::map<SemanticLanguageEnum, std::map<const LingdbMeaning*, int> >& pLangToMeaningsPtr,
 const std::filesystem::path &pLinguisticDatabasesPath)
{
  // generate concepts to meanings for every languages
  WlksDatabaseLoader::WlksDatabaseLoader_WorkState wlksWorkState(*this);
  for (std::size_t i = 0; i < fLanguages.size(); ++i)
  {
    SemanticLanguageEnum langType = fLanguages[i].langGroundingsType;
    if (langType == SemanticLanguageEnum::UNKNOWN ||
        fLanguages[i].dynDbVersion == -1)
    {
      continue;
    }

    std::string langStr = semanticLanguageEnum_toLegacyStr(langType);
    WlksDatabaseLoader::WlksDatabaseLoader_LangSpec& langSpec =
        wlksWorkState.strToLangSpecs[langStr];
    langSpec.lingDatabase = fLanguages[i].dynDb;
    langSpec.lingDatabase->getConceptToMeanings(langSpec.conceptToMeanings);
  }

  // load translations
  WlksDatabaseLoader wlksLoader;
  wlksLoader.load(wlksWorkState, fInputResourcesDir + "/common/traductions.wlks");

  // save translations
  BinaryTradSaver tradSaver;
  tradSaver.save(wlksWorkState, pLangToMeaningsPtr, pLinguisticDatabasesPath);
}




void LingdbTree::xClearDirectory
(const std::string& pDirectory) const
{
  std::filesystem::path pathToRemove(pDirectory);
  for (std::filesystem::directory_iterator end_dir_it, itFilesToRemove(pathToRemove);
       itFilesToRemove != end_dir_it; ++itFilesToRemove)
  {
    std::filesystem::remove_all(itFilesToRemove->path());
  }
}



std::string LingdbTree::xGetDynDatabaseFilename
(const std::string& pLanguage) const
{
  return pLanguage + "." + getExtDynDatabase();
}


std::string LingdbTree::xGetStatDatabaseFilename
(const std::string& pLanguage) const
{
  return pLanguage + "database." + getExtBinaryDatabase();
}


void LingdbTree::getHoldingFolder
(std::string& pFolder,
 const std::filesystem::path& pFilename) const
{
  std::error_code ec;
  if (std::filesystem::is_regular_file(pFilename, ec))
  {
    pFolder = pFilename.parent_path().string();
    return;
  }
  pFolder = pFilename.string();
}


void LingdbTree::xCreateDirectory
(const std::string& pPath) const
{
  std::filesystem::create_directory(pPath);
}


void LingdbTree::xRemoveDirectory
(const std::filesystem::path &pPath) const
{
  std::filesystem::remove_all(pPath);
}

void LingdbTree::xCopyFile
(const std::string& pFrom,
 const std::string& pTo) const
{
  /*
  std::filesystem::path from(pFrom.c_str());
  std::filesystem::path to(pTo.c_str());
  std::filesystem::copy_file(from, to, std::filesystem::copy_option::overwrite_if_exists);
  */

  std::ifstream source(pFrom, std::ifstream::binary);
  std::ofstream dest(pTo, std::ofstream::binary);
  dest << source.rdbuf();
  source.close();
  dest.close();

}



} // End of namespace onsem
