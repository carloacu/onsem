#include "partofspeechrules_french.hpp"
#include <list>
#include <vector>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcustomfilter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include "../../tool/partofspeech/partofspeechdelbigramimpossibilities.hpp"
#include "../../tool/partofspeech/partofspeechpatternmatcher.hpp"

namespace onsem {
namespace linguistics {
namespace partofspeechrules {
namespace french {

// Impossible successions
std::vector<std::pair<PartOfSpeech, PartOfSpeech>> _getImpSuccessions() {
    return {{PartOfSpeech::ADJECTIVE, PartOfSpeech::PRONOUN_COMPLEMENT},
            {PartOfSpeech::DETERMINER, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::PARTITIVE, PartOfSpeech::VERB},
            {PartOfSpeech::PREPOSITION, PartOfSpeech::LINKBETWEENWORDS},
            {PartOfSpeech::PREPOSITION, PartOfSpeech::PUNCTUATION},
            {PartOfSpeech::PREPOSITION, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::PRONOUN_SUBJECT, PartOfSpeech::NOUN},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::NOUN},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::UNKNOWN},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::PROPER_NOUN},
            {PartOfSpeech::VERB, PartOfSpeech::PARTITIVE},
            {PartOfSpeech::PUNCTUATION, PartOfSpeech::PARTITIVE},
            {PartOfSpeech::AUX, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::SUBORDINATING_CONJONCTION, PartOfSpeech::NOUN},
            {PartOfSpeech::PROPER_NOUN, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::PROPER_NOUN, PartOfSpeech::NOUN}};
}

// Successions where we need to check compatibility
std::vector<std::pair<PartOfSpeech, PartOfSpeech>> _getCheckCompatibility() {
    return {{PartOfSpeech::ADVERB, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::ADVERB, PartOfSpeech::NOUN},
            {PartOfSpeech::DETERMINER, PartOfSpeech::DETERMINER},
            {PartOfSpeech::DETERMINER, PartOfSpeech::NOUN},
            {PartOfSpeech::DETERMINER, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::DETERMINER, PartOfSpeech::PROPER_NOUN},
            {PartOfSpeech::INTERJECTION, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::INTERJECTION, PartOfSpeech::NOUN},
            {PartOfSpeech::PUNCTUATION, PartOfSpeech::SUBORDINATING_CONJONCTION},
            {PartOfSpeech::PUNCTUATION, PartOfSpeech::VERB},
            {PartOfSpeech::ADJECTIVE, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::ADJECTIVE, PartOfSpeech::NOUN},
            {PartOfSpeech::ADJECTIVE, PartOfSpeech::PROPER_NOUN},
            {PartOfSpeech::PRONOUN, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::PRONOUN, PartOfSpeech::NOUN},
            {PartOfSpeech::PRONOUN, PartOfSpeech::PRONOUN_COMPLEMENT},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::DETERMINER},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::ADVERB},
            {PartOfSpeech::PRONOUN_COMPLEMENT, PartOfSpeech::VERB},
            {PartOfSpeech::NOUN, PartOfSpeech::ADJECTIVE},
            {PartOfSpeech::NOUN, PartOfSpeech::NOUN},
            {PartOfSpeech::NOUN, PartOfSpeech::PRONOUN},
            {PartOfSpeech::NOUN, PartOfSpeech::PRONOUN_COMPLEMENT},
            {PartOfSpeech::NOUN, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::VERB, PartOfSpeech::VERB},
            {PartOfSpeech::VERB, PartOfSpeech::INTERJECTION},
            {PartOfSpeech::VERB, PartOfSpeech::SUBORDINATING_CONJONCTION},
            {PartOfSpeech::PREPOSITION, PartOfSpeech::VERB}};
}

std::list<std::unique_ptr<PartOfSpeechContextFilter>> getPartOfSpeechRules(
    const InflectionsChecker& pInfls,
    const SpecificLinguisticDatabase& pSpecLingDb) {
    std::list<std::unique_ptr<PartOfSpeechContextFilter>> rules;

    // del bigrams
    rules.emplace_back(std::make_unique<PartOfSpeechDelBigramImpossibilities>(
        "del bigrams", pInfls, _getImpSuccessions(), _getCheckCompatibility()));

    // aux-verb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("aux-verb links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::AUX,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_ALL_OTHERS,
                                                                                 ActionIfNotLinked::REMOVE));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks betweenTypes(CanBeEmpty::YES, CanHaveMany::YES);
                betweenTypes.elts.emplace_back(PartOfSpeech::PRONOUN_SUBJECT, FinderConstraint::FIRST_ELT);
                betweenTypes.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                betweenTypes.elts.emplace_back(PartOfSpeech::AUX);
                resContext.after.emplace_back(betweenTypes);
            }

            {
                TaggerListOfTokenChecks verbGramType;
                verbGramType.elts.emplace_back(PartOfSpeech::VERB,
                                               FinderConstraint::FIRST_ELT,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_ALL_OTHERS);
                verbGramType.elts.emplace_back(PartOfSpeech::VERB,
                                               FinderConstraint::HAS,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_ALL_OTHERS,
                                               ActionIfNotLinked::NOTHING,
                                               LinkedValue::UNKNOWN);
                resContext.after.emplace_back(verbGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // impossible verb-verb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>(
            "impossible verb-verb links",
            pInfls,
            TaggerTokenCheck(PartOfSpeech::VERB, FinderConstraint::ONLY_ONE_ELT, CompatibilityCheck::IS_COMPATIBLE));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                resContext.after.emplace_back(advGramType);
            }

            {
                TaggerListOfTokenChecks verbGramType;
                verbGramType.elts.emplace_back(PartOfSpeech::VERB,
                                               FinderConstraint::FIRST_ELT,
                                               CompatibilityCheck::ISNT_COMPATIBLE,
                                               ActionIfLinked::DEL_THIS_POSSIBILITY);
                resContext.after.emplace_back(verbGramType);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(advGramType);
            }

            {
                TaggerListOfTokenChecks verbGramType;
                verbGramType.elts.emplace_back(PartOfSpeech::VERB,
                                               FinderConstraint::FIRST_ELT,
                                               CompatibilityCheck::ISNT_COMPATIBLE,
                                               ActionIfLinked::DEL_THIS_POSSIBILITY);
                resContext.before.emplace_back(verbGramType);
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
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_LESSER_PROBA,
                                                                                 ActionIfNotLinked::REMOVE));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks adjGramType(CanBeEmpty::YES, CanHaveMany::YES);
                adjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE, FinderConstraint::FIRST_ELT);
                adjGramType.elts.emplace_back(PartOfSpeech::DETERMINER);
                resContext.after.emplace_back(adjGramType);
            }

            {
                TaggerListOfTokenChecks advGramType;
                advGramType.elts.emplace_back(PartOfSpeech::NOUN,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_LESSER_PROBA);
                advGramType.elts.emplace_back(PartOfSpeech::PROPER_NOUN,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_LESSER_PROBA);
                advGramType.elts.emplace_back(PartOfSpeech::UNKNOWN,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_LESSER_PROBA);
                advGramType.elts.emplace_back(PartOfSpeech::ADVERB,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_LESSER_PROBA);
                advGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_LESSER_PROBA);
                resContext.after.emplace_back(advGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // det-det number links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>(
            "det-det number links",
            pInfls,
            TaggerTokenCheck(
                [](const InflectedWord& pInflWord) {
                    return pInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER
                        && ConceptSet::haveAConceptThatBeginWith(pInflWord.infos.concepts, "number");
                },
                FinderConstraint::HAS,
                CompatibilityCheck::IS_COMPATIBLE,
                ActionIfLinked::DEL_ALL_OTHERS));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advGramType;
                advGramType.elts.emplace_back(PartOfSpeech::DETERMINER,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_LESSER_PROBA);
                resContext.after.emplace_back(advGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // pronoun_comp-verb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>(
            "pronoun_comp/subject-verb links",
            pInfls,
            TaggerTokenCheck(
                [](const InflectedWord& pIGram) {
                    return pIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT
                        || pIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT;
                },
                FinderConstraint::HAS,
                CompatibilityCheck::IS_COMPATIBLE,
                ActionIfLinked::NOTHING,
                ActionIfNotLinked::REMOVE));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advGramType.elts.emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
                resContext.before.emplace_back(advGramType);
            }

            {
                TaggerListOfTokenChecks verbGramType;
                verbGramType.elts.emplace_back(
                    PartOfSpeech::VERB, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                verbGramType.elts.emplace_back(
                    PartOfSpeech::AUX, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                resContext.before.emplace_back(verbGramType);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
                advGramType.elts.emplace_back(PartOfSpeech::ADVERB);
                advGramType.elts.emplace_back(PartOfSpeech::PRONOUN_COMPLEMENT);
                resContext.after.emplace_back(advGramType);
            }

            {
                TaggerListOfTokenChecks verbGramType;
                verbGramType.elts.emplace_back(
                    PartOfSpeech::VERB, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                verbGramType.elts.emplace_back(
                    PartOfSpeech::AUX, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                resContext.after.emplace_back(verbGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // det-noun links 2
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("det-noun links2",
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
                TaggerListOfTokenChecks adjGramType(CanBeEmpty::YES, CanHaveMany::YES);
                adjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::DONT_CARE,
                                              ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(adjGramType);
            }

            {
                TaggerListOfTokenChecks nounGramType;
                nounGramType.elts.emplace_back(PartOfSpeech::NOUN,
                                               FinderConstraint::HAS,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_ALL_OTHERS);
                nounGramType.elts.emplace_back(PartOfSpeech::UNKNOWN,
                                               FinderConstraint::HAS,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(nounGramType);
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

    // adj-conj-adj links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("adj-conj-adj links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::ADJECTIVE,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_ALL_OTHERS));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks conjGramType;
                conjGramType.elts.emplace_back(PartOfSpeech::CONJUNCTIVE,
                                               FinderConstraint::FIRST_ELT,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(conjGramType);
            }

            {
                TaggerListOfTokenChecks adjGramType;
                adjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE,
                                              FinderConstraint::HAS,
                                              CompatibilityCheck::IS_COMPATIBLE,
                                              ActionIfLinked::DEL_ALL_OTHERS);
                resContext.after.emplace_back(adjGramType);
            }
            return resContext;
        }());

        return res;
    }());

    // gather linked meanings
    rules.emplace_back(std::make_unique<PartOfSpeechCustomFilter>(
        "gather linked meanings", pInfls, pSpecLingDb, "gatherLinkedMeanings"));

    // noun at bottom if two times in a row
    rules.emplace_back(std::make_unique<PartOfSpeechCustomFilter>(
        "noun at bottom if two times in a row", pInfls, pSpecLingDb, "nounAtBottomIfTwoTimesInARow"));

    // intj links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("intj links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::INTERJECTION,
                                                                                 FinderConstraint::FIRST_ELT,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_THIS_POSSIBILITY));

        TaggerPattern& pattern = res->getPattern();
        TaggerListOfTokenChecks adjNounGramType;
        adjNounGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE, FinderConstraint::FIRST_ELT);
        adjNounGramType.elts.emplace_back(PartOfSpeech::NOUN, FinderConstraint::FIRST_ELT);

        pattern.possibilities.emplace_back([&adjNounGramType] {
            AIGramContext resContext;
            resContext.before.emplace_back(adjNounGramType);
            resContext.after.emplace_back(adjNounGramType);
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
                                                                                 ActionIfLinked::DEL_LESSER_PROBA));

        TaggerPattern& pattern = res->getPattern();
        TaggerListOfTokenChecks advAdjGramType(CanBeEmpty::YES, CanHaveMany::YES);
        advAdjGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
        advAdjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE, FinderConstraint::FIRST_ELT);

        TaggerListOfTokenChecks advGramType(CanBeEmpty::YES, CanHaveMany::YES);
        advGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);

        TaggerListOfTokenChecks nounGramType;
        nounGramType.elts.emplace_back(PartOfSpeech::NOUN, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);

        pattern.possibilities.emplace_back([&advAdjGramType, &nounGramType] {
            AIGramContext resContext;
            resContext.after.emplace_back(advAdjGramType);
            resContext.after.emplace_back(nounGramType);
            return resContext;
        }());

        pattern.possibilities.emplace_back([&advAdjGramType, &nounGramType] {
            AIGramContext resContext;
            resContext.before.emplace_back(advAdjGramType);
            resContext.before.emplace_back(nounGramType);
            return resContext;
        }());

        pattern.possibilities.emplace_back([&advGramType] {
            AIGramContext resContext;
            resContext.after.emplace_back(advGramType);
            {
                TaggerListOfTokenChecks finalGramType;
                finalGramType.elts.emplace_back(PartOfSpeech::UNKNOWN,
                                                FinderConstraint::HAS,
                                                CompatibilityCheck::IS_COMPATIBLE,
                                                ActionIfLinked::DEL_LESSER_PROBA);
                finalGramType.elts.emplace_back(
                    PartOfSpeech::PRONOUN, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                finalGramType.elts.emplace_back(PartOfSpeech::CONJUNCTIVE,
                                                FinderConstraint::HAS,
                                                CompatibilityCheck::IS_COMPATIBLE,
                                                ActionIfLinked::DEL_LESSER_PROBA);
                resContext.before.emplace_back(finalGramType);
            }
            return resContext;
        }());

        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks skipableGramType(CanBeEmpty::YES, CanHaveMany::YES);
                skipableGramType.elts.emplace_back(PartOfSpeech::PRONOUN_SUBJECT, FinderConstraint::FIRST_ELT);
                skipableGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
                skipableGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE, FinderConstraint::FIRST_ELT);
                resContext.before.emplace_back(skipableGramType);
            }
            {
                TaggerListOfTokenChecks verbGramTypes;
                verbGramTypes.elts.emplace_back(
                    PartOfSpeech::VERB, FinderConstraint::HAS, CompatibilityCheck::IS_COMPATIBLE);
                resContext.before.emplace_back(verbGramTypes);
            }
            return resContext;
        }());

        return res;
    }());

    // remove adj impossibilities
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("remove adj impossibilities",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::ADJECTIVE,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::DEL_THIS_POSSIBILITY));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;

            TaggerListOfTokenChecks skipPronouns(CanBeEmpty::YES, CanHaveMany::YES);
            skipPronouns.elts.emplace_back(PartOfSpeech::PRONOUN_SUBJECT, FinderConstraint::FIRST_ELT);
            resContext.before.emplace_back(skipPronouns);

            TaggerListOfTokenChecks detOrPunctuationGramTypes;
            detOrPunctuationGramTypes.elts.emplace_back(
                [](const InflectedWord& pInflWord) {
                    return pInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER
                        || pInflWord.word.partOfSpeech == PartOfSpeech::PUNCTUATION;
                },
                FinderConstraint::FIRST_ELT);
            resContext.before.emplace_back(detOrPunctuationGramTypes);

            TaggerListOfTokenChecks advAdjGramType(CanBeEmpty::YES, CanHaveMany::YES);
            advAdjGramType.elts.emplace_back(PartOfSpeech::ADVERB, FinderConstraint::FIRST_ELT);
            advAdjGramType.elts.emplace_back(PartOfSpeech::ADJECTIVE, FinderConstraint::FIRST_ELT);
            resContext.after.emplace_back(advAdjGramType);

            TaggerListOfTokenChecks notNounGramType;
            notNounGramType.elts.emplace_back(
                [](const InflectedWord& pInflWord) { return !partOfSpeech_isNominal(pInflWord.word.partOfSpeech); },
                FinderConstraint::FIRST_ELT);
            resContext.after.emplace_back(notNounGramType);
            return resContext;
        }());

        return res;
    }());

    // pron_subj-verb links
    rules.emplace_back([&pInfls] {
        auto res = std::make_unique<PartOfSpeechPatternMatcher>("pron_subj-verb links",
                                                                pInfls,
                                                                TaggerTokenCheck(PartOfSpeech::PRONOUN_SUBJECT,
                                                                                 FinderConstraint::HAS,
                                                                                 CompatibilityCheck::IS_COMPATIBLE,
                                                                                 ActionIfLinked::PUT_ON_TOP));

        TaggerPattern& pattern = res->getPattern();
        pattern.possibilities.emplace_back([] {
            AIGramContext resContext;
            {
                TaggerListOfTokenChecks prepGramType;
                prepGramType.elts.emplace_back(PartOfSpeech::VERB,
                                               FinderConstraint::HAS,
                                               CompatibilityCheck::IS_COMPATIBLE,
                                               ActionIfLinked::PUT_ON_TOP);
                resContext.after.emplace_back(prepGramType);
            }
            return resContext;
        }());

        return res;
    }());

    return rules;
}

}    // End of namespace french
}    // End of namespace partofspeechrules
}    // End of namespace linguistics
}    // End of namespace onsem
