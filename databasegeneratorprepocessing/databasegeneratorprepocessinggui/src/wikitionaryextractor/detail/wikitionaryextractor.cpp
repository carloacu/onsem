#include "../wikitionaryextractor.hpp"
#include "addcomposedwords.hpp"
#include "addcomposedwords.hpp"
#include "addcomposedwords.hpp"
#include <fstream>
#include <filesystem>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/savers/traductionwriter.hpp>
#include "dataextractor.hpp"
#include "frwiki/frpatternrecognizer.hpp"
#include "frwiki/frwikikeywords.hpp"
#include "enwiki/enpatternrecognizer.hpp"
#include "enwiki/enwikikeywords.hpp"
#include "transitiveverbextractor.hpp"
#include "conceptsextractor.hpp"

namespace onsem
{

void extractDataFromSpecifcWiki
(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsInToOut,
 std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsOutToIn,
 const LinguisticIntermediaryDatabase& pInLingDatabase,
 const LinguisticIntermediaryDatabase& pOutLingDatabase,
 const WikiKeyWords& pWikiKeyWords,
 const PatternRecognizer& pPatternReco,
 const std::string& pWikionaryFilename,
 const std::string& pTradInToOutFilename,
 const std::string& pTradOutToInFilename)
{
  std::ifstream wikionaryFile(pWikionaryFilename.c_str(), std::ifstream::in);
  if (!wikionaryFile.is_open())
  {
    std::cerr << "Error: Can't open " << pWikionaryFilename << " file !" << std::endl;
    exit(1);
  }

  std::cout << "read the wikitionary" << std::endl;

  WikiDataExtractor dataExt(pWikiKeyWords, pPatternReco);
  dataExt.extractDatasFromFile(pTradsInToOut, pTradsOutToIn, wikionaryFile,
                               pInLingDatabase, pOutLingDatabase);

  // write the traductions
  std::cout << "write the traductions" << std::endl;
  LingdbSaverTraductionWriter tradWriter;
  tradWriter.writeTraductionsForOneWiki(pTradInToOutFilename, pTradsInToOut);
  tradWriter.writeTraductionsForOneWiki(pTradOutToInFilename, pTradsOutToIn);
}


void extractDataFromFrWiki
(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsInToOut,
 std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsOutToIn,
 const LinguisticIntermediaryDatabase& pInLingDatabase,
 const LinguisticIntermediaryDatabase& pOutLingDatabase,
 const std::string& pWikionaryFilename)
{
  std::string tradInToOutFilename = "french_to_english_frwiktionary.wlks";
  std::string tradOutToInFilename = "english_to_french_frwiktionary.wlks";
  FRWikiKeyWords frWikiKeyWords;
  FrPatternRecognizer frPatternReco;

  extractDataFromSpecifcWiki(pTradsInToOut, pTradsOutToIn,
                             pInLingDatabase, pOutLingDatabase, frWikiKeyWords, frPatternReco,
                             pWikionaryFilename, tradInToOutFilename, tradOutToInFilename);
}


void extractDataFromEnWiki
(std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsInToOut,
 std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTradsOutToIn,
 const LinguisticIntermediaryDatabase& pInLingDatabase,
 const LinguisticIntermediaryDatabase& pOutLingDatabase,
 const std::string& pWikionaryFilename)
{
  std::string tradInToOutFilename = "english_to_french_enwiktionary.wlks";
  std::string tradOutToInFilename = "french_to_english_enwiktionary.wlks";
  EnWikiKeyWords enWikiKeyWords;
  EnPatternRecognizer enPatternReco;

  extractDataFromSpecifcWiki(pTradsInToOut, pTradsOutToIn,
                             pInLingDatabase, pOutLingDatabase, enWikiKeyWords, enPatternReco,
                             pWikionaryFilename, tradInToOutFilename, tradOutToInFilename);
}


void addComposedWords
(const std::string& pWikionaryFilename,
 const LinguisticIntermediaryDatabase& pLingDatabase,
 const AddComposedWords& pCompWordsAdder,
 SemanticLanguageEnum pLangEnum,
 const std::string& pOutFilename)
{
  std::set<Wikitionary_ComposedWord> newComposedWords;

  std::ifstream wikionaryFile(pWikionaryFilename.c_str(), std::ifstream::in);
  if (!wikionaryFile.is_open())
  {
    std::cerr << "Error: Can't open " << pWikionaryFilename << " file !" << std::endl;
    exit(1);
  }
  pCompWordsAdder.extractDatasFromFile(newComposedWords,
                                       wikionaryFile,
                                       pLingDatabase, pLangEnum);
  pCompWordsAdder.writeNewComposedWords(newComposedWords,
                                        pOutFilename);
}



namespace wikitionaryExtractor
{
static const std::string localPathToFrenchWikitionary = "/resources/french/frwiktionary-latest-pages-articles.xml";
static const std::string localPathToEnglishWikitionary = "/resources/english/enwiktionary-latest-pages-articles.xml";


void _loadLingDb(LinguisticIntermediaryDatabase& pLingDatabase,
                 const LingdbTree& pLingDbTree,
                 SemanticLanguageEnum pLanguage)
{
  pLingDatabase.setLanguage(semanticLanguageEnum_toLegacyStr(pLanguage));
  pLingDatabase.load(pLingDbTree.getDynamicDatabasesFolder() + "/" +
                     pLingDatabase.getLanguage()->toStr() + "." +
                     pLingDbTree.getExtDynDatabase());
}


void runTraductions(const LingdbTree& pLingDbTree,
                    const std::string& pMyDataMiningPath,
                    const std::string& pTmpFolder)
{
  const std::string frWikionaryFilename = pMyDataMiningPath + localPathToFrenchWikitionary;
  const std::string enWikionaryFilename = pMyDataMiningPath + localPathToEnglishWikitionary;
  LinguisticIntermediaryDatabase frLingDatabase;
  _loadLingDb(frLingDatabase, pLingDbTree, SemanticLanguageEnum::FRENCH);
  LinguisticIntermediaryDatabase enLingDatabase;
  _loadLingDb(enLingDatabase, pLingDbTree, SemanticLanguageEnum::ENGLISH);

  std::map<LingdbMeaning*, onsem::LingdbSaverOutLinks> tradsFrToEn_FrWiki;
  std::map<LingdbMeaning*, onsem::LingdbSaverOutLinks> tradsEnToFr_FrWiki;
  extractDataFromFrWiki(tradsFrToEn_FrWiki, tradsEnToFr_FrWiki,
                        frLingDatabase, enLingDatabase,
                        frWikionaryFilename);

  std::map<LingdbMeaning*, onsem::LingdbSaverOutLinks> tradsEnToFr_EnWiki;
  std::map<LingdbMeaning*, onsem::LingdbSaverOutLinks> tradsFrToEn_EnWiki;
  extractDataFromEnWiki(tradsEnToFr_EnWiki, tradsFrToEn_EnWiki,
                        enLingDatabase, frLingDatabase,
                        enWikionaryFilename);

  auto resultFolderPath = pTmpFolder + "/wikitionaryExtractor_traductions";
  std::filesystem::create_directory(resultFolderPath);
  const std::string frToEnFilename = resultFolderPath + "/wikionary_fr_to_en.wlks";
  const std::string enToFrFilename = resultFolderPath + "/wikionary_en_to_fr.wlks";

  onsem::LingdbSaverTraductionWriter tradWriter;
  std::cout << "Write file: " << frToEnFilename << std::endl;
  tradWriter.writeTranslations(frToEnFilename,
                               tradsFrToEn_FrWiki, tradsFrToEn_EnWiki,
                               frLingDatabase);

  std::cout << "Write file: " << enToFrFilename << std::endl;
  tradWriter.writeTranslations(enToFrFilename,
                               tradsEnToFr_EnWiki, tradsEnToFr_FrWiki,
                               enLingDatabase);
}


void addComposedWords(const LingdbTree& pLingDbTree,
                      const std::string& pMyDataMiningPath,
                      const std::string& pInputResourcePath)
{
  const std::string frWikionaryFilename = pMyDataMiningPath + localPathToFrenchWikitionary;
  const std::string enWikionaryFilename = pMyDataMiningPath + localPathToEnglishWikitionary;
  LinguisticIntermediaryDatabase frLingDatabase;
  _loadLingDb(frLingDatabase, pLingDbTree, SemanticLanguageEnum::FRENCH);
  LinguisticIntermediaryDatabase enLingDatabase;
  _loadLingDb(enLingDatabase, pLingDbTree, SemanticLanguageEnum::ENGLISH);

  const std::string frOutputFilename = pInputResourcePath + "/french/readonly/frComposedWords.xml";
  const std::string enOutputFilename = pInputResourcePath + "/english/readonly/enComposedWords.xml";

  FrPatternRecognizer frPatternReco;
  onsem::addComposedWords(frWikionaryFilename, frLingDatabase,
                          onsem::AddComposedWords(frPatternReco),
                          onsem::SemanticLanguageEnum::FRENCH, frOutputFilename);

  EnPatternRecognizer enPatternReco;
  onsem::addComposedWords(enWikionaryFilename, enLingDatabase,
                          onsem::AddComposedWords(enPatternReco),
                          onsem::SemanticLanguageEnum::ENGLISH, enOutputFilename);
}



void addTransitiveVerbs(const LingdbTree& pLingDbTree,
                        const std::string& pMyDataMiningPath,
                        const std::string& pInputResourcePath)
{
  const std::string enWikionaryFilename = pMyDataMiningPath + localPathToEnglishWikitionary;
  std::ifstream wikionaryFile(enWikionaryFilename.c_str(), std::ifstream::in);
  if (!wikionaryFile.is_open())
  {
    std::cerr << "Error: Can't open " << enWikionaryFilename << " file !" << std::endl;
    exit(1);
  }
  LinguisticIntermediaryDatabase enLingDatabase;
  _loadLingDb(enLingDatabase, pLingDbTree, SemanticLanguageEnum::ENGLISH);

  std::set<LingdbMeaning*> transitiveVerbs;
  EnWikiKeyWords enWikiKeyWords;
  EnPatternRecognizer enPatternReco;
  WikiTransitiveVerbExtractor enTransExtractor(enWikiKeyWords, enPatternReco);
  enTransExtractor.extract(transitiveVerbs, wikionaryFile, enLingDatabase);

  const std::string enOutputFilename = pInputResourcePath + "/english/readonly/english_contextinfos_transitiveverb.xml";

  enTransExtractor.writeNewTransitiveVerbs(transitiveVerbs, enOutputFilename);
}


void addConcepts(const LingdbTree& pLingDbTree,
                 const std::string& pMyDataMiningPath,
                 const std::string& pInputResourcePath)
{
  const std::string frWikionaryFilename = pMyDataMiningPath + localPathToFrenchWikitionary;
  std::ifstream frWikionaryFile(frWikionaryFilename.c_str(), std::ifstream::in);
  if (!frWikionaryFile.is_open())
  {
    std::cerr << "Error: Can't open " << frWikionaryFilename << " file !" << std::endl;
    exit(1);
  }

  const std::string frOutputFilename = pInputResourcePath + "/french/readonly/concepts/concepts.xml";

  LinguisticIntermediaryDatabase frLingDatabase;
  _loadLingDb(frLingDatabase, pLingDbTree, SemanticLanguageEnum::FRENCH);

  FRWikiKeyWords frWikiKeyWords;
  FrPatternRecognizer frPatternReco;
  ConceptsExtractor cptsExtractor(frWikiKeyWords, frPatternReco);
  std::map<std::string, std::set<LingdbMeaning*>> conceptToMeanings;
  cptsExtractor.extractDatasFromFile(conceptToMeanings, frWikionaryFile, frLingDatabase);
  cptsExtractor.writeConcepts(frOutputFilename, conceptToMeanings);
}


} // End of namespace wikitionaryExtractor

} // End of namespace onsem
