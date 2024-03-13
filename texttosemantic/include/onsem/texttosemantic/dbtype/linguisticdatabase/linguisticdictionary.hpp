#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_LINGUISTICDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_LINGUISTICDICTIONARY_HPP

#include <list>
#include <vector>
#include <map>
#include <mutex>
#include <onsem/common/utility/radix_map_forward_declaration.hpp>
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include "../../api.hpp"

namespace onsem {
class StaticConceptSet;
struct Inflections;
struct GroundedExpression;
namespace linguistics {
struct InflectedWord;

class ONSEM_TEXTTOSEMANTIC_API LinguisticDictionary {
public:
    LinguisticDictionary(std::istream* pDictIStreamPtr,
                         const StaticConceptSet& pStaticConceptSet,
                         SemanticLanguageEnum pLangEnum);
    ~LinguisticDictionary();

    LinguisticDictionary(const LinguisticDictionary& pOther);
    LinguisticDictionary& operator=(const LinguisticDictionary& pOther);

    void addInflectedWord(const std::string& pInflectedFrom,
                          const SemanticWord& pWord,
                          WordAssociatedInfos& pInfos,
                          const Inflections* pInflections);

    void addInfosToAWord(const SemanticWord& pWord, const WordAssociatedInfos* pWordAssociatedInfos);
    void removeAWord(const SemanticWord& pWord);

    void getInfoGram(InflectedWord& pIGram, const LinguisticMeaning& pMeaning) const;
    void getContextualInfos(std::set<WordContextualInfos>& pContextualInfos, const LinguisticMeaning& pMeaning) const;
    void getConcepts(std::map<std::string, char>& pConcepts, const LinguisticMeaning& pMeaning) const;
    void getConceptsFromWord(std::map<std::string, char>& pConcepts, const SemanticWord& pWord) const;

    void reset();

    SemanticLanguageEnum getLanguage() const { return _language; }

    std::size_t getLengthOfLongestWord(const std::string& pStr, std::size_t pBeginStr) const;

    void getGramPossibilitiesAndPutUnknownIfNothingFound(std::list<InflectedWord>& pInfosGram,
                                                         const std::string& pWord,
                                                         std::size_t pBeginPos,
                                                         std::size_t pSizeOfWord) const;

    void getGramPossibilities(std::list<InflectedWord>& pInfosGram,
                              const std::string& pWord,
                              std::size_t pBeginPos,
                              std::size_t pSizeOfWord) const;

    SemanticRequestType aloneWordToRequest(const SemanticWord& pWord) const;
    SemanticRequestType semWordToRequest(const SemanticWord& pWord) const;

    bool hasContextualInfo(WordContextualInfos pContextualInfo, const SemanticWord& pWord) const;
    bool hasContextualInfo(WordContextualInfos pContextualInfo, const LinguisticMeaning& pMeaning) const;

    StaticLinguisticDictionary& statDb;

    const StaticLinguisticDictionary::StaticWord& getBeAux() const { return statDb.getBeAux(); }
    const StaticLinguisticDictionary::StaticWord& getHaveAux() const { return statDb.getHaveAux(); }
    const StaticLinguisticDictionary::StaticWord& getBeVerb() const { return statDb.getBeVerb(); }
    const StaticLinguisticDictionary::StaticWord& getSayVerb() const { return statDb.getSayVerb(); }

private:
    /// /!\ No ownership in that structs. It only keeps pointer to existing objects.
    struct WordAndInfos {
        WordAndInfos(const SemanticWord* pWord, WordAssociatedInfos* pInfos)
            : _wordPtr(pWord)
            , _infosPtr(pInfos) {}

        const SemanticWord& word() const { return *_wordPtr; }
        WordAssociatedInfos& infos() const { return *_infosPtr; }

    private:
        const SemanticWord* _wordPtr;
        WordAssociatedInfos* _infosPtr;
    };
    struct InflectedInfos {
        InflectedInfos(const SemanticWord& pWord, WordAssociatedInfos& pInfos, const Inflections* pInflections);
        InflectedInfos(const WordAndInfos& pWordAndInfos, const Inflections* pInflections);

        InflectedInfos(InflectedInfos&& pOther);
        InflectedInfos& operator=(InflectedInfos&& pOther);
        InflectedInfos(const InflectedInfos& pOther);
        InflectedInfos& operator=(const InflectedInfos& pOther);

        void fillIGram(InflectedWord& pIGram) const;

        std::unique_ptr<WordAndInfos> wordAndInfos;
        const Inflections* inflections;
    };
    SemanticLanguageEnum _language;
    std::map<SemanticWord, const WordAssociatedInfos*> _wordToAssocInfos;
    std::unique_ptr<mystd::radix_map_str<std::list<PartOfSpeech>>> _lemmaToPosOfWordToRemoveFromStaticDico;
    std::unique_ptr<mystd::radix_map_str<std::list<InflectedInfos>>> _inflectedCharaters;

    static std::mutex _pathToStatDbsMutex;
    static std::map<SemanticLanguageEnum, std::unique_ptr<StaticLinguisticDictionary>> _pathToStatDbs;
    static StaticLinguisticDictionary& _getStatDbInstance(std::istream* pDictIStreamPtr,
                                                          const StaticConceptSet& pStaticConceptSet,
                                                          SemanticLanguageEnum pLangEnum);
    bool _isARemovedWord(const SemanticWord& pWord) const;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_LINGUISTICDICTIONARY_HPP
