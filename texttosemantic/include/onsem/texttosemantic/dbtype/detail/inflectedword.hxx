#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGINFOSGRAM_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGINFOSGRAM_HXX

#include "../inflectedword.hpp"

namespace onsem
{
namespace linguistics
{

inline LingWordsGroup::LingWordsGroup(const LingWordsGroup& pOther)
  : rootWord(std::make_unique<SemanticWord>()),
    linkedMeanings()
{
  _set(pOther);
}

inline LingWordsGroup& LingWordsGroup::operator=(const LingWordsGroup& pOther)
{
  _set(pOther);
  return *this;
}

inline LingWordsGroup::LingWordsGroup(LingWordsGroup&& pOther)
  : rootWord(std::move(pOther.rootWord)),
    linkedMeanings(std::move(pOther.linkedMeanings))
{
}

inline LingWordsGroup& LingWordsGroup::operator=(LingWordsGroup&& pOther)
{
  rootWord = std::move(pOther.rootWord);
  linkedMeanings = std::move(pOther.linkedMeanings);
  return *this;
}

inline void LingWordsGroup::_set(const LingWordsGroup& pOther)
{
  rootWord = std::make_unique<SemanticWord>(*pOther.rootWord);

  for (const auto& currLinkedMeanings : pOther.linkedMeanings)
  {
    linkedMeanings.emplace_back();
    auto& lastLinkedMeanings = linkedMeanings.back();
    lastLinkedMeanings.first = std::make_unique<SemanticWord>(*currLinkedMeanings.first);
    lastLinkedMeanings.second = currLinkedMeanings.second;
  }
}


inline void WordAssociatedInfos::mergeWith(const WordAssociatedInfos& pOther)
{
  for (const auto& currCpt : pOther.concepts)
    concepts[currCpt.first] = currCpt.second;
  for (const auto& currCI : pOther.contextualInfos)
    contextualInfos.insert(currCI);
  if (pOther.linkedMeanings)
    linkedMeanings = pOther.linkedMeanings;
  for (const auto& currMetaMeaning : pOther.metaMeanings)
    metaMeanings.push_back(currMetaMeaning);
}


inline void WordAssociatedInfos::clear()
{
  concepts.clear();
  contextualInfos.clear();
  linkedMeanings.reset();
  metaMeanings.clear();
}

inline bool WordAssociatedInfos::hasContextualInfo(WordContextualInfos pContextualInfo) const
{
  return contextualInfos.find(pContextualInfo) != contextualInfos.end();
}


inline InflectedWord::InflectedWord()
  : word(),
    infos(),
    _inflections(std::make_unique<EmptyInflections>())
{}

inline InflectedWord::InflectedWord
(PartOfSpeech pPartOfSpeech,
 std::unique_ptr<Inflections> pInflections)
  : word(),
    infos(),
    _inflections(std::move(pInflections))
{
  assert(_inflections);
  word.partOfSpeech = pPartOfSpeech;
}

inline InflectedWord::InflectedWord(InflectedWord&& pOther)
  : word(std::move(pOther.word)),
    infos(std::move(pOther.infos)),
    _inflections(std::move(pOther._inflections))
{
  assert(_inflections);
}

inline InflectedWord& InflectedWord::operator=
(InflectedWord&& pOther)
{
  word = std::move(pOther.word);
  infos = std::move(pOther.infos);
  _inflections = std::move(pOther._inflections);
  assert(_inflections);
  return *this;
}

inline InflectedWord::InflectedWord(const InflectedWord& pOther)
  : word(pOther.word),
    infos(pOther.infos),
    _inflections(pOther._inflections->clone())
{
  assert(_inflections);
}

inline InflectedWord& InflectedWord::operator=
(const InflectedWord& pOther)
{
  word = pOther.word;
  infos = pOther.infos;
  _inflections = pOther._inflections->clone();
  assert(_inflections);
  return *this;
}

inline void InflectedWord::clear(bool pExceptFlexions)
{
  word.clear();
  infos.clear();
  if (!pExceptFlexions)
    _inflections = std::make_unique<EmptyInflections>();
}

inline void InflectedWord::moveInflections(std::unique_ptr<Inflections> pInflections)
{
  _inflections = std::move(pInflections);
  assert(_inflections);
}

inline InflectedWord InflectedWord::getPuntuationIGram()
{
  InflectedWord res;
  res.word.partOfSpeech = PartOfSpeech::PUNCTUATION;
  return res;
}


inline bool InflectedWord::isSameInflectedFormThan
(const InflectedWord& pOther) const
{
  return word == pOther.word &&
      inflections() == pOther.inflections();
}

} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGINFOSGRAM_HXX
