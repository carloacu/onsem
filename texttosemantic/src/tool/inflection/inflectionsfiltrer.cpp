#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/inflections.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>

namespace onsem
{
namespace linguistics
{

template<typename INFL1, typename INFL2>
bool _areInflectionsCompatibles(const INFL1& pInfls1,
                                const INFL2& pInfls2)
{
  bool res = true;
  for (auto it1 = pInfls1.inflections.begin(); it1 != pInfls1.inflections.end(); )
  {
    bool it1IsOk = false;
    for (const auto& currInfl2 : pInfls2.inflections)
    {
      if (gendersAreWeaklyEqual(it1->gender, currInfl2.gender) &&
          numbersAreWeaklyEqual(it1->number, currInfl2.number))
      {
        it1IsOk = true;
        break;
      }
    }
    if (it1IsOk)
      return true;
    res = false;
    ++it1;
  }
  return res;
}


template<typename INFL1, typename INFL2>
bool _isInflectionsFilterWithOneConst(INFL1& pInfls1,
                                      const INFL2& pInfls2)
{
  bool finalResult = false;
  for (auto it1 = pInfls1.inflections.begin(); it1 != pInfls1.inflections.end(); )
  {
    bool it1IsOk = false;
    for (const auto& currInfl2 : pInfls2.inflections)
    {
      if (gendersAreWeaklyEqual(it1->gender, currInfl2.gender) &&
          numbersAreWeaklyEqual(it1->number, currInfl2.number))
      {
        it1IsOk = true;
        break;
      }
    }
    if (!it1IsOk)
    {
      it1 = pInfls1.inflections.erase(it1);
      finalResult = true;
    }
    else
    {
      ++it1;
    }
  }
  return finalResult;
}

template<typename INFL1, typename INFL2>
bool _isInflectionsFilterWithSpecTypes(INFL1& pInfls1,
                                       INFL2& pInfls2)
{
  if (pInfls1.inflections.empty() || pInfls2.inflections.empty())
    return false;
  bool res = _isInflectionsFilterWithOneConst(pInfls1, pInfls2);
  res = _isInflectionsFilterWithOneConst(pInfls2, pInfls1) || res;
  return res;
}


bool _inflectionsFilter(Inflections& pInflections1,
                        Inflections& pInflections2)
{
  if (pInflections1.type == InflectionType::NOMINAL &&
      pInflections2.type == InflectionType::NOMINAL)
  {
    return _isInflectionsFilterWithSpecTypes(pInflections1.getNominalI(), pInflections2.getNominalI());
  }
  if (pInflections1.type == InflectionType::NOMINAL &&
      pInflections2.type == InflectionType::ADJECTIVAL)
  {
    return _isInflectionsFilterWithSpecTypes(pInflections1.getNominalI(), pInflections2.getAdjectivalI());
  }
  if (pInflections1.type == InflectionType::ADJECTIVAL &&
      pInflections2.type == InflectionType::NOMINAL)
  {
    return _isInflectionsFilterWithSpecTypes(pInflections1.getAdjectivalI(), pInflections2.getNominalI());
  }
  if (pInflections1.type == InflectionType::ADJECTIVAL &&
      pInflections2.type == InflectionType::ADJECTIVAL)
  {
    return _isInflectionsFilterWithSpecTypes(pInflections1.getAdjectivalI(), pInflections2.getAdjectivalI());
  }
  return false;
}


bool InflectionsChecker::areInflectionsCompatibles(const Inflections& pInfls1,
                                                   const Inflections& pInfls2)
{
  switch (pInfls1.type)
  {
  case InflectionType::NOMINAL:
  {
    if (pInfls2.type == InflectionType::NOMINAL)
      return _areInflectionsCompatibles(pInfls1.getNominalI(), pInfls2.getNominalI());
    if (pInfls2.type == InflectionType::ADJECTIVAL)
      return _areInflectionsCompatibles(pInfls1.getNominalI(), pInfls2.getAdjectivalI());
    if (pInfls2.type == InflectionType::PRONOMINAL)
      return _areInflectionsCompatibles(pInfls1.getNominalI(), pInfls2.getPronominalI());
    break;
  }
  case InflectionType::ADJECTIVAL:
  {
    if (pInfls2.type == InflectionType::NOMINAL)
      return _areInflectionsCompatibles(pInfls1.getAdjectivalI(), pInfls2.getNominalI());
    if (pInfls2.type == InflectionType::ADJECTIVAL)
      return _areInflectionsCompatibles(pInfls1.getAdjectivalI(), pInfls2.getAdjectivalI());
    if (pInfls2.type == InflectionType::PRONOMINAL)
      return _areInflectionsCompatibles(pInfls1.getAdjectivalI(), pInfls2.getPronominalI());
    break;
  }
  case InflectionType::PRONOMINAL:
  {
    if (pInfls2.type == InflectionType::NOMINAL)
      return _areInflectionsCompatibles(pInfls1.getPronominalI(), pInfls2.getNominalI());
    if (pInfls2.type == InflectionType::ADJECTIVAL)
      return _areInflectionsCompatibles(pInfls1.getPronominalI(), pInfls2.getAdjectivalI());
    if (pInfls2.type == InflectionType::PRONOMINAL)
      return _areInflectionsCompatibles(pInfls1.getPronominalI(), pInfls2.getPronominalI());
    break;
  }
  case InflectionType::ADVERBIAL:
  case InflectionType::VERBAL:
  case InflectionType::EMPTY:
    break;
  }
  return true;
}


bool InflectionsChecker::isAdjCompatibleWithNumberType(const Inflections& pAdjInfls,
                                                       SemanticNumberType pNumberType)
{
  if (pAdjInfls.type != InflectionType::ADJECTIVAL)
    return true;
  const AdjectivalInflections& adjInfls = pAdjInfls.getAdjectivalI();
  for (const auto& currAdjInfl : adjInfls.inflections)
    if (numbersAreWeaklyEqual(currAdjInfl.number, pNumberType))
      return true;
  return false;
}


bool InflectionsChecker::filterIncompatibleInflections(const Token* pPrevPrevToken,
                                                       Token& pToken1,
                                                       Token& pToken2) const
{
  InflectedWord& inflWord1 = pToken1.inflWords.front();
  InflectedWord& inflWord2 = pToken2.inflWords.front();
  if (ConceptSet::haveAConceptThatBeginWith(inflWord1.infos.concepts, "number_") &&
      !ConceptSet::haveAConcept(inflWord1.infos.concepts, "number_1"))
    return false;

  switch (inflWord1.word.partOfSpeech)
  {
  case PartOfSpeech::DETERMINER:
  case PartOfSpeech::PARTITIVE:
  {
    switch (inflWord2.word.partOfSpeech)
    {
    case PartOfSpeech::DETERMINER:
    case PartOfSpeech::ADJECTIVE:
    case PartOfSpeech::NOUN:
    case PartOfSpeech::UNKNOWN:
    {
      return _inflectionsFilter(inflWord1.inflections(), inflWord2.inflections());
    }
    default:
    {
      return false;
    }
    }
  }
  case PartOfSpeech::ADJECTIVE:
  case PartOfSpeech::NOUN:
  {
    switch (inflWord2.word.partOfSpeech)
    {
    case PartOfSpeech::ADJECTIVE:
    case PartOfSpeech::NOUN:
    case PartOfSpeech::UNKNOWN:
    {
      if (inflWord1.word.partOfSpeech == PartOfSpeech::NOUN &&
          inflWord2.word.partOfSpeech == PartOfSpeech::NOUN)
        return false;
      return _inflectionsFilter(inflWord1.inflections(), inflWord2.inflections());
    }
    default:
    {
      return false;
    }
    }
  }

  case PartOfSpeech::PRONOUN_COMPLEMENT:
  {
    if (inflWord2.word.partOfSpeech != PartOfSpeech::VERB ||
        fBinDico.getLanguage() != SemanticLanguageEnum::FRENCH)
      return false;

    if (pPrevPrevToken != nullptr &&
        pPrevPrevToken->inflWords.front().word.lemma == "ne")
      return false;

    bool res = false;
    // remove imperative inflections
    Inflections& infls2 = inflWord2.inflections();
    if (infls2.type == InflectionType::VERBAL)
    {
      VerbalInflections& verbInlfs = infls2.getVerbalI();
      for (auto itInfl = verbInlfs.inflections.begin();
           itInfl != verbInlfs.inflections.end(); )
      {
        if (itInfl->tense == LinguisticVerbTense::PRESENT_IMPERATIVE)
        {
          itInfl = verbInlfs.inflections.erase(itInfl);
          res = true;
        }
        else
          ++itInfl;
      }
    }
    return res;
  }

  case PartOfSpeech::VERB:
  {
    if (inflWord2.word.partOfSpeech == PartOfSpeech::VERB &&
        inflWord1.word.language == SemanticLanguageEnum::ENGLISH &&
        (pToken1.str == "wanna" || pToken1.str == "Wanna"))
      inflWord2.moveInflections(std::make_unique<VerbalInflections>(std::vector<std::string>{"W"}));
    return false; // false because it's not sure that we decrease the number of inflections, if no it can cause an infinite loop
  }

  default:
    return false;
  };
}


} // End of namespace linguistics
} // End of namespace onsem
