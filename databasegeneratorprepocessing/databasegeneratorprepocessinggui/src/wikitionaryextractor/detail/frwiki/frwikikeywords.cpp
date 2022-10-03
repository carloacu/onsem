#include "frwikikeywords.hpp"
#include <iostream>

namespace onsem
{

FRWikiKeyWords::FRWikiKeyWords()
  : WikiKeyWords(),
    fGramStrToGraEnum()
{
  fGramStrToGraEnum["adverbe"].emplace_back(PartOfSpeech::ADVERB);
  fGramStrToGraEnum["adverbe interrogatif"].emplace_back(PartOfSpeech::ADVERB);
  fGramStrToGraEnum["adverbe relatif"].emplace_back(PartOfSpeech::ADVERB);

  fGramStrToGraEnum["adjectif"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adjectif numéral"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adjectif numéral"].emplace_back(PartOfSpeech::NOUN);
  fGramStrToGraEnum["adjectif possessif"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adjectif indéfini"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adjectif démonstratif"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adj"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adjectif interrogatif"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adjectif exclamatif"].emplace_back(PartOfSpeech::ADJECTIVE);
  fGramStrToGraEnum["adjectif exclamatif"].emplace_back(PartOfSpeech::INTERJECTION);

  fGramStrToGraEnum["article défini"].emplace_back(PartOfSpeech::DETERMINER);
  fGramStrToGraEnum["article indéfini"].emplace_back(PartOfSpeech::DETERMINER);
  fGramStrToGraEnum["article partitif"].emplace_back(PartOfSpeech::DETERMINER);
  fGramStrToGraEnum["article défini"].emplace_back(PartOfSpeech::PARTITIVE);
  fGramStrToGraEnum["article indéfini"].emplace_back(PartOfSpeech::PARTITIVE);
  fGramStrToGraEnum["article partitif"].emplace_back(PartOfSpeech::PARTITIVE);

  fGramStrToGraEnum["conjonction de coordination"].emplace_back(PartOfSpeech::CONJUNCTIVE);
  fGramStrToGraEnum["conjonction"].emplace_back(PartOfSpeech::CONJUNCTIVE);
  fGramStrToGraEnum["conjonction"].emplace_back(PartOfSpeech::SUBORDINATING_CONJONCTION);

  fGramStrToGraEnum["erreur"].emplace_back(PartOfSpeech::BOOKMARK);

  fGramStrToGraEnum["interjection"].emplace_back(PartOfSpeech::INTERJECTION);

  fGramStrToGraEnum["lettre"].emplace_back(PartOfSpeech::BOOKMARK);

  fGramStrToGraEnum["nom propre"].emplace_back(PartOfSpeech::PROPER_NOUN);

  fGramStrToGraEnum["onomatopée"].emplace_back(PartOfSpeech::INTERJECTION);

  fGramStrToGraEnum["particule"].emplace_back(PartOfSpeech::BOOKMARK);
  fGramStrToGraEnum["préfixe"].emplace_back(PartOfSpeech::BOOKMARK);

  fGramStrToGraEnum["préposition"].emplace_back(PartOfSpeech::PREPOSITION);

  fGramStrToGraEnum["pronom"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["pronom"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["pronom"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);

  fGramStrToGraEnum["pronom démonstratif"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["pronom démonstratif"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["pronom démonstratif"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);

  fGramStrToGraEnum["pronom indéfini"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["pronom indéfini"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["pronom indéfini"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);

  fGramStrToGraEnum["pronom interrogatif"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["pronom interrogatif"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["pronom interrogatif"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);

  fGramStrToGraEnum["pronom personnel"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["pronom personnel"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["pronom personnel"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);

  fGramStrToGraEnum["pronom possessif"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["pronom possessif"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["pronom possessif"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);

  fGramStrToGraEnum["pronom relatif"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["pronom relatif"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["pronom relatif"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);

  fGramStrToGraEnum["postposition"].emplace_back(PartOfSpeech::BOOKMARK);
  fGramStrToGraEnum["symbole"].emplace_back(PartOfSpeech::BOOKMARK);
  fGramStrToGraEnum["variante typographique"].emplace_back(PartOfSpeech::BOOKMARK);

  fGramStrToGraEnum["verbe"].emplace_back(PartOfSpeech::VERB);
  fGramStrToGraEnum["verbe"].emplace_back(PartOfSpeech::AUX);
  fGramStrToGraEnum["verbe pronominal"].emplace_back(PartOfSpeech::VERB);
  fGramStrToGraEnum["verbe pronominal"].emplace_back(PartOfSpeech::AUX);
  fGramStrToGraEnum["verbe pronominal"].emplace_back(PartOfSpeech::PRONOUN);
  fGramStrToGraEnum["verbe pronominal"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
  fGramStrToGraEnum["verbe pronominal"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);


  fGramStrToGraEnum["nom"].emplace_back(PartOfSpeech::NOUN);
}


void FRWikiKeyWords::getGramEnum
(std::set<PartOfSpeech>& pRes,
 const std::string& pGramStr,
 const std::string& pLine) const
{
  if (pLine.compare(0, 4, "=== ") != 0)
  {
    return;
  }

  std::map<std::string, std::vector<PartOfSpeech> >::const_iterator
      it = fGramStrToGraEnum.find(pGramStr);
  if (it != fGramStrToGraEnum.end())
  {
    bool findAGram = false;
    for (std::size_t i = 0; i < it->second.size(); ++i)
    {
      if (it->second[i] != PartOfSpeech::BOOKMARK)
      {
        if (!findAGram)
        {
          pRes.clear();
          findAGram = true;
        }
        pRes.insert(it->second[i]);
      }
    }
  }
  else
  {
    std::cerr << "gram type \"" << pGramStr << "\" is unknown (line: " << pLine << ")" << std::endl;
  }
}


} // End of namespace onsem



/**
PartOfSpeech::BOOKMARK > erreur lettre particule "variante typographique" symbole
PartOfSpeech::ADJECTIVE > adjectif "adjectif numéral" "adjectif possessif" "adjectif indéfini" "adjectif démonstratif" adj "adjectif interrogatif"
PartOfSpeech::ADVERB > adverbe "adverbe interrogatif" "adverbe relatif"
PartOfSpeech::CONJUNCTIVE > "conjonction de coordination" conjonction
PartOfSpeech::SUBORDINATING_CONJONCTION > conjonction
PartOfSpeech::DETERMINER > "article défini" "article indéfini" "article partitif"
PartOfSpeech::PARTITIVE >
PartOfSpeech::INTERJECTION > interjection onomatopée
PartOfSpeech::NOUN > nom "adjectif numéral"
PartOfSpeech::PREPOSITION > préposition
PartOfSpeech::PRONOUN > "pronom personnel" "pronom indéfini" "verbe pronominal" "pronom" "pronom relatif" "pronom interrogatif" "pronom démonstratif" "pronom possessif"
PartOfSpeech::VERB > verbe "verbe pronominal"
PartOfSpeech::AUX > verbe "verbe pronominal"
PartOfSpeech::PRONOUN_COMPLEMENT > "pronom personnel" "pronom indéfini" "verbe pronominal" "pronom" "pronom relatif" "pronom interrogatif" "pronom démonstratif" "pronom possessif"
PartOfSpeech::PRONOUN_SUBJECT > "pronom personnel" "pronom indéfini" "verbe pronominal" "pronom" "pronom relatif" "pronom interrogatif" "pronom démonstratif" "pronom possessif"
PartOfSpeech::PROPER_NOUN > "nom propre"
 */
