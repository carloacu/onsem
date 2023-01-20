#ifndef ONSEM_COMPILERMODEL_LOADERS_DELADATABASELOADER_HPP
#define ONSEM_COMPILERMODEL_LOADERS_DELADATABASELOADER_HPP

#include <vector>
#include <string>
#include <map>
#include <set>
#include <onsem/common/enum/partofspeech.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;

/// This class load a database from a dela dictionnary file.
class DelaDatabaseLoader
{

public:
  /**
   * @brief Constructor.
   * @param pSaveWords If we want to save words
   * (for a debug test only).
   */
  DelaDatabaseLoader(bool pSaveWords = false);

  void simplifyDelaFile
  (const std::string& pInFilename,
   const std::string& pOutFilename,
   const std::set<std::string>& pLemmaToKeep,
   bool pRemoveHum,
   bool pRemoveDnum);

  /**
   * @brief Merge the current database with a Dela text file.
   * @param pFilename Filename of the Dela text file.
   * @param pWords The database currently in memory.
   */
  void merge
  (const std::string &pFilename, LinguisticIntermediaryDatabase& pWords);


private:
  /// If we have to save words. (only for debug)
  const bool fSaveWords;
  /// All the words saved. (only for debug)
  std::vector<std::string> fWords;

  struct NewWordInfos
  {
    NewWordInfos()
      : word(),
        lemma(),
        gram(PartOfSpeech::BOOKMARK),
        flexions(),
        otherInfos()
    {
    }

    std::string word;
    std::string lemma;
    PartOfSpeech gram;
    std::vector<std::string> flexions;
    std::vector<std::string> otherInfos;
  };

  void xFillNewWordInfos
  (NewWordInfos& pNewWord,
   const std::string& pLine) const;

  std::size_t xGetStringUntilAChar
  (std::string& pStrResult,
   std::size_t pBeginOfSearch,
   char pEndingChar,
   const std::string& pLine) const;

  void xReadWordInfo
  (NewWordInfos& pNewWord,
   std::size_t& pBeforeInfo,
   std::size_t& pBeginOfInfo,
   std::size_t& pEndOfInfo,
   bool& pFlexionsAreSet,
   const std::string& pLine) const;

  PartOfSpeech xReadGram
  (const std::string& pStr) const;

};

} // End of namespace onsem

#endif // ONSEM_COMPILERMODEL_LOADERS_DELADATABASELOADER_HPP
