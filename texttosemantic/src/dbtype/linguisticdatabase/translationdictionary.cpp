#include <onsem/texttosemantic/dbtype/linguisticdatabase/translationdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/statictranslationdictionary.hpp>


namespace onsem
{
namespace linguistics
{
std::mutex TranslationDictionary::_pathToStatDbsMutex;
std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, std::unique_ptr<StaticTranslationDictionary>>> TranslationDictionary::_pathToStatDbs{};


std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, const StaticTranslationDictionary*>> TranslationDictionary::_getStatDbInstance(LinguisticDatabaseStreams& pIStreams)
{
  std::lock_guard<std::mutex> lock(_pathToStatDbsMutex);

  if (_pathToStatDbs.empty())
  {
    for (const auto& currInLangEnum : semanticLanguageEnum_allValues)
    {
      if (currInLangEnum != SemanticLanguageEnum::UNKNOWN)
      {
        for (const auto& currOutLangEnum : semanticLanguageEnum_allValues)
        {
          if (currOutLangEnum != SemanticLanguageEnum::UNKNOWN &&
              currInLangEnum != currOutLangEnum)
            _pathToStatDbs[currInLangEnum].emplace
                (currOutLangEnum, std::make_unique<StaticTranslationDictionary>(pIStreams, currInLangEnum, currOutLangEnum));
        }
      }
    }
  }

  std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, const StaticTranslationDictionary*>> res;
  for (const auto& currInLang : _pathToStatDbs)
    for (const auto& currOutLang : currInLang.second)
      res[currInLang.first].emplace(currOutLang.first, &*currOutLang.second);
  return res;
}



TranslationDictionary::TranslationDictionary(LinguisticDatabaseStreams& pIStreams)
  : _binTranslations(_getStatDbInstance(pIStreams)),
    _translations()
{
}



void TranslationDictionary::addTranslation(const SemanticWord& pFromWord,
                                           const SemanticWord& pToWord)
{
  _translations[pFromWord.language][pToWord.language].emplace(pFromWord, pToWord);
}


void TranslationDictionary::getTranslation(int32_t& pTranslatedMeaningId,
                                           const SemanticWord& pWord,
                                           SemanticLanguageEnum pOutLanguage,
                                           const StaticLinguisticDictionary& pStatLingDico) const
{
  assert(pStatLingDico.getLanguageType() == pWord.language);
  StaticLinguisticMeaning lingMeaning = pStatLingDico.getLingMeaning(pWord.lemma,
                                                               pWord.partOfSpeech, true);
  if (pWord.language == pOutLanguage)
  {
    pTranslatedMeaningId = lingMeaning.meaningId;
    return;
  }
  if (lingMeaning.isEmpty() ||
      pWord.language == SemanticLanguageEnum::UNKNOWN ||
      pOutLanguage == SemanticLanguageEnum::UNKNOWN)
  {
    pTranslatedMeaningId = LinguisticMeaning_noMeaningId;
    return;
  }

  const StaticTranslationDictionary* staticTranslationDictionaryPtr =
      getStaticTranslationDictionaryPtr(pWord.language, pOutLanguage);
  if (staticTranslationDictionaryPtr != nullptr)
    pTranslatedMeaningId = staticTranslationDictionaryPtr->getTranslation(pWord.lemma, lingMeaning);
}


bool TranslationDictionary::getTranslation(LinguisticMeaning& pTranslatedMeaningId,
                                           const SemanticWord& pWord,
                                           SemanticLanguageEnum pOutLanguage,
                                           const LinguisticDictionary& pLingDico) const
{
  assert(pLingDico.statDb.getLanguageType() == pWord.language);
  if (pWord.lemma.empty())
    return false;

  if (pWord.language == pOutLanguage)
  {
    pTranslatedMeaningId.emplace_word(pWord);
    return true;
  }

  {
    auto itInLang = _translations.find(pWord.language);
    if (itInLang != _translations.end())
    {
      auto itOutLang = itInLang->second.find(pOutLanguage);
      if (itOutLang != itInLang->second.end())
      {
        auto itTransWord = itOutLang->second.find(pWord);
        if (itTransWord != itOutLang->second.end())
        {
          pTranslatedMeaningId.emplace_word(itTransWord->second);
          return true;
        }
      }
    }
  }

  StaticLinguisticMeaning lingMeaning = pLingDico.statDb.getLingMeaning(pWord.lemma,
                                                                        pWord.partOfSpeech, true);
  if (lingMeaning.isEmpty() ||
      pWord.language == SemanticLanguageEnum::UNKNOWN ||
      pOutLanguage == SemanticLanguageEnum::UNKNOWN)
  {
    return false;
  }

  const StaticTranslationDictionary* staticTranslationDictionaryPtr =
      getStaticTranslationDictionaryPtr(pWord.language, pOutLanguage);
  if (staticTranslationDictionaryPtr != nullptr)
  {
    int transId = staticTranslationDictionaryPtr->getTranslation(pWord.lemma, lingMeaning);
    if (transId != LinguisticMeaning_noMeaningId)
    {
      pTranslatedMeaningId.emplace_id(pOutLanguage, transId);
      return true;
    }
  }
  return false;
}


const StaticTranslationDictionary* TranslationDictionary::getStaticTranslationDictionaryPtr(
    SemanticLanguageEnum pInLanguage,
    SemanticLanguageEnum pOutLanguage) const
{
  auto itBinTrans1 = _binTranslations.find(pInLanguage);
  if (itBinTrans1 != _binTranslations.end())
  {
    auto itBinTrans2 = itBinTrans1->second.find(pOutLanguage);
    if (itBinTrans2 != itBinTrans1->second.end())
      return itBinTrans2->second;
  }
  return nullptr;
}



} // End of namespace linguistics
} // End of namespace onsem
