#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTIC_WORDTOSYNTHESIZE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTIC_WORDTOSYNTHESIZE_HPP

#include <string>
#include <list>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include "../../api.hpp"

namespace onsem
{
struct WordToSynthesize;


struct ONSEM_TEXTTOSEMANTIC_API InflectionToSynthesize
{
  InflectionToSynthesize
  (const std::string& pStr,
   bool pIfCanHaveSpaceBefore = true,
   bool pIfCanHaveSpaceAfter = true,
   bool (*pConditions)(const WordToSynthesize&) = [](const WordToSynthesize&){ return true; })
    : str(pStr),
      ifCanHaveSpaceBefore(pIfCanHaveSpaceBefore),
      ifCanHaveSpaceAfter(pIfCanHaveSpaceAfter),
      canHavePunctionAfter(true),
      conditions(pConditions),
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
                   WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY)
    : word(pWord),
      inflections(1, pOutBloc),
      concepts(),
      tag(pTag)
  {
  }

  WordToSynthesize(const SemanticWord& pWord,
                   const InflectionToSynthesize& pOutBloc,
                   const std::map<std::string, char>& pConcepts,
                   WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY)
    : word(pWord),
      inflections(1, pOutBloc),
      concepts(pConcepts),
      tag(pTag)
  {
  }

  void keepOnlyLastInflection()
  {
    while (inflections.size() > 1)
      inflections.pop_front();
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
};


} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTIC_WORDTOSYNTHESIZE_HPP
