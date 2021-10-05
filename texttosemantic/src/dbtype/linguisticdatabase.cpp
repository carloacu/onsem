#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <iostream>
#include <boost/property_tree/xml_parser.hpp>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticanimationdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include "linguisticdatabase/partofspeechrules_english.hpp"
#include "linguisticdatabase/partofspeechrules_french.hpp"
#include "linguisticdatabase/childspecification.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{

auto _langToContextFilter = [](
    const InflectionsChecker& pInfls,
    const SpecificLinguisticDatabase& pSpecLingDb)
{
  switch (pSpecLingDb.language)
  {
  case SemanticLanguageEnum::ENGLISH:
    return linguistics::partofspeechrules::english::getPartOfSpeechRules(pInfls, pSpecLingDb);
  case SemanticLanguageEnum::FRENCH:
    return linguistics::partofspeechrules::french::getPartOfSpeechRules(pInfls, pSpecLingDb);
  case SemanticLanguageEnum::JAPANESE:
  default:
    return std::list<std::unique_ptr<linguistics::PartOfSpeechContextFilter>>();
  }
};


void _addWord(const boost::property_tree::ptree& pTree,
              SpecificLinguisticDatabase& pSpecLingDb,
              const std::map<std::string, int>& pBookmarkToPos)
{
  auto& wordAttributes = pTree.get_child("<xmlattr>");
  SemanticWord word(pSpecLingDb.language,
                    wordAttributes.get<std::string>("lemma"),
                    partOfSpeech_fromStr(wordAttributes.get<std::string>("pos")));
  InflectionType inflType = inflectionType_fromPartOfSpeech(word.partOfSpeech);

  for (const auto& currSubWordTree : pTree)
  {
    const std::string beaconName = currSubWordTree.first;
    const boost::property_tree::ptree& childTree = currSubWordTree.second;
    if (beaconName == "inflectedWord")
    {
      std::string inflectWordStr = childTree.get<std::string>("<xmlattr>.str", word.lemma);
      std::string inflectWordInflections = childTree.get<std::string>("<xmlattr>.inflections", "");
      char frequency = mystd::lexical_cast<char>(childTree.get<std::string>("<xmlattr>.frequency", "1"));
      pSpecLingDb.addInflectedWord(inflectWordStr, word,
                                   Inflections::create(inflType, inflectWordInflections), frequency);
    }
    else if (beaconName == "contextInfo")
    {
      linguistics::WordAssociatedInfos wordAssocInfos;
      wordAssocInfos.contextualInfos.insert(wordContextualInfos_fromStr(childTree.get<std::string>("<xmlattr>.val")));
      pSpecLingDb.addInfosToAWord(word, wordAssocInfos);
    }
    else if (currSubWordTree.first == "include_child_list_template")
    {
      const boost::property_tree::ptree& childTree = currSubWordTree.second;
      const std::string templateName = childTree.get<std::string>("<xmlattr>.name");
      pSpecLingDb.getSemFrameDict().addWordToTemplate(word, templateName);
    }
    else if (currSubWordTree.first == "child")
    {
      const boost::property_tree::ptree& childTree = currSubWordTree.second;
      ChildSpecification childSpec;
      fillChildSpecsFromBookmark(childSpec, childTree, pSpecLingDb.language, pBookmarkToPos);
      pSpecLingDb.getSemFrameDict().addWordToChildSpecifications(word, std::move(childSpec));
    }
    else if (beaconName != "<xmlattr>")
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}


void _addChildTemplate(const boost::property_tree::ptree& pTree,
                       SpecificLinguisticDatabase& pSpecLingDb,
                       const std::map<std::string, int>& pBookmarkToPos)
{
  auto& wordAttributes = pTree.get_child("<xmlattr>");
  const std::string templateName = wordAttributes.get<std::string>("name");

  for (const auto& currSubWordTree : pTree)
  {
    const std::string beaconName = currSubWordTree.first;
    const boost::property_tree::ptree& childTree = currSubWordTree.second;
    if (currSubWordTree.first == "child")
    {
      ChildSpecification childSpec;
      fillChildSpecsFromBookmark(childSpec, childTree, pSpecLingDb.language, pBookmarkToPos);
      pSpecLingDb.getSemFrameDict().addTemplateNameToChildSpecifications(templateName, std::move(childSpec));
    }
    else if (beaconName != "<xmlattr>")
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}

void _addAnyVerbInfos(const boost::property_tree::ptree& pTree,
                      SpecificLinguisticDatabase& pSpecLingDb,
                      int& pLastTemplatePos,
                      std::map<std::string, int>& pBookmarkToPos)
{
  for (const auto& currSubWordTree : pTree)
  {
    if (currSubWordTree.first == "child")
    {
      const boost::property_tree::ptree& childTree = currSubWordTree.second;
      ChildSpecification childSpec;
      fillChildSpecsFromId(childSpec, childTree, pSpecLingDb.language, pLastTemplatePos);
      pSpecLingDb.getSemFrameDict().addAChildSpecificationsByDefault(std::move(childSpec));
    }
    else if (currSubWordTree.first == "bookmark")
    {
      const auto bookmarkNameOpt = currSubWordTree.second.get_optional<std::string>("<xmlattr>.name");
      if (bookmarkNameOpt)
      {
        pBookmarkToPos.emplace(*bookmarkNameOpt, pLastTemplatePos);
        ++pLastTemplatePos;
      }
    }
  }
}

void _addNoVerbInfos(const boost::property_tree::ptree& pTree,
                     SpecificLinguisticDatabase& pSpecLingDb)
{
  for (const auto& currSubWordTree : pTree)
  {
    if (currSubWordTree.first == "child")
    {
      const boost::property_tree::ptree& childTree = currSubWordTree.second;
      ChildSpecification childSpec;
      fillChildSpecs(childSpec, childTree, pSpecLingDb.language);
      pSpecLingDb.getSemFrameDict().addAChildSpecificationsWithoutVerbByDefault(std::move(childSpec));
    }
  }
}

// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)

}



SpecificLinguisticDatabase::SpecificLinguisticDatabase(KeyToStreams& pIStreams,
                                                       LinguisticDatabase& pLingDb,
                                                       SemanticLanguageEnum pLanguage)
  : language(pLanguage),
    lingDb(pLingDb),
    lingDico(*pIStreams.mainDicToStream, pLingDb.conceptSet.statDb, pLanguage),
    synthDico(*pIStreams.synthesizerToStream, pLingDb.conceptSet.statDb, lingDico, pLanguage),
    _semFrameDict(mystd::make_unique<SemanticFrameDictionary>()),
    _wordToSavedInfos(),
    _inflectionsCheckerPtr(mystd::make_unique<InflectionsChecker>(*this)),
    _contextFilters(_langToContextFilter(*_inflectionsCheckerPtr, *this))
{
}

SpecificLinguisticDatabase::~SpecificLinguisticDatabase()
{
}


void SpecificLinguisticDatabase::addInflectedWord
(const std::string& pInflectedFrom,
 const SemanticWord& pWord,
 std::unique_ptr<Inflections> pInflections,
 char pFrequency)
{
  auto itWordToInfos = _saveWordInfos(pWord);
  itWordToInfos->second.inflectedFormInfos.emplace_back(pInflectedFrom, std::move(pInflections), pFrequency);

  const Inflections* inflections = &*itWordToInfos->second.inflectedFormInfos.back().inflections;
  lingDico.addInflectedWord(pInflectedFrom, itWordToInfos->first,
                            itWordToInfos->second.infos,
                            inflections);
  synthDico.addInflectedWord(pInflectedFrom, pWord, *inflections, pFrequency);
}


void SpecificLinguisticDatabase::addProperNoun(const std::string& pWordLemma)
{
  SemanticWord word(SemanticLanguageEnum::UNKNOWN, pWordLemma, PartOfSpeech::PROPER_NOUN);
  auto inflections = mystd::make_unique<EmptyInflections>();
  addInflectedWord(pWordLemma, word, inflections->clone(), 4);
  std::string wordLemmaLowerCase = pWordLemma;
  lowerCaseFirstLetter(wordLemmaLowerCase);
  addInflectedWord(wordLemmaLowerCase, word, std::move(inflections), 4);
}


void SpecificLinguisticDatabase::addInfosToAWord
(const SemanticWord& pWord,
 const WordAssociatedInfos& pWordInfos)
{
  auto itWordToInfos = _saveWordInfos(pWord);
  itWordToInfos->second.infos.mergeWith(pWordInfos);
  lingDico.addInfosToAWord(pWord, &itWordToInfos->second.infos);
  synthDico.addInfosToAWord(pWord, &itWordToInfos->second.infos);
}


void SpecificLinguisticDatabase::reset()
{
  lingDico.reset();
  synthDico.reset();
  _wordToSavedInfos.clear();
}


std::map<SemanticWord, SpecificLinguisticDatabase::InflectionsInfosForAWord>::iterator SpecificLinguisticDatabase::_saveWordInfos
(const SemanticWord& pWord)
{
  auto it = _wordToSavedInfos.find(pWord);
  if (it == _wordToSavedInfos.end())
  {
    _wordToSavedInfos[pWord];
    it = _wordToSavedInfos.find(pWord);
  }
  return it;
}





LinguisticDatabase::LinguisticDatabase(LinguisticDatabaseStreams& pIStreams)
  : conceptSet(*pIStreams.concepts),
    langToSpec(semanticLanguageEnum_size, [&](std::size_t i) { auto lang = semanticLanguageEnum_fromChar(static_cast<char>(i)); return mystd::make_unique<SpecificLinguisticDatabase>(pIStreams.languageToStreams[lang], *this, lang); }),
    transDict(pIStreams),
    treeConverter(pIStreams),
    langToAnimDic()
{
  std::list<SemanticLanguageEnum> languageTypes;
  getAllLanguageTypes(languageTypes);
  for (const auto& currLang : languageTypes)
    langToAnimDic.emplace(currLang,
                          mystd::make_unique<AnimationDictionary>
                          (*pIStreams.languageToStreams[currLang].animationsToStream, conceptSet.statDb, currLang));
  addDynamicContent(pIStreams.dynamicContentStreams);
}



void LinguisticDatabase::reset()
{
  auto size = langToSpec.size();
  for (std::size_t i = 0; i < size; ++i)
    langToSpec[i].reset();
  conceptSet.reset();
}


void LinguisticDatabase::addProperNouns(const std::set<std::string>& pProperNouns)
{
  auto& specLinDb = langToSpec[SemanticLanguageEnum::UNKNOWN];
  for (const auto& currProperNoun : pProperNouns)
    specLinDb.addProperNoun(currProperNoun);
}

void LinguisticDatabase::addDynamicContent(std::stringstream& pSs)
{
  boost::property_tree::ptree pt;
  boost::property_tree::xml_parser::read_xml(pSs, pt);
  _addDynamicContent(pt);
}

void LinguisticDatabase::addDynamicContent(std::list<std::istream*>& pIstreams)
{
  for (auto& currStream : pIstreams)
  {
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(*currStream, pt);
    _addDynamicContent(pt);
  }
}


void LinguisticDatabase::_addDynamicContent(const boost::property_tree::ptree& pTree)
{
  for (const auto& currSubTree : pTree)
  {
    const std::string beaconName = currSubTree.first;
    const boost::property_tree::ptree& childTree = currSubTree.second;
    if (beaconName == "dictionary_modification")
      _addDynamicModification(childTree);
    else if (beaconName == "dictionary_translation")
      _addDynamicTranslation(childTree);
    else if (beaconName == "dictionary_concept")
      _addDynamicConcept(childTree);
    else
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}


bool LinguisticDatabase::hasContextualInfo(WordContextualInfos pContextualInfo,
                                           const LinguisticMeaning& pMeaning) const
{
  SemanticLanguageEnum meaningLanguage = SemanticLanguageEnum::UNKNOWN;
  if (pMeaning.getLanguageIfNotEmpty(meaningLanguage))
    return langToSpec[meaningLanguage].lingDico.hasContextualInfo(pContextualInfo, pMeaning);
  return false;
}

void LinguisticDatabase::_addDynamicConcept(const boost::property_tree::ptree& pTree)
{
  for (const auto& currSubTree : pTree)
  {
    const std::string beaconName = currSubTree.first;
    const boost::property_tree::ptree& childTree = currSubTree.second;

    if (beaconName == "concept")
    {
      std::string conceptName = childTree.get<std::string>("<xmlattr>.name");
      _addDynamicConceptInfo(childTree, conceptName);
    }
    else if (beaconName != "<xmlattr>")
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}


void LinguisticDatabase::_addDynamicConceptInfo(const boost::property_tree::ptree& pTree,
                                                const std::string& pConceptName)
{
  for (const auto& currSubTree : pTree)
  {
    const std::string beaconName = currSubTree.first;
    const boost::property_tree::ptree& childTree = currSubTree.second;

    if (beaconName == "opposite_concept")
    {
      std::string conceptName = childTree.get<std::string>("<xmlattr>.name");
      conceptSet.notifyOppositeConcepts(pConceptName, conceptName);
    }
    else if (beaconName != "<xmlattr>")
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}


void LinguisticDatabase::_addDynamicTranslation(const boost::property_tree::ptree& pTree)
{
  auto& translationAttributes = pTree.get_child("<xmlattr>");
  SemanticLanguageEnum fromLanguage = SemanticLanguageEnum::UNKNOWN;
  semanticLanguageEnum_fromLegacyStrIfExist(fromLanguage, translationAttributes.get<std::string>("from_language"));
  SemanticLanguageEnum toLanguage = SemanticLanguageEnum::UNKNOWN;
  semanticLanguageEnum_fromLegacyStrIfExist(toLanguage, translationAttributes.get<std::string>("to_language"));

  for (const auto& currSubTree : pTree)
  {
    const std::string beaconName = currSubTree.first;
    const boost::property_tree::ptree& childTree = currSubTree.second;

    if (beaconName == "from_word")
    {
      auto& wordAttributes = childTree.get_child("<xmlattr>");
      SemanticWord fromWord(fromLanguage,
                            wordAttributes.get<std::string>("lemma"),
                            partOfSpeech_fromStr(wordAttributes.get<std::string>("pos")));
      _addDynamicTranslationToWord(childTree, fromWord, toLanguage);
    }
    else if (beaconName != "<xmlattr>")
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}


void LinguisticDatabase::_addDynamicTranslationToWord(const boost::property_tree::ptree& pTree,
                                                      const SemanticWord& pFromWord,
                                                      SemanticLanguageEnum pToLanguage)
{
  for (const auto& currSubTree : pTree)
  {
    const std::string beaconName = currSubTree.first;
    const boost::property_tree::ptree& childTree = currSubTree.second;

    if (beaconName == "to_word")
    {
      auto& wordAttributes = childTree.get_child("<xmlattr>");
      SemanticWord toWord(pToLanguage,
                          wordAttributes.get<std::string>("lemma"),
                          partOfSpeech_fromStr(wordAttributes.get<std::string>("pos")));
      transDict.addTranslation(pFromWord, toWord);
    }
    else if (beaconName != "<xmlattr>")
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}


void LinguisticDatabase::_addDynamicModification(const boost::property_tree::ptree& pTree)
{
  SemanticLanguageEnum language = SemanticLanguageEnum::UNKNOWN;
  semanticLanguageEnum_fromLegacyStrIfExist(language, pTree.get<std::string>("<xmlattr>.language"));
  auto& specLingDb = langToSpec[language];
  auto& lastId = specLingDb.getSemFrameDict().getLastTemplatePos();
  std::map<std::string, int>& bookmarkToIds = specLingDb.getSemFrameDict().getBookmarkToTemplatePos();

  for (const auto& currSubTree : pTree)
  {
    const std::string beaconName = currSubTree.first;
    const boost::property_tree::ptree& childTree = currSubTree.second;

    if (beaconName == "word")
      _addWord(childTree, specLingDb, bookmarkToIds);
    else if (beaconName == "child_list_template")
      _addChildTemplate(childTree, specLingDb, bookmarkToIds);
    else if (beaconName == "any_verb")
      _addAnyVerbInfos(childTree, specLingDb, lastId, bookmarkToIds);
    else if (beaconName == "no_verb")
      _addNoVerbInfos(childTree, specLingDb);
    else if (beaconName == "concept")
      _addConcept(childTree, specLingDb);
    else if (beaconName != "<xmlattr>")
      throw std::runtime_error("Invalid beacon: " + beaconName);
  }
}


void LinguisticDatabase::_addConcept(const boost::property_tree::ptree& pTree,
                                     SpecificLinguisticDatabase& pSpecLingDb)
{
  std::string conceptName = pTree.get<std::string>("<xmlattr>.name");
  linguistics::WordAssociatedInfos wordAssocInfos;
  wordAssocInfos.concepts.emplace(conceptName, 4);

  for (const auto& currSubWordTree : pTree)
  {
    const std::string beaconName = currSubWordTree.first;
    const boost::property_tree::ptree& childTree = currSubWordTree.second;
    if (beaconName == "word")
    {
      auto& wordAttributes = childTree.get_child("<xmlattr>");
      SemanticWord word(pSpecLingDb.language,
                        wordAttributes.get<std::string>("lemma"),
                        partOfSpeech_fromStr(wordAttributes.get<std::string>("pos")));
      pSpecLingDb.addInfosToAWord(word, wordAssocInfos);
    }
  }
}


void LinguisticDatabase::getInflectedNoun(std::string& pRes,
                                          SemanticLanguageEnum pLanguage,
                                          const SemanticWord& pWord,
                                          SemanticGenderType& pGender,
                                          SemanticNumberType& pNumber) const
{
  LinguisticMeaning lingMeaning;
  SemExpGetter::wordToAMeaning(lingMeaning, pWord, pLanguage, *this);
  SemanticLanguageEnum meaningLanguage = SemanticLanguageEnum::UNKNOWN;
  if (lingMeaning.getLanguageIfNotEmpty(meaningLanguage))
    langToSpec[meaningLanguage].synthDico.getNounForm(pRes, lingMeaning, pGender, pNumber);
}



} // End of namespace linguistics
} // End of namespace onsem
