#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_LINGUISTICDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_LINGUISTICDICTIONARY_HPP

#include <list>
#include <vector>
#include <map>
#include <mutex>
#include <onsem/common/utility/radix_map_forward_declaration.hpp>
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include "../../api.hpp"

namespace onsem
{
class StaticConceptSet;
struct Inflections;
struct GroundedExpression;
namespace linguistics
{
struct InflectedWord;


class ONSEM_TEXTTOSEMANTIC_API LinguisticDictionary
{
public:
  LinguisticDictionary(std::istream& pDictIStream,
                       const StaticConceptSet& pStaticConceptSet,
                       SemanticLanguageEnum pLangEnum);
  ~LinguisticDictionary();

  LinguisticDictionary(const LinguisticDictionary& pOther);
  LinguisticDictionary& operator=(const LinguisticDictionary& pOther);

  void addInflectedWord(const std::string& pInflectedFrom,
                        const SemanticWord& pWord,
                        WordAssociatedInfos& pInfos,
                        const Inflections* pInflections);

  void addInfosToAWord(const SemanticWord& pWord,
                       const WordAssociatedInfos* pWordAssociatedInfos);
  void removeAWord(const SemanticWord& pWord);

  void getInfoGram(InflectedWord& pIGram,
                   const LinguisticMeaning& pMeaning) const;
  void getConcepts(std::map<std::string, char>& pConcepts,
                   const SemanticWord& pWord) const;

  void reset();

  SemanticLanguageEnum getLanguage() const { return _language; }

  std::size_t getLengthOfLongestWord
  (const std::string& pStr, std::size_t pBeginStr) const;

  void getGramPossibilitiesAndPutUnknownIfNothingFound
  (std::list<InflectedWord>& pInfosGram,
   const std::string& pWord,
   std::size_t pBeginPos,
   std::size_t pSizeOfWord) const;

  void getGramPossibilities
  (std::list<InflectedWord>& pInfosGram,
   const std::string& pWord,
   std::size_t pBeginPos,
   std::size_t pSizeOfWord) const;

  SemanticRequestType aloneWordToRequest(const SemanticWord& pWord) const;
  SemanticRequestType semWordToRequest(const SemanticWord& pWord) const;

  bool hasContextualInfo(WordContextualInfos pContextualInfo,
                         const SemanticWord& pWord) const;
  bool hasContextualInfo(WordContextualInfos pContextualInfo,
                         const LinguisticMeaning& pMeaning) const;

  StaticLinguisticDictionary& statDb;

  struct StaticWord
  {
    StaticWord(const StaticLinguisticDictionary& pStatBinDico);
    void setContent(const std::string& pLemma,
                    PartOfSpeech pPartOfSpeech);
    void clear();

    SemanticWord word{};
    StaticLinguisticMeaning meaning{};
  private:
    const StaticLinguisticDictionary& _statBinDico;
  };

  const StaticWord& getBeAux() const;
  const StaticWord& getHaveAux() const;
  const StaticWord& getBeVerb() const;
  const StaticWord& getSayVerb() const;


private:
  /// /!\ No ownership in that structs. It only keeps pointer to existing objects.
  struct WordAndInfos
  {
   WordAndInfos(const SemanticWord* pWord,
                WordAssociatedInfos* pInfos)
     : _wordPtr(pWord),
       _infosPtr(pInfos)
   {
   }

   const SemanticWord& word() const { return *_wordPtr; }
   WordAssociatedInfos& infos() const { return *_infosPtr; }

  private:
   const SemanticWord* _wordPtr;
   WordAssociatedInfos* _infosPtr;
  };
  struct InflectedInfos
  {
   InflectedInfos(const SemanticWord& pWord,
                  WordAssociatedInfos& pInfos,
                  const Inflections* pInflections);
   InflectedInfos(const WordAndInfos& pWordAndInfos,
                  const Inflections* pInflections);

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
  StaticWord _beAux;
  StaticWord _haveAux;
  StaticWord _beVerb;
  StaticWord _sayVerb;


  static std::mutex _pathToStatDbsMutex;
  static std::map<SemanticLanguageEnum, std::unique_ptr<StaticLinguisticDictionary>> _pathToStatDbs;
  static StaticLinguisticDictionary& _getStatDbInstance(std::istream& pDictIStream,
                                                        const StaticConceptSet& pStaticConceptSet,
                                                        SemanticLanguageEnum pLangEnum);
  bool _isARemovedWord(const SemanticWord& pWord) const;
};






inline const LinguisticDictionary::StaticWord& LinguisticDictionary::getBeAux() const
{
  return _beAux;
}

inline const LinguisticDictionary::StaticWord& LinguisticDictionary::getHaveAux() const
{
  return _haveAux;
}

inline const LinguisticDictionary::StaticWord& LinguisticDictionary::getBeVerb() const
{
  return _beVerb;
}

inline const LinguisticDictionary::StaticWord& LinguisticDictionary::getSayVerb() const
{
  return _sayVerb;
}


} // End of namespace linguistics
} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_LINGUISTICDICTIONARY_HPP
