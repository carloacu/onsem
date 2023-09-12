#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_WORDTOSYNTHESIZE_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_WORDTOSYNTHESIZE_HPP

#include <string>
#include <list>
#include <set>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include "../../api.hpp"

namespace onsem
{
struct WordToSynthesize;
struct OutSentence;


struct ONSEM_TEXTTOSEMANTIC_API InflectionToSynthesize
{
  InflectionToSynthesize
  (const std::string& pStr,
   bool pIfCanHaveSpaceBefore = true,
   bool pIfCanHaveSpaceAfter = true,
   bool (*pConditions)(const WordToSynthesize&) = [](const WordToSynthesize&){ return true; },
   bool (*pContextCondition)(const OutSentence*) = [](const OutSentence*){ return true; })
    : str(pStr),
      ifCanHaveSpaceBefore(pIfCanHaveSpaceBefore),
      ifCanHaveSpaceAfter(pIfCanHaveSpaceAfter),
      canHavePunctionAfter(true),
      conditions(pConditions),
      contextCondition(pContextCondition),
      fromResourcePtr(nullptr)
  {
  }

  bool operator==(const InflectionToSynthesize& pOther) const
  {
    return str == pOther.str &&
        ifCanHaveSpaceBefore == pOther.ifCanHaveSpaceBefore &&
        ifCanHaveSpaceAfter == pOther.ifCanHaveSpaceAfter &&
        canHavePunctionAfter == pOther.canHavePunctionAfter &&
        fromResourcePtr == pOther.fromResourcePtr;
  }
  bool operator!=(const InflectionToSynthesize& pOther) const
  {
    return !operator==(pOther);
  }

  std::string str;
  bool ifCanHaveSpaceBefore;
  bool ifCanHaveSpaceAfter;
  bool canHavePunctionAfter;
  bool (*conditions)(const WordToSynthesize&);
  bool (*contextCondition)(const OutSentence*);
  const SemanticResourceGrounding* fromResourcePtr;
};

enum class WordToSynthesizeTag
{
  ANY,
  DATE
};

struct ONSEM_TEXTTOSEMANTIC_API WordToSynthesize
{
  WordToSynthesize(const SemanticWord& pWord,
                   const InflectionToSynthesize& pOutBloc,
                   WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY,
                   const std::set<WordContextualInfos>* pContextualInfosPtr = nullptr,
                   int pPriorityOffset = 0)
    : word(pWord),
      inflections(1, pOutBloc),
      concepts(),
      tag(pTag),
      contextualInfos(pContextualInfosPtr != nullptr ? *pContextualInfosPtr : std::set<WordContextualInfos>()),
      priorityOffset(pPriorityOffset)
 {
 }

  WordToSynthesize(const SemanticWord& pWord,
                   const InflectionToSynthesize& pOutBloc,
                   const std::map<std::string, char>& pConcepts,
                   WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY)
    : word(pWord),
      inflections(1, pOutBloc),
      concepts(pConcepts),
      tag(pTag),
      contextualInfos(),
      priorityOffset(0)
  {
  }

  bool operator==(const WordToSynthesize& pOther) const
  {
    return word == pOther.word &&
        inflections == pOther.inflections;
  }
  bool operator!=(const WordToSynthesize& pOther) const
  {
    return !operator==(pOther);
  }

  SemanticWord word;
  std::list<InflectionToSynthesize> inflections;
  std::map<std::string, char> concepts;
  WordToSynthesizeTag tag;
  std::set<WordContextualInfos> contextualInfos;
  int priorityOffset;
};


} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_WORDTOSYNTHESIZE_HPP
