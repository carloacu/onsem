#include "enwikikeywords.hpp"
#include <iostream>

namespace onsem {

EnWikiKeyWords::EnWikiKeyWords()
    : WikiKeyWords()
    , fGramStrToGraEnum() {
    fGramStrToGraEnum["Abbreviation"].emplace_back(PartOfSpeech::NOUN);
    fGramStrToGraEnum["Abbreviation"].emplace_back(PartOfSpeech::PROPER_NOUN);
    fGramStrToGraEnum["Acronym"].emplace_back(PartOfSpeech::NOUN);

    fGramStrToGraEnum["Adjective"].emplace_back(PartOfSpeech::ADJECTIVE);

    fGramStrToGraEnum["Adverb"].emplace_back(PartOfSpeech::ADVERB);

    fGramStrToGraEnum["Article"].emplace_back(PartOfSpeech::DETERMINER);
    fGramStrToGraEnum["Article"].emplace_back(PartOfSpeech::PARTITIVE);

    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::CONJUNCTIVE);
    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::SUBORDINATING_CONJONCTION);
    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::PREPOSITION);
    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::DETERMINER);
    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::PARTITIVE);
    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::NOUN);
    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::ADJECTIVE);
    fGramStrToGraEnum["Cmavo"].emplace_back(PartOfSpeech::ADVERB);

    fGramStrToGraEnum["Conjunction"].emplace_back(PartOfSpeech::CONJUNCTIVE);
    fGramStrToGraEnum["Conjunction"].emplace_back(PartOfSpeech::SUBORDINATING_CONJONCTION);

    fGramStrToGraEnum["Determiner"].emplace_back(PartOfSpeech::DETERMINER);

    fGramStrToGraEnum["Gerund"].emplace_back(PartOfSpeech::VERB);
    fGramStrToGraEnum["Gerund"].emplace_back(PartOfSpeech::AUX);
    fGramStrToGraEnum["Gerund"].emplace_back(PartOfSpeech::NOUN);
    fGramStrToGraEnum["Gerund"].emplace_back(PartOfSpeech::ADJECTIVE);

    fGramStrToGraEnum["Initialism"].emplace_back(PartOfSpeech::NOUN);
    fGramStrToGraEnum["Initialism"].emplace_back(PartOfSpeech::PROPER_NOUN);

    fGramStrToGraEnum["Interjection"].emplace_back(PartOfSpeech::INTERJECTION);

    fGramStrToGraEnum["Noun"].emplace_back(PartOfSpeech::NOUN);

    fGramStrToGraEnum["Numeral"].emplace_back(PartOfSpeech::NOUN);
    fGramStrToGraEnum["Numeral"].emplace_back(PartOfSpeech::DETERMINER);
    fGramStrToGraEnum["Numeral"].emplace_back(PartOfSpeech::ADJECTIVE);

    fGramStrToGraEnum["Particle"].emplace_back(PartOfSpeech::INTERJECTION);
    fGramStrToGraEnum["Particle"].emplace_back(PartOfSpeech::ADVERB);

    fGramStrToGraEnum["Postposition"].emplace_back(PartOfSpeech::PREPOSITION);

    fGramStrToGraEnum["Prepositional phrase"].emplace_back(PartOfSpeech::PREPOSITION);

    fGramStrToGraEnum["Prepositional pronoun"].emplace_back(PartOfSpeech::PREPOSITION);
    fGramStrToGraEnum["Prepositional pronoun"].emplace_back(PartOfSpeech::PRONOUN);
    fGramStrToGraEnum["Prepositional pronoun"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
    fGramStrToGraEnum["Prepositional pronoun"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);
    fGramStrToGraEnum["Prepositional pronoun"].emplace_back(PartOfSpeech::DETERMINER);

    fGramStrToGraEnum["Preposition"].emplace_back(PartOfSpeech::PREPOSITION);

    fGramStrToGraEnum["Pronoun"].emplace_back(PartOfSpeech::PRONOUN);
    fGramStrToGraEnum["Pronoun"].emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
    fGramStrToGraEnum["Pronoun"].emplace_back(PartOfSpeech::PRONOUN_SUBJECT);
    fGramStrToGraEnum["Pronoun"].emplace_back(PartOfSpeech::DETERMINER);

    fGramStrToGraEnum["Proper noun"].emplace_back(PartOfSpeech::PROPER_NOUN);
    fGramStrToGraEnum["Proper Noun"].emplace_back(PartOfSpeech::PROPER_NOUN);

    fGramStrToGraEnum["Punctuation mark"].emplace_back(PartOfSpeech::PUNCTUATION);

    fGramStrToGraEnum["Verb"].emplace_back(PartOfSpeech::VERB);
    fGramStrToGraEnum["Verb"].emplace_back(PartOfSpeech::AUX);
    fGramStrToGraEnum["Verb form"].emplace_back(PartOfSpeech::VERB);
    fGramStrToGraEnum["Verb form"].emplace_back(PartOfSpeech::AUX);

    fGramStrToGraEnum["Abbreviations"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Affix"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alternate forms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alternative spelling"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alternate spellings"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alternative form"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alternative forms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alterative forms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["alternative forms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alternative spellings"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Alternative Spellings"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Anagrams"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Antonym"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Antonyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Antomyns"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Anttonyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Brivla"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Cardinal number"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Cardinal numeral"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Citations"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Classifier"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Compounds"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Conjugation"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Conjugattion"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Contraction"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Coordinate terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Cyrillic spelling"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Declension"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Declensions"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Derivatives"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Derived"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Derived term"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Derived terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["derived terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Derived words"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Descendants"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Descendents"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Etymology"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Etyymology"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Etymology1"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Etymology2"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Examples"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["External link"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["External links"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Forms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Gallery"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Gismu"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Holonyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Homonyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Hypernyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Hyponyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Idiom"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Idioms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Infix"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Inflection"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Letter"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Meronyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Mutation"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Number"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Note"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Ordinal number"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Ordinal numeral"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Paronyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Participle"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Participles"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Phrase"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Phrases"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Prefix"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Proverb"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Pronunciation"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Pronunciaiton"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Pronunciations"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Quotations"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Rafsi"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["References"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Related"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum[" Related terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Related terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Related Terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Related and derived terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Romanization"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Shorthand"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["See also"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["See Also"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["See related"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Similar terms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Statistics"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Syllable"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Symbol"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Synonym"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Synonyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Suffix"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["translation"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Translation"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Trnslations"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Translations"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Trivia"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Troponyms"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Usage note"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Usage Note"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Usage notes"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Usage Notes"].emplace_back(PartOfSpeech::BOOKMARK);
    fGramStrToGraEnum["Variant"].emplace_back(PartOfSpeech::BOOKMARK);
}

void EnWikiKeyWords::getGramEnum(std::set<PartOfSpeech>& pRes,
                                 const std::string& pGramStr,
                                 const std::string& pLine) const {
    std::map<std::string, std::vector<PartOfSpeech> >::const_iterator it = fGramStrToGraEnum.find(pGramStr);

    if (it != fGramStrToGraEnum.end()) {
        xUpdateGramList(pRes, it->second);
        return;
    }

    if (pGramStr.size() > 3 && pGramStr[pGramStr.size() - 2] == ' '
        && (pGramStr[pGramStr.size() - 1] == '1' || pGramStr[pGramStr.size() - 1] == '2'
            || pGramStr[pGramStr.size() - 1] == '3' || pGramStr[pGramStr.size() - 1] == '4'
            || pGramStr[pGramStr.size() - 1] == '5' || pGramStr[pGramStr.size() - 1] == '6'
            || pGramStr[pGramStr.size() - 1] == '7' || pGramStr[pGramStr.size() - 1] == '8'
            || pGramStr[pGramStr.size() - 1] == '9')) {
        it = fGramStrToGraEnum.find(pGramStr.substr(0, pGramStr.size() - 2));
        if (it != fGramStrToGraEnum.end()) {
            xUpdateGramList(pRes, it->second);
            return;
        }
    }

    if (pGramStr.size() > 2 && pGramStr[0] == ' ' && pGramStr[pGramStr.size() - 1] == ' ') {
        it = fGramStrToGraEnum.find(pGramStr.substr(1, pGramStr.size() - 2));
        if (it != fGramStrToGraEnum.end()) {
            xUpdateGramList(pRes, it->second);
            return;
        }
    }

    if (pGramStr.size() > 2 && pGramStr[pGramStr.size() - 1] == ' ') {
        it = fGramStrToGraEnum.find(pGramStr.substr(0, pGramStr.size() - 1));
        if (it != fGramStrToGraEnum.end()) {
            xUpdateGramList(pRes, it->second);
            return;
        }
    }

    if (pGramStr.empty() || (pGramStr.size() > 2 && pGramStr.substr(0, 2) == "{{")) {
        return;
    }

    std::cerr << "gram type \"" << pGramStr << "\" is unknown (line: " << pLine << ")" << std::endl;
}

void EnWikiKeyWords::xUpdateGramList(std::set<PartOfSpeech>& pRes, const std::vector<PartOfSpeech>& pVertGram) {
    bool findAGram = false;
    for (std::size_t i = 0; i < pVertGram.size(); ++i) {
        if (pVertGram[i] != PartOfSpeech::BOOKMARK) {
            if (!findAGram) {
                pRes.clear();
                findAGram = true;
            }
            pRes.insert(pVertGram[i]);
        }
    }
}

}    // End of namespace onsem

/**
PartOfSpeech::BOOKMARK > Anagrams References "See also" ...
PartOfSpeech::PUNCTUATION > "Punctuation mark"
PartOfSpeech::ADJECTIVE > Adjective Numeral Gerund
PartOfSpeech::ADVERB > Adverb
PartOfSpeech::CONJUNCTIVE > Conjunction
PartOfSpeech::SUBORDINATING_CONJONCTION > Conjunction
PartOfSpeech::DETERMINER > Determiner Article Numeral Pronoun "Prepositional pronoun"
PartOfSpeech::PARTITIVE > Article
PartOfSpeech::INTERJECTION > Interjection
PartOfSpeech::NOUN > Noun Numeral Abbreviation Acronym Gerund Initialism
PartOfSpeech::PREPOSITION > Preposition "Prepositional pronoun" Postposition "Prepositional phrase"
PartOfSpeech::PRONOUN > Pronoun "Prepositional pronoun"
PartOfSpeech::VERB > Verb "Verb form" Gerund
PartOfSpeech::AUX > Verb "Verb form" Gerund
PartOfSpeech::PRONOUN_COMPLEMENT > Pronoun "Prepositional pronoun"
PartOfSpeech::PRONOUN_SUBJECT > Pronoun "Prepositional pronoun"
PartOfSpeech::PROPER_NOUN > "Proper noun" Abbreviation Initialism
 */
