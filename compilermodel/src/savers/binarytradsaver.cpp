#include <onsem/compilermodel/savers/binarytradsaver.hpp>
#include <fstream>
#include <onsem/common/linguisticmeaning_nomeaningid.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>

namespace onsem
{


void BinaryTradSaver::save
(const WlksDatabaseLoader::WlksDatabaseLoader_WorkState& pTrads,
 const std::map<SemanticLanguageEnum, std::map<const LingdbMeaning*, int> >& pLangToMeaningsPtr,
 const std::filesystem::path& pOutFolder) const
{
  binarymasks::Ptr mem = ::operator new(pTrads.maxOccupatedSize());
  if (mem.ptr == nullptr)
  {
    throw std::runtime_error("Bad allocation in BinaryTradSaver.");
  }

  for (std::list<WlksDatabaseLoader::WlksDatabaseLoader_TradSpec>::const_iterator
       it = pTrads.tradSpecs.begin();
       it != pTrads.tradSpecs.end(); ++it)
  {
    const std::map<const LingdbMeaning*, int>& inMeaningsPtr =
        xGetMeaningsPtrOfALang(pLangToMeaningsPtr, *it->inLingDb.lingDatabase);
    const std::map<const LingdbMeaning*, int>& outMeaningsPtr =
        xGetMeaningsPtrOfALang(pLangToMeaningsPtr, *it->outLingDb.lingDatabase);

    LinguisticIntermediaryDatabase treeOfWords;
    for (std::map<LingdbMeaning*, std::set<MeaningAndConfidence> >::const_iterator
         itWords = it->traductions.begin();
         itWords != it->traductions.end(); ++itWords)
    {
      std::string currWord = itWords->first->getLemma()->getWord();
      treeOfWords.addWord(currWord, currWord,
                          itWords->first->getPartOfSpeech(), std::vector<std::string>(), 4);
    }

    TreeTradWorkState treeTradWS(it->traductions, *it->inLingDb.lingDatabase,
                                 inMeaningsPtr, outMeaningsPtr);
    binarymasks::Ptr endMemory = xCreateRootNode(treeTradWS,
                                               treeOfWords.getFPAlloc().first<LingdbDynamicTrieNode>(),
                                               mem, mem);
    std::size_t sizeMemory = endMemory.val - mem.val;

    std::string twoLangsOfTheTrad = it->inLingDb.lingDatabase->getLanguage()->toStr() +
        "_to_" + it->outLingDb.lingDatabase->getLanguage()->toStr();
    std::filesystem::path tradFilename(twoLangsOfTheTrad + "." + pTrads.lingbTree.getExtBinaryDatabase());
    std::ofstream outfile((pOutFolder / tradFilename).string(), std::ofstream::binary);
    outfile.write(reinterpret_cast<const char*>(&fFormalism), sizeof(fFormalism));

    // Write memory size
    binarysaver::writePtr(outfile, sizeMemory);

    // Write all the memory
    outfile.write(reinterpret_cast<const char*>(mem.ptr), sizeMemory);

    outfile.close();
  }

  ::operator delete(mem.ptr);
}




const std::map<const LingdbMeaning*, int>& BinaryTradSaver::xGetMeaningsPtrOfALang
(const std::map<SemanticLanguageEnum, std::map<const LingdbMeaning*, int> >& pLangToMeaningsPtr,
 const LinguisticIntermediaryDatabase& pLingDb) const
{
  auto itMeaningPtr = pLangToMeaningsPtr.find(semanticLanguageTypeGroundingEnumFromStr(pLingDb.getLanguage()->toStr()));
  if (itMeaningPtr == pLangToMeaningsPtr.end())
  {
    throw std::runtime_error("Cannot find meanings offests for a language");
  }
  return itMeaningPtr->second;
}



binarymasks::Ptr BinaryTradSaver::TreeTradWorkState::printEndOfANode
(LingdbDynamicTrieNode* pNode,
 int,
 binarymasks::Ptr pEndMemory)
{
  pEndMemory = binarysaver::alignMemory(pEndMemory);

  // for each word form
  const ForwardPtrList<LingdbWordForms>* wfl = pNode->getWordForms();
  while (wfl != nullptr)
  {
    LingdbMeaning* inMeaning = lingDb.getMeaning(wfl->elt->getMeaning()->getLemma()->getWord(),
                                                   wfl->elt->getMeaning()->getPartOfSpeech());
    if (inMeaning == nullptr)
    {
      throw std::runtime_error("Input meaning pointer has changed");
    }
    auto itInMeaning = inMeaningsPtr.find(inMeaning);
    if (itInMeaning == inMeaningsPtr.end())
    {
      throw std::runtime_error("Cannot find meanings offests for input language");
    }
    binarysaver::writeInThreeBytes(pEndMemory.pchar, itInMeaning->second);
    pEndMemory.val += sizeof(int);

    std::map<LingdbMeaning*, std::set<MeaningAndConfidence> >::const_iterator
        itTradMeaning = meaningsToTrads.find(inMeaning);
    if (itTradMeaning == meaningsToTrads.end() ||
        itTradMeaning->second.empty())
    {
      binarysaver::writeInThreeBytes(pEndMemory.pchar, LinguisticMeaning_noMeaningId);
      pEndMemory.val += sizeof(int);
    }
    else
    {
      auto itOutMeaning = outMeaningsPtr.find(itTradMeaning->second.begin()->meaning);
      if (itOutMeaning == outMeaningsPtr.end())
      {
        throw std::runtime_error("Cannot find meanings offests for output language");
      }

      binarysaver::writeInThreeBytes(pEndMemory.pchar, itOutMeaning->second);
      pEndMemory.val += sizeof(int);
    }

    wfl = wfl->next;
  }
  return pEndMemory;
}



} // End of namespace onsem
