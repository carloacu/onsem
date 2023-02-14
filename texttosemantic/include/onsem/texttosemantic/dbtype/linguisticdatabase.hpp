#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_HPP

#include <string>
#include <assert.h>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <onsem/common/keytostreams.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/utility/enum_vector_initialized.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/synthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/treeconverter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/translationdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/animationdictionary.hpp>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcontextfilter.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include "../api.hpp"

namespace onsem
{
namespace linguistics
{
class SemanticFrameDictionary;
class InflectionsChecker;
struct LinguisticDatabase;


struct ONSEM_TEXTTOSEMANTIC_API SpecificLinguisticDatabase
{
  SpecificLinguisticDatabase(KeyToStreams& pIStreams,
                             LinguisticDatabase& pLingDb,
                             SemanticLanguageEnum pLanguage);
  ~SpecificLinguisticDatabase();

  SpecificLinguisticDatabase(SpecificLinguisticDatabase&& pOther) = delete;
  SpecificLinguisticDatabase& operator=(SpecificLinguisticDatabase&& pOther) = delete;
  SpecificLinguisticDatabase(const SpecificLinguisticDatabase&) = delete;
  SpecificLinguisticDatabase& operator=(const SpecificLinguisticDatabase&) = delete;

  void addInflectedWord(const std::string& pInflectedFrom,
                        const SemanticWord& pWord,
                        std::unique_ptr<Inflections> pInflections,
                        char pFrequency);
  void addProperNoun(const std::string& pWordLemma);
  void addInfosToAWord(const SemanticWord& pWord,
                       const WordAssociatedInfos& pWordInfos);
  void removeAWord(const SemanticWord& pWord);
  void reset();

  const InflectionsChecker& inflectionsChecker() const
  { assert(_inflectionsCheckerPtr);
    return *_inflectionsCheckerPtr; }

  const std::list<std::unique_ptr<PartOfSpeechContextFilter>>& getContextFilters() const
  { return _contextFilters; }

  const SemanticLanguageEnum language;
  LinguisticDatabase& lingDb;
  LinguisticDictionary lingDico;
  SynthesizerDictionary synthDico;

  SemanticFrameDictionary& getSemFrameDict() { return *_semFrameDict; }
  const SemanticFrameDictionary& getSemFrameDict() const  { return *_semFrameDict; }

  struct InflectedFormInfos
  {
    InflectedFormInfos(const std::string& pInflectedFrom,
                       std::unique_ptr<Inflections> pInflections,
                       char pFrequency)
      : inflectedFrom(pInflectedFrom),
        inflections(std::move(pInflections)),
        frequency(pFrequency)
    {
    }

    std::string inflectedFrom;
    std::unique_ptr<Inflections> inflections;
    char frequency;
  };
  struct InflectionsInfosForAWord
  {
    WordAssociatedInfos infos{};
    std::list<InflectedFormInfos> inflectedFormInfos{};
  };

  const std::map<SemanticWord, InflectionsInfosForAWord>& getWordToSavedInfos() const { return _wordToSavedInfos; }

private:
  std::unique_ptr<SemanticFrameDictionary> _semFrameDict;
  std::map<SemanticWord, InflectionsInfosForAWord> _wordToSavedInfos;
  std::unique_ptr<InflectionsChecker> _inflectionsCheckerPtr;
  const std::list<std::unique_ptr<linguistics::PartOfSpeechContextFilter>> _contextFilters;

  std::map<SemanticWord, InflectionsInfosForAWord>::iterator _saveWordInfos(const SemanticWord& pWord);
};



struct ONSEM_TEXTTOSEMANTIC_API LinguisticDatabase
{
  LinguisticDatabase(LinguisticDatabaseStreams& pIStreams);

  LinguisticDatabase(LinguisticDatabase&& pOther) = delete;
  LinguisticDatabase& operator=(LinguisticDatabase&& pOther) = delete;
  LinguisticDatabase(const LinguisticDatabase&) = delete;
  LinguisticDatabase& operator=(const LinguisticDatabase&) = delete;

  const SpecificLinguisticDatabase& commonSpecificLingDb() const { return langToSpec[SemanticLanguageEnum::UNKNOWN]; }
  void reset();

  bool hasContextualInfo(WordContextualInfos pContextualInfo,
                         const LinguisticMeaning& pMeaning) const;

  void addProperNouns(const std::set<std::string>& pProperNouns);
  void addDynamicContent(std::stringstream& pSs);
  void addDynamicContent(std::list<std::istream*>& pIstreams);

  void getInflectedNoun(std::string& pRes,
                        std::set<WordContextualInfos>& pContextualInfos,
                        SemanticLanguageEnum pLanguage,
                        const SemanticWord& pWord,
                        SemanticGenderType& pGender,
                        SemanticNumberType& pNumber) const;

  ConceptSet conceptSet;
  mystd::enum_vector_initialized<SemanticLanguageEnum, SpecificLinguisticDatabase> langToSpec;
  TranslationDictionary transDict;
  TreeConverter treeConverter;
  std::map<SemanticLanguageEnum, std::unique_ptr<AnimationDictionary>> langToAnimDic;

private:
  void _addDynamicContent(const boost::property_tree::ptree& pTree);
  void _addDynamicConcept(const boost::property_tree::ptree& pTree);
  void _addDynamicConceptInfo(const boost::property_tree::ptree& pTree,
                              const std::string& pConceptName);
  void _addDynamicTranslation(const boost::property_tree::ptree& pTree);
  void _addDynamicTranslationToWord(const boost::property_tree::ptree& pTree,
                                    const SemanticWord& pFromWord,
                                    SemanticLanguageEnum pToLanguage);
  void _addDynamicModification(const boost::property_tree::ptree& pTree);
  void _addConcept(const boost::property_tree::ptree& pTree,
                   SpecificLinguisticDatabase& pSpecLingDb);
};


} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_HPP
