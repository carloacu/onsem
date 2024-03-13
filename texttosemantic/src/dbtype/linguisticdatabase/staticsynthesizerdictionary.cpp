#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/common/binary/binaryloader.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>

namespace onsem {
namespace linguistics {

StaticSynthesizerDictionary::StaticSynthesizerDictionary(std::istream* pIStreamPtr,
                                                         const StaticConceptSet& pConceptsDb,
                                                         const StaticLinguisticDictionary& pStatLingDic,
                                                         SemanticLanguageEnum pLangEnum)
    : VirtualSemBinaryDatabase()
    , fStatLingDic(pStatLingDic)
    , fConceptsDb(pConceptsDb)
    , fLangEnum(pLangEnum)
    , fPtrConjugaisons(nullptr)
    , fConceptsToMeanings(nullptr)
    , fMeaningsFromConcepts(nullptr) {
    if (pIStreamPtr != nullptr)
        xLoad(*pIStreamPtr);
}

StaticSynthesizerDictionary::~StaticSynthesizerDictionary() {
    xUnload();
}

void StaticSynthesizerDictionary::xUnload() {
    binaryloader::deallocMemZone(&fPtrConjugaisons);
    binaryloader::deallocMemZone(&fConceptsToMeanings);
    binaryloader::deallocMemZone(&fMeaningsFromConcepts);
    fTotalSize = 0;
    fErrorMessage = "NOT_LOADED";
}

void StaticSynthesizerDictionary::xLoad(std::istream& pIStream) {
    assert(!xIsLoaded());
    DatabaseHeader header;
    pIStream.read(header.charValues, sizeof(DatabaseHeader));
    if (header.intValues[0] != fFormalism) {
        fErrorMessage = "BAD_FORMALISM";
        return;
    }
    std::size_t conjugaisonsSize = static_cast<std::size_t>(header.intValues[1]);
    std::size_t conceptsToMeaningsSize = static_cast<std::size_t>(header.intValues[2]);
    std::size_t meaningsFromConceptsSize = static_cast<std::size_t>(header.intValues[3]);
    fTotalSize = conjugaisonsSize + conceptsToMeaningsSize + meaningsFromConceptsSize;

    if (!binaryloader::allocMemZone(&fPtrConjugaisons, pIStream, conjugaisonsSize)
        || !binaryloader::allocMemZone(&fConceptsToMeanings, pIStream, conceptsToMeaningsSize)
        || !binaryloader::allocMemZone(&fMeaningsFromConcepts, pIStream, meaningsFromConceptsSize)) {
        xUnload();
        fErrorMessage = "BAD_ALLOC";
        return;
    }

    // Close database file
    fErrorMessage = "";
}

std::string StaticSynthesizerDictionary::getLemma(const StaticLinguisticMeaning& pMeaning,
                                                  bool pWithLinkMeanings) const {
    return fStatLingDic.getLemma(pMeaning, pWithLinkMeanings);
}

StaticLinguisticMeaning StaticSynthesizerDictionary::conceptToMeaning(const std::string& pConcept) const {
    int conceptId = fConceptsDb.getConceptId(pConcept);
    auto* cptStruct = xGetPtrToMeaningLinkedAndMeaningLinkedBefore(conceptId);
    if (cptStruct == nullptr) {
        return StaticLinguisticMeaning();
    }
    char nbOfMeaning = cptStruct[0];
    if (nbOfMeaning > 0) {
        return StaticLinguisticMeaning(fLangEnum,
                                       binaryloader::alignedDecToInt(*(reinterpret_cast<const int*>(cptStruct) + 1)));
    }
    return StaticLinguisticMeaning();
}

signed char* StaticSynthesizerDictionary::xGetPtrToMeaningLinkedAndMeaningLinkedBefore(int pConceptId) const {
    assert(xIsLoaded());
    if (pConceptId == StaticConceptSet::noConcept) {
        return nullptr;
    }
    int cptToMeaningId = fConceptsDb.getConceptToMeaningId(pConceptId);
    int meaningFromConceptId =
        binaryloader::alignedDecToInt(*reinterpret_cast<const int*>(fConceptsToMeanings + cptToMeaningId));
    if (meaningFromConceptId == 0) {
        return nullptr;
    }
    return fMeaningsFromConcepts + meaningFromConceptId;
}

void StaticSynthesizerDictionary::getImperativeVerbForm(std::list<WordToSynthesize>& pVerbForm,
                                                        const StaticLinguisticMeaning& pMeaning,
                                                        RelativePerson pRelativePerson,
                                                        bool pIsPositiveForm) const {
    assert(xIsLoaded());
    assert(pMeaning.meaningId != LinguisticMeaning_noMeaningId);
    assert(pMeaning.language == fLangEnum);
    int conjId;
    if (!fStatLingDic.getConjugaisionsId(conjId, pMeaning)) {
        return;
    }
    if (fLangEnum == SemanticLanguageEnum::ENGLISH && !pIsPositiveForm) {
        pVerbForm.emplace_back(SemanticWord(fLangEnum, "do", PartOfSpeech::AUX), InflectionToSynthesize("don't"));
    }
    const int decImperative = 18;
    int wordEndingNode = 0;
    switch (pRelativePerson) {
        case RelativePerson::FIRST_PLUR:
            wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + decImperative + 1);
            break;
        case RelativePerson::SECOND_PLUR:
            wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + decImperative + 2);
            break;
        default: wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + decImperative); break;
    }

    if (wordEndingNode != 0) {
        std::string res;
        fStatLingDic.getWord(res, binaryloader::alignedDecToInt(wordEndingNode));
        if (!res.empty()) {
            const std::string wordLemma = fStatLingDic.getLemma(pMeaning, false);
            pVerbForm.emplace_back(SemanticWord(fLangEnum, wordLemma, PartOfSpeech::VERB), InflectionToSynthesize(res));
        }
    }
}

bool StaticSynthesizerDictionary::_beAuxIsPossibleForThisFrenchVerb(int pConjId) const {
    return *(reinterpret_cast<const int*>(fPtrConjugaisons + pConjId) + 23) != 0;
}

void StaticSynthesizerDictionary::getVerbForm(std::list<WordToSynthesize>& pVerbForm,
                                              std::string& pNegationAfterVerb,
                                              const StaticLinguisticMeaning& pMeaning,
                                              RelativePerson pRelativePerson,
                                              SemanticGenderType pSubectGender,
                                              LinguisticVerbTense pVerbTense,
                                              VerbGoalEnum pVerbGoal,
                                              bool pIsPositiveForm,
                                              bool pHasASubject,
                                              bool pPartOfAComposedVerb,
                                              bool pIsPassive,
                                              bool pIsASubordinateWithoutPreposition,
                                              bool pQuestionForm) const {
    assert(xIsLoaded());
    assert(pMeaning.meaningId != LinguisticMeaning_noMeaningId);
    assert(pMeaning.language == fLangEnum);
    int conjId;
    if (!fStatLingDic.getConjugaisionsId(conjId, pMeaning)) {
        return;
    }
    int wordEndingNode = 0;

    linguistics::InflectedWord verbIGram;
    fStatLingDic.getInfoGram(verbIGram, pMeaning);

    std::list<WordToSynthesize> afterAuxVerbForm;
    if (fLangEnum == SemanticLanguageEnum::ENGLISH && verbIGram.word.partOfSpeech != PartOfSpeech::AUX) {
        switch (pVerbTense) {
            case LinguisticVerbTense::FUTURE_INDICATIVE: {
                pVerbForm.emplace_back(SemanticWord(fLangEnum, "will", PartOfSpeech::ADVERB),
                                       InflectionToSynthesize("will"));
                pVerbTense = LinguisticVerbTense::INFINITIVE;
                if (pRelativePerson == RelativePerson::THIRD_SING) {
                    pRelativePerson = RelativePerson::FIRST_SING;
                }
                break;
            }
            case LinguisticVerbTense::IMPERFECT_INDICATIVE:
            case LinguisticVerbTense::SIMPLE_PAST_INDICATIVE: {
                if (!pIsPositiveForm) {
                    std::string lemma = fStatLingDic.getLemma(pMeaning, false);
                    if (lemma == "be" || lemma == "must") {
                        pNegationAfterVerb = "not";
                    } else if (pIsPassive) {
                        afterAuxVerbForm.emplace_back(SemanticWord(fLangEnum, "not", PartOfSpeech::ADVERB),
                                                      InflectionToSynthesize("not"));
                    } else {
                        pVerbForm.emplace_back(SemanticWord(fLangEnum, "do", PartOfSpeech::AUX),
                                               InflectionToSynthesize("didn't"));
                        pVerbTense = LinguisticVerbTense::PRESENT_INDICATIVE;
                        pRelativePerson = RelativePerson::FIRST_SING;
                    }
                } else if (pQuestionForm) {
                    std::string lemma = fStatLingDic.getLemma(pMeaning, false);
                    if (!pIsPassive && lemma != "be" && lemma != "must") {
                        pVerbForm.emplace_back(SemanticWord(fLangEnum, "do", PartOfSpeech::AUX),
                                               InflectionToSynthesize(pIsPositiveForm ? "did" : "didn't"));
                        pVerbTense = LinguisticVerbTense::PRESENT_INDICATIVE;
                        pRelativePerson = RelativePerson::FIRST_SING;
                    }
                }
                break;
            }
            case LinguisticVerbTense::INFINITIVE: {
                pVerbForm.emplace_back(SemanticWord(fLangEnum, "to", PartOfSpeech::UNKNOWN),
                                       InflectionToSynthesize("to"));
                if (pIsPassive) {
                    pVerbForm.emplace_back(SemanticWord(fLangEnum, "be", PartOfSpeech::AUX),
                                           InflectionToSynthesize("be"));
                    pIsPassive = false;
                    pVerbTense = LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
                }
                break;
            }
            case LinguisticVerbTense::PRESENT_CONTINUOUS: {
                if (pIsPassive) {
                    pVerbTense = LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
                } else {
                    getVerbForm(pVerbForm,
                                pNegationAfterVerb,
                                fStatLingDic.getBeAux().meaning,
                                pRelativePerson,
                                pSubectGender,
                                LinguisticVerbTense::PRESENT_INDICATIVE,
                                pVerbGoal,
                                true,
                                pHasASubject,
                                false,
                                false,
                                false,
                                false);
                }
                if (!pIsPositiveForm)
                    pNegationAfterVerb = "not";
                break;
            }
            case LinguisticVerbTense::PRETERIT_CONTINUOUS: {
                if (pIsPassive) {
                    pVerbTense = LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
                } else {
                    getVerbForm(pVerbForm,
                                pNegationAfterVerb,
                                fStatLingDic.getBeAux().meaning,
                                pRelativePerson,
                                pSubectGender,
                                LinguisticVerbTense::IMPERFECT_INDICATIVE,
                                pVerbGoal,
                                true,
                                pHasASubject,
                                false,
                                false,
                                false,
                                false);
                }
                if (!pIsPositiveForm)
                    pNegationAfterVerb = "not";
                break;
            }
            case LinguisticVerbTense::PRESENT_INDICATIVE: {
                if (!pIsPassive && fStatLingDic.hasContextualInfo(WordContextualInfos::BEISTHEAUXILIARY, pMeaning)) {
                    getVerbForm(pVerbForm,
                                pNegationAfterVerb,
                                fStatLingDic.getBeAux().meaning,
                                pRelativePerson,
                                pSubectGender,
                                pVerbTense,
                                pVerbGoal,
                                true,
                                pHasASubject,
                                false,
                                false,
                                false,
                                false);
                    pVerbTense = LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
                } else if (!pIsPositiveForm) {
                    std::string lemma = fStatLingDic.getLemma(pMeaning, false);
                    if (lemma == "be" || lemma == "must")
                        pNegationAfterVerb = "not";
                    else if (pIsPassive)
                        afterAuxVerbForm.emplace_back(SemanticWord(fLangEnum, "not", PartOfSpeech::ADVERB),
                                                      InflectionToSynthesize("not"));
                    else if (pRelativePerson == RelativePerson::THIRD_SING) {
                        pVerbForm.emplace_back(SemanticWord(fLangEnum, "do", PartOfSpeech::AUX),
                                               InflectionToSynthesize("doesn't"));
                        pRelativePerson = RelativePerson::FIRST_SING;
                    } else
                        pVerbForm.emplace_back(SemanticWord(fLangEnum, "do", PartOfSpeech::AUX),
                                               InflectionToSynthesize("don't"));
                } else if (pQuestionForm) {
                    std::string lemma = fStatLingDic.getLemma(pMeaning, false);
                    if (!pIsPassive && lemma != "be" && lemma != "must") {
                        if (pRelativePerson == RelativePerson::THIRD_SING) {
                            pVerbForm.emplace_back(SemanticWord(fLangEnum, "do", PartOfSpeech::AUX),
                                                   InflectionToSynthesize(pIsPositiveForm ? "does" : "doesn't"));
                            pRelativePerson = RelativePerson::FIRST_SING;
                        } else {
                            pVerbForm.emplace_back(SemanticWord(fLangEnum, "do", PartOfSpeech::AUX),
                                                   InflectionToSynthesize(pIsPositiveForm ? "do" : "don't"));
                        }
                    }
                }

                break;
            }
            default: break;
        }
    }

    // write the infinitive
    if (pVerbTense == LinguisticVerbTense::INFINITIVE
        && !(fLangEnum == SemanticLanguageEnum::FRENCH && pVerbGoal == VerbGoalEnum::NOTIFICATION && pHasASubject)) {
        const std::string wordLemma = fStatLingDic.getLemma(pMeaning, false);
        pVerbForm.emplace_back(SemanticWord(fLangEnum, wordLemma, PartOfSpeech::VERB),
                               InflectionToSynthesize(wordLemma));
        return;
    }

    if (verbIGram.word.partOfSpeech != PartOfSpeech::AUX
        && (pIsPassive
            || (fLangEnum == SemanticLanguageEnum::FRENCH
                && pVerbTense == LinguisticVerbTense::SIMPLE_PAST_INDICATIVE))) {
        LinguisticVerbTense auxTense = LinguisticVerbTense::PRESENT_INDICATIVE;
        if (pVerbTense == LinguisticVerbTense::FUTURE_INDICATIVE
            || pVerbTense == LinguisticVerbTense::IMPERFECT_INDICATIVE)
            auxTense = pVerbTense;
        else if (pVerbGoal == VerbGoalEnum::ABILITY && pIsPassive)
            auxTense = LinguisticVerbTense::INFINITIVE;

        bool useOfBeAux = false;
        if (!pIsASubordinateWithoutPreposition
            || (fLangEnum == SemanticLanguageEnum::FRENCH && pVerbTense == LinguisticVerbTense::PRESENT_INDICATIVE
                && verbIGram.word.lemma != "naître"
                && !ConceptSet::haveAConcept(verbIGram.infos.concepts, "verb_die"))) {
            if (fLangEnum == SemanticLanguageEnum::FRENCH) {
                if (pPartOfAComposedVerb || auxTense == LinguisticVerbTense::INFINITIVE
                    || (pIsPassive && pVerbTense != LinguisticVerbTense::SIMPLE_PAST_INDICATIVE)
                    || fStatLingDic.hasContextualInfo(WordContextualInfos::BEISTHEAUXILIARY, pMeaning)) {
                    useOfBeAux = true;
                } else if (!_beAuxIsPossibleForThisFrenchVerb(conjId)
                           || fStatLingDic.hasContextualInfo(WordContextualInfos::HAVEISTHEAUXILIARY, pMeaning)) {
                    getVerbForm(pVerbForm,
                                pNegationAfterVerb,
                                fStatLingDic.getHaveAux().meaning,
                                pRelativePerson,
                                pSubectGender,
                                auxTense,
                                pVerbGoal,
                                true,
                                pHasASubject,
                                false,
                                false,
                                false,
                                false);
                    if (pIsPassive) {
                        pVerbForm.emplace_back(SemanticWord(fLangEnum, "être", PartOfSpeech::AUX),
                                               InflectionToSynthesize("été"));
                    }
                } else {
                    useOfBeAux = true;
                }
            } else if (pIsPassive) {
                useOfBeAux = true;
            }

            if (useOfBeAux) {
                if (pIsPassive && pVerbTense == LinguisticVerbTense::SIMPLE_PAST_INDICATIVE
                    && auxTense != LinguisticVerbTense::INFINITIVE)
                    auxTense = LinguisticVerbTense::IMPERFECT_INDICATIVE;
                getVerbForm(pVerbForm,
                            pNegationAfterVerb,
                            fStatLingDic.getBeAux().meaning,
                            pRelativePerson,
                            pSubectGender,
                            auxTense,
                            pVerbGoal,
                            true,
                            pHasASubject,
                            false,
                            false,
                            false,
                            false);
            }
        } else {
            useOfBeAux = true;
        }
        pVerbForm.splice(pVerbForm.end(), afterAuxVerbForm);

        if (fLangEnum == SemanticLanguageEnum::FRENCH && (useOfBeAux || pIsPassive)) {
            if (pRelativePerson == RelativePerson::FIRST_PLUR || pRelativePerson == RelativePerson::SECOND_PLUR
                || pRelativePerson == RelativePerson::THIRD_PLUR) {
                if (pSubectGender == SemanticGenderType::FEMININE) {
                    wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + 25);
                } else {
                    wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + 23);
                }
            } else {
                if (pSubectGender == SemanticGenderType::FEMININE) {
                    wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + 24);
                } else {
                    wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + 22);
                }
            }
        } else if (fLangEnum == SemanticLanguageEnum::ENGLISH
                   && pVerbTense == LinguisticVerbTense::SIMPLE_PAST_INDICATIVE) {
            wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId));
        } else {
            wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + 22);
        }
    } else if (pVerbTense == LinguisticVerbTense::PRESENT_PARTICIPLE
               || (fLangEnum == SemanticLanguageEnum::ENGLISH
                   && (pVerbTense == LinguisticVerbTense::PRESENT_CONTINUOUS
                       || pVerbTense == LinguisticVerbTense::PRETERIT_CONTINUOUS))) {
        wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + 21);
    } else {
        int timeDecalage = 0;    // imperfect
        if (isAPresentVerbTense(pVerbTense)) {
            if (fLangEnum == SemanticLanguageEnum::FRENCH && pVerbGoal == VerbGoalEnum::CONDITIONAL) {
                timeDecalage = 26;    // conditional /!\ IMPORTANT: conditional exists only on the french database !
            } else {
                timeDecalage = 6;    // present
            }
        } else if (pVerbTense == LinguisticVerbTense::FUTURE_INDICATIVE) {
            timeDecalage = 12;    // future
        } else if (fLangEnum == SemanticLanguageEnum::FRENCH
                   && (pVerbTense == LinguisticVerbTense::PRESENT_SUBJONCTIVE
                       || pVerbTense == LinguisticVerbTense::INFINITIVE)) {
            timeDecalage = 32;    // present of subjunctive /!\ IMPORTANT: present of subjunctive exists only on the
                                  // french database !
        }

        switch (pRelativePerson) {
            case RelativePerson::FIRST_SING:
                wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + timeDecalage);
                break;
            case RelativePerson::SECOND_SING:
                wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + timeDecalage + 1);
                break;
            case RelativePerson::FIRST_PLUR:
                wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + timeDecalage + 3);
                break;
            case RelativePerson::SECOND_PLUR:
                wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + timeDecalage + 4);
                break;
            case RelativePerson::THIRD_PLUR:
                wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + timeDecalage + 5);
                break;
            default:    // RelativePerson::THIRD_SING
                wordEndingNode = *(reinterpret_cast<const int*>(fPtrConjugaisons + conjId) + timeDecalage + 2);
                break;
        }
    }

    if (wordEndingNode != 0) {
        std::string res;
        fStatLingDic.getWord(res, binaryloader::alignedDecToInt(wordEndingNode));
        const std::string wordLemma = fStatLingDic.getLemma(pMeaning, false);
        pVerbForm.emplace_back(SemanticWord(fLangEnum, wordLemma, PartOfSpeech::VERB), InflectionToSynthesize(res));
    }
}

void StaticSynthesizerDictionary::getNounGenders(std::set<SemanticGenderType>& pGenders,
                                                 const StaticLinguisticMeaning& pMeaning,
                                                 SemanticNumberType pNumber) const {
    assert(xIsLoaded());
    if (pMeaning.meaningId == LinguisticMeaning_noMeaningId)
        return;
    assert(pMeaning.language == fLangEnum);
    int conjId;
    if (!fStatLingDic.getConjugaisionsId(conjId, pMeaning)) {
        return;
    }

    const auto* conjPtr = fPtrConjugaisons + conjId;
    if (pNumber == SemanticNumberType::PLURAL) {
        if (xGetMascPluralPtr(conjPtr))
            pGenders.insert(SemanticGenderType::MASCULINE);
        if (xGetFemPluralPtr(conjPtr))
            pGenders.insert(SemanticGenderType::FEMININE);
        if (xGetneutralPluralPtr(conjPtr))
            pGenders.insert(SemanticGenderType::NEUTRAL);
        return;
    }
    if (xGetMascSingularPtr(conjPtr))
        pGenders.insert(SemanticGenderType::MASCULINE);
    if (xGetFemSingularlPtr(conjPtr))
        pGenders.insert(SemanticGenderType::FEMININE);
    if (xGetNeutralSingularPtr(conjPtr))
        pGenders.insert(SemanticGenderType::NEUTRAL);
}

void StaticSynthesizerDictionary::getNounForm(std::string& pRes,
                                              const StaticLinguisticMeaning& pMeaning,
                                              SemanticGenderType& pGender,
                                              SemanticNumberType& pNumber) const {
    int wordEndingNode = xGetnounOrAdjForm(true, pMeaning, pGender, pNumber, ComparisonOperator::EQUAL);
    if (wordEndingNode != 0)
        fStatLingDic.getWord(pRes, binaryloader::alignedDecToInt(wordEndingNode));
}

void StaticSynthesizerDictionary::getAdjForm(std::string& pRes,
                                             const StaticLinguisticMeaning& pMeaning,
                                             SemanticGenderType pGender,
                                             SemanticNumberType pNumber,
                                             ComparisonOperator pComparisonOperator) const {
    std::string beforeAdj;
    if (pMeaning.language == SemanticLanguageEnum::ENGLISH) {
        if (pComparisonOperator == ComparisonOperator::LESS) {
            beforeAdj = "less ";
        }
    } else {
        if (pComparisonOperator == ComparisonOperator::MORE) {
            beforeAdj = "plus ";
        } else if (pComparisonOperator == ComparisonOperator::LESS) {
            beforeAdj = "moins ";
        }
    }

    int wordEndingNode = xGetnounOrAdjForm(false, pMeaning, pGender, pNumber, pComparisonOperator);
    if (wordEndingNode != 0)
        fStatLingDic.getWord(pRes, binaryloader::alignedDecToInt(wordEndingNode));
    if (!beforeAdj.empty()) {
        pRes = beforeAdj + pRes;
    }
}

int StaticSynthesizerDictionary::xGetnounOrAdjForm(bool nounOrAdj,
                                                   const StaticLinguisticMeaning& pMeaning,
                                                   SemanticGenderType& pGender,
                                                   SemanticNumberType& pNumber,
                                                   ComparisonOperator pComparisonOperator) const {
    assert(xIsLoaded());
    assert(pMeaning.meaningId != LinguisticMeaning_noMeaningId);
    assert((pMeaning.language == fLangEnum));
    int conjId;
    if (!fStatLingDic.getConjugaisionsId(conjId, pMeaning)) {
        return 0;
    }

    if (!nounOrAdj && fLangEnum == SemanticLanguageEnum::ENGLISH && pComparisonOperator == ComparisonOperator::MORE) {
        int wordEndingNode = xGetEnglishComparativePtr(fPtrConjugaisons + conjId);
        if (wordEndingNode != 0)
            return wordEndingNode;
    }

    if (pNumber == SemanticNumberType::PLURAL) {
        int wordEndingNode = 0;

        switch (pGender) {
            case SemanticGenderType::MASCULINE: wordEndingNode = xGetMascPluralPtr(fPtrConjugaisons + conjId); break;
            case SemanticGenderType::FEMININE: wordEndingNode = xGetFemPluralPtr(fPtrConjugaisons + conjId); break;
            case SemanticGenderType::NEUTRAL: wordEndingNode = xGetneutralPluralPtr(fPtrConjugaisons + conjId); break;
            default: break;
        }
        if (wordEndingNode != 0)
            return wordEndingNode;

        wordEndingNode = xGetneutralPluralPtr(fPtrConjugaisons + conjId);
        if (wordEndingNode != 0) {
            pGender = SemanticGenderType::NEUTRAL;
            return wordEndingNode;
        }

        wordEndingNode = xGetMascPluralPtr(fPtrConjugaisons + conjId);
        if (wordEndingNode != 0) {
            pGender = SemanticGenderType::MASCULINE;
            return wordEndingNode;
        }

        wordEndingNode = xGetFemPluralPtr(fPtrConjugaisons + conjId);
        if (wordEndingNode != 0) {
            pGender = SemanticGenderType::FEMININE;
            return wordEndingNode;
        }
    }

    int wordEndingNode = 0;
    switch (pGender) {
        case SemanticGenderType::MASCULINE: wordEndingNode = xGetMascSingularPtr(fPtrConjugaisons + conjId); break;
        case SemanticGenderType::FEMININE: wordEndingNode = xGetFemSingularlPtr(fPtrConjugaisons + conjId); break;
        case SemanticGenderType::NEUTRAL: wordEndingNode = xGetNeutralSingularPtr(fPtrConjugaisons + conjId); break;
        default: break;
    }

    if (wordEndingNode != 0) {
        pNumber = SemanticNumberType::SINGULAR;
        return wordEndingNode;
    }

    // s
    wordEndingNode = xGetNeutralSingularPtr(fPtrConjugaisons + conjId);
    if (wordEndingNode != 0) {
        pNumber = SemanticNumberType::SINGULAR;
        pGender = SemanticGenderType::NEUTRAL;
        return wordEndingNode;
    }

    wordEndingNode = xGetMascSingularPtr(fPtrConjugaisons + conjId);
    if (wordEndingNode != 0) {
        pNumber = SemanticNumberType::SINGULAR;
        pGender = SemanticGenderType::MASCULINE;
        return wordEndingNode;
    }

    wordEndingNode = xGetFemSingularlPtr(fPtrConjugaisons + conjId);
    if (wordEndingNode != 0) {
        pNumber = SemanticNumberType::SINGULAR;
        pGender = SemanticGenderType::FEMININE;
        return wordEndingNode;
    }

    // p
    wordEndingNode = xGetneutralPluralPtr(fPtrConjugaisons + conjId);
    if (wordEndingNode != 0) {
        pNumber = SemanticNumberType::PLURAL;
        pGender = SemanticGenderType::NEUTRAL;
        return wordEndingNode;
    }

    wordEndingNode = xGetMascPluralPtr(fPtrConjugaisons + conjId);
    if (wordEndingNode != 0) {
        pNumber = SemanticNumberType::PLURAL;
        pGender = SemanticGenderType::MASCULINE;
        return wordEndingNode;
    }

    wordEndingNode = xGetFemPluralPtr(fPtrConjugaisons + conjId);
    if (wordEndingNode != 0) {
        pNumber = SemanticNumberType::PLURAL;
        pGender = SemanticGenderType::FEMININE;
        return wordEndingNode;
    }

    return 0;
}

}    // End of namespace linguistics
}    // End of namespace onsem
