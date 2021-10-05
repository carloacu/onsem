#ifndef ALLINGDBQUESTIONWORDS_H
#define ALLINGDBQUESTIONWORDS_H

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <onsem/lingdbeditor/allingdbstring.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/common/enum/wordtoverbrelation.hpp>

namespace onsem
{
template <typename T>
struct ForwardPtrList;
class ALLingdbMeaning;
class LinguisticIntermediaryDatabase;

class ALLingdbQuestionWords
{
public:
  ALLingdbQuestionWords();

  void init();

  void load
  (ALCompositePoolAllocator& pAlloc,
   const boost::property_tree::ptree& pXml);


  signed char* exportToBin
  (signed char* pEndMemory,
   const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
   const LinguisticIntermediaryDatabase& pLingDatabase);

private:
  friend class LinguisticIntermediaryDatabase;

  struct ALAQuestionWord
  {
    ALAQuestionWord()
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
    (ALCompositePoolAllocator& pFPAlloc,
     const std::string& pWord,
     char pGram,
     WordToVerbRelation pCanBeBeforeVerb,
     WordToVerbRelation pCanBeAfterVerb,
     bool pCanBeAlone,
     char pRequest,
     bool pFollowedByRequestedWord)
    {
      word = pFPAlloc.allocate<ALLingdbString>(1);
      word->xInit(pFPAlloc, pWord);
      gram = pGram;
      canBeBeforeVerb = pCanBeBeforeVerb;
      canBeAfterVerb = pCanBeAfterVerb;
      canBeAlone = pCanBeAlone;
      request = pRequest;
      followedByRequestedWord = pFollowedByRequestedWord;
    }

    void xDeallocate
    (ALCompositePoolAllocator& pAlloc)
    {
      word->xDeallocate(pAlloc);
      pAlloc.deallocate<ALAQuestionWord>(this);
    }

    static void xGetPointers
    (std::vector<const void*>& pRes, void* pVar)
    {
      pRes.emplace_back(&reinterpret_cast<ALAQuestionWord*>
                     (pVar)->word);
    }

    ALLingdbString* word;
    char gram;
    WordToVerbRelation canBeBeforeVerb;
    WordToVerbRelation canBeAfterVerb;
    bool canBeAlone;
    char request;
    bool followedByRequestedWord;
  };
  ForwardPtrList<ALAQuestionWord>* fQWords;

  void xClear
  (ALCompositePoolAllocator& pAlloc);

  void xDeallocate
  (ALCompositePoolAllocator& pAlloc);

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

};


inline void ALLingdbQuestionWords::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbQuestionWords*>
                 (pVar)->fQWords);
}


} // End of namespace onsem

#endif // ALLINGDBQUESTIONWORDS_H
