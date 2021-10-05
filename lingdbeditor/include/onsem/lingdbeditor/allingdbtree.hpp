#ifndef ALLINGDBTREE_H
#define ALLINGDBTREE_H

#include <string>
#include <vector>
#include <memory>
#include <boost/filesystem/path.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALLingdbMeaning;

class ALLingdbTree
{
public:
  explicit ALLingdbTree(const boost::filesystem::path& pInputResourcesDir);

  boost::filesystem::path getDynDbFilenameForLanguage(const std::string& pLang) const;


  void update(const boost::filesystem::path& pSdkShareDir,
              const boost::filesystem::path& pLoadDatabasesDir,
              const std::string& pDynamicdictionaryPath,
              bool pClearTmpFolder);

  void clearAll() const;

  void getHoldingFolder
  (boost::filesystem::path& pFolder,
   const boost::filesystem::path& pFilename) const;


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


  boost::filesystem::path getDynamicDatabasesFolder() const;

  std::string getRlaDatabase() const;

  std::string getCptsDatabase() const;

  std::string getWlksDatabase() const;

  const boost::filesystem::path& getInputResourcesDir() const { return fInputResourcesDir; }


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
  const boost::filesystem::path fInputResourcesDir;
  boost::filesystem::path fDynamicDatabasesFolder;
  std::vector<LanguageStruct> fLanguages;
  /// If the machine is a 32 bits (64 bits otherwise)
  bool fIn32Bits;

  bool xExtractVersionFromFile
  (std::string& pFormalismVersion,
   std::string& pVersion,
   const boost::filesystem::path& pFilePath);

  std::string xGetDynDatabaseFilename
  (const std::string& pLanguage) const;

  std::string xGetStatDatabaseFilename
  (const std::string& pLanguage) const;

  void xCreateDirectory
  (const boost::filesystem::path& pPath) const;

  void xRemoveDirectory
  (const boost::filesystem::path& pPath) const;

  void xCopyFile
  (const boost::filesystem::path& pFrom,
   const boost::filesystem::path& pTo) const;

  void xClearDirectory
  (const boost::filesystem::path& pDirectory) const;

  bool xTryToLoadDynamicDbs();

  void xGenerateTranslations
  (const std::map<SemanticLanguageEnum, std::map<const ALLingdbMeaning*, int> >& pLangToMeaningsPtr,
   const boost::filesystem::path& pLinguisticDatabasesPath);
};


} // End of namespace onsem

#include "details/allingdbtree.hxx"

#endif // ALLINGDBTREE_H
