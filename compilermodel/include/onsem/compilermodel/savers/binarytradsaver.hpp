#ifndef ONSEM_COMPILERMODEL_SAVERS_BINARYTRADSAVER_HPP
#define ONSEM_COMPILERMODEL_SAVERS_BINARYTRADSAVER_HPP

#include <string>
#include <filesystem>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/compilermodel/loaders/wlksdatabaseloader.hpp>
#include "binarydatabasesaver.hpp"


namespace onsem
{


class BinaryTradSaver : public BinaryDatabaseSaver
{
public:

  void save
  (const WlksDatabaseLoader::WlksDatabaseLoader_WorkState& pTrads,
   const std::map<SemanticLanguageEnum, std::map<const LingdbMeaning*, int> >& pLangToMeaningsPtr,
   const std::filesystem::path &pOutFolder) const;

private:
  struct TreeTradWorkState : public BinaryDatabaseSaver
  {
    TreeTradWorkState
    (const std::map<LingdbMeaning*, std::set<MeaningAndConfidence> >& pMeaningsToTrads,
     const LinguisticIntermediaryDatabase& pLingDb,
     const std::map<const LingdbMeaning*, int>& pInMeaningsPtr,
     const std::map<const LingdbMeaning*, int>& pOutMeaningsPtr)
      : meaningsToTrads(pMeaningsToTrads),
        lingDb(pLingDb),
        inMeaningsPtr(pInMeaningsPtr),
        outMeaningsPtr(pOutMeaningsPtr)
    {
    }

    binarymasks::Ptr printEndOfANode
    (LingdbDynamicTrieNode* pNode,
     int,
     binarymasks::Ptr pEndMemory);

    const std::map<LingdbMeaning*, std::set<MeaningAndConfidence> >& meaningsToTrads;
    const LinguisticIntermediaryDatabase& lingDb;
    const std::map<const LingdbMeaning*, int>& inMeaningsPtr;
    const std::map<const LingdbMeaning*, int>& outMeaningsPtr;
  };


  const std::map<const LingdbMeaning*, int>& xGetMeaningsPtrOfALang
  (const std::map<SemanticLanguageEnum, std::map<const LingdbMeaning*, int> >& pLangToMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDb) const;

};

} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_SAVERS_BINARYTRADSAVER_HPP
