#ifndef ALBINARYTRADSAVER_H
#define ALBINARYTRADSAVER_H

#include <string>
#include <boost/filesystem/path.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/lingdbeditor/loaders/alwlksdatabaseloader.hpp>
#include "albinarydatabasesaver.hpp"


namespace onsem
{


class ALBinaryTradSaver : public ALBinaryDatabaseSaver
{
public:

  void save
  (const ALWlksDatabaseLoader::ALWlksDatabaseLoader_WorkState& pTrads,
   const std::map<SemanticLanguageEnum, std::map<const ALLingdbMeaning*, int> >& pLangToMeaningsPtr,
   const boost::filesystem::path& pOutFolder) const;

private:
  struct TreeTradWorkState : public ALBinaryDatabaseSaver
  {
    TreeTradWorkState
    (const std::map<ALLingdbMeaning*, std::set<MeaningAndConfidence> >& pMeaningsToTrads,
     const LinguisticIntermediaryDatabase& pLingDb,
     const std::map<const ALLingdbMeaning*, int>& pInMeaningsPtr,
     const std::map<const ALLingdbMeaning*, int>& pOutMeaningsPtr)
      : meaningsToTrads(pMeaningsToTrads),
        lingDb(pLingDb),
        inMeaningsPtr(pInMeaningsPtr),
        outMeaningsPtr(pOutMeaningsPtr)
    {
    }

    binarymasks::Ptr printEndOfANode
    (ALLingdbDynamicTrieNode* pNode,
     int,
     binarymasks::Ptr pEndMemory);

    const std::map<ALLingdbMeaning*, std::set<MeaningAndConfidence> >& meaningsToTrads;
    const LinguisticIntermediaryDatabase& lingDb;
    const std::map<const ALLingdbMeaning*, int>& inMeaningsPtr;
    const std::map<const ALLingdbMeaning*, int>& outMeaningsPtr;
  };


  const std::map<const ALLingdbMeaning*, int>& xGetMeaningsPtrOfALang
  (const std::map<SemanticLanguageEnum, std::map<const ALLingdbMeaning*, int> >& pLangToMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDb) const;

};

} // End of namespace onsem


#endif // ALBINARYTRADSAVER_H
