#ifndef ALWLKSDATABASELOADER_H
#define ALWLKSDATABASELOADER_H

#include <string>
#include <map>
#include <list>
#include <set>
#include <memory>
#include <boost/filesystem/path.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALLingdbTree;
class ALLingdbMeaning;
class ALLingdbConcept;
class ALLingdbLinkToAConcept;
struct MeaningAndConfidence;

class ALWlksDatabaseLoader
{
public:
  enum ALWordsLinksDatabaseLoader_NextLineSpec
  {
    ALWLKSDBLOADER_NEXTLINES_TRADUCTION,
    ALWLKSDBLOADER_NEXTLINES_NOTDEFINED
  };
  struct ALWlksDatabaseLoader_LangSpec
  {
    ALWlksDatabaseLoader_LangSpec()
      : lingDatabase(),
        conceptToMeanings()
    {
    }

    std::shared_ptr<LinguisticIntermediaryDatabase> lingDatabase;
    std::map<const ALLingdbConcept*, std::set<MeaningAndConfidence> > conceptToMeanings;
  };
  struct ALWlksDatabaseLoader_TradSpec
  {
    ALWlksDatabaseLoader_TradSpec
    (ALWlksDatabaseLoader_LangSpec& pInLingDb,
     ALWlksDatabaseLoader_LangSpec& pOutLingDb)
      : inLingDb(pInLingDb),
        outLingDb(pOutLingDb),
        traductions()
    {
    }

    ALWlksDatabaseLoader_LangSpec& inLingDb;
    ALWlksDatabaseLoader_LangSpec& outLingDb;
    std::map<ALLingdbMeaning*, std::set<MeaningAndConfidence> > traductions;
  };
  struct ALWlksDatabaseLoader_WorkState
  {
    ALWlksDatabaseLoader_WorkState
    (const ALLingdbTree& pLingbTree)
      : lingbTree(pLingbTree),
        strToLangSpecs(),
        tradSpecs(),
        nextLinesSpec(ALWLKSDBLOADER_NEXTLINES_NOTDEFINED)
    {
    }

    std::size_t maxOccupatedSize() const;
    ALWlksDatabaseLoader_LangSpec* getLangSpec(const std::string& pLanguage);
    std::list<ALWlksDatabaseLoader_TradSpec*> getTraductionsOfALanguage(const std::string& pLanguage);

    const ALLingdbTree& lingbTree;
    std::map<std::string, ALWlksDatabaseLoader_LangSpec> strToLangSpecs;
    std::list<ALWlksDatabaseLoader_TradSpec> tradSpecs;
    ALWordsLinksDatabaseLoader_NextLineSpec nextLinesSpec;
  };


  void loadAndSave
  (const boost::filesystem::path& pFilename,
   const ALLingdbTree& pLingbTree) const;


  void load
  (ALWlksDatabaseLoader_WorkState& pWorkState,
   const boost::filesystem::path& pFilename) const;


private:
  void xLoadDynNewDb
  (ALWlksDatabaseLoader_WorkState& pWorkState,
   const std::string& pLang) const;

  void xGetNextMeaningInLine
  (ALLingdbMeaning** pMeaning,
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

#endif // ALWLKSDATABASELOADER_H
