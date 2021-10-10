#ifndef ONSEM_COMPILERMODEL_LOADERS_WLKSDATABASELOADER_HPP
#define ONSEM_COMPILERMODEL_LOADERS_WLKSDATABASELOADER_HPP

#include <string>
#include <map>
#include <list>
#include <set>
#include <memory>
#include <boost/filesystem/path.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class LingdbTree;
class LingdbMeaning;
class LingdbConcept;
class LingdbLinkToAConcept;
struct MeaningAndConfidence;

class WlksDatabaseLoader
{
public:
  enum WordsLinksDatabaseLoader_NextLineSpec
  {
    ALWLKSDBLOADER_NEXTLINES_TRADUCTION,
    ALWLKSDBLOADER_NEXTLINES_NOTDEFINED
  };
  struct WlksDatabaseLoader_LangSpec
  {
    WlksDatabaseLoader_LangSpec()
      : lingDatabase(),
        conceptToMeanings()
    {
    }

    std::shared_ptr<LinguisticIntermediaryDatabase> lingDatabase;
    std::map<const LingdbConcept*, std::set<MeaningAndConfidence> > conceptToMeanings;
  };
  struct WlksDatabaseLoader_TradSpec
  {
    WlksDatabaseLoader_TradSpec
    (WlksDatabaseLoader_LangSpec& pInLingDb,
     WlksDatabaseLoader_LangSpec& pOutLingDb)
      : inLingDb(pInLingDb),
        outLingDb(pOutLingDb),
        traductions()
    {
    }

    WlksDatabaseLoader_LangSpec& inLingDb;
    WlksDatabaseLoader_LangSpec& outLingDb;
    std::map<LingdbMeaning*, std::set<MeaningAndConfidence> > traductions;
  };
  struct WlksDatabaseLoader_WorkState
  {
    WlksDatabaseLoader_WorkState
    (const LingdbTree& pLingbTree)
      : lingbTree(pLingbTree),
        strToLangSpecs(),
        tradSpecs(),
        nextLinesSpec(ALWLKSDBLOADER_NEXTLINES_NOTDEFINED)
    {
    }

    std::size_t maxOccupatedSize() const;
    WlksDatabaseLoader_LangSpec* getLangSpec(const std::string& pLanguage);
    std::list<WlksDatabaseLoader_TradSpec*> getTraductionsOfALanguage(const std::string& pLanguage);

    const LingdbTree& lingbTree;
    std::map<std::string, WlksDatabaseLoader_LangSpec> strToLangSpecs;
    std::list<WlksDatabaseLoader_TradSpec> tradSpecs;
    WordsLinksDatabaseLoader_NextLineSpec nextLinesSpec;
  };


  void loadAndSave
  (const boost::filesystem::path& pFilename,
   const LingdbTree& pLingbTree) const;


  void load
  (WlksDatabaseLoader_WorkState& pWorkState,
   const boost::filesystem::path& pFilename) const;


private:
  void xLoadDynNewDb
  (WlksDatabaseLoader_WorkState& pWorkState,
   const std::string& pLang) const;

  void xGetNextMeaningInLine
  (LingdbMeaning** pMeaning,
   char& pConfidence,
   std::size_t& pCurrPos,
   const LinguisticIntermediaryDatabase& pLingDb,
   const std::string& pLine) const;

  bool xHasSlash
  (const std::string& pStr) const;

  void xRemoveSlashesBeforeSpecifcChars
  (std::string& pStr) const;
};



} // End of namespace onsem

#endif // ONSEM_COMPILERMODEL_LOADERS_WLKSDATABASELOADER_HPP
