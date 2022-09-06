#include <gtest/gtest.h>
#include <boost/property_tree/info_parser.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/serialization.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "../semanticcontroller/operators/operator_answer.hpp"
#include "../semanticcontroller/operators/operator_check.hpp"
#include "../semanticcontroller/operators/operator_inform.hpp"
#include "../semanticcontroller/operators/operator_resolveCommand.hpp"
#include "../semanticreasonergtests.hpp"


using namespace onsem;

namespace
{

const SemanticGenericGrounding _getObjectGenGrd(const std::string& pSentence,
                                                const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExp =
      converter::textToSemExp(pSentence,
                              TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                    SemanticAgentGrounding::me,
                                                    SemanticLanguageEnum::UNKNOWN),
                              pLingDb);
  const GroundedExpression* verbGrdExp = semExp->getGrdExpPtr();
  if (verbGrdExp != nullptr)
  {
    auto itObject = verbGrdExp->children.find(GrammaticalType::OBJECT);
    if (itObject != verbGrdExp->children.end())
    {
      const GroundedExpression* objectGrdExp = itObject->second->getGrdExpPtr();
      if (objectGrdExp != nullptr)
      {
        const SemanticGenericGrounding* objectGenGrd = objectGrdExp->grounding().getGenericGroundingPtr();
        if (objectGenGrd != nullptr)
          return *objectGenGrd;
      }
    }
  }
  return SemanticGenericGrounding();
}


const std::map<std::string, char> _getRootConcepts(const std::string& pSentence,
                                                   const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExp =
      converter::textToSemExp(pSentence,
                              TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                    SemanticAgentGrounding::me,
                                                    SemanticLanguageEnum::UNKNOWN),
                              pLingDb);
  const GroundedExpression* verbGrdExp = semExp->getGrdExpPtr();
  if (verbGrdExp != nullptr)
    return verbGrdExp->grounding().concepts;
  return std::map<std::string, char>();
}



std::string _getTranslation(const std::string& pSentence,
                            const linguistics::LinguisticDatabase& pLingDb,
                            SemanticLanguageEnum pOutLanguage)
{
  auto semExp =
      converter::textToSemExp(pSentence,
                              TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                    SemanticAgentGrounding::me,
                                                    SemanticLanguageEnum::UNKNOWN),
                              pLingDb);

  std::string res;
  SemanticMemory semanticMemory;
  converter::semExpToText(res, std::move(semExp),
                          TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                SemanticAgentGrounding::me,
                                                pOutLanguage),
                          true, semanticMemory, pLingDb, nullptr);
  return res;
}


void _getLoadedNewLingDb(linguistics::LinguisticDatabase& pLoadedLingDb,
                         const linguistics::LinguisticDatabase& pLingDb)
{
  boost::property_tree::ptree propTree;
  serialization::saveLingDatabase(propTree, pLingDb);
  serialization::loadLingDatabase(propTree, pLoadedLingDb);
  boost::property_tree::ptree propTreeAfterLoad;
  serialization::saveLingDatabase(propTreeAfterLoad, pLoadedLingDb);
  if (propTree != propTreeAfterLoad)
  {
    std::stringstream ss;
    boost::property_tree::info_parser::write_info(ss, propTree);
    std::stringstream ssAfterLoad;
    boost::property_tree::info_parser::write_info(ssAfterLoad, propTreeAfterLoad);
    std::cout << "============================= FIRST OUTPUT =================="  << std::endl;
    std::cout << ss.str()  << std::endl;
    std::cout << "============================= SECOND OUTPUT =================="  << std::endl;
    std::cout << ssAfterLoad.str()  << std::endl;
    std::cout << "============================= END OUTPUT =================="  << std::endl;
    ASSERT_TRUE(false);
  }
}

}


TEST_F(SemanticReasonerGTests, addNewWordsAndConcepts)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticWord newWord;
  newWord.language = SemanticLanguageEnum::FRENCH;
  newWord.lemma = "nouvMot en français";
  newWord.partOfSpeech = PartOfSpeech::NOUN;

  const std::string likeSentence = "j'aime " + newWord.lemma;
  // add a word
  {
    const SemanticGenericGrounding objectGenGrd =
        _getObjectGenGrd(likeSentence, lingDb);
    ASSERT_NE(newWord, objectGenGrd.word);
    ASSERT_TRUE(objectGenGrd.concepts.empty());
  }
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"french\">\n"
       << "  <word lemma=\"nouvMot en français\" pos=\"noun\">\n"
       << "    <inflectedWord/>\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  {
    const SemanticGenericGrounding objectGenGrd =
        _getObjectGenGrd(likeSentence, lingDb);
    ASSERT_EQ(newWord, objectGenGrd.word);
    ASSERT_TRUE(objectGenGrd.concepts.empty());
  }

  // add concept to a word
  const std::string dynConceptName = "newDynamicConceptForTest";
  linguistics::WordAssociatedInfos wordAssocInfos;
  wordAssocInfos.concepts.emplace(dynConceptName, 4);
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"french\">\n"
       << "  <concept name=\"newDynamicConceptForTest\">\n"
       << "    <word lemma=\"nouvMot en français\" pos=\"noun\"/>\n"
       << "  </concept>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  {
    const SemanticGenericGrounding objectGenGrd =
        _getObjectGenGrd(likeSentence, lingDb);
    ASSERT_EQ(newWord, objectGenGrd.word);
    ASSERT_FALSE(objectGenGrd.concepts.empty());
    auto itConcept = objectGenGrd.concepts.find(dynConceptName);
    ASSERT_NE(objectGenGrd.concepts.end(), itConcept);
    ASSERT_EQ(dynConceptName, itConcept->first);
  }
  lingDb.reset();
  {
    const SemanticGenericGrounding objectGenGrd =
        _getObjectGenGrd(likeSentence, lingDb);
    ASSERT_NE(newWord, objectGenGrd.word);
    ASSERT_TRUE(objectGenGrd.concepts.empty());
  }

  // add a concept to a word that already have a concept
  const std::string beVerbSentence = "je suis un homme";
  std::size_t beVerbNbOfCocepts;
  {
    const std::map<std::string, char> concepts =
        _getRootConcepts(beVerbSentence, lingDb);
    beVerbNbOfCocepts = concepts.size();
    ASSERT_NE(0u, concepts.count("verb_equal_be"));
  }
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"french\">\n"
       << "  <concept name=\"newDynamicConceptForTest\">\n"
       << "    <word lemma=\"être\" pos=\"verb\"/>\n"
       << "  </concept>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  {
    const std::map<std::string, char> concepts =
        _getRootConcepts(beVerbSentence, lingDb);
    ASSERT_NE(0u, concepts.count("verb_equal_be"));
    ASSERT_NE(0u, concepts.count(dynConceptName));
    ASSERT_EQ(beVerbNbOfCocepts + 1, concepts.size());
  }

  // test the serialization of the linguistic database
  {
    auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
    linguistics::LinguisticDatabase lingDbLoaded(iStreams.linguisticDatabaseStreams);
    iStreams.close();
    _getLoadedNewLingDb(lingDbLoaded, lingDb);
    const std::map<std::string, char> concepts =
        _getRootConcepts(beVerbSentence, lingDb);
    ASSERT_NE(concepts.end(), concepts.find(dynConceptName));
  }
}


TEST_F(SemanticReasonerGTests, addNewTranslations)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticWord newFrWord;
  newFrWord.language = SemanticLanguageEnum::FRENCH;
  newFrWord.lemma = "nouvMot en français";
  newFrWord.partOfSpeech = PartOfSpeech::NOUN;

  SemanticWord newEnWord;
  newEnWord.language = SemanticLanguageEnum::ENGLISH;
  newEnWord.lemma = "newWord in english";
  newEnWord.partOfSpeech = PartOfSpeech::NOUN;

  {
    std::stringstream ss;
    ss << "<dictionary_concept>\n"
       << "  <concept name=\"newDynamicConceptTestedForTranslations\">\n"
       << "    <opposite_concept name=\"oppositeConceptForTests\"/>\n"
       << "  </concept>\n"
       << "</dictionary_concept>";
    lingDb.addDynamicContent(ss);
  }

  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"french\">\n"
       << "  <word lemma=\"nouvMot en français\" pos=\"noun\">\n"
       << "    <inflectedWord/>\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"english\">\n"
       << "  <word lemma=\"newWord in english\" pos=\"noun\">\n"
       << "    <inflectedWord/>\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  ASSERT_EQ("Paul is a nouvMot en français.",
            _getTranslation("Paul est un nouvMot en français", lingDb,
                           SemanticLanguageEnum::ENGLISH));

  // translation link with words that have a concept in common
  const std::string dynConceptName = "newDynamicConceptTestedForTranslations";
  {
    {
      std::stringstream ss;
      ss << "<dictionary_modification language=\"french\">\n"
         << "  <concept name=\"newDynamicConceptTestedForTranslations\">\n"
         << "    <word lemma=\"nouvMot en français\" pos=\"noun\" />\n"
         << "  </concept>\n"
         << "</dictionary_modification>";
      lingDb.addDynamicContent(ss);
    }
    {
      std::stringstream ss;
      ss << "<dictionary_modification language=\"english\">\n"
         << "  <concept name=\"newDynamicConceptTestedForTranslations\">\n"
         << "    <word lemma=\"newWord in english\" pos=\"noun\" />\n"
         << "  </concept>\n"
         << "</dictionary_modification>";
      lingDb.addDynamicContent(ss);
    }
    ASSERT_EQ("Paul is a newWord in english.",
              _getTranslation("Paul est un nouvMot en français", lingDb,
                              SemanticLanguageEnum::ENGLISH));
  }

  // notify a translation
  {
    std::stringstream ss;
    ss << "<dictionary_translation from_language=\"french\" to_language=\"english\">\n"
       << "  <from_word lemma=\"nouvMot en français\" pos=\"noun\">\n"
       << "    <to_word lemma=\"better english translation\" pos=\"noun\" />\n"
       << "  </from_word>\n"
       << "</dictionary_translation>";
    lingDb.addDynamicContent(ss);
  }
  ASSERT_EQ("Paul is a better english translation.",
            _getTranslation("Paul est un nouvMot en français", lingDb,
                           SemanticLanguageEnum::ENGLISH));

  // can link to answer questions, some words that have a concept in common
  SemanticMemory userSemExp;
  operator_mergeAndInform("Paul regarde un nouvMot en français", userSemExp, lingDb);
  ONSEM_TRUE(operator_check("Does Paul look at a newWord in english", userSemExp, lingDb));
  ONSEM_ANSWER_EQ("Paul looks at a better english translation.",
                  operator_answer("What does Paul look at", userSemExp, lingDb));

  // test we handle the opposite concepts
  {
    {
      std::stringstream ss;
      ss << "<dictionary_modification language=\"english\">\n"
         << "  <word lemma=\"invert of newWord in english\" pos=\"noun\">\n"
         << "    <inflectedWord/>\n"
         << "  </word>\n"
         << "</dictionary_modification>";
      lingDb.addDynamicContent(ss);
    }
    ONSEM_UNKNOWN(operator_check("Does Paul look at invert of newWord in english", userSemExp, lingDb));
    {
      std::stringstream ss;
      ss << "<dictionary_modification language=\"english\">\n"
         << "  <concept name=\"oppositeConceptForTests\">\n"
         << "    <word lemma=\"invert of newWord in english\" pos=\"noun\" />\n"
         << "  </concept>\n"
         << "</dictionary_modification>";
      lingDb.addDynamicContent(ss);
    }
    ONSEM_FALSE(operator_check("Does Paul look at invert of newWord in english", userSemExp, lingDb));
  }

  // translate an existing word
  {
    {
      std::stringstream ss;
      ss << "<dictionary_translation from_language=\"french\" to_language=\"english\">\n"
         << "  <from_word lemma=\"cheval\" pos=\"noun\">\n"
         << "    <to_word lemma=\"horseNewWord\" pos=\"noun\" />\n"
         << "  </from_word>\n"
         << "</dictionary_translation>";
      lingDb.addDynamicContent(ss);
    }
    operator_mergeAndInform("Paul a peint un cheval", userSemExp, lingDb);
    ONSEM_ANSWER_EQ("Paul painted a horseNewWord.",
                    operator_answer("What did Paul paint", userSemExp, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, addNewInflectionsToAnExistingWord)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"french\">\n"
       << "  <word lemma=\"charisme\" pos=\"noun\">\n"
       << "    <inflectedWord />\n"
       << "    <inflectedWord str=\"Charisme\" inflections=\"ms\" />\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }

  const std::string sentence1Str = "Paul a le charisme pour manager cette équipe";
  const std::string sentence2Str = "Paul a le Charisme pour manager cette équipe";
  ONSEM_UNKNOWN(operator_check(sentence1Str, semMem, lingDb));
  ONSEM_UNKNOWN(operator_check(sentence2Str, semMem, lingDb));
  operator_inform(sentence1Str, semMem, lingDb);
  ONSEM_TRUE(operator_check(sentence1Str, semMem, lingDb));
  ONSEM_TRUE(operator_check(sentence2Str, semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, addNewAConceptToAnExistingWord)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;

  // link by the concepts with an existing word
  ONSEM_UNKNOWN(operator_check("Does Paul like the newRed ?", semMem, lingDb));

  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"english\">\n"
       << "  <word lemma=\"newRed\" pos=\"noun\">\n"
       << "    <inflectedWord />\n"
       << "    <inflectedWord str=\"r e d\" inflections=\"ms\" />\n"
       << "  </word>\n"
       << "  <concept name=\"myRed\">\n"
       << "    <word lemma=\"newRed\" pos=\"noun\" />\n"
       << "  </concept>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"french\">\n"
       << "  <concept name=\"myRed\">\n"
       << "    <word lemma=\"rouge\" pos=\"noun\" />\n"
       << "  </concept>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }

  operator_mergeAndInform("Paul aime le rouge", semMem, lingDb);
  operator_mergeAndInform("r e d is a nice color", semMem, lingDb);
  ONSEM_TRUE(operator_check("Does Paul like the newRed ?", semMem, lingDb));
  ONSEM_TRUE(operator_check("r e d is a nice color", semMem, lingDb));

  // test the serialization of the linguistic database
  {
    auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
    linguistics::LinguisticDatabase lingDbLoaded(iStreams.linguisticDatabaseStreams);
    iStreams.close();
    ONSEM_UNKNOWN(operator_check("Does Paul like the newRed ?", semMem, lingDbLoaded));
    ONSEM_UNKNOWN(operator_check("r e d is a nice color", semMem, lingDbLoaded));
    _getLoadedNewLingDb(lingDbLoaded, lingDb);
    ONSEM_TRUE(operator_check("Does Paul like the newRed ?", semMem, lingDbLoaded));
    ONSEM_TRUE(operator_check("r e d is a nice color", semMem, lingDbLoaded));
  }
}


TEST_F(SemanticReasonerGTests, considerAuthorNameAsInterjection)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  EXPECT_EQ("", operator_resolveCommand("n5 say yes", semMem, lingDb));
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"english\">\n"
       << "  <word lemma=\"N5F\" pos=\"interjection\">\n"
       << "    <inflectedWord str=\"n5\" />\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  EXPECT_EQ("Yes", operator_resolveCommand("n5 say yes", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, considerInflectionInReformulation)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  EXPECT_EQ("", operator_resolveCommand("n5 say yes", semMem, lingDb));
  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"french\">\n"
       << "  <word lemma=\"Test_bla_bla\" pos=\"noun\">\n"
       << "    <inflectedWord inflections=\"ms\" />\n"
       << "    <inflectedWord str=\"test_bla_bla\" inflections=\"ms\" />\n"
       << "    <inflectedWord str=\"test_bla_blaS\" inflections=\"mp\" />\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  EXPECT_EQ("J'ai 4 test_bla_blaS.", _getTranslation("J'ai 4 test_bla_blaS", lingDb, SemanticLanguageEnum::FRENCH));
}



TEST_F(SemanticReasonerGTests, considerContextInfos)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"english\">\n"
       << "  <word lemma=\"Test_bla_bla\" pos=\"noun\">\n"
       << "    <inflectedWord inflections=\"ms\" />\n"
       << "    <inflectedWord str=\"test_bla_bla\" inflections=\"ms\" />\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  EXPECT_EQ("I like Dede test_bla_bla.", _getTranslation("I like test_bla_bla Dede", lingDb, SemanticLanguageEnum::ENGLISH));

  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"english\">\n"
       << "  <word lemma=\"Test_bla_bla\" pos=\"noun\">\n"
       << "    <contextInfo val=\"canBeBeforeNoun\" />\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  EXPECT_EQ("I like test_bla_bla Dede.", _getTranslation("I like test_bla_bla Dede", lingDb, SemanticLanguageEnum::ENGLISH));
}



TEST_F(SemanticReasonerGTests, considerInflectedWordFrequency)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"english\">\n"
       << "  <word lemma=\"Test_bla_bla1\" pos=\"noun\">\n"
       << "    <inflectedWord inflections=\"ms\" frequency=\"3\" />\n"
       << "    <inflectedWord str=\"test_bla_bla1\" inflections=\"ms\" />\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  EXPECT_EQ("I like Dede Test_bla_bla1.", _getTranslation("I like Dede test_bla_bla1", lingDb, SemanticLanguageEnum::ENGLISH));

  {
    std::stringstream ss;
    ss << "<dictionary_modification language=\"english\">\n"
       << "  <word lemma=\"Test_bla_bla2\" pos=\"noun\">\n"
       << "    <inflectedWord inflections=\"ms\" />\n"
       << "    <inflectedWord str=\"test_bla_bla2\" inflections=\"ms\" frequency=\"3\" />\n"
       << "  </word>\n"
       << "</dictionary_modification>";
    lingDb.addDynamicContent(ss);
  }
  EXPECT_EQ("I like Dede test_bla_bla2.", _getTranslation("I like Dede test_bla_bla2", lingDb, SemanticLanguageEnum::ENGLISH));
}




TEST_F(SemanticReasonerGTests, removeAWord)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;

  auto& commonDico = lingDb.langToSpec[SemanticLanguageEnum::UNKNOWN].lingDico;
  SemanticWord word;
  word.lemma = "Paul";
  word.partOfSpeech = PartOfSpeech::PROPER_NOUN;

  {
    std::list<linguistics::InflectedWord> infosGram;
    commonDico.getGramPossibilities(infosGram, word.lemma, 0, word.lemma.size());
    EXPECT_EQ(1, infosGram.size());
  }

  commonDico.removeAWord(word);

  {
    std::list<linguistics::InflectedWord> infosGram;
    commonDico.getGramPossibilities(infosGram, word.lemma, 0, word.lemma.size());
    EXPECT_EQ(0, infosGram.size());
  }
}
