#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/tool/peoplefiller.hpp>
#include <onsem/semantictotext/type/reactionoptions.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "operators/operator_inform.hpp"
#include "triggers/triggers_add.hpp"
#include "triggers/triggers_match.hpp"


using namespace onsem;


namespace
{
const std::string _resourceLabel = "resLabel";

UniqueSemanticExpression _idToSemExp(const std::string& pId,
                                     SemanticLanguageEnum pLanguage)
{
  auto res = std::make_unique<MetadataExpression>(
        std::make_unique<GroundedExpression>(
          std::make_unique<SemanticResourceGrounding>(_resourceLabel, pLanguage, pId)));
  res->references.push_back(pId);
  return std::move(res);
}

std::shared_ptr<ExpressionWithLinks> _inform(
    const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::list<std::string>& pReferences)
{
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  SemanticLanguageEnum::UNKNOWN);
  inContext.setUsAsEverybody();
  inContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{"hc_resource_html"});

  auto agentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>();
  return operator_inform_withTextProc(pText, pSemanticMemory, pLingDb, pReferences,
                                      nullptr, inContext);
}

}


TEST_F(SemanticReasonerGTests, test_bigMemory)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  auto language = SemanticLanguageEnum::FRENCH;
  SemanticMemory semMem;

  const std::string peopleXmlFilename = peoplePath + "/people_fr.xml";
  std::ifstream peopleXmlFile(peopleXmlFilename, std::ifstream::in);
  if (!peopleXmlFile.is_open())
    throw std::runtime_error("Can't open " + peopleXmlFilename + " file !");
  SemanticMemoryBlock peopledatabase;
  peopleFiller::addPeople(peopledatabase, peopleXmlFile, language, lingDb);
  semMem.memBloc.subBlockPtr = &peopledatabase;

  const std::string textFilename = corpusPath + "/triggerAndTexts/triggersAndTextsCorpus.txt";
  std::ifstream triggersAndTextsCorpusFile(textFilename, std::ifstream::in);
  if (!triggersAndTextsCorpusFile.is_open())
    throw std::runtime_error("Can't open " + textFilename + " file !");

  std::map<std::string, std::string> triggersToReferenceOfAnswer;

  std::string currentLabel;
  std::string currentId;
  std::string currentText;

  auto fillCurrentInfos = [&]() {
    if (currentLabel == "trigger")
    {
      triggers_addToSemExpAnswer(currentText, _idToSemExp(currentId, language), semMem, lingDb, language);
      triggersToReferenceOfAnswer.emplace(currentText, currentId);
    }
    if (currentLabel == "inform" || currentLabel == "message")
      _inform(currentText, semMem, lingDb, {currentId});
  };

  std::string line;
  while (getline(triggersAndTextsCorpusFile, line))
  {
    if (line.empty())
      continue;
    if (line[0] == '#')
    {
      auto endOfLabel = line.find_first_of('#', 1);
      if (endOfLabel == std::string::npos)
      {
        std::cerr << "End of label not found for line: " << line << std::endl;
        continue;
      }

      auto label = line.substr(1, endOfLabel - 1);
      if (label == "trigger" || label == "inform" || label == "message")
      {
        auto beginOfId = endOfLabel + 1;
        auto endOfId = line.find_first_of('#', beginOfId);
        if (endOfId == std::string::npos)
        {
          std::cerr << "End of id not found for line: " << line << std::endl;
          continue;
        }
        auto id = line.substr(beginOfId, endOfId - beginOfId);

        fillCurrentInfos();

        currentLabel = label;
        currentId = id;
        auto beginOfText = endOfId + 1;
        currentText = line.substr(beginOfText, line.size() - beginOfText);
        continue;
      }
    }

    currentText += "\n" + line;
  }
  triggersAndTextsCorpusFile.close();
  fillCurrentInfos();

  std::cout << "nbOfKnowledges: " << semMem.memBloc.nbOfKnowledges() << std::endl;

  ReactionOptions reactionOptionsForTriggerTests;
  reactionOptionsForTriggerTests.canAnswerWithAllTheTriggers = true;
  for (const auto& currTrigger : triggersToReferenceOfAnswer)
  {
    auto answer = triggers_match(currTrigger.first, semMem, lingDb, language, &reactionOptionsForTriggerTests);
    auto refIt = std::find(answer.references.begin(), answer.references.end(), currTrigger.second);
    if (refIt == answer.references.end())
    {
      std::string refStrs = "[";
      for (auto& currRef : answer.references)
      {
        if (refStrs != "[")
          refStrs += ", ";
        refStrs += currRef;
      }
      refStrs += "]";
      std::cerr << "Error in trigger: \"" << currTrigger.first << "\" expected ref: \"" << currTrigger.second << "\" get refs: " << refStrs
                << " asnwer: \"" << answer.answer << "\"" << std::endl;
      EXPECT_TRUE(false);
    }
  }

  ReactionOptions reactionOptions;
  reactionOptions.canReactToANoun = true;
  reactionOptions.canSayOkToAnAffirmation = false;
  reactionOptions.reactWithTextAndResource = true;
  ONSEM_ANSWER_EQ("\\resLabel=#fr_FR#c-est-quoi-les-limbes\\",
                  operator_react("C'est quoi les limbes ?", semMem, lingDb, language, &reactionOptions));


  /*
  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "(\t\\resLabel=#fr_FR#qu-est-ce-que-c-est-le-saint-esprit\\\tTHEN\tLe Saint-Esprit est cet Amour échangé le Père et le Fils, la source d'énergie de Dieu et cela.\t)",
        "\"qu-est-ce-que-c-est-le-saint-esprit\","
        "\"l-esprit-saint-est-une-des-trois-personnes-de-la-sainte-trinite-l-esprit-saint-est-un-seul-dieu-avec-le-pere-et-le-fils-il-est-l-amour-mutuel-le-don-total-echange-entre-le-pere-et-le-fils-le-pere-engendre-le-fils-dans-le-feu-de-l-amour-il-se-donne-tout-entier-a-lui-dans-l-amour-le-fils-se-recoit-du-pere-il-se-redonne-tout-entier-au-pere-dans-le-meme-amour-que-celui-qu-il-a-recu-cet-amour-echange-entre-le-pere-et-le-fils-c-est-l-esprit-saint\","
        "\"le-fondateur-de-ce-peuple-est-dieu-le-pere-celui-qui-le-dirige-est-jesus-christ-sa-source-d-energie-est-l-esprit-saint-la-porte-d-entree-du-peuple-de-dieu-est-le-bapteme-sa-dignite-est-la-liberte-des-enfants-de-dieu-sa-loi-est-l-amour-quand-ce-peuple-reste-fidele-a-dieu-et-cherche-d-abord-le-royaume-de-dieu-il-transforme-le-monde\","
        "\"deja-dans-l-ancien-testament-le-peuple-de-dieu-attendait-l-effusion-la-venue-de-l-esprit-saint-sur-le-messie-jesus-vecut-durant-toute-sa-vie-dans-un-esprit-tout-particulier-d-amour-et-de-communion-parfaite-avec-son-pere-du-ciel-cet-esprit-de-jesus-etait-l-esprit-saint-que-le-peuple-d-israel-desirait-et-ce-fut-ce-meme-esprit-que-jesus-promit-a-ses-disciples-ce-meme-esprit-qui-descendit-sur-les-disciples-cinquante-jours-apres-paques-le-jour-de-la-pentecote-et-c-est-encore-cet-esprit-saint-de-jesus-qui-descend-sur-tous-ceux-qui-recoivent-le-sacrement-de-la-confirmation\"",
        operator_react("C'est quoi l'Esprit Saint ?", semMem, lingDb, language, &reactionOptions));
*/
  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "Oui, Jésus est ressuscité.",
        "\"jesus-est-ressuscite\"",
        operator_react("Jésus est-il ressuscité ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "Oui, Dieu est omniscient.",
        "\"dieu-est-omniscient\"",
        operator_react("Dieu est-il omniscient ?", semMem, lingDb, language, &reactionOptions));
  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "Oui, Jésus est omniscient.",
        "\"dieu-est-omniscient\"",
        operator_react("Jésus est-il omniscient ?", semMem, lingDb, language, &reactionOptions));
  /*
  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "Oui, Jésus est omniscient.",
        "\"dieu-est-omniscient\"",
        operator_react("L'Esprit Saint est-il omniscient ?", semMem, lingDb, language, &reactionOptions));
  */

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "(\t\\resLabel=#fr_FR#jesus-est-il-dieu\\\tTHEN\tOui, Jésus est Dieu.\t)",
        "\"jesus-est-il-dieu\", \"jesus-est-dieu\"",
        operator_react("Jésus est-il Dieu ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#l-immaculee-conception-c-est-dans-la-bible\\",
        "\"l-immaculee-conception-c-est-dans-la-bible\"",
        operator_react("L'immaculée conception, c'est dans la Bible ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#que-dit-la-bible-sur-le-couple\\",
        "\"que-dit-la-bible-sur-le-couple\"",
        operator_react("Que dit la Bible sur le couple ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#pourquoi-certaines-personnes-vivent-une-vie-consacree-alors-que-d-autres-se-marient\\",
        "\"pourquoi-certaines-personnes-vivent-une-vie-consacree-alors-que-d-autres-se-marient\"",
        operator_react("Pourquoi certaines personnes vivent une vie consacrée alors que d'autres se marient ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#martin-luther-est-un-fondateur-un-reformateur-un-homme-de-dieu-ou-un-deformateur\\",
        "\"martin-luther-est-un-fondateur-un-reformateur-un-homme-de-dieu-ou-un-deformateur\"",
        operator_react("Martin Luther est un fondateur, un réformateur, un homme de Dieu ou un déformateur....?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "(\t\\resLabel=#fr_FR#la-resurrection-du-christ-simple-legende-ou-fait-historique\\\tTHEN\tLa simple de légende ou de fait historique résurrection de Christ est Pâques.\t)",
        "\"la-resurrection-du-christ-simple-legende-ou-fait-historique\", \"paques-c-est-la-commemoration-de-la-resurrection-de-jesus\"",
        operator_react("La résurrection du Christ, simple légende ou fait historique ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#comment-faire-un-deuil\\",
        "\"comment-faire-un-deuil\"",
        operator_react("Comment faire un deuil ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#qu-est-ce-que-le-dimanche-du-gaudete\\",
        "\"qu-est-ce-que-le-dimanche-du-gaudete\"",
        operator_react("Qu'est-ce que le dimanche du Gaudete ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#qu-est-ce-que-moise-a-fait\\",
        "\"qu-est-ce-que-moise-a-fait\"",
        operator_react("Qu'est-ce que Moïse a fait ?", semMem, lingDb, language, &reactionOptions));


  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "\\resLabel=#fr_FR#qui-est-moise\\",
        "\"qui-est-moise\"",
        operator_react("Qui est Moïse ?", semMem, lingDb, language, &reactionOptions));

  ONSEM_ANSWER_WITH_REFERENCES_EQ(
        "(\t\\resLabel=#fr_FR#qui-est-saint-joseph\\\tTHEN\tSaint Joseph est le p\xC3\xA8re adoptif de J\xC3\xA9sus.\t)",
        "\"qui-est-saint-joseph\", \"saint-joseph-est-le-pere-adoptif-de-jesus\"",
        operator_react("Qui est Saint Joseph ?", semMem, lingDb, language, &reactionOptions));
}

