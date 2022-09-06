#include "../semanticreasonergtests.hpp"
#include <onsem/tester/reactOnTexts.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "operators/operator_inform.hpp"

using namespace onsem;


TEST_F(SemanticReasonerGTests, userDisambiguation_wikpedia)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticLanguageEnum frLanguage = SemanticLanguageEnum::FRENCH;
  SemanticLanguageEnum enLanguage = SemanticLanguageEnum::ENGLISH;
  SemanticMemory semMem;

  semMem.memBloc.disableOldContrarySentences = false;
  operator_inform_withAgentNameFilter
      ("Adolphe de Chesnel",
       "Pierre-François-Adolphe de Chesnel, né à Paris en 1791 et mort en 1862, est un polygraphe.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Alexander Prent",
       "Alexander Prent est un footballeur néerlandais né le 25 mai 1983 à Utrecht. Alexander Prent aime un footballeur né le 3 septembre 1988",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Anand Amritraj",
       "Anand Amritraj est le frère des deux joueurs de tennis Vijay Amritraj et le père du joueur Stephen Amritraj.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter(
        "Barack Obama",
        "Barack Hussein Obama II, né le 4 août 1961 à Honolulu (Hawaï), est un homme d'État américain.\n"
        "Paul aime chanter.",
        semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Barack Obama, Sr.",
       "Barack Hussein Obama, né en 1936 et mort d'un accident de la route le 24 novembre 1982, est un économiste travaillant pour le ministère des Finances kényan.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Emilio Nsue",
       "Emilio Nsue López est un footballeur international équato-guinéen, né le 30 septembre 1989 à Palma de Majorque en Espagne. Il évolue actuellement au poste d'attaquant à Middlesbrough.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Evandro Soldati",
       "Evandro Soldati a fait une apparition dans le clip Alejandro de Lady Gaga.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Fabien Barthez",
       "Fabien Barthez, né le 28 juin 1971 à Lavelanet (Ariège), est un ancien footballeur international français évoluant au poste de gardien de but entre 1990 et 2007.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Issa Gouo",
       "Issa Gouo est un footballeur burkinabé né le 9 septembre 1989 à Bobo-Dioulasso. Il évolue au poste de défenseur à l'AS Kaloum.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Jean-Charles Cirilli",
       "Jean-Charles Cirilli est un footballeur français né le 10 septembre 1982 à Saint-Étienne.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Joseph Camus",
       "Joseph is a computer scientist. He likes Barack Obama.",
       semMem, enLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Karim Boudjema",
       // the wrong spelling is on purpose!
       "Karim Boudjema est footballeur né le 8 septembre 1988. Footballeur né le 8 septembre 1988",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Kim Hyun-ki",
       "Hyun Ki KimFichier:Kim Hyun-ki Oslo 2011 (training) 2.jpg Kim Hyun-ki à Holmenkollen en 2011. Kim Hyun-ki (né le 9 février 1983 à Gangneung) est un sauteur à ski sud-coréen. "
       "Il a participé à cinq éditions des Jeux olympiques entre 1998 et 2014..",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Kim Hyun-seok",
       "Kim Hyun-seok est un footballeur sud-coréen né le 5 mai 1967.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Lady Gaga",
       "Stefani Joanne Angelina Germanotta dite Lady Gaga, née le 28 mars 1986 dans le quartier de Manhattan à New York (États-Unis), est une auteure-compositrice-interprète et actrice américaine.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Mesut Özil",
       "Mesut Özil, né le 15 octobre 1988 à Gelsenkirchen, est un footballeur international allemand.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Nuri Şahin",
       "Nuri Kazım Şahin,  né le 5 septembre 1988 à Lüdenscheid en Allemagne, est un footballeur international turc qui évolue au poste de milieu de terrain au Borussia Dortmund.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Santino Marella",
       "Santino Marella est la première et dernière personne à avoir détenu.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Sefyu",
       "je suis Sefyu",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Sébastien Chardonnet",
       "Sébastien Chardonnet (né le 17 octobre 1988 à Paris) est un pilote de rallye français.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Souleymane Oularé",
       "Souleymane Oularé, né le 16 octobre 1972 à Conakry, est un joueur de football international guinéen aujourd'hui retraité",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Takuya Aoki",
       "Takuya Aoki (青木 拓矢, Aoki Takuya) est un footballeur japonais né le 16 septembre 1989. Il évolue au poste de milieu défensif au Urawa Red Diamonds.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Yannick Boli",
       "Yannick Boli (né le 13 janvier 1988 à Saint-Maur-des-Fossés, France) est un footballeur franco-ivoirien. Il est le neveu de Basile et de Roger Boli. Il est par ailleurs le cousin de Kévin Boli.",
       semMem, frLanguage, lingDb);
  operator_inform_withAgentNameFilter
      ("Eminem",
       "Eminem (souvent stylisé EMINƎM), de son vrai nom Marshall Bruce Mathers III, né le 17 octobre 1972 à Saint Joseph dans l'État du Missouri, est un auteur-compositeur-interprète de rap américain, "
       "également producteur et acteur. En plus de sa carrière solo, il est aussi membre du groupe D12 dont il est le cofondateur et compose le duo Bad Meets Evil avec Royce da 5'9\". Eminem est l'un des "
       "artistes ayant vendu le plus d'albums dans l'histoire de l'industrie musicale et est le plus gros vendeur de disques à l'international, de la décennie allant de 1999 à 2009. Il compte plus de 240 "
       "récompenses, dont 15 Grammy Awards et un Oscar. Il compte à lui seul plus de 105 millions d'albums vendus dans le monde, dont 63 millions aux États-Unis (sans compter ses travaux avec D12 et "
       "Bad Meets Evil) et plus de 120 millions de singles, ce qui fait un total de 225 millions d'exemplaires de disques vendus dans le monde. Avec ses travaux en groupe, ses ventes de disques tournent "
       "autour des 240 millions d'exemplaires vendus à travers le monde, ce qui fait de lui le rappeur qui a le plus vendu d'albums dans l'industrie musicale. Il est cité dans de nombreuses listes énumérant "
       "les meilleurs artistes de tous les temps, le magazine Rolling Stone l'ayant même déclaré « Roi du hip-hop ». En incluant ses travaux avec D12 et Bad Meets Evil, il porte dix albums au sommet des "
       "classements américains. Il possède deux disques certifiés diamants (The Marshall Mathers LP et The Eminem Show). Encore inconnu du grand public, Eminem publie son premier album, intitulé Infinite, "
       "en 1996. Le disque est un échec critique et commercial. Il n'obtient une popularité mondiale qu'après la sortie de son deuxième album, The Slim Shady LP en 1999, le premier publié au label "
       "du producteur et rappeur Dr. Dre. Cet album lui vaut son premier Grammy Award, celui du « meilleur album rap ». Il remporte le même trophée pour ses deux albums suivants, The Marshall Mathers LP "
       "et The Eminem Show, ce qui fait de lui le recordman du nombre de victoires consécutives pour ce prix. Le rappeur enchaîne en 2004 avec l'album Encore. Après une retraite forcée de trois ans, Eminem "
       "fait son retour en 2009 avec l'album Relapse, laissant derrière lui ses problèmes de drogue. L'année suivante, il sort son septième album studio, Recovery, qui est un succès planétaire. Il est l'album "
       "le plus vendu de l'année 2010, tout comme The Eminem Show en 2002. Fin 2013, il publie son huitième album studio The Marshall Mathers LP 2 qui lui permet de gagner deux Grammys Awards, un pour "
       "The Monster et le second dans la catégorie de le meilleur album rap de l'année. À sa sortie, l'album fut écoulé à plus de 750 000 exemplaires en une semaine, et se hissa à la première place du "
       "classement Billboard. The Marshall Mathers LP 2 est aussi le second album le plus vendu aux États-Unis bien qu'il soit sorti deux mois avant la fin de l'année . En novembre 2014 Eminem publie Shady XV, "
       "une compilation pour fêter les 15 ans de son label Shady Records. De très nombreux rappeurs de Shady Records sont présents. Le groupe D12 est également présent. Eminem est également un entrepreneur "
       "actif. Il a notamment créé son propre label, Shady Records, une station de radio, Shade 45 et une fondation caritative. Eminem débute également une carrière d'acteur en 2002 avec le rôle de "
       "Jimmy Smith Jr. dans le film du réalisateur Curtis Hanson, 8 Mile. Pour ce rôle aux côtés de Kim Basinger, il obtient l'Oscar de la meilleure chanson originale pour la chanson Lose Yourself. "
       "Il devient ainsi un des premiers artistes hip-hop à remporter ce prix.",
       semMem, frLanguage, lingDb);

  // has to be at the end
  operator_inform_withAgentNameFilter
      ("Hilaire de Chardonnet",
       "Louis Marie Hilaire Bernigaud de Grange, comte de Chardonnet (1er mai 1839 - 11 mars 1924), est un ingénieur scientifique et industriel de Besançon, inventeur de la soie artificielle et fondateur de la Société de la soie Chardonnet.",
       semMem, frLanguage, lingDb);
  semMem.memBloc.disableOldContrarySentences = true;


  ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tComment t'appelles-tu ?\t)",
                    operator_react("comment je m'appelle ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Adolphe de Chesnel et Sébastien Chardonnet",
                  operator_react("qui est né à Paris", semMem, lingDb));
  ONSEM_ANSWER_EQ("Un footballeur né le 3 septembre 1988, Karim Boudjema, Mesut Özil, Nuri Şahin et Sébastien Chardonnet",
                  operator_react("qui est né en 1988", semMem, lingDb));
  ONSEM_ANSWER_EQ("Karim Boudjema, Mesut Özil et Nuri Şahin",
                  operator_react("qui est un footballeur né en 1988", semMem, lingDb));
  ONSEM_ANSWER_EQ("Jean-Charles Cirilli",
                  operator_react("qui est un footballeur français né en septembre", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est le père de Karim Boudjema.",
                          operator_react("qui est le père de Karim Boudjema", semMem, lingDb));
  ONSEM_ANSWER_EQ("Emilio Nsue, Issa Gouo et Takuya Aoki",
                  operator_react("qui est un footballeur né en septembre 1989", semMem, lingDb));
  ONSEM_ANSWER_EQ("Joseph Camus", operator_react("Who is a computer scientist?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Barack Obama",
                  operator_react("qui est un homme d'État américain", semMem, lingDb));
  ONSEM_ANSWER_EQ("Barack Obama Sr",
                  operator_react("qui est un économiste", semMem, lingDb));
  ONSEM_ANSWER_EQ("Barack Obama est un homme d'État américain.",
                  operator_react("qui est Barack Obama", semMem, lingDb));
  ONSEM_ANSWER_EQ("Barack Obama est né le 4 août 1961.",
                  operator_react("quand est né Barack Obama", semMem, lingDb));
  ONSEM_ANSWER_EQ("Barack Obama",
                  operator_react("qui est né le 4 août 1961", semMem, lingDb));
  ONSEM_ANSWER_EQ("Joseph Camus",
                  operator_react("qui aime Barack Obama", semMem, lingDb));
  ONSEM_ANSWER_EQ("Barack Hussein Obama II est un homme d'État américain.",
                  operator_react("qui est Barack Hussein Obama II", semMem, lingDb));
  ONSEM_ANSWER_EQ("Lady Gaga est un auteure-compositrice-interprète et une actrice américaine.",
                  operator_react("qui est Lady Gaga", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Oui, je connais Lady Gaga.",
                    operator_react("Tu connais Lady Gaga", semMem, lingDb));
  ONSEM_ANSWER_EQ("C'est un auteure-compositrice-interprète et une actrice américaine.",
                  operator_react("qui c'est", semMem, lingDb));
  // Check that we don't consider sentences that are not related to the subject
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si Paul aime chanter.",
                          operator_react("Paul aime chanter ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Kim Hyun-seok est née le 5 mai 1967.",
                  operator_react("Il est né quand Kim Hyun-seok", semMem, lingDb));
  ONSEM_ANSWER_EQ("Emilio Nsue, Issa Gouo, Jean-Charles Cirilli, Karim Boudjema, Nuri Şahin et Takuya Aoki",
                  operator_react("qui est un footballeur né en septembre", semMem, lingDb));
  ONSEM_ANSWER_EQ("Mesut \xC3\x96zil et Souleymane Oularé",
                  operator_react("qui est un footballeur né en octobre", semMem, lingDb));
  ONSEM_ANSWER_EQ("Eminem est un auteur-compositeur-interprète de rap américain, producteur, acteur, aussi membre du groupe D12 dont il est le cofondateur "
                  "et dont le duo Bad Meets Evil est composé avec Royce Da de 5 9\" et aussi un entrepreneur actif.",
                  operator_react("Qui est Eminem", semMem, lingDb));
  ONSEM_ANSWER_EQ("Eminem is an American rap interpreter, producer, actor, also member of the D12 group he is the cofondateur and the Bad Meets Evil duo "
                  "is composed with Royce Da 5 9\" and an active also entrepreneur.",
                  operator_react("Who is Eminem", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, userDisambiguation_wikpedia2)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  std::set<std::string> properNouns;
  SemanticLanguageEnum frLanguage = SemanticLanguageEnum::FRENCH;
  SemanticMemory semMem;

  semMem.memBloc.disableOldContrarySentences = false;
  operator_inform_withAgentNameFilter
      ("Barack Obama",
       "Barack Hussein Obama II, communément appelé simplement Barack Obama, né le 4 août 1961 à Honolulu (Hawaï), est un homme d'État américain. Il est l'actuel et le 44e président des États-Unis, élu pour un premier mandat le 4 novembre 2008 et réélu le 6 novembre 2012.",
       semMem, frLanguage, lingDb);

  ONSEM_ANSWER_EQ("Barack Obama est un homme d'État américain et l'actuel et 44e président des États-Unis élu le 4 novembre 2008 pour un premier mandat et réélu le 6 novembre 2012.",
                  operator_react("qui est Barack Obama", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, userDisambiguation_distinguishWellSomePeople)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  SemanticLanguageEnum enLanguage{SemanticLanguageEnum::ENGLISH};

  // "Alexis Denisof" and "Alexis" are considered as the same person
  operator_inform("Alexis Denisof is an actor", semMem, lingDb);
  auto semExpWrapper = operator_inform("Alexis is a computer scientist", semMem, lingDb);
  ONSEM_ANSWER_EQ("Alexis is an actor and a computer scientist.",
                  operator_react("Who is Alexis", semMem, lingDb));
  semMem.memBloc.removeExpression(*semExpWrapper, lingDb, nullptr);
  ONSEM_ANSWER_EQ("Alexis is an actor.",
                  operator_react("Who is Alexis", semMem, lingDb));

  // Notify that "Alexis", "Obi", "Alexou Lamotte" refer to a single person
  const std::vector<std::string> equNames{"Alexis", "Obi", "Alexou Lamotte"};
  for (const auto& currName : equNames)
  {
    std::set<std::string> properNouns;
    linguistics::extractProperNounsThatDoesntHaveAnyOtherGrammaticalTypes(properNouns, currName, enLanguage, lingDb);
    lingDb.addProperNouns(properNouns);
  }
  semMem.memBloc.addASetOfEquivalentNames(equNames, enLanguage, lingDb);

  // Test that "Alexis", "Obi", "Alexou Lamotte" refer to a specific person different from "Alexis Denisof"
  operator_inform("Alexis is a computer scientist", semMem, lingDb);
  ONSEM_ANSWER_EQ("Alexis is a computer scientist.",
                  operator_react("Who is Alexis", semMem, lingDb));
  SemanticMemory semMem2;
  semMem2.memBloc.subBlockPtr = &semMem.memBloc;
  ONSEM_ANSWER_EQ("Obi is a computer scientist.",
                  operator_react("Who is Obi", semMem, lingDb));
  ONSEM_ANSWER_EQ("Obi is a computer scientist.",
                  operator_react("Who is obi", semMem, lingDb));

  static const std::string memoryFilename("distinguishWellSomePeople.smb");
  semMem.memBloc.writeInBinaryFile(lingDb, memoryFilename, 3000000000);

  SemanticMemory semMemBinary;
  semMemBinary.memBloc.loadBinaryFile(memoryFilename);
  ONSEM_ANSWER_EQ("Alexou Lamotte is a computer scientist.",
                  operator_react("Who is alexou lamotte", semMemBinary, lingDb));
  ONSEM_ANSWER_EQ("Alexou Lamotte is a computer scientist.",
                  operator_react("Who is Alexou Lamotte", semMemBinary, lingDb));
}
