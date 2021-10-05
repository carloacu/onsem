#include <onsem/lingdbeditor/allingdbtree.hpp>
#include <sstream>
#include <boost/filesystem.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/lingdbeditor/savers/albinarydatabaseconceptssaver.hpp>
#include <onsem/lingdbeditor/savers/albinarydatabasedicosaver.hpp>
#include <onsem/lingdbeditor/savers/albinarytradsaver.hpp>
#include <onsem/lingdbeditor/loaders/alanydatabaseloader.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <onsem/lingdbeditor/loaders/alwlksdatabaseloader.hpp>
#include "concept/allingdbconcept.hpp"

namespace onsem
{
static const std::string semanticdb_formalismVersion = "1.0.5";


namespace {

void _printAllSubPaths(std::ostream& pFStream,
                       const std::string& pRelativePath,
                       const std::string& pFolderPath)
{
  boost::filesystem::directory_iterator itFolder(pFolderPath);
  boost::filesystem::directory_iterator endit;
  while (itFolder != endit)
  {
    const auto& currPath = itFolder->path();
    const std::string absolutePath = currPath.string();
    std::string relativePath = pRelativePath;
    if (!relativePath.empty())
      relativePath += "/";
    relativePath += itFolder->path().filename().string();
    if (boost::filesystem::is_directory(currPath))
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


ALLingdbTree::ALLingdbTree(const boost::filesystem::path& pInputResourcesDir)
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




bool ALLingdbTree::xExtractVersionFromFile
(std::string& pFormalismVersion,
 std::string& pVersion,
 const boost::filesystem::path& pFilePath)
{
  boost::filesystem::ifstream versionFile(pFilePath);
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




void ALLingdbTree::clearAll() const
{
  xClearDirectory(fDynamicDatabasesFolder);
}


boost::filesystem::path ALLingdbTree::getDynDbFilenameForLanguage
(const std::string& pLang) const
{
  return fDynamicDatabasesFolder / xGetDynDatabaseFilename(pLang);
}


void ALLingdbTree::update(const boost::filesystem::path& pSdkShareDir,
                          const boost::filesystem::path& pLoadDatabasesDir,
                          const std::string& pDynamicdictionaryPath,
                          bool pClearTmpFolder)
{
  boost::filesystem::path inputVersionFilePath = pLoadDatabasesDir / "version.txt";
  std::string inputDbFormalismVersion;
  std::string inputDbVersion;
  if (!xExtractVersionFromFile(inputDbFormalismVersion,
                               inputDbVersion,
                               inputVersionFilePath))
  {
    throw std::runtime_error(inputVersionFilePath.string() + " is missing!");
  }
  if (inputDbFormalismVersion != semanticdb_formalismVersion)
  {
    throw std::runtime_error("The tool semanticdbgenerator is too old! Please regenerate it!\n"
                             "If you are using qibuild the command to do is: \"qibuild make-host-tools\"");
  }

  boost::filesystem::path linguisticPath = pSdkShareDir / "linguistic";
  xCreateDirectory(linguisticPath);
  boost::filesystem::path linguisticDatabasesPath = linguisticPath / "databases";
  xCreateDirectory(linguisticDatabasesPath);
  boost::filesystem::path installedVersionFilePath = linguisticDatabasesPath / "version.txt";
  fDynamicDatabasesFolder = linguisticPath / "tmpdatabases";

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
    ALAnyDatabaseLoader anyLoader;
    anyLoader.open(pLoadDatabasesDir / "reloadall.rla", fInputResourcesDir,
                   currLingdb, *this);
    xTryToLoadDynamicDbs();
    xCopyFile(inputVersionFilePath, installedVersionFilePath);

    std::map<std::string, ALLingdbConcept*> cptStrToCptStruct;
    for (std::size_t i = 0; i < fLanguages.size(); ++i)
    {
      if (fLanguages[i].dynDbVersion == -1)
      {
        std::string lang = semanticLanguageEnum_toLanguageFilenameStr(fLanguages[i].langGroundingsType);
        std::cerr << "Error: The language " << lang << " has no databases associated" << std::endl;
        continue;
      }

      auto& fpAlloc = fLanguages[i].dynDb->getFPAlloc();
      ALLingdbConcept* conceptElt = fpAlloc.first<ALLingdbConcept>();
      while (conceptElt != nullptr)
      {
        std::string conceptStr = conceptElt->getName()->toStr();
        cptStrToCptStruct.emplace(conceptStr, conceptElt);
        conceptElt = fpAlloc.next<ALLingdbConcept>(conceptElt);
      }
    }

    std::map<std::string, ConceptsBinMem> conceptsOffsets;
    ALBinaryDatabaseConceptsSaver conceceptsDicoSaver;
    conceceptsDicoSaver.saveConceptsDb
        (conceptsOffsets, cptStrToCptStruct,
         linguisticDatabasesPath / boost::filesystem::path("concepts." + getExtBinaryDatabase()));


    std::map<SemanticLanguageEnum, std::map<const ALLingdbMeaning*, int> > langToMeaningsPtr;
    ALBinaryDatabaseDicoSaver binDicoSaver;
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
          linguisticDatabasesPath / xGetStatDatabaseFilename(lang),
          linguisticDatabasesPath / boost::filesystem::path(lang + "animations." + getExtBinaryDatabase()),
          linguisticDatabasesPath / boost::filesystem::path(lang + "synthesizer." + getExtBinaryDatabase()),
          *fLanguages[i].dynDb);
    }

    xGenerateTranslations(langToMeaningsPtr, linguisticDatabasesPath);
    if (pClearTmpFolder)
      xRemoveDirectory(fDynamicDatabasesFolder);

    // List the paths
    if (!pDynamicdictionaryPath.empty())
    {
      {
        std::ofstream wordsrelativePathsFile(linguisticPath.string() + "/wordsrelativePaths.txt");
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
        std::ofstream treeConvertionsPathsFile(linguisticPath.string() + "/treeConvertionsPaths.txt");

        auto treeConverterPath = pDynamicdictionaryPath + "/treeconversions";
        boost::filesystem::directory_iterator itTreeConvsFolders(treeConverterPath);
        boost::filesystem::directory_iterator endit;
        while (itTreeConvsFolders != endit)
        {
          if (boost::filesystem::is_directory(itTreeConvsFolders->path()))
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


bool ALLingdbTree::xTryToLoadDynamicDbs()
{
  for (std::size_t i = 0; i < fLanguages.size(); ++i)
  {
    std::string lang = semanticLanguageEnum_toLanguageFilenameStr(fLanguages[i].langGroundingsType);

    fLanguages[i].dynDb = mystd::make_unique<LinguisticIntermediaryDatabase>();
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



void ALLingdbTree::xGenerateTranslations
(const std::map<SemanticLanguageEnum, std::map<const ALLingdbMeaning*, int> >& pLangToMeaningsPtr,
 const boost::filesystem::path& pLinguisticDatabasesPath)
{
  // generate concepts to meanings for every languages
  ALWlksDatabaseLoader::ALWlksDatabaseLoader_WorkState wlksWorkState(*this);
  for (std::size_t i = 0; i < fLanguages.size(); ++i)
  {
    SemanticLanguageEnum langType = fLanguages[i].langGroundingsType;
    if (langType == SemanticLanguageEnum::UNKNOWN ||
        fLanguages[i].dynDbVersion == -1)
    {
      continue;
    }

    std::string langStr = semanticLanguageEnum_toLegacyStr(langType);
    ALWlksDatabaseLoader::ALWlksDatabaseLoader_LangSpec& langSpec =
        wlksWorkState.strToLangSpecs[langStr];
    langSpec.lingDatabase = fLanguages[i].dynDb;
    langSpec.lingDatabase->getConceptToMeanings(langSpec.conceptToMeanings);
  }

  // load translations
  ALWlksDatabaseLoader wlksLoader;
  wlksLoader.load(wlksWorkState, fInputResourcesDir / "common" / "traductions.wlks");

  // save translations
  ALBinaryTradSaver tradSaver;
  tradSaver.save(wlksWorkState, pLangToMeaningsPtr, pLinguisticDatabasesPath);
}




void ALLingdbTree::xClearDirectory
(const boost::filesystem::path& pDirectory) const
{
  boost::filesystem::path pathToRemove(pDirectory);
  for (boost::filesystem::directory_iterator end_dir_it, itFilesToRemove(pathToRemove);
       itFilesToRemove != end_dir_it; ++itFilesToRemove)
  {
    boost::filesystem::remove_all(itFilesToRemove->path());
  }
}



std::string ALLingdbTree::xGetDynDatabaseFilename
(const std::string& pLanguage) const
{
  return pLanguage + "." + getExtDynDatabase();
}


std::string ALLingdbTree::xGetStatDatabaseFilename
(const std::string& pLanguage) const
{
  return pLanguage + "database." + getExtBinaryDatabase();
}


void ALLingdbTree::getHoldingFolder
(boost::filesystem::path& pFolder,
 const boost::filesystem::path& pFilename) const
{
  boost::system::error_code ec;
  if (boost::filesystem::is_regular_file(pFilename, ec))
  {
    pFolder = pFilename.parent_path();
    return;
  }
  pFolder = pFilename;
}


void ALLingdbTree::xCreateDirectory
(const boost::filesystem::path& pPath) const
{
  boost::filesystem::create_directory(pPath);
}


void ALLingdbTree::xRemoveDirectory
(const boost::filesystem::path& pPath) const
{
  boost::filesystem::remove_all(pPath);
}

void ALLingdbTree::xCopyFile
(const boost::filesystem::path& pFrom,
 const boost::filesystem::path& pTo) const
{
  /*
  boost::filesystem::path from(pFrom.c_str());
  boost::filesystem::path to(pTo.c_str());
  boost::filesystem::copy_file(from, to, boost::filesystem::copy_option::overwrite_if_exists);
  */

  boost::filesystem::ifstream source(pFrom, boost::filesystem::ifstream::binary);
  boost::filesystem::ofstream dest(pTo, boost::filesystem::ofstream::binary);
  dest << source.rdbuf();
  source.close();
  dest.close();

}



} // End of namespace onsem
