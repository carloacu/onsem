#include <onsem/compilermodel/databaserules/lingdbquestionwords.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/wordtoverbrelation.hpp>

namespace onsem
{
// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)


LingdbQuestionWords::LingdbQuestionWords()
  : fQWords(nullptr)
{
}


void LingdbQuestionWords::init()
{
  fQWords = nullptr;
}

void LingdbQuestionWords::load
(ALCompositePoolAllocator& pAlloc,
 const boost::property_tree::ptree& pXml)
{
  xClear(pAlloc);
  auto size = pXml.size();
  assert(size == 2);

  childLoop(pXml, currQWordTree, "list")
  {
    if (currQWordTree.first == "<xmlattr>")
      continue;
    const boost::property_tree::ptree& attrs = currQWordTree.second.get_child("<xmlattr>");
    AQuestionWord* newQW = pAlloc.allocate<AQuestionWord>(1);
    newQW->init(pAlloc,
                attrs.get<std::string>("lemme", ""),
                static_cast<char>(partOfSpeech_fromStr(attrs.get<std::string>("gram", ""))),
                wordToVerbRelations_fromStr(attrs.get<std::string>("beforeVerb", "")),
                wordToVerbRelations_fromStr(attrs.get<std::string>("afterVerb", "")),
                std::string(attrs.get<std::string>("canBeAlone", "no")) == "yes",
                static_cast<char>(semanticRequestType_fromStr(attrs.get<std::string>("request", ""))),
                std::string(attrs.get<std::string>("followedByRequestedWord", "no")) == "yes");
    forwardListPushFront(pAlloc, &fQWords, newQW);
  }
}


signed char* LingdbQuestionWords::exportToBin(
    signed char* pEndMemory,
    const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
    const LinguisticIntermediaryDatabase& pLingDatabase)
{
  if (fQWords == nullptr)
  {
    *(pEndMemory++) = 0;
    return pEndMemory;
  }
  *(pEndMemory++) = fQWords->length();

  ForwardPtrList<AQuestionWord>* qws = fQWords;
  while (qws != nullptr)
  {
    LingdbMeaning* meaning = pLingDatabase.getMeaning(qws->elt->word->toStr(),
                                                        static_cast<PartOfSpeech>(qws->elt->gram));
    auto itMeaning = pMeaningsPtr.find(meaning);
    if (itMeaning == pMeaningsPtr.end())
    {
      throw std::runtime_error("word: " + qws->elt->word->toStr() + ", with gram: " +
                               partOfSpeech_toStr(static_cast<PartOfSpeech>(qws->elt->gram)) + " doesn't exist in the database");
    }
    int* meaningPtr = reinterpret_cast<int*>(pEndMemory);
    *meaningPtr = itMeaning->second;
    pEndMemory += sizeof(int);
    *(pEndMemory++) = wordToVerbRelations_toChar(qws->elt->canBeBeforeVerb);
    *(pEndMemory++) = wordToVerbRelations_toChar(qws->elt->canBeAfterVerb);
    *(pEndMemory++) = qws->elt->canBeAlone;
    *(pEndMemory++) = qws->elt->request;
    *(pEndMemory++) = qws->elt->followedByRequestedWord;
    qws = qws->next;
  }
  return pEndMemory;
}


void LingdbQuestionWords::xClear
(ALCompositePoolAllocator& pAlloc)
{
  if (fQWords != nullptr)
  {
    fQWords->clearComposedElts(pAlloc);
    fQWords = nullptr;
  }
}


void LingdbQuestionWords::xDeallocate
(ALCompositePoolAllocator& pAlloc)
{
  xClear(pAlloc);
  pAlloc.deallocate<LingdbQuestionWords>(this);
}


} // End of namespace onsem
