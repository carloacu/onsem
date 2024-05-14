#include "partofspeechrules_english.hpp"
#include <list>
#include <vector>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcustomfilter.hpp>
#include "../../tool/partofspeech/partofspeechdelbigramimpossibilities.hpp"
#include "../../tool/partofspeech/partofspeechpatternmatcher.hpp"

namespace onsem {
namespace linguistics {
namespace partofspeechrules {
namespace english {

// Impossible successions
std::vector<std::pair<PartOfSpeech, PartOfSpeech>> _getImpSuccessions() {
    return {{PartOfSpeech::ADJECTIVE, PartOfSpeech::PROPER_NOUN},
            {PartOfSpeech::CONJUNCTIVE, PartOfSpeech::SUBORDINATING_CONJONCTION},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::LINKBETWEENWORDS},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::ADVERB},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::CONJUNCTIVE},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::PREPOSITION},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::PUNCTUATION},
            {PartOfSpeech::DETERMINER, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::DETERMINER, PartOfSpeech::PREPOSITION},
            {PartOfSpeech::DETERMINER, PartOfSpeech::VERB},
            {PartOfSpeech::DETERMINER, PartOfSpeech::SUBORDINATING_CONJONCTION},
            {PartOfSpeech::DETERMINER, PartOfSpeech::CONJUNCTIVE},
            {PartOfSpeech::DETERMINER, PartOfSpeech::PUNCTUATION},
            {PartOfSpeech::PRONOUN, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::PRONOUN_SUBJECT, PartOfSpeech::PRONOUN_SUBJECT},
            {PartOfSpeech::PRONOUN_SUBJECT, PartOfSpeech::SUBORDINATING_CONJONCTION},
            {PartOfSpeech::PRONOUN_SUBJECT, PartOfSpeech::NOUN},
            {PartOfSpeech::PRONOUN_SUBJECT, PartOfSpeech::UNKNOWN},
            {PartOfSpeech::PRONOUN_SUBJECT, PartOfSpeech::PUNCTUATION},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::NOUN},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::DETERMINER},
            {PartOfSpeech::AUX, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::AUX, PartOfSpeech::SUBORDINATING_CONJONCTION}};
}

// Successions where we need to check compatibility
std::vector<std::pair<PartOfSpeech, PartOfSpeech>> _getCheckCompatibility() {
    return {{PartOfSpeech::PUNCTUATION, PartOfSpeech::SUBORDINATING_CONJONCTION},
            {PartOfSpeech::PUNCTUATION, PartOfSpeech::VERB},
            {PartOfSpeech::ADJECTIVE, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::ADJECTIVE, PartOfSpeech::NOUN},
            {PartOfSpeech::ADJECTIVE, PartOfSpeech::VERB},
            {PartOfSpeech::DETERMINER, PartOfSpeech::NOUN},
            {PartOfSpeech::DETERMINER, PartOfSpeech::DETERMINER},
            {PartOfSpeech::DETERMINER, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::DETERMINER, PartOfSpeech::PROPER_NOUN},
            {PartOfSpeech::PRONOUN, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::PRONOUN, PartOfSpeech::NOUN},
            {PartOfSpeech::NOUN, PartOfSpeech::DETERMINER},
            {PartOfSpeech::NOUN, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::NOUN, PartOfSpeech::PRONOUN},
            {PartOfSpeech::NOUN, PartOfSpeech::PRONOUN_COMPLEMENT},
            {PartOfSpeech::VERB, PartOfSpeech::INTERJECTION}};
}

std::list<std::unique_ptr<PartOfSpeechContextFilter>> getPartOfSpeechRules(
    const InflectionsChecker& pInfls,
    const SpecificLinguisticDatabase& pSpecLingDb) {
    std::list<std::unique_ptr<PartOfSpeechContextFilter>> rules;

    // remove meanings that cannot be at the beginning
    rules.emplace_back(
        std::make_unique<PartOfSpeechCustomFilter>("remove meanings that cannot be at the beginning or at the ending",
                                                   pInfls,
                                                   pSpecLingDb,
                                                   "removeMeaningsThatCannotBeAtTheBeginning"));

    // del bigrams
    rules.emplace_back(std::make_unique<PartOfSpeechDelBigramImpossibilities>(
        "del bigrams", pInfls, _getImpSuccessions(), _getCheckCompatibility()));

    // for-verb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>(
            "for-verb",
            pInfls,
            TaggerTokenCheck([](const InflectedWord& pInflWord) { return pInflWord.word.lemma == "for"; },
                             FinderConstraint::FIRST_ELT));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([&pInfls] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks pron_subjGramType;
                pron_subjGramType.elts.emplace_back(
                    [&pInfls](const InflectedWord& pInflWord) {
                        return pInflWord.word.partOfSpeech == PartOfSpeech::VERB
                            && !pInfls.verbIsAtPresentParticiple(pInflWord);
                    },
                    FinderConstraint::HAS,
                    CompatibilityCheck::DONT_CARE,
                    ActionIfLinked::DEL_THIS_POSSIBILITY);
                resContext.after.emplace_back(pron_subjGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // aux-noun-verb
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("aux-noun-verb",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::AUX,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_ALL_OTHERS));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks pron_subjGramType;
                pron_subjGramType.elts.emplace_back(PartOfSpeech::PROPER_NOUN,
                                                    FinderConstraint::HAS,
                                                    CompatibilityCheck::DONT_CARE,
                                                    ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(pron_subjGramType);
            }

            {
                TaggerListOfTokenChecks pron_subjGramType;
                pron_subjGramType.elts.emplace_back(PartOfSpeech::VERB,
                                                    FinderConstraint::HAS,
                                                    CompatibilityCheck::IS_COMPATIBLE,
                                                    ActionIfLinked::DEL_ALL_EXPECT_AUX);
                resContext.after.emplace_back(pron_subjGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // is a possible verb
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>(
            "is a possible verb",
            pInfls,
            TaggerTokenCheck(
                [](const InflectedWord& pInflWord) {
                    return pInflWord.word.partOfSpeech == PartOfSpeech::VERB
                        && !InflectionsChecker::verbCanBeAtThridOfSingularExceptImperative(pInflWord)
                        && !InflectionsChecker::verbIsAtPresentParticiple(pInflWord);
                },
                FinderConstraint::HAS,
                CompatibilityCheck::IS_COMPATIBLE,
                ActionIfLinked::NOTHING,
                ActionIfNotLinked::REMOVE));

        TaggerPattern& pattern = res->getPattern();

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks adjGramType(CanBeEmpty::YES, CanHaveMany::YES);
                adjGramType.elts.emplace_back(PartOfSpeech::INTERJECTION);
                resContext.before.emplace_back(adjGramType);
            }
            {
                TaggerListOfTokenChecks nounGramTypes;
                nounGramTypes.elts.emplace_back(PartOfSpeech::LINKBETWEENWORDS);
                nounGramTypes.elts.emplace_back(PartOfSpeech::PUNCTUATION);
                nounGramTypes.elts.emplace_back([](const InflectedWord& pInflWord) {
                    return pInflWord.word.partOfSpeech == PartOfSpeech::VERB
                        && InflectionsChecker::verbIsAtInfinitive(pInflWord);
                });
                resContext.before.emplace_back(nounGramTypes);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks adjGramType(CanBeEmpty::YES, CanHaveMany::YES);
                adjGramType.elts.emplace_back(PartOfSpeech::INTERJECTION);
                adjGramType.elts.emplace_back(PartOfSpeech::NOUN);
                adjGramType.elts.emplace_back(PartOfSpeech::UNKNOWN);
                adjGramType.elts.emplace_back(PartOfSpeech::PROPER_NOUN);
                adjGramType.elts.emplace_back(PartOfSpeech::DETERMINER);
                adjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE);
                adjGramType.elts.emplace_back(PartOfSpeech::ADVERB);
                adjGramType.elts.emplace_back(PartOfSpeech::PRONOUN_SUBJECT);
                resContext.before.emplace_back(adjGramType);
            }

            {
                TaggerListOfTokenChecks nounGramTypes;
                nounGramTypes.elts.emplace_back(
                    [](const InflectedWord& pInflWord) {
                        return pInflWord.word.partOfSpeech == PartOfSpeech::NOUN
                            && InflectionsChecker::nounCanBePlural(pInflWord);
                    },
                    FinderConstraint::HAS,
                    CompatibilityCheck::IS_COMPATIBLE);
                nounGramTypes.elts.emplace_back(
                    PartOfSpeech::PRONOUN, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                nounGramTypes.elts.emplace_back(
                    PartOfSpeech::PRONOUN_SUBJECT, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                nounGramTypes.elts.emplace_back(PartOfSpeech::CONJUNCTIVE);
                nounGramTypes.elts.emplace_back(
                    PartOfSpeech::AUX, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                nounGramTypes.elts.emplace_back([](const InflectedWord& pInflWord) {
                    return pInflWord.word.partOfSpeech == PartOfSpeech::VERB
                        && (pInflWord.word.lemma == "say" || pInflWord.word.lemma == "tell"
                            || pInflWord.word.lemma == "ask" || pInflWord.word.lemma == "want"
                            || pInflWord.word.lemma == "let");
                });
                resContext.before.emplace_back(nounGramTypes);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks nounGramTypes;
                nounGramTypes.elts.emplace_back(
                    PartOfSpeech::PRONOUN, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                nounGramTypes.elts.emplace_back(
                    PartOfSpeech::PRONOUN_SUBJECT, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                resContext.after.emplace_back(nounGramTypes);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks adjGramType(CanBeEmpty::YES, CanHaveMany::YES);
                adjGramType.elts.emplace_back([](const InflectedWord& pInflWord) {
                    return pInflWord.word.partOfSpeech != PartOfSpeech::PREPOSITION
                        && pInflWord.word.partOfSpeech != PartOfSpeech::SUBORDINATING_CONJONCTION
                        && pInflWord.word.partOfSpeech != PartOfSpeech::PUNCTUATION;
                });
                resContext.before.emplace_back(adjGramType);
            }

            {
                TaggerListOfTokenChecks nounGramTypes;
                nounGramTypes.elts.emplace_back(PartOfSpeech::PREPOSITION);
                nounGramTypes.elts.emplace_back(PartOfSpeech::SUBORDINATING_CONJONCTION);
                resContext.before.emplace_back(nounGramTypes);
            }
            return resContext;
        }());

        return res;
    }());

    // det-noun links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("det-noun links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::DETERMINER,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_ALL_OTHERS,
                                                                                 ActionIfNotLinked::REMOVE));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks adjGramType(CanBeEmpty::YES, CanHaveMany::YES, 11);
                adjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(adjGramType);
            }

            {
                TaggerListOfTokenChecks nounGramTypes;
                nounGramTypes.elts.emplace_back(
                    [](const InflectedWord& pInflWord) {
                        return pInflWord.word.partOfSpeech == PartOfSpeech::NOUN
                            && !ConceptSet::haveAConceptThatBeginWith(pInflWord.infos.concepts, "number_");
                    },
                    FinderConstraint::FIRST_ELT,
                    CompatibilityCheck::IS_COMPATIBLE,
                    ActionIfLinked::DEL_ALL_OTHERS);
                nounGramTypes.elts.emplace_back(PartOfSpeech::UNKNOWN,
                                                FinderConstraint::HAS,
                                                CompatibilityCheck::IS_COMPATIBLE,
                                                ActionIfLinked::DEL_ALL_OTHERS);
                nounGramTypes.elts.emplace_back(PartOfSpeech::PROPER_NOUN, FinderConstraint::FIRST_ELT);
                nounGramTypes.elts.emplace_back(PartOfSpeech::NOUN,
                                                FinderConstraint::HAS,
                                                CompatibilityCheck::IS_COMPATIBLE,
                                                ActionIfLinked::NOTHING,
                                                ActionIfNotLinked::NOTHING,
                                                LinkedValue::UNKNOWN);
                resContext.after.emplace_back(nounGramTypes);
            }
            return resContext;
        }());

        return res;
    }());

    // gather linked meanings
    rules.emplace_back(std::make_unique<PartOfSpeechCustomFilter>(
        "gather linked meanings", pInfls, pSpecLingDb, "gatherLinkedMeanings"));

    // aux links
    rules.emplace_back([&pSpecLingDb, &pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("aux links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::AUX,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_ALL_OTHERS,
                                                                                 ActionIfNotLinked::REMOVE));

        TaggerListOfTokenChecks auxGramType(CanBeEmpty::YES);
        auxGramType.elts.emplace_back(PartOfSpeech::AUX,
                                      FinderConstraint::HAS,
                                      CompatibilityCheck::IS_COMPATIBLE,
                                      ActionIfLinked::DEL_ALL_OTHERS);

        TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
        advGramType.elts.emplace_back(PartOfSpeech::ADVERB,
                                      FinderConstraint::FIRST_ELT,
                                      CompatibilityCheck::IS_COMPATIBLE,
                                      ActionIfLinked::DEL_ALL_OTHERS);

        TaggerListOfTokenChecks intjGramType(CanBeEmpty::YES, CanHaveMany::YES);
        intjGramType.elts.emplace_back(PartOfSpeech::INTERJECTION,
                                       FinderConstraint::FIRST_ELT,
                                       CompatibilityCheck::IS_COMPATIBLE,
                                       ActionIfLinked::DEL_ALL_OTHERS);

        TaggerListOfTokenChecks verbGramType;
        verbGramType.elts.emplace_back(PartOfSpeech::VERB,
                                       FinderConstraint::HAS,
                                       CompatibilityCheck::IS_COMPATIBLE,
                                       ActionIfLinked::DEL_ALL_EXPECT_AUX);

        std::list<TaggerListOfTokenChecks> afterForAQuestion;
        {
            TaggerListOfTokenChecks betwGramType(CanBeEmpty::YES, CanHaveMany::YES);
            betwGramType.elts.emplace_back(
                [](const InflectedWord& pIGram) {
                    return pIGram.word.partOfSpeech == PartOfSpeech::DETERMINER
                        || pIGram.word.partOfSpeech == PartOfSpeech::ADVERB
                        || pIGram.word.partOfSpeech == PartOfSpeech::AUX
                        || pIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE;
                },
                FinderConstraint::HAS,
                CompatibilityCheck::IS_COMPATIBLE,
                ActionIfLinked::DEL_ALL_OTHERS);
            afterForAQuestion.emplace_back(betwGramType);
        }

        {
            TaggerListOfTokenChecks betwGramType;
            betwGramType.elts.emplace_back(
                [](const InflectedWord& pIGram) {
                    return pIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT
                        || pIGram.word.partOfSpeech == PartOfSpeech::PRONOUN
                        || pIGram.word.partOfSpeech == PartOfSpeech::NOUN
                        || pIGram.word.partOfSpeech == PartOfSpeech::UNKNOWN
                        || pIGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN;
                },
                FinderConstraint::FIRST_ELT,
                CompatibilityCheck::IS_COMPATIBLE,
                ActionIfLinked::DEL_ALL_OTHERS);
            afterForAQuestion.emplace_back(betwGramType);
        }

        afterForAQuestion.emplace_back(advGramType);
        afterForAQuestion.emplace_back(intjGramType);
        afterForAQuestion.emplace_back(verbGramType);

        TaggerPattern& pattern = res->getPattern();
        // for a question WITHOUT word between the question word and the auxiliary
        pattern.possibilities.emplace_back([&pSpecLingDb, &afterForAQuestion] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks betwGramType;
                betwGramType.elts.emplace_back(
                    [&pSpecLingDb](const InflectedWord& pIGram) {
                        return pIGram.word.partOfSpeech == PartOfSpeech::PUNCTUATION
                            || pIGram.word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS
                            || pSpecLingDb.lingDico.statDb.wordToQuestionWord(pIGram.word, false, false) != nullptr;
                    },
                    FinderConstraint::HAS);
                resContext.before.emplace_back(betwGramType);
            }
            resContext.after = afterForAQuestion;
            return resContext;
        }());

        // for a question WITH word between the question word and the auxiliary
        pattern.possibilities.emplace_back([&pSpecLingDb, &afterForAQuestion] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks betwGramType(CanBeEmpty::NO, CanHaveMany::YES);
                betwGramType.elts.emplace_back(
                    [&pSpecLingDb](const InflectedWord& pIGram) {
                        return pIGram.word.partOfSpeech != PartOfSpeech::PUNCTUATION
                            && pIGram.word.partOfSpeech != PartOfSpeech::LINKBETWEENWORDS
                            && pIGram.word.partOfSpeech != PartOfSpeech::VERB
                            && pSpecLingDb.lingDico.statDb.wordToQuestionWord(pIGram.word, false, false) == nullptr;
                    },
                    FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(betwGramType);
            }

            {
                TaggerListOfTokenChecks betwGramType;
                betwGramType.elts.emplace_back(
                    [&pSpecLingDb](const InflectedWord& pIGram) {
                        return pSpecLingDb.lingDico.statDb.wordToQuestionWord(pIGram.word, false, false) != nullptr;
                    },
                    FinderConstraint::HAS);
                resContext.before.emplace_back(betwGramType);
            }

            resContext.after = afterForAQuestion;
            return resContext;
        }());

        // for an affirmation
        pattern.possibilities.emplace_back([&auxGramType, &advGramType, &verbGramType] {
            AIGramContext resContext;
            resContext.before.emplace_back(auxGramType);
            resContext.after.emplace_back(advGramType);
            resContext.after.emplace_back(verbGramType);
            return resContext;
        }());

        return res;
    }());

    // trans_verb-pron_subj links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("trans_verb-pron_subj links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::PRONOUN_SUBJECT,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_THIS_POSSIBILITY));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks prepGramType(CanBeEmpty::YES, CanHaveMany::YES, 11);
                prepGramType.elts.emplace_back(PartOfSpeech::PREPOSITION, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(prepGramType);
            }

            {
                TaggerListOfTokenChecks punctGramType;
                punctGramType.elts.emplace_back(
                    [](const InflectedWord& pIGram) {
                        return pIGram.word.partOfSpeech == PartOfSpeech::VERB
                            && pIGram.infos.hasContextualInfo(WordContextualInfos::TRANSITIVEVERB);
                    },
                    FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(punctGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // pron_subj-verb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("pron_subj-verb links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::PRONOUN_SUBJECT,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_LESSER_PROBA));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(advGramType);
            }

            {
                TaggerListOfTokenChecks prepGramType;
                prepGramType.elts.emplace_back(PartOfSpeech::AUX,
                                               FinderConstraint::HAS,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_LESSER_PROBA);
                resContext.after.emplace_back(prepGramType);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks prepGramType(CanBeEmpty::YES, CanHaveMany::YES, 11);
                prepGramType.elts.emplace_back(PartOfSpeech::PREPOSITION, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(prepGramType);
            }

            {
                TaggerListOfTokenChecks punctGramType;
                punctGramType.elts.emplace_back([](const InflectedWord& pIGram) {
                    return pIGram.word.partOfSpeech != PartOfSpeech::PREPOSITION
                        && pIGram.word.partOfSpeech != PartOfSpeech::VERB;
                });
                resContext.before.emplace_back(punctGramType);
            }

            {
                TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advGramType.elts.emplace_back(
                    [](const InflectedWord& pIGram) {
                        return pIGram.word.partOfSpeech == PartOfSpeech::ADVERB
                            || pIGram.word.partOfSpeech == PartOfSpeech::AUX;
                    },
                    FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(advGramType);
            }

            {
                TaggerListOfTokenChecks prepGramType;
                prepGramType.elts.emplace_back(
                    [](const InflectedWord& pIGram) {
                        return pIGram.word.partOfSpeech == PartOfSpeech::ADVERB
                            || pIGram.word.partOfSpeech == PartOfSpeech::INTERJECTION
                            || pIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE;
                    },
                    FinderConstraint::FIRST_ELT,
                    CompatibilityCheck::DONT_CARE,
                    ActionIfLinked::NOTHING,
                    ActionIfNotLinked::NOTHING,
                    LinkedValue::UNKNOWN);
                prepGramType.elts.emplace_back(PartOfSpeech::VERB,
                                               FinderConstraint::HAS,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(prepGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // verb-prep links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("verb-prep links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::VERB,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_LESSER_PROBA));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advGramType.elts.emplace_back(PartOfSpeech::ADVERB,
                                              FinderConstraint::FIRST_ELT,
                                              CompatibilityCheck::DONT_CARE,
                                              ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(advGramType);
            }

            {
                TaggerListOfTokenChecks prepGramType;
                prepGramType.elts.emplace_back(
                    PartOfSpeech::PREPOSITION, FinderConstraint::FIRST_ELT, CompatibilityCheck::IS_COMPATIBLE);
                resContext.after.emplace_back(prepGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // verb-pron_comp links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("verb-pron_comp links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::VERB,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_ALL_OTHERS));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks prepGramType;
                prepGramType.elts.emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT,
                                               FinderConstraint::FIRST_ELT,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(prepGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // verb-pron_comp links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("verb negationned links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::VERB,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_ALL_OTHERS));
        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;

            TaggerListOfTokenChecks notInfl(CanBeEmpty::YES);
            notInfl.elts.emplace_back([](const InflectedWord& pInflWord) {
                return pInflWord.word.partOfSpeech == PartOfSpeech::ADVERB
                    && pInflWord.word.lemma == "not";
            });
            resContext.before.emplace_back(notInfl);

            TaggerListOfTokenChecks toInfl;
            toInfl.elts.emplace_back([](const InflectedWord& pInflWord) {
                return pInflWord.word.partOfSpeech == PartOfSpeech::PREPOSITION
                    && pInflWord.word.lemma == "to";
            });
            resContext.before.emplace_back(toInfl);

            return resContext;
        }());

        return res;
    }());


    // propernoun-verb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("propernoun-verb links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::VERB,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::PUT_ON_TOP));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks nounGramType;
                nounGramType.elts.emplace_back(
                    PartOfSpeech::PROPER_NOUN, FinderConstraint::FIRST_ELT, CompatibilityCheck::IS_COMPATIBLE);
                resContext.before.emplace_back(nounGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // sub_conj-verb links
    rules.emplace_back([&pInfls] {
        auto res =
            std::make_unique<PartOfSpeechPatternMatcher>("sub_conj-verb links",
                                                         pInfls,
                                                         TaggerTokenCheck(PartOfSpeech::SUBORDINATING_CONJONCTION,
                                                                          FinderConstraint::FIRST_ELT,
                                                                          CompatibilityCheck::IS_COMPATIBLE,
                                                                          ActionIfLinked::PUT_ON_TOP));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([&pInfls] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks nounGramType;
                nounGramType.elts.emplace_back(
                    [&pInfls](const InflectedWord& pInflWord) {
                        return pInflWord.word.partOfSpeech == PartOfSpeech::VERB
                            && pInfls.verbIsConjAtPerson(pInflWord, RelativePerson::THIRD_SING);
                    },
                    FinderConstraint::HAS,
                    CompatibilityCheck::IS_COMPATIBLE,
                    ActionIfLinked::PUT_ON_TOP);
                resContext.after.emplace_back(nounGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // prep at end links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("prep at end links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::PREPOSITION,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::DONT_CARE,
                                                                                 ActionIfLinked::DEL_THIS_POSSIBILITY));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks detGramType;
                detGramType.elts.emplace_back(PartOfSpeech::VERB,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::DONT_CARE,
                                              ActionIfLinked::NOTHING,
                                              ActionIfNotLinked::NOTHING,
                                              LinkedValue::UNKNOWN);
                resContext.before.emplace_back(detGramType);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks punctGramType;
                punctGramType.elts.emplace_back(PartOfSpeech::PUNCTUATION);
                resContext.after.emplace_back(punctGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // adverb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("adverb links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::ADVERB,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::DONT_CARE,
                                                                                 ActionIfLinked::DEL_THIS_POSSIBILITY));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks detGramType;
                detGramType.elts.emplace_back(PartOfSpeech::DETERMINER, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(detGramType);
            }

            {
                TaggerListOfTokenChecks punctGramType;
                punctGramType.elts.emplace_back(PartOfSpeech::PUNCTUATION);
                resContext.after.emplace_back(punctGramType);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks detGramType;
                detGramType.elts.emplace_back(PartOfSpeech::DETERMINER, FinderConstraint::FIRST_ELT);
                detGramType.elts.emplace_back(PartOfSpeech::NOUN, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(detGramType);
            }

            {
                TaggerListOfTokenChecks punctGramType;
                punctGramType.elts.emplace_back(PartOfSpeech::NOUN, FinderConstraint::FIRST_ELT);
                punctGramType.elts.emplace_back(PartOfSpeech::PRONOUN, FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(punctGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // verb-interjection links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("verb-interjection links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::INTERJECTION,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_THIS_POSSIBILITY));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks adjGramType(CanBeEmpty::YES, CanHaveMany::YES, 11);
                adjGramType.elts.emplace_back(
                    PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT, CompatibilityCheck::IS_COMPATIBLE);
                resContext.before.emplace_back(adjGramType);
            }

            {
                TaggerListOfTokenChecks nounGramTypes;
                nounGramTypes.elts.emplace_back(
                    PartOfSpeech::VERB, FinderConstraint::FIRST_ELT, CompatibilityCheck::ISNT_COMPATIBLE);
                resContext.before.emplace_back(nounGramTypes);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks pronGramType;
                pronGramType.elts.emplace_back(
                    [](const InflectedWord& pIGram) {
                        return pIGram.word.partOfSpeech == PartOfSpeech::PRONOUN
                            || pIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT;
                    },
                    FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(pronGramType);
            }

            {
                TaggerListOfTokenChecks nounGramTypes;
                nounGramTypes.elts.emplace_back(PartOfSpeech::VERB, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(nounGramTypes);
            }

            {
                TaggerListOfTokenChecks punctGramType;
                punctGramType.elts.emplace_back(PartOfSpeech::PUNCTUATION);
                resContext.after.emplace_back(punctGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // adj links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("adj links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::ADJECTIVE,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_LESSER_PROBA,
                                                                                 ActionIfNotLinked::REMOVE));

        TaggerPattern& pattern = res->getPattern();
        TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
        advGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);

        TaggerListOfTokenChecks advConjGramType(CanBeEmpty::YES, CanHaveMany::YES);
        advConjGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
        advConjGramType.elts.emplace_back(PartOfSpeech::CONJUNCTIVE, FinderConstraint::FIRST_ELT);

        TaggerListOfTokenChecks adjGramType;
        adjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE,
                                      FinderConstraint::HAS,
                                      CompatibilityCheck::IS_COMPATIBLE,
                                      ActionIfLinked::DEL_LESSER_PROBA);

        pattern.possibilities.emplace_back([&advGramType] {
            AIGramContext resContext;
            resContext.after.emplace_back(advGramType);
            TaggerListOfTokenChecks nounGramTypes;
            nounGramTypes.elts.emplace_back(
                PartOfSpeech::NOUN, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
            nounGramTypes.elts.emplace_back(PartOfSpeech::PROPER_NOUN,
                                            FinderConstraint::HAS,
                                            CompatibilityCheck::IS_COMPATIBLE,
                                            ActionIfLinked::DEL_LESSER_PROBA);
            nounGramTypes.elts.emplace_back(PartOfSpeech::PRONOUN,
                                            FinderConstraint::HAS,
                                            CompatibilityCheck::IS_COMPATIBLE,
                                            ActionIfLinked::DEL_LESSER_PROBA);
            resContext.after.emplace_back(nounGramTypes);
            return resContext;
        }());

        pattern.possibilities.emplace_back([&advConjGramType, &adjGramType] {
            AIGramContext resContext;
            resContext.after.emplace_back(advConjGramType);
            resContext.after.emplace_back(adjGramType);
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advSubjGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advSubjGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                advSubjGramType.elts.emplace_back(PartOfSpeech::PRONOUN_SUBJECT, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(advSubjGramType);
            }
            {
                TaggerListOfTokenChecks verbGramTypes;
                verbGramTypes.elts.emplace_back(PartOfSpeech::VERB,
                                                FinderConstraint::FIRST_ELT,
                                                CompatibilityCheck::IS_COMPATIBLE,
                                                ActionIfLinked::DEL_LESSER_PROBA);
                verbGramTypes.elts.emplace_back(PartOfSpeech::VERB,
                                                FinderConstraint::HAS,
                                                CompatibilityCheck::IS_COMPATIBLE,
                                                ActionIfLinked::DEL_LESSER_PROBA,
                                                ActionIfNotLinked::NOTHING,
                                                LinkedValue::UNKNOWN);
                resContext.before.emplace_back(verbGramTypes);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([&advGramType] {
            AIGramContext resContext;
            resContext.before.emplace_back(advGramType);
            TaggerListOfTokenChecks nounGramTypes;
            nounGramTypes.elts.emplace_back(
                [](const InflectedWord& pIGram) {
                    return pIGram.word.partOfSpeech == PartOfSpeech::NOUN
                        || pIGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN
                        || pIGram.word.partOfSpeech == PartOfSpeech::PRONOUN
                        || pIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE;
                },
                FinderConstraint::HAS,
                CompatibilityCheck::IS_COMPATIBLE,
                ActionIfLinked::DEL_LESSER_PROBA,
                ActionIfNotLinked::NOTHING,
                LinkedValue::UNKNOWN);
            resContext.before.emplace_back(nounGramTypes);
            return resContext;
        }());

        return res;
    }());

    // subconj-endOfSentence links
    rules.emplace_back([&pInfls] {
        auto res =
            std::make_unique<PartOfSpeechPatternMatcher>("subconj-endOfSentence links",
                                                         pInfls,
                                                         TaggerTokenCheck(PartOfSpeech::SUBORDINATING_CONJONCTION,
                                                                          FinderConstraint::HAS,
                                                                          CompatibilityCheck::IS_COMPATIBLE,
                                                                          ActionIfLinked::DEL_THIS_POSSIBILITY));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advList(CanBeEmpty::YES, CanHaveMany::YES);
                advList.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                advList.elts.emplace_back(PartOfSpeech::ADJECTIVE, FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(advList);
                TaggerListOfTokenChecks punctElt;
                punctElt.elts.emplace_back(PartOfSpeech::LINKBETWEENWORDS, FinderConstraint::FIRST_ELT);
                punctElt.elts.emplace_back(PartOfSpeech::PUNCTUATION, FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(punctElt);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advList(CanBeEmpty::NO, CanHaveMany::YES);
                advList.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                advList.elts.emplace_back(PartOfSpeech::ADJECTIVE, FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(advList);
                TaggerListOfTokenChecks punctElt;
                punctElt.elts.emplace_back(PartOfSpeech::SUBORDINATING_CONJONCTION, FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(punctElt);
            }
            return resContext;
        }());

        return res;
    }());

    return rules;
}

}    // End of namespace english
}    // End of namespace partofspeechrules
}    // End of namespace linguistics
}    // End of namespace onsem
