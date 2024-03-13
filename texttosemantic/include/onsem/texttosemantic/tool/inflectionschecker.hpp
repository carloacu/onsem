#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKER_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKER_HPP

#include <list>
#include <set>
#include <memory>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/relativeperson.hpp>
#include "../api.hpp"

namespace onsem {
struct Inflections;
struct VerbalInflections;
namespace linguistics {
struct InflectedWord;
struct SpecificLinguisticDatabase;
class LinguisticDictionary;
struct WordAssociatedInfos;
class InflectionsCheckerVirtual;
struct Token;

class ONSEM_TEXTTOSEMANTIC_API InflectionsChecker {
public:
    explicit InflectionsChecker(const SpecificLinguisticDatabase& pSpecLingDb);
    virtual ~InflectionsChecker();

    InflectionsChecker(InflectionsChecker&& pOther) = delete;
    InflectionsChecker& operator=(InflectionsChecker&& pOther) = delete;
    InflectionsChecker(const InflectionsChecker&) = delete;
    InflectionsChecker& operator=(const InflectionsChecker&) = delete;

    bool areCompatibles(const InflectedWord& pIGram1, const InflectedWord& pIGram2) const;

    static bool areInflectionsCompatibles(const Inflections& pInfls1, const Inflections& pInfls2);
    static bool isAdjCompatibleWithNumberType(const Inflections& pAdjInfls, SemanticNumberType pNumberType);

    bool filterIncompatibleInflections(const Token* pPrevPrevToken, Token& pToken1, Token& pToken2) const;

    bool canBeAssociatedInAList(const InflectedWord& pIGram1, const InflectedWord& pIGram2) const;

    void unionOfSameVerbTenses(VerbalInflections& pResVerInfls,
                               const Inflections& pInflections1,
                               const Inflections& pInflections2) const;

    void intersectionOfSameVerbTenses(VerbalInflections& pExistingVerbInfls,
                                      const VerbalInflections& pNewVerbInfls) const;

    bool verbIsOnlyAtPastParticiple(const InflectedWord& pIGram) const;
    bool verbIsOnlyAtPresentOrPastParticiple(const InflectedWord& pIGram) const;

    static bool verbIsAtPastParticiple(const InflectedWord& pIGram);
    static bool verbCanBeAtImperative(const InflectedWord& pIGram);
    static bool verbCanBeAtThridOfSingularExceptImperative(const InflectedWord& pInflWord);
    static bool verbIsAtPresentIndicative(const InflectedWord& pIGram);
    static bool verbIsAtPresentParticiple(const InflectedWord& pIGram);
    static bool verbIsAtInfinitive(const InflectedWord& pIGram);
    static bool verbIsConjugated(const InflectedWord& pIGram);
    static bool verbalInflAreAtPresentParticiple(const VerbalInflections& pVerbalInfls);

    static bool verbRemoveAllInflectionsThatAreNotAtPastParticiple(InflectedWord& pInflWord);

    static bool isuncountable(const WordAssociatedInfos& pWordWithInfos);

    bool verbCanBeSingular(const InflectedWord& pIGram) const;
    bool verbCanBePlural(const InflectedWord& pIGram) const;

    bool verbIsConjAtPerson(const InflectedWord& pInflWord, RelativePerson pPerson) const;

    bool verbCanHaveAnAuxiliary(const InflectedWord& pIGram) const;

    bool verbHasToHaveAnAuxiliary(const InflectedWord& pIGram) const;

    bool verbCantHaveSubject(const InflectedWord& pIGram) const;
    static bool verbCanHaveNoSubject(const InflectedWord& pInflWord);

    RelativePerson imperativeVerbToRelativePerson(const InflectedWord& pIGram) const;

    bool areVerbAndPronComplCanBeLinked(const Inflections& pPronComplInflections,
                                        const Inflections& pVerbInflections) const;

    static bool nounCanBePlural(const InflectedWord& pIGram);
    static void getNounNumberAndGender(SemanticNumberType& pNumber,
                                       SemanticGenderType& pGender,
                                       const InflectedWord& pIGram);

    bool adjCanBeComparative(const InflectedWord& pIGram) const;

    bool pronounCanBePlural(const InflectedWord& pIGram) const;
    bool pronounSetAtSamePers(const InflectedWord& pIGram1, const InflectedWord& pIGram2) const;
    bool pronounSetAt3ePers(const InflectedWord& pIGram) const;
    bool pronounCanReferToA3ePers(const InflectedWord& pIGram) const;
    RelativePerson pronounGetPerson(const InflectedWord& pIGram) const;
    void pronounRemovePluralPossibility(InflectedWord& pIGram) const;
    void pronounRemoveSingularPossibility(InflectedWord& pIGram) const;

    void initGenderSetFromIGram(std::set<SemanticGenderType>& pPossibleGenders, const InflectedWord& pIGram) const;

    void verbTenseAndGoalFromInflections(SemanticVerbTense& pRes,
                                         VerbGoalEnum& pVerbGoal,
                                         const Inflections& pInflections,
                                         bool pRootIsAVerb = false) const;

protected:
    const LinguisticDictionary& fBinDico;
    std::unique_ptr<InflectionsCheckerVirtual> _impl;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TOOL_INFLECTION_INFLECTIONSCHECKER_HPP
