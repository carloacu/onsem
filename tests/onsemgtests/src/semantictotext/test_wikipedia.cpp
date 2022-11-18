#include "../semanticreasonergtests.hpp"
#include "operators/operator_inform.hpp"
#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/tester/reactOnTexts.hpp>

using namespace onsem;



TEST_F(SemanticReasonerGTests, test_wikipediaTexts)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;

  ONSEM_NOANSWER
      (operator_react
       ("Alexander Boris de Pfeffel Johnson, dit Boris Johnson, né le 19 juin 1964 à New York, est un homme politique britannico-américain, actuel secrétaire d'État des Affaires étrangères et du Commonwealth et du Royaume-Uni.\n"
        "Marroquín est devenu professeur à l'Université de Osma où il a rencontré Mgr García de Loaysa, un conseiller de l'empereur Charles Quint.\n"
        "Emmanuel Macron est un haut fonctionnaire, banquier d'affaires et homme politique français, né à Amiens le 21 décembre 1977.\n"
        "Neymar da Silva Santos Júnior dit Neymar Jr., plus couramment appelé Neymar, né le 5 février 1992 à Mogi das Cruzes, est un footballeur international brésilien évoluant au poste d'attaquant au Paris Saint-Germain, et pour l'équipe nationale du Brésil, dont il est le capitaine depuis 2014.",
        semMem, lingDb));

  operator_mergeAndInform("Pedro Miguel Pauleta, de son vrai nom Pedro Miguel Carreiro Resendes, né le 28 avril 1973 aux Açores, sur l'île de São Miguel (Ponta Delgada), est un ancien footballeur international portugais ayant évolué au poste d'avant-centre.\n"
                          "François Hollande, né le 12 août 1954 à Rouen, en Seine-Maritime, est un homme d'État français.\n"
                          "Nicolas Sarközy de Nagy-Bocsa, dit Nicolas Sarkozy, né le 28 janvier 1955 à Paris, est un avocat et homme d'État français.\n"
                          "Katheryn Elizabeth Hudson, connue sous son nom de scène Katy Perry, est une chanteuse pop rock, guitariste, compositeur et actrice américaine, née le 25 octobre 1984 à Santa Barbara en Californie. Elle connaît un succès international en 2008 grâce au titre I Kissed a Girl, suivi de deux albums, One of the Boys et Teenage Dream, écoulés à près de 11 millions d'exemplaires.\n"
                          "Zlatan Ibrahimović est un footballeur international suédois qui a aussi la nationalité bosnienne, né le 3 octobre 1981 à Malmö (en Suède).\n"
                          "Sylver est un groupe de dance belge plus particulièrement connu par son titre Turn the Tide. La chanteuse du groupe est Silvy De Bie (née le 4 janvier 1981), le claviériste/compositeur est Wout Van Dessel (né le 19 octobre 1974), et le producteur et co-auteur est Regi Penxten.\n"
                          "Edinson Roberto Cavani Gómez, surnommé El Matador ou encore plus rarement El Botija, né le 14 février 1987 à Salto, est un footballeur international uruguayen qui évolue au poste d'attaquant au Paris Saint-Germain.\n"
                          "Victor Hugo, né le 26 février 1802 à Besançon et mort le 22 mai 1885 à Paris, est un poète, dramaturge et prosateur romantique considéré comme l’un des plus importants écrivains de langue française. Il est aussi une personnalité politique et un intellectuel engagé qui a compté dans l’Histoire du XIXe siècle.\n"
                          "William Henry Gates III, dit « Bill Gates », est un informaticien, un entrepreneur américain, pionnier dans le domaine de la micro informatique, né le 28 octobre 1955 à Seattle, dans l'État de Washington, aux États-Unis.\n"
                          "Barack Obama, né le 4 août 1961 à Honolulu dans l'État d'Hawaï, est un homme d'État américain. Il est l'actuel et le 44e président des États-Unis d'Amérique, élu pour un premier mandat le 4 novembre 2008, et réélu le 6 novembre 2012.\n"
                          "Harvey Wright Cohn (né le 4 décembre 1884 à New York et décédé en juillet 19655 dans le Massachusetts) est un athlète américain spécialiste du demi-fond.\n"
                          "La France métropolitaine possède une grande variété de paysages, entre des plaines agricoles ou boisées, des chaînes de montagnes plus ou moins érodées, des littoraux diversifiés et des vallées mêlant villes et espaces néo-naturels.\n"
                          "Carlos Ray Norris, alias Chuck Norris, est un acteur américain né le 10 mars 1940 à Ryan (Oklahoma). Spécialiste en arts martiaux, il s'est essentiellement consacré au cinéma d'action.\n"
                          "Franck Dubosc est un humoriste et acteur français né le 7 novembre 1963 au Petit-Quevilly en Seine-Maritime.\n"
                          "Christophe Maé, de son vrai nom Christophe Martichon, est un auteur-compositeur-interprète et acteur français né le 16 octobre 1975 à Carpentras dans le département de Vaucluse, en France.\n"
                          "Mariah Carey est une auteur-compositrice-interprète, productrice et actrice américaine, née le 27 mars 1970 à Long Island, dans l'État de New York.\n"
                          "Ellie Goulding (née Elena Jane Goulding le 30 décembre 1986), est une chanteuse, compositrice et guitariste anglaise, qui a su imposer son style musical, l'electropop. Elle devint célèbre après avoir gagné le Critics Choice Award aux BRIT 2010 sur la chaine BBC en 2010. Après avoir signé avec polydor Records en 2009, Ellie a sorti son premier album studio, Lights, en 2010. En France, elle est surtout connue pour son single Starry Eyed. Ce titre a connu un énorme succès sur Internet, particulièrement sur YouTube, et a été le plus remixé après le titre de Ke$ha Tik Tok.\n"
                          "Nelson Rolihlahla Mandela (prononcé en xhosa [xoˈliːɬaɬa manˈdeːla]), dont le nom du clan tribal est « Madiba », né le 18 juillet 1918 à Mvezo (Union d'Afrique du Sud) et mort le 5 décembre 2013 à Johannesburg, est un homme d'État sud-africain ; il a été l'un des dirigeants historiques de la lutte contre le système politique institutionnel de ségrégation raciale (apartheid) avant de devenir président de la République d'Afrique du Sud de 1994 à 1999, à la suite des premières élections nationales non raciales de l'histoire du pays.\n"
                          "Stefani Joanne Angelina Germanotta dite Lady Gaga, née le 28 mars 1986 dans le quartier de Manhattan à New York (États-Unis), est une auteure-compositrice-interprète et actrice américaine.\n"
                          "Elvis Aaron Presley, né le 8 janvier 1935 à Tupelo, Mississippi et mort le 16 août 1977 à Memphis, Tennessee, est un chanteur et acteur américain.",
                          semMem, lingDb);

  operator_mergeAndInform("Fernanda Françoise Raoult, known professionally as Fernande Albany (22 December 1889, Lison – 25 November 1966, Paris), was a French actress in theatre and film.",
                          semMem, lingDb);

  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est barack obama.",
                    operator_react("qui est barack obama", semMem, lingDb));
    std::set<std::string> properNouns;
    linguistics::extractProperNounsThatDoesntHaveAnyOtherGrammaticalTypes(properNouns, "Barack Hussein Obama II", SemanticLanguageEnum::FRENCH, lingDb);
    lingDb.addProperNouns(properNouns);
    ONSEM_ANSWER_EQ("Barack Obama est un homme d'État américain et l'actuel et 44e président des États-Unis d'Amérique élu le 4 novembre 2008 pour un premier mandat et réélu le 6 novembre 2012.",
                    operator_react("qui est barack obama", semMem, lingDb));
  }

  ONSEM_ANSWER_EQ("Victor Hugo est né le 26 février 1802.",
                  operator_react("quand Victor Hugo est né", semMem, lingDb));
  ONSEM_ANSWER_EQ("Victor Hugo est un poète, un dramaturge et un prosateur romantique.",
                  operator_react("qui est Victor Hugo", semMem, lingDb));
  ONSEM_ANSWER_EQ("Victor Hugo est mort le 22 mai 1885.",
                  operator_react("quand Victor Hugo est mort", semMem, lingDb));
  ONSEM_ANSWER_EQ("Harvey Wright Cohn est mort en juillet.",
                  operator_react("quand Harvey Wright Cohn est mort", semMem, lingDb));
  ONSEM_ANSWER_EQ("Franck Dubosc",
                  operator_react("Qui est né le 7 novembre 1963", semMem, lingDb));
  ONSEM_ANSWER_EQ("Mariah Carey est née le 27 mars 1970.",
                  operator_react("quand est née Mariah Carey", semMem, lingDb));
  ONSEM_ANSWER_EQ("Sylver est un groupe de dance belge connu plus particulièrement par son titre Turn the Tide.",
                  operator_react("Sylver c'est quoi", semMem, lingDb));
  ONSEM_ANSWER_EQ("Victor Hugo's birthdate is February 26, 1802.",
                  operator_react("what's Victor Hugo's birthdate?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Mandela est né le 18 juillet 1918.",
                  operator_react("quand Mandela est né", semMem, lingDb));
  ONSEM_ANSWER_EQ("Nelson Rolihlahla Mandela est né le 18 juillet 1918.",
                  operator_react("quand est né Nelson Rolihlahla Mandela", semMem, lingDb));
  ONSEM_ANSWER_EQ("Mandela died in Johannesburg.",
                  operator_react("where Mandela died", semMem, lingDb));
  ONSEM_ANSWER_EQ("Nelson Rolihlahla Mandela",
                  operator_react("qui est mort à Johannesburg", semMem, lingDb));
  ONSEM_ANSWER_EQ("Nelson Rolihlahla Mandela",
                  operator_react("who died in Johannesburg", semMem, lingDb));
  ONSEM_ANSWER_EQ("Lady Gaga est un auteure-compositrice-interprète et une actrice américaine.",
                  operator_react("qui est Lady Gaga", semMem, lingDb));
  ONSEM_ANSWER_EQ("Lady Gaga est né le 28 mars 1986.",
                  operator_react("Lady Gaga est née quand", semMem, lingDb));
  ONSEM_ANSWER_EQ("Fernande Albany était une actrice française.",
                  operator_react("Qui était Fernande Albany", semMem, lingDb));
  ONSEM_ANSWER_EQ("Elvis Presley est un chanteur et un acteur américain.",
                  operator_react("qui est Elvis Presley", semMem, lingDb));
  ONSEM_ANSWER_EQ("Cavani est un footballeur international uruguayen qui évolue au poste d'attaquant Paris Saint-Germain.",
                  operator_react("qui est Cavani", semMem, lingDb));
  ONSEM_ANSWER_EQ("Boris Johnson est un homme politique britannico-américain et secrétaire actuel d'État d'Affaires.",
                  operator_react("qui est Boris Johnson", semMem, lingDb));
  ONSEM_ANSWER_EQ("Neymar Da Silva Santos Júnior, Pedro Miguel Pauleta, Zlatan Ibrahimović et Edinson Roberto Cavani Gómez",
                  operator_react("qui est un footballeur", semMem, lingDb));
  ONSEM_ANSWER_EQ("Macron est né le 21 décembre 1977.",
                  operator_react("Macron est né quand", semMem, lingDb));
  ONSEM_ANSWER_EQ("Macron est un haut fonctionnaire de banquier d'affaires et d'homme politique français né le 21 décembre 1977 à Amiens.",
                  operator_react("Qui est Macron", semMem, lingDb));
  ONSEM_ANSWER_EQ("Neymar est un footballeur international brésilien qui évolue au poste d'attaquant Paris Saint-Germain et à l'équipe nationale du Brésil dont il est le capitaine depuis 2014 et le capitaine.",
                  operator_react("Qui est Neymar", semMem, lingDb));
  ONSEM_ANSWER_EQ("Da Silva Santos est un footballeur international brésilien qui évolue au poste d'attaquant Paris Saint-Germain et à l'équipe nationale du Brésil dont il est le capitaine depuis 2014 et le capitaine.",
                  operator_react("Qui est Da Silva Santos", semMem, lingDb));

  // english
  ONSEM_ANSWER_EQ("Neymar Da Silva Santos Júnior, Pedro Miguel Pauleta, Zlatan Ibrahimović and Edinson Roberto Cavani Gómez",
                  operator_react("Who is a footballer", semMem, lingDb));
  ONSEM_ANSWER_EQ("Neymar's birthdate is February 5, 1992.",
                  operator_react("When is Neymar's birthdate", semMem, lingDb));
  ONSEM_ANSWER_EQ("Neymar's birthdate is February 5, 1992.",
                  operator_react("What is Neymar's birthdate", semMem, lingDb));
  ONSEM_ANSWER_EQ("Neymar's birthday is February 5.",
                  operator_react("When is Neymar's birthday", semMem, lingDb));
  ONSEM_ANSWER_EQ("Neymar's birthday is February 5.",
                  operator_react("What is Neymar's birthday", semMem, lingDb));

  /*
  ONSEM_ANSWER_EQ("aa",
                  operator_react("Which footballer was born in 1987", semMem, lingDb));
  ONSEM_ANSWER_EQ("bb",
                  operator_react("Who is a footballer born in 1987", semMem, lingDb));
                  */

}

