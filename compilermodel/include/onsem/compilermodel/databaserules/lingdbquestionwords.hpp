#ifndef ONSEM_COMPILERMODEL_LOADERS_LINGDBQUESTIONWORDS_HPP
#define ONSEM_COMPILERMODEL_LOADERS_LINGDBQUESTIONWORDS_HPP

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <onsem/compilermodel/lingdbstring.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/common/enum/wordtoverbrelation.hpp>

namespace onsem
{
template <typename T>
struct ForwardPtrList;
class LingdbMeaning;
class LinguisticIntermediaryDatabase;

class LingdbQuestionWords
{
public:
  LingdbQuestionWords();

  void init();

  void load
  (CompositePoolAllocator& pAlloc,
   const boost::property_tree::ptree& pXml);


  signed char* exportToBin
  (signed char* pEndMemory,
   const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDatabase);

private:
  friend class LinguisticIntermediaryDatabase;

  struct AQuestionWord
  {
    AQuestionWord()
      : word(nullptr),
        gram(0),
        canBeBeforeVerb(WordToVerbRelation::NO),
        canBeAfterVerb(WordToVerbRelation::NO),
        canBeAlone(false),
        request(0),
        followedByRequestedWord(false)
    {
    }

    void init
    (CompositePoolAllocator& pFPAlloc,
     const std::string& pWord,
     char pGram,
     WordToVerbRelation pCanBeBeforeVerb,
     WordToVerbRelation pCanBeAfterVerb,
     bool pCanBeAlone,
     char pRequest,
     bool pFollowedByRequestedWord)
    {
      word = pFPAlloc.allocate<LingdbString>(1);
      word->xInit(pFPAlloc, pWord);
      gram = pGram;
      canBeBeforeVerb = pCanBeBeforeVerb;
      canBeAfterVerb = pCanBeAfterVerb;
      canBeAlone = pCanBeAlone;
      request = pRequest;
      followedByRequestedWord = pFollowedByRequestedWord;
    }

    void xDeallocate
    (CompositePoolAllocator& pAlloc)
    {
      word->xDeallocate(pAlloc);
      pAlloc.deallocate<AQuestionWord>(this);
    }

    static void xGetPointers
    (std::vector<const void*>& pRes, void* pVar)
    {
      pRes.emplace_back(&reinterpret_cast<AQuestionWord*>
                     (pVar)->word);
    }

    LingdbString* word;
    char gram;
    WordToVerbRelation canBeBeforeVerb;
    WordToVerbRelation canBeAfterVerb;
    bool canBeAlone;
    char request;
    bool followedByRequestedWord;
  };
  ForwardPtrList<AQuestionWord>* fQWords;

  void xClear
  (CompositePoolAllocator& pAlloc);

  void xDeallocate
  (CompositePoolAllocator& pAlloc);

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

};


inline void LingdbQuestionWords::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbQuestionWords*>
                 (pVar)->fQWords);
}


} // End of namespace onsem

#endif // ONSEM_COMPILERMODEL_LOADERS_LINGDBQUESTIONWORDS_HPP
