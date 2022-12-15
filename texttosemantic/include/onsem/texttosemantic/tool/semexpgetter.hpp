#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SEMEXPGETTER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SEMEXPGETTER_HPP

#include <set>
#include <list>
#include <map>
#include <vector>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/semanticentitytype.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/texttosemantic/dbtype/misc/coreference.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include "../api.hpp"


namespace onsem
{
namespace linguistics
{
class StaticLinguisticDictionary;
class TranslationDictionary;
struct LinguisticDatabase;
struct InflectedWord;
}
struct GroundedExpression;
struct ConditionExpression;
struct ListExpression;
struct SetOfFormsExpression;
struct QuestExpressionFrom;
struct SemanticTextGrounding;
struct SemanticStatementGrounding;
struct SemanticTimeGrounding;
struct SemanticGenericGrounding;
struct SemanticAgentGrounding;
struct SemanticWord;
struct LinguisticMeaning;
struct StaticLinguisticMeaning;

namespace SemExpGetter
{

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<bool> isUncountableFromGrd(const SemanticGenericGrounding& pGrounding,
                                           const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<bool> isUncountableSemExp(const SemanticExpression& pSemExp,
                                          const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
bool isGrdReflexive(const SemanticGrounding& pGrd);

ONSEM_TEXTTOSEMANTIC_API
SemanticEntityType getEntity(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
VerbGoalEnum getGoal(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isPassive(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
SemanticVerbTense getVerbTense(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
SemanticNumberType getNumber(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
PartOfSpeech getMainPartOfSpeech(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
SemanticNumberType getNumberFromGrd(const SemanticGrounding& pGrounding);

ONSEM_TEXTTOSEMANTIC_API
SemanticReferenceType getReferenceTypeFromGrd(const SemanticGrounding& pGrd);

ONSEM_TEXTTOSEMANTIC_API
SemanticNumberType getNumberFromInflections(const linguistics::InflectedWord& pInflections);

ONSEM_TEXTTOSEMANTIC_API
SemanticGenderType possibleGendersToGender(const std::set<SemanticGenderType>& pPossibleGenders);

ONSEM_TEXTTOSEMANTIC_API
SemanticGenderType getGenderFromGenGrd(const SemanticGenericGrounding& pGenGrd);

ONSEM_TEXTTOSEMANTIC_API
SemanticLanguageEnum getLanguage(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations);

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<int> getNumberOfElementsFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<int> getNumberOfElements(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
std::unique_ptr<SemanticGrounding> extractQuantity(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
std::unique_ptr<SemanticGrounding> mergeQuantities(const SemanticGrounding& pPreviousQuantity,
                                                   std::unique_ptr<SemanticGrounding> pNewQuantity);

ONSEM_TEXTTOSEMANTIC_API
int getNumberOfRepetitions(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations);

ONSEM_TEXTTOSEMANTIC_API
const SemanticExpression* getUntilChild(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations);

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<int64_t> getTimeDurationInMilliseconds(const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations);

ONSEM_TEXTTOSEMANTIC_API
bool hasASpecificWord(const SemanticGenericGrounding& pGrounding);

ONSEM_TEXTTOSEMANTIC_API
bool doesSemExpContainsOnlyARequest(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
const SemanticRequests* getRequestList(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
SemanticRequests* getRequestList(GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
GroundedExpression* getGrdExpChild(GroundedExpression& pGrdExp,
                                   GrammaticalType pChildType);

ONSEM_TEXTTOSEMANTIC_API
const GroundedExpression* getGrdExpChild(const GroundedExpression& pGrdExp,
                                         GrammaticalType pChildType);

ONSEM_TEXTTOSEMANTIC_API
const SemanticExpression* getChildFromGrdExp(const GroundedExpression& pGrdExp,
                                             GrammaticalType pChildType);

ONSEM_TEXTTOSEMANTIC_API
const SemanticExpression* getChildFromSemExp(const SemanticExpression& pSemExp,
                                             GrammaticalType pChildType);

ONSEM_TEXTTOSEMANTIC_API
SemanticExpression& getDirectObjectOrIdentityRecursively(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool hasChild(const GroundedExpression& pGrdExp,
              GrammaticalType pChildType);

ONSEM_TEXTTOSEMANTIC_API
const GroundedExpression* getUnnamedGrdExpPtr(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isNow(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
const SemanticExpression* getGrammTypeInfo(const SemanticExpression& pSemExp,
                                           GrammaticalType pGrammaticalType);

ONSEM_TEXTTOSEMANTIC_API
bool doesSemExpCanBeCompletedWithContext(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
const SemanticExpression* getTimeInfo(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
SemanticReferenceType getReference(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isDefinite(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isDefiniteModifier(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool hasSpecifications(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isItAnActionLabeling(const GroundedExpression*& pLabelSemExp,
                          const SemanticStatementGrounding*& pEqualityStatement,
                          const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAModifierFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAModifier(const SemanticExpression& pSemExp, bool pFollowInterpretations = true);

ONSEM_TEXTTOSEMANTIC_API
bool hasAChildModifier(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
SemanticRequestType convertSemGramToRequestType(GrammaticalType pGramType);

ONSEM_TEXTTOSEMANTIC_API
bool isSemExpEqualToAnAgent(const SemanticExpression& pSemExp,
                       const SemanticAgentGrounding& pAgentGrd);

ONSEM_TEXTTOSEMANTIC_API
bool doesSemExpHaveAnAgent(const SemanticExpression& pSemExp,
                         const SemanticAgentGrounding& pAgentGrd);

ONSEM_TEXTTOSEMANTIC_API
bool doSemExpHoldUserId(const SemanticExpression& pSemExp,
                       const std::string& pUserId);


ONSEM_TEXTTOSEMANTIC_API
const GroundedExpression* getOriginalGrdExpForm(const SetOfFormsExpression& pSetOfFormsExp);

ONSEM_TEXTTOSEMANTIC_API
const GroundedExpression* splitMainGrdAndOtherOnes
(std::list<const GroundedExpression*>& pOtherGrdExps,
 bool& pHasOriginalForm,
 const std::list<std::unique_ptr<QuestExpressionFrom>>& pForms);

ONSEM_TEXTTOSEMANTIC_API
bool grdExpIsAnEmptyStatementGrd(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool semExpIsAnEmptyStatementGrd(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool semExphasAStatementGrd(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void extractReferences(std::list<std::string>& pReferences,
                       const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
ContextualAnnotation extractContextualAnnotation(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
SemanticSourceEnum extractSource(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool doesContainADialogSource(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
const SemanticAgentGrounding* extractAgentGrdPtr(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
const SemanticAgentGrounding* extractAuthor(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
const SemanticExpression* extractAuthorSemExp(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool semExpHasACoreferenceOrAnAgent(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAHumanFromGrd(const SemanticGrounding& pGrd);

ONSEM_TEXTTOSEMANTIC_API
bool isASpecificHuman(const SemanticGenericGrounding& pGenGrd);

ONSEM_TEXTTOSEMANTIC_API
bool isASpecificHumanFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAResourceFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAResource(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAResourceOrATextFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAResourceOrAText(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool hasGroundingType(const SemanticExpression& pSemExp,
                      const std::set<SemanticGroundingType>& pGroundingType);

ONSEM_TEXTTOSEMANTIC_API
SemanticRequestType getMainRequestTypeFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isGrdExpAQuestion(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isSemExpAQuestion(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void wordToAStaticMeaning(StaticLinguisticMeaning& pRes,
                          const SemanticWord& pWord,
                          SemanticLanguageEnum pToLanguage,
                          const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
void wordToAMeaning(LinguisticMeaning& pRes,
                    const SemanticWord& pWord,
                    SemanticLanguageEnum pToLanguage,
                    const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
void wordToAStaticMeaningInLanguage(StaticLinguisticMeaning& pRes,
                                    const SemanticWord& pWord,
                                    SemanticLanguageEnum pToLanguage,
                                    const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
void wordToAMeaningInLanguage(LinguisticMeaning& pRes,
                              const SemanticWord& pWord,
                              SemanticLanguageEnum pToLanguage,
                              const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
int wordToMeaningId(const SemanticWord& pWord,
                    SemanticLanguageEnum pToLanguage,
                    const linguistics::LinguisticDatabase& pLingDb);



ONSEM_TEXTTOSEMANTIC_API
std::string getUserIdOfSubject(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void extractSubjectAndObjectOfAVerbDefinition(
    const GroundedExpression*& pSubjectGrdPtr,
    const SemanticExpression*& pInfCommandToDo,
    const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void extractTeachElements(
    const GroundedExpression*& pPurposeGrdPtr,
    const SemanticExpression*& pObjectSemExp,
    const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAnActionDefinition(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
std::list<GroundedExpression*> iterateOnListOfGrdExps(SemanticExpression& pUSemExp);

ONSEM_TEXTTOSEMANTIC_API
std::list<const GroundedExpression*> iterateOnListOfGrdExps(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
std::list<UniqueSemanticExpression*> iterateOnList(UniqueSemanticExpression& pUSemExp);

ONSEM_TEXTTOSEMANTIC_API
std::list<const SemanticExpression*> iterateOnList(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAnInfinitiveGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isATeachingElement(const GroundedExpression& pGrdExp);


ONSEM_TEXTTOSEMANTIC_API
bool isNothing(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool isDoNothingSemExp(const SemanticExpression& pSemExp);


ONSEM_TEXTTOSEMANTIC_API
int getRank(const SemanticExpression& pSemExp);


ONSEM_TEXTTOSEMANTIC_API
const GroundedExpression* getGrdExpToDo(const GroundedExpression& pGrdExp,
                                        const SemanticStatementGrounding& pStatementGrd,
                                        const std::string& pAuthorUserId);


ONSEM_TEXTTOSEMANTIC_API
const GroundedExpression* getLastGrdExpPtr(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void getConceptsOfGrdExp(std::set<std::string>& pConcepts,
                         const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void getConceptsOfSemExp(std::set<std::string>& pConcepts,
                         const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
std::unique_ptr<GroundedExpression> getCopyExceptChild
(const GroundedExpression& pGrdExp,
 GrammaticalType pGrammaticalTypeToSkip);

ONSEM_TEXTTOSEMANTIC_API
bool agentIsTheSubject(const GroundedExpression& pGrdExp,
                       const std::string& pSubjectUserId);

ONSEM_TEXTTOSEMANTIC_API
bool isACoreference(const SemanticExpression& pSemExp,
                    CoreferenceDirectionEnum pDirection,
                    bool pShouldNotHaveALemma = false);

ONSEM_TEXTTOSEMANTIC_API
bool isACoreferenceFromGenericGrounding(const SemanticGenericGrounding& pGenGrd,
                                        CoreferenceDirectionEnum pDirection,
                                        bool pShouldNotHaveALemma = false);

ONSEM_TEXTTOSEMANTIC_API
bool isACoreferenceFromStatementGrounding(const SemanticStatementGrounding& pStatGrd,
                                          CoreferenceDirectionEnum pDirection,
                                          bool pShouldNotHaveALemma = false);

ONSEM_TEXTTOSEMANTIC_API
GrammaticalType childGrammaticalTypeOfParentCoreference(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool hasACoreference(const SemanticExpression& pSemExp,
                     CoreferenceDirectionEnum pCorefenceDirection = CoreferenceDirectionEnum::UNKNOWN);

ONSEM_TEXTTOSEMANTIC_API
const UniqueSemanticExpression* getCoreferenceAfterFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression* getCoreferenceAfterFromGrdExp(GroundedExpression& pGrdExp);

static inline bool isGrdExpComplete(const GroundedExpression& pGrdExp)
{
  return getCoreferenceAfterFromGrdExp(pGrdExp) == nullptr;
}

ONSEM_TEXTTOSEMANTIC_API
bool isGrdExpPositive(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool getSubjectAndObjectUserIdsOfNameAssignement(std::string& pSubjectUserId,
                                                 std::string& pObjectUserId,
                                                 const GroundedExpression*& pSubjectGrdExpPtr,
                                                 const GroundedExpression*& pObjectGrdExpPtr,
                                                 const GroundedExpression& pGrdExp);


ONSEM_TEXTTOSEMANTIC_API
bool isANameAssignement(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression returnAPositiveSemExpBasedOnAnInput(const GroundedExpression& pInputGrdExp);

ONSEM_TEXTTOSEMANTIC_API
bool extractExternalTeachingLabel(const GroundedExpression& pGrdExp,
                                  const SemanticStatementGrounding& pStatGrd,
                                  const SemanticExpression*& pExternalTeachingLabelSemExpPtr,
                                  SemanticLanguageEnum& pTeachingLanguage,
                                  const std::string& pAuthorUserId);

ONSEM_TEXTTOSEMANTIC_API
bool isAnExtractExternalTeaching(const GroundedExpression& pGrdExp,
                                 const std::string& pAuthorUserId);

ONSEM_TEXTTOSEMANTIC_API
bool isWhoSemExp(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool isAnything(const GroundedExpression& pGrdExpToLookFor);

ONSEM_TEXTTOSEMANTIC_API
bool isAnythingFromSemExp(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool haveAGrdExpThatModifyTheMeaning(const SemanticExpression& pSemExp,
                                     bool pFollowInterpretations = true);

ONSEM_TEXTTOSEMANTIC_API
void getStatementSubordinates(std::set<const SemanticExpression*>& pStatementSubordinates,
                              const SemanticExpression& pSemExp,
                              bool pSearchOnRootSemExp = false);

ONSEM_TEXTTOSEMANTIC_API
GrammaticalType invertGrammaticalType(GrammaticalType pGrammaticalType);

ONSEM_TEXTTOSEMANTIC_API
std::unique_ptr<GroundedExpression> getASimplifiedVersionFromGrdExp(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression getASimplifiedVersion(const SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool hasGenericConcept(const UniqueSemanticExpression* pUSemExpPtr);

} // End of namespace SemExpGetter

} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SEMEXPGETTER_HPP
