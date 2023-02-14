#ifndef ONSEM_COMPILERMODEL_LINGDBTREE_HPP
#define ONSEM_COMPILERMODEL_LINGDBTREE_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <onsem/common/enum/semanticlanguageenum.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class LingdbMeaning;

class LingdbTree
{
public:
  explicit LingdbTree(const std::string& pInputResourcesDir);

  std::string getDynDbFilenameForLanguage(const std::string& pLang) const;


  void update(const std::string& pSdkShareDir,
              const std::string& pLoadDatabasesDir,
              const std::string& pDynamicdictionaryPath,
              bool pClearTmpFolder);

  void clearAll() const;

  void getHoldingFolder
  (std::string& pFolder,
   const std::filesystem::path& pFilename) const;


  /**
   * @brief Get the extension of a dynamic database file.
   * @return The extension of a dynamic database file.
   */
  std::string getExtDynDatabase() const;


  /**
   * @brief Get the extension of a dela database file.
   * @return The extension of a dela database file.
   */
  std::string getExtDelaDatabase() const;

  std::string getExtGfsDatabase() const;


  /**
   * @brief Get the extension of an xml database file.
   * @return The extension of an xml database file.
   */
  std::string getExtXmlDatabase() const;


  /**
   * @brief Get the extension of a binary database file.
   * @return The extension of a binary database file.
   */
  std::string getExtBinaryDatabase() const;


  std::string getDynamicDatabasesFolder() const;

  std::string getRlaDatabase() const;

  std::string getCptsDatabase() const;

  std::string getWlksDatabase() const;

  const std::string& getInputResourcesDir() const { return fInputResourcesDir; }


private:
  struct LanguageStruct
  {
    LanguageStruct
    (SemanticLanguageEnum pLangGroundingsType)
      : langGroundingsType(pLangGroundingsType),
        dynDb(),
        dynDbVersion(-1),
        statDbVersion(-1)
    {
    }

    SemanticLanguageEnum langGroundingsType;
    std::shared_ptr<LinguisticIntermediaryDatabase> dynDb;
    int dynDbVersion;
    int statDbVersion;
  };
  const std::string fInputResourcesDir;
  std::string fDynamicDatabasesFolder;
  std::vector<LanguageStruct> fLanguages;
  /// If the machine is a 32 bits (64 bits otherwise)
  bool fIn32Bits;

  bool xExtractVersionFromFile
  (std::string& pFormalismVersion,
   std::string& pVersion,
   const std::string& pFilePath);

  std::string xGetDynDatabaseFilename
  (const std::string& pLanguage) const;

  std::string xGetStatDatabaseFilename
  (const std::string& pLanguage) const;

  void xCreateDirectory
  (const std::string& pPath) const;

  void xRemoveDirectory
  (const std::filesystem::path &pPath) const;

  void xCopyFile
  (const std::string& pFrom,
   const std::string& pTo) const;

  void xClearDirectory
  (const std::string& pDirectory) const;

  bool xTryToLoadDynamicDbs();

  void xGenerateTranslations
  (const std::map<SemanticLanguageEnum, std::map<const LingdbMeaning*, int> >& pLangToMeaningsPtr,
   const std::filesystem::path& pLinguisticDatabasesPath);
};


} // End of namespace onsem

#include "details/lingdbtree.hxx"

#endif // ONSEM_COMPILERMODEL_LINGDBTREE_HPP
