#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include <onsem/common/utility/radix_map.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>


namespace onsem
{
namespace linguistics
{
std::mutex LinguisticDictionary::_pathToStatDbsMutex{};
std::map<SemanticLanguageEnum, std::unique_ptr<StaticLinguisticDictionary>> LinguisticDictionary::_pathToStatDbs{};


StaticLinguisticDictionary& LinguisticDictionary::_getStatDbInstance(
    std::istream& pDictIStream,
    const StaticConceptSet& pStaticConceptSet,
    SemanticLanguageEnum pLangEnum)
{
  std::lock_guard<std::mutex> lock(_pathToStatDbsMutex);
  const std::string languageStr = semanticLanguageEnum_toLanguageFilenameStr(pLangEnum);
  auto itElt = _pathToStatDbs.find(pLangEnum);
  if (itElt == _pathToStatDbs.end())
  {
    bool inserted = false;
    std::tie(itElt, inserted) = _pathToStatDbs.emplace
        (pLangEnum,
         std::make_unique<StaticLinguisticDictionary>(pDictIStream, pStaticConceptSet, pLangEnum));
  }
  return *itElt->second;
}


LinguisticDictionary::StaticWord::StaticWord
(const StaticLinguisticDictionary& pStatBinDico)
  : word(),
    meaning(),
    _statBinDico(pStatBinDico)
{
}

void LinguisticDictionary::StaticWord::setContent
(const std::string& pLemma,
 PartOfSpeech pPartOfSpeech)
{
  word.setContent(_statBinDico.getLanguageType(), pLemma, pPartOfSpeech);
  meaning = _statBinDico.getLingMeaning(pLemma, pPartOfSpeech, false);
}

void LinguisticDictionary::StaticWord::clear()
{
  word.clear();
  meaning.clear();
}



LinguisticDictionary::InflectedInfos::InflectedInfos
(const SemanticWord& pWord,
 WordAssociatedInfos& pInfos,
 const Inflections* pInflections)
  : wordAndInfos(std::make_unique<WordAndInfos>(&pWord, &pInfos)),
    inflections(pInflections)
{
}

LinguisticDictionary::InflectedInfos::InflectedInfos
(const WordAndInfos& pWordAndInfos,
 const Inflections* pInflections)
  : wordAndInfos(std::make_unique<WordAndInfos>(&pWordAndInfos.word(), &pWordAndInfos.infos())),
    inflections(pInflections)
{
}

LinguisticDictionary::InflectedInfos::InflectedInfos(InflectedInfos&& pOther)
  : wordAndInfos(std::move(pOther.wordAndInfos)),
    inflections(std::move(pOther.inflections))
{
}

LinguisticDictionary::InflectedInfos& LinguisticDictionary::InflectedInfos::operator=
(InflectedInfos&& pOther)
{
  wordAndInfos = std::move(pOther.wordAndInfos);
  inflections = std::move(pOther.inflections);
  return *this;
}

LinguisticDictionary::InflectedInfos::InflectedInfos(const InflectedInfos& pOther)
  : wordAndInfos(std::make_unique<WordAndInfos>
                 (&pOther.wordAndInfos->word(), &pOther.wordAndInfos->infos())),
    inflections(pOther.inflections)
{
}

LinguisticDictionary::InflectedInfos& LinguisticDictionary::InflectedInfos::operator=
(const InflectedInfos& pOther)
{
  assert(pOther.wordAndInfos);
  wordAndInfos = std::make_unique<WordAndInfos>
      (&pOther.wordAndInfos->word(), &pOther.wordAndInfos->infos());
  inflections = pOther.inflections;
  return *this;
}

void LinguisticDictionary::InflectedInfos::fillIGram(InflectedWord& pIGram) const
{
  pIGram.word = wordAndInfos->word();
  pIGram.infos.mergeWith(wordAndInfos->infos());
  if (inflections)
    pIGram.moveInflections(inflections->clone());
}


LinguisticDictionary::LinguisticDictionary(std::istream& pDictIStream,
                                           const StaticConceptSet& pStaticConceptSet,
                                           SemanticLanguageEnum pLangEnum)
  : statDb(_getStatDbInstance(pDictIStream, pStaticConceptSet, pLangEnum)),
    _language(pLangEnum),
    _wordToAssocInfos(),
    _lemmaToPosOfWordToRemoveFromStaticDico(std::make_unique<mystd::radix_map_str<std::list<PartOfSpeech>>>()),
    _inflectedCharaters(std::make_unique<mystd::radix_map_str<std::list<InflectedInfos>>>()),
    _beAux(statDb),
    _haveAux(statDb),
    _beVerb(statDb),
    _sayVerb(statDb)
{
  if (pLangEnum == SemanticLanguageEnum::FRENCH)
  {
    _beAux.setContent("être", PartOfSpeech::AUX);
    _haveAux.setContent("avoir", PartOfSpeech::AUX);
    _beVerb.setContent("être", PartOfSpeech::VERB);
    _sayVerb.setContent("dire", PartOfSpeech::VERB);
  }
  else if (pLangEnum == SemanticLanguageEnum::ENGLISH)
  {
    _beAux.setContent("be", PartOfSpeech::AUX);
    _haveAux.setContent("have", PartOfSpeech::AUX);
    _beVerb.setContent("be", PartOfSpeech::VERB);
    _sayVerb.setContent("say", PartOfSpeech::VERB);
  }
}

LinguisticDictionary::LinguisticDictionary(const LinguisticDictionary& pOther)
  : statDb(pOther.statDb),
    _wordToAssocInfos(pOther._wordToAssocInfos),
    _lemmaToPosOfWordToRemoveFromStaticDico(std::make_unique<mystd::radix_map_str<std::list<PartOfSpeech>>>(*pOther._lemmaToPosOfWordToRemoveFromStaticDico)),
    _inflectedCharaters(std::make_unique<mystd::radix_map_str<std::list<InflectedInfos>>>(*pOther._inflectedCharaters)),
    _beAux(pOther._beAux),
    _haveAux(pOther._haveAux),
    _beVerb(pOther._beVerb),
    _sayVerb(pOther._sayVerb)
{
}

LinguisticDictionary::~LinguisticDictionary()
{
}


LinguisticDictionary& LinguisticDictionary::operator=(const LinguisticDictionary& pOther)
{
  _wordToAssocInfos = pOther._wordToAssocInfos;
  _lemmaToPosOfWordToRemoveFromStaticDico = std::make_unique<mystd::radix_map_str<std::list<PartOfSpeech>>>(*pOther._lemmaToPosOfWordToRemoveFromStaticDico);
  _inflectedCharaters = std::make_unique<mystd::radix_map_str<std::list<InflectedInfos>>>(*pOther._inflectedCharaters);
  return *this;
}


void LinguisticDictionary::addInflectedWord
(const std::string& pInflectedFrom,
 const SemanticWord& pWord,
 WordAssociatedInfos& pInfos,
 const Inflections* pInflections)
{
  (*_inflectedCharaters)[pInflectedFrom].emplace_back(pWord, pInfos, pInflections);
}


void LinguisticDictionary::addInfosToAWord(const SemanticWord& pWord,
                                           const WordAssociatedInfos* pWordAssociatedInfos)
{
  _wordToAssocInfos.emplace(pWord, pWordAssociatedInfos);
}

void LinguisticDictionary::removeAWord(const SemanticWord& pWord)
{
  _wordToAssocInfos.erase(pWord);
  auto statMeaning = statDb.getLingMeaning(pWord.lemma, pWord.partOfSpeech, true);
  if (!statMeaning.isEmpty())
    (*_lemmaToPosOfWordToRemoveFromStaticDico)[pWord.lemma].emplace_back(pWord.partOfSpeech);
}


void LinguisticDictionary::getInfoGram(InflectedWord& pIGram,
                                       const LinguisticMeaning& pMeaning) const
{
  switch (pMeaning.getLinguisticMeaningType())
  {
  case LinguisticMeaningType::ID:
  {
    const auto& statLingMeaning = pMeaning.getStaticMeaning();
    InflectedWord inflWord;
    statDb.getInfoGram(inflWord, statLingMeaning);
    if (!_isARemovedWord(inflWord.word))
      pIGram = std::move(inflWord);
    break;
  }
  case LinguisticMeaningType::WORD:
  {
    const auto& word = pMeaning.getWord();
    if (_isARemovedWord(word))
      break;

    {
      auto statLingMeaning = statDb.getLingMeaning(word.lemma,
                                                   word.partOfSpeech, true);
      if (!statLingMeaning.isEmpty())
        statDb.getInfoGram(pIGram, statLingMeaning);
    }
    pIGram.word = word;

    auto itWordToInfosGram = _wordToAssocInfos.find(word);
    if (itWordToInfosGram != _wordToAssocInfos.end())
      pIGram.infos.mergeWith(*itWordToInfosGram->second);

    const std::list<InflectedInfos>* dynInfosGram =
        _inflectedCharaters->find_ptr(word.lemma, 0, word.lemma.size());
    if (dynInfosGram != nullptr)
    {
      // merge with static infosGram
      for (const auto& currInfleForms : *dynInfosGram)
      {
        if (word == currInfleForms.wordAndInfos->word())
        {
          pIGram.word = word;
          pIGram.infos.mergeWith(currInfleForms.wordAndInfos->infos());
          if (currInfleForms.inflections)
            pIGram.moveInflections(currInfleForms.inflections->clone());
          return;
        }
      }
    }
    break;
  }
  case LinguisticMeaningType::EMPTY:
    break;
  }
}



void LinguisticDictionary::getConcepts(std::map<std::string, char>& pConcepts,
                                       const SemanticWord& pWord) const
{
  if (_isARemovedWord(pWord))
    return;

  {
    auto statLingMeaning = statDb.getLingMeaning(pWord.lemma,
                                                 pWord.partOfSpeech, true);
    if (!statLingMeaning.isEmpty())
      statDb.getConcepts(pConcepts, statLingMeaning);
  }

  auto itWordToInfosGram = _wordToAssocInfos.find(pWord);
  if (itWordToInfosGram != _wordToAssocInfos.end())
    for (const auto& currCpt : itWordToInfosGram->second->concepts)
      pConcepts[currCpt.first] = currCpt.second;

  const std::list<InflectedInfos>* dynInfosGram =
      _inflectedCharaters->find_ptr(pWord.lemma, 0, pWord.lemma.size());
  if (dynInfosGram != nullptr)
  {
    // merge with static infosGram
    for (const auto& currInfleForms : *dynInfosGram)
    {
      if (pWord == currInfleForms.wordAndInfos->word())
      {
        for (const auto& currCpt : currInfleForms.wordAndInfos->infos().concepts)
          pConcepts[currCpt.first] = currCpt.second;
        return;
      }
    }
  }
}


void LinguisticDictionary::reset()
{
  _wordToAssocInfos.clear();
  _lemmaToPosOfWordToRemoveFromStaticDico->clear();
  _inflectedCharaters->clear();
}


bool LinguisticDictionary::hasContextualInfo(WordContextualInfos pContextualInfo,
                                             const SemanticWord& pWord) const
{
  if (_isARemovedWord(pWord))
    return false;

  StaticLinguisticMeaning statLingMeaning = statDb.getLingMeaning(pWord.lemma,
                                                                  pWord.partOfSpeech, true);
  if (!statLingMeaning.isEmpty() &&
      statDb.hasContextualInfo(pContextualInfo, statLingMeaning))
    return true;

  auto itWordToInfosGram = _wordToAssocInfos.find(pWord);
  if (itWordToInfosGram != _wordToAssocInfos.end())
    return itWordToInfosGram->second->contextualInfos.count(pContextualInfo) > 0;
  return false;
}


SemanticRequestType LinguisticDictionary::aloneWordToRequest(const SemanticWord& pWord) const
{
  return statDb.aloneWordToRequest(pWord);
}

SemanticRequestType LinguisticDictionary::semWordToRequest(const SemanticWord& pWord) const
{
  return statDb.semWordToRequest(pWord);
}


bool LinguisticDictionary::hasContextualInfo(WordContextualInfos pContextualInfo,
                                             const LinguisticMeaning& pMeaning) const
{
  switch (pMeaning.getLinguisticMeaningType())
  {
  case LinguisticMeaningType::ID:
    return statDb.hasContextualInfo(pContextualInfo, pMeaning.getStaticMeaning());
  case LinguisticMeaningType::WORD:
  {
    const auto& word = pMeaning.getWord();
    return hasContextualInfo(pContextualInfo, word);
  }
  case LinguisticMeaningType::EMPTY:
    return false;
  }
  return false;
}


std::size_t LinguisticDictionary::getLengthOfLongestWord
(const std::string& pStr, std::size_t pBeginStr) const
{
  std::size_t statMaxLength = statDb.getLengthOfLongestWord(pStr, pBeginStr);
  std::size_t dynMaxLength = _inflectedCharaters->getMaxLength(pStr, pBeginStr);
  return std::max(statMaxLength, dynMaxLength);
}


void LinguisticDictionary::getGramPossibilitiesAndPutUnknownIfNothingFound
(std::list<InflectedWord>& pInfosGram,
 const std::string& pWord,
 std::size_t pBeginPos,
 std::size_t pSizeOfWord) const
{
  getGramPossibilities(pInfosGram, pWord, pBeginPos, pSizeOfWord);
  if (pInfosGram.empty())
  {
    pInfosGram.emplace_back();
    auto& newInflWord = pInfosGram.back();
    newInflWord.word.lemma = pWord.substr(pBeginPos, pSizeOfWord);
    if (newInflWord.word.lemma.size() > 1 &&
        newInflWord.word.lemma[0] == '@')
    {
      newInflWord.word.partOfSpeech = PartOfSpeech::PROPER_NOUN;
      std::string userId = newInflWord.word.lemma.substr(1, newInflWord.word.lemma.size() - 1);
      mystd::replace_all(userId, "_", "-");
      newInflWord.infos.concepts.emplace("agent_userId_" + userId, 4);
    }
  }
}


void LinguisticDictionary::getGramPossibilities
(std::list<InflectedWord>& pInfWords,
 const std::string& pWord,
 std::size_t pBeginPos,
 std::size_t pSizeOfWord) const
{
  statDb.getGramPossibilities(pInfWords, pWord, pBeginPos, pSizeOfWord);

  for (auto it = pInfWords.begin(); it != pInfWords.end(); )
  {
    if (_isARemovedWord(it->word))
    {
      it = pInfWords.erase(it);
      continue;
    }
    auto itWordToInfosGram = _wordToAssocInfos.find(it->word);
    if (itWordToInfosGram != _wordToAssocInfos.end())
      it->infos.mergeWith(*itWordToInfosGram->second);
    ++it;
  }

  const std::list<InflectedInfos>* dynInfosGram =
      _inflectedCharaters->find_ptr(pWord, pBeginPos, pSizeOfWord);
  if (dynInfosGram != nullptr)
  {
    // merge with static infosGram
    for (const auto& currInfleForms : *dynInfosGram)
    {
      const auto& currWord = currInfleForms.wordAndInfos->word();
      bool iGramIsMerged = false;
      for (InflectedWord& currIGram : pInfWords)
      {
        if (currIGram.word == currWord)
        {
          currIGram.infos.mergeWith(currInfleForms.wordAndInfos->infos());
          if (currInfleForms.inflections)
            currIGram.moveInflections(currInfleForms.inflections->clone());
          iGramIsMerged = true;
          break;
        }
      }
      if (!iGramIsMerged)
      {
        pInfWords.emplace_front();
        auto& newInflWord = pInfWords.front();
        auto statLingDb = statDb.getLingMeaning(currWord.lemma, currWord.partOfSpeech, true);
        if (!statLingDb.isEmpty())
          statDb.getInfoGram(newInflWord, statLingDb);
        currInfleForms.fillIGram(newInflWord);
        if (beginWithUppercase(pWord, pBeginPos))
          newInflWord.infos.contextualInfos.insert(WordContextualInfos::BEGINSWITHUPPERCHARACTER);
      }
    }
  }
}

bool LinguisticDictionary::_isARemovedWord(const SemanticWord& pWord) const
{
  auto* listOfPosToRemove = _lemmaToPosOfWordToRemoveFromStaticDico->find_ptr(pWord.lemma, 0, pWord.lemma.size());
  if (listOfPosToRemove != nullptr)
    for (auto currPos : *listOfPosToRemove)
      if (currPos == pWord.partOfSpeech)
        return true;
  return false;
}

} // End of namespace linguistics
} // End of namespace onsem
