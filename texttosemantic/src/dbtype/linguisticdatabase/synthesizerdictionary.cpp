#include <onsem/texttosemantic/dbtype/linguisticdatabase/synthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>

namespace onsem {
namespace linguistics {
std::mutex SynthesizerDictionary::_pathToStatDbsMutex{};
std::map<SemanticLanguageEnum, std::unique_ptr<StaticSynthesizerDictionary>> SynthesizerDictionary::_statDbs{};

const StaticSynthesizerDictionary& SynthesizerDictionary::_getStatDbInstance(
    std::istream* pIStreamPtr,
    const StaticConceptSet& pConceptsDb,
    const StaticLinguisticDictionary& pStatLingDic,
    SemanticLanguageEnum pLangEnum) {
    std::lock_guard<std::mutex> lock(_pathToStatDbsMutex);
    auto it = _statDbs.find(pLangEnum);
    if (it == _statDbs.end()) {
        auto& res = _statDbs[pLangEnum];
        res = std::make_unique<StaticSynthesizerDictionary>(pIStreamPtr, pConceptsDb, pStatLingDic, pLangEnum);
        return *res;
    }
    return *it->second;
}

SynthesizerDictionary::SynthesizerDictionary(std::istream* pIStreamPtr,
                                             const StaticConceptSet& pConceptsDb,
                                             const StaticLinguisticDictionary& pStatLingDic,
                                             SemanticLanguageEnum pLangEnum)
    : statDb(_getStatDbInstance(pIStreamPtr, pConceptsDb, pStatLingDic, pLangEnum))
    , _lingDict(pStatLingDic)
    , _wordToInflections()
    , _conceptToInfoGrams() {}

void SynthesizerDictionary::addInflectedWord(const std::string& pInflectedFrom,
                                             const SemanticWord& pWord,
                                             const Inflections& pInflections,
                                             char pFrequency) {
    auto& inflectionElt = _wordToInflections[pWord];
    const NominalInflections* nominalInflectionsPtr = pInflections.getNominalIPtr();
    if (nominalInflectionsPtr != nullptr && !nominalInflectionsPtr->inflections.empty()) {
        for (const auto& currNominal : nominalInflectionsPtr->inflections)
            _addInflectionsIfMoreFrequent(inflectionElt, currNominal, InflFormAndFrequency(pInflectedFrom, pFrequency));
    } else {
        _addInflectionsIfMoreFrequent(
            inflectionElt, NominalInflection(), InflFormAndFrequency(pInflectedFrom, pFrequency));
    }
}

void SynthesizerDictionary::addInfosToAWord(const SemanticWord& pWord,
                                            const WordAssociatedInfos* pWordAssociatedInfos) {
    for (const auto& currCpt : pWordAssociatedInfos->concepts)
        _conceptToInfoGrams[currCpt.first].emplace(pWord, pWordAssociatedInfos);
}

void SynthesizerDictionary::reset() {
    _wordToInflections.clear();
    _conceptToInfoGrams.clear();
}

LinguisticMeaning SynthesizerDictionary::conceptToMeaning(const std::string& pConcept) const {
    LinguisticMeaning res;

    auto it = _conceptToInfoGrams.find(pConcept);
    if (it != _conceptToInfoGrams.end()) {
        if (!it->second.empty()) {
            res.emplace_word(it->second.begin()->first);
            return res;
        }
    }

    StaticLinguisticMeaning statMeaning = statDb.conceptToMeaning(pConcept);
    if (!statMeaning.isEmpty())
        res.emplace_id(statMeaning.language, statMeaning.meaningId);
    return res;
}

std::string SynthesizerDictionary::getLemma(const LinguisticMeaning& pMeaning, bool pWithLinkMeanings) const {
    switch (pMeaning.getLinguisticMeaningType()) {
        case LinguisticMeaningType::ID: {
            const auto& statLingMeaning = pMeaning.getStaticMeaning();
            return statDb.getLemma(statLingMeaning, pWithLinkMeanings);
        }
        case LinguisticMeaningType::WORD: {
            const auto& word = pMeaning.getWord();
            return word.lemma;
        }
        default: break;
    }
    return "";
}

void SynthesizerDictionary::getNounGendersFromWord(std::set<SemanticGenderType>& pGenders,
                                                   const SemanticWord& pWord,
                                                   SemanticNumberType pNumber) const {
    auto itWordToInfl = _wordToInflections.find(pWord);
    if (itWordToInfl != _wordToInflections.end())
        for (const auto& currInfl : itWordToInfl->second)
            if (currInfl.first.number == pNumber)
                pGenders.insert(currInfl.first.gender);

    StaticLinguisticMeaning statLingMeaning = _lingDict.getLingMeaning(pWord.lemma, pWord.partOfSpeech, true);
    if (!statLingMeaning.isEmpty())
        statDb.getNounGenders(pGenders, statLingMeaning, pNumber);
}

void SynthesizerDictionary::getNounGenders(std::set<SemanticGenderType>& pGenders,
                                           const LinguisticMeaning& pMeaning,
                                           SemanticNumberType pNumber) const {
    switch (pMeaning.getLinguisticMeaningType()) {
        case LinguisticMeaningType::ID: {
            const auto& statLingMeaning = pMeaning.getStaticMeaning();
            statDb.getNounGenders(pGenders, statLingMeaning, pNumber);
            break;
        }
        case LinguisticMeaningType::WORD: {
            const auto& word = pMeaning.getWord();
            getNounGendersFromWord(pGenders, word, pNumber);
            break;
        }
        default: break;
    }
}

void SynthesizerDictionary::getNounForm(std::string& pRes,
                                        const LinguisticMeaning& pLinguisticMeaning,
                                        SemanticGenderType& pGender,
                                        SemanticNumberType& pNumber) const {
    switch (pLinguisticMeaning.getLinguisticMeaningType()) {
        case LinguisticMeaningType::ID: {
            const auto& statLingMeaning = pLinguisticMeaning.getStaticMeaning();
            statDb.getNounForm(pRes, statLingMeaning, pGender, pNumber);
            break;
        }
        case LinguisticMeaningType::WORD: {
            const auto& word = pLinguisticMeaning.getWord();

            auto itWordToInfl = _wordToInflections.find(word);
            if (itWordToInfl != _wordToInflections.end()) {
                const auto& inflections = itWordToInfl->second;

                if (_tryGetInflectedFrom(pRes, pGender, pNumber, inflections))
                    return;
                if (pNumber == SemanticNumberType::PLURAL) {
                    if (pGender != SemanticGenderType::NEUTRAL
                        && _tryGetInflectedFrom(pRes, SemanticGenderType::NEUTRAL, pNumber, inflections)) {
                        pGender = SemanticGenderType::NEUTRAL;
                        return;
                    }
                    if (pGender != SemanticGenderType::MASCULINE
                        && _tryGetInflectedFrom(pRes, SemanticGenderType::MASCULINE, pNumber, inflections)) {
                        pGender = SemanticGenderType::MASCULINE;
                        return;
                    }
                    if (pGender != SemanticGenderType::FEMININE
                        && _tryGetInflectedFrom(pRes, SemanticGenderType::FEMININE, pNumber, inflections)) {
                        pGender = SemanticGenderType::FEMININE;
                        return;
                    }
                }

                if (_tryGetInflectedFrom(
                        pRes, SemanticGenderType::NEUTRAL, SemanticNumberType::SINGULAR, inflections)) {
                    pNumber = SemanticNumberType::SINGULAR;
                    pGender = SemanticGenderType::NEUTRAL;
                    return;
                }
                if (_tryGetInflectedFrom(
                        pRes, SemanticGenderType::MASCULINE, SemanticNumberType::SINGULAR, inflections)) {
                    pNumber = SemanticNumberType::SINGULAR;
                    pGender = SemanticGenderType::MASCULINE;
                    return;
                }
                if (_tryGetInflectedFrom(
                        pRes, SemanticGenderType::FEMININE, SemanticNumberType::SINGULAR, inflections)) {
                    pNumber = SemanticNumberType::SINGULAR;
                    pGender = SemanticGenderType::FEMININE;
                    return;
                }

                if (_tryGetInflectedFrom(pRes, SemanticGenderType::NEUTRAL, SemanticNumberType::PLURAL, inflections)) {
                    pNumber = SemanticNumberType::PLURAL;
                    pGender = SemanticGenderType::NEUTRAL;
                    return;
                }
                if (_tryGetInflectedFrom(
                        pRes, SemanticGenderType::MASCULINE, SemanticNumberType::PLURAL, inflections)) {
                    pNumber = SemanticNumberType::PLURAL;
                    pGender = SemanticGenderType::MASCULINE;
                    return;
                }
                if (_tryGetInflectedFrom(pRes, SemanticGenderType::FEMININE, SemanticNumberType::PLURAL, inflections)) {
                    pNumber = SemanticNumberType::PLURAL;
                    pGender = SemanticGenderType::FEMININE;
                    return;
                }
            }

            StaticLinguisticMeaning statLingMeaning = _lingDict.getLingMeaning(word.lemma, word.partOfSpeech, true);
            if (!statLingMeaning.isEmpty())
                statDb.getNounForm(pRes, statLingMeaning, pGender, pNumber);
            break;
        }
        default: break;
    }
}

void SynthesizerDictionary::_addInflectionsIfMoreFrequent(std::map<NominalInflection, InflFormAndFrequency>& pMap,
                                                          const NominalInflection& pNominalInflection,
                                                          const InflFormAndFrequency& pInflFormAndFrequency) {
    auto it = pMap.find(pNominalInflection);
    if (it == pMap.end())
        pMap.emplace(pNominalInflection, pInflFormAndFrequency);
    else if (it->second < pInflFormAndFrequency)
        it->second = pInflFormAndFrequency;
}

bool SynthesizerDictionary::_tryGetInflectedFrom(
    std::string& pRes,
    SemanticGenderType pGender,
    SemanticNumberType pNumber,
    const std::map<NominalInflection, InflFormAndFrequency>& pInflections) const {
    auto it = pInflections.find(NominalInflection(pGender, pNumber));
    if (it != pInflections.end()) {
        pRes = it->second.inflectedFrom;
        return true;
    }
    return false;
}

}    // End of namespace linguistics
}    // End of namespace onsem
