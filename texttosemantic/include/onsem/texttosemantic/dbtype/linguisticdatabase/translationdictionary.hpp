#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_TRANSLATIONDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_TRANSLATIONDICTIONARY_HPP

#include <memory>
#include <mutex>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/statictranslationdictionary.hpp>
#include "../../api.hpp"

namespace onsem {
namespace linguistics {
class StaticLinguisticDictionary;
class LinguisticDictionary;

class ONSEM_TEXTTOSEMANTIC_API TranslationDictionary {
public:
    TranslationDictionary(LinguisticDatabaseStreams& pIStreams);

    void addTranslation(const SemanticWord& pFromWord, const SemanticWord& pToWord);

    bool getTranslation(LinguisticMeaning& pTranslatedMeaningId,
                        const SemanticWord& pWord,
                        SemanticLanguageEnum pOutLanguage,
                        const LinguisticDictionary& pLingDico) const;

    void getTranslation(int32_t& pTranslatedMeaningId,
                        const SemanticWord& pWord,
                        SemanticLanguageEnum pOutLanguage,
                        const linguistics::StaticLinguisticDictionary& pStatLingDico) const;

    const StaticTranslationDictionary* getStaticTranslationDictionaryPtr(SemanticLanguageEnum pInLanguage,
                                                                         SemanticLanguageEnum pOutLanguage) const;

    const std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, std::map<SemanticWord, SemanticWord>>>&
        getAllTranslations() const {
        return _translations;
    }

private:
    std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, const StaticTranslationDictionary*>>
        _binTranslations{};
    std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, std::map<SemanticWord, SemanticWord>>>
        _translations{};

    static std::mutex _pathToStatDbsMutex;
    static std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, std::unique_ptr<StaticTranslationDictionary>>>
        _pathToStatDbs;
    static std::map<SemanticLanguageEnum, std::map<SemanticLanguageEnum, const StaticTranslationDictionary*>>
        _getStatDbInstance(LinguisticDatabaseStreams& pIStreams);
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_TRANSLATIONDICTIONARY_HPP
