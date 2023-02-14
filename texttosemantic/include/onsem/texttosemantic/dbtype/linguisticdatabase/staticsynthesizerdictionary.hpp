#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICSYNTHESIZERDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICSYNTHESIZERDICTIONARY_HPP

#include <set>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/virtualsembinarydatabase.hpp>
#include <onsem/common/enum/linguisticverbtense.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/common/enum/relativeperson.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/wordtosynthesize.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include "../../api.hpp"

namespace onsem
{
class StaticConceptSet;
namespace linguistics
{
class StaticLinguisticDictionary;

class ONSEM_TEXTTOSEMANTIC_API StaticSynthesizerDictionary : public VirtualSemBinaryDatabase
{
public:
  StaticSynthesizerDictionary(std::istream* pIStreamPtr,
                              const StaticConceptSet& pConceptsDb,
                              const StaticLinguisticDictionary& pStatLingDic,
                              SemanticLanguageEnum pLangEnum);
  ~StaticSynthesizerDictionary();


  std::string getLemma(const StaticLinguisticMeaning& pMeaning,
                       bool pWithLinkMeanings) const;

  StaticLinguisticMeaning conceptToMeaning(const std::string& pConcept) const;

  void getImperativeVerbForm
  (std::list<WordToSynthesize>& pVerbForm,
   const StaticLinguisticMeaning& pMeaning,
   RelativePerson pRelativePerson,
   bool pIsPositiveForm) const;

  void getVerbForm
  (std::list<WordToSynthesize>& pVerbForm,
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
   bool pQuestionForm) const;


  void getNounForm
  (std::string& pRes,
   const StaticLinguisticMeaning& pMeaning,
   SemanticGenderType& pGender,
   SemanticNumberType& pNumber) const;

  void getAdjForm
  (std::string& pRes,
   const StaticLinguisticMeaning& pMeaning,
   SemanticGenderType pGender,
   SemanticNumberType pNumber,
   ComparisonOperator pComparisonOperator) const;

  void getNounGenders
  (std::set<SemanticGenderType>& pGenders,
   const StaticLinguisticMeaning& pMeaning,
   SemanticNumberType pNumber) const;



private:
  /// Type to store the header of the database in memory
  union DatabaseHeader
  {
    int intValues[4];
    char charValues[16];
  };
  const StaticLinguisticDictionary& fStatLingDic;
  const StaticConceptSet& fConceptsDb;
  SemanticLanguageEnum fLangEnum;
  /// The conjugaisons.
  signed char* fPtrConjugaisons;
  /// The concepts to meanings.
  signed char* fConceptsToMeanings;
  /// The meanings from concepts.
  signed char* fMeaningsFromConcepts;


  bool xIsLoaded() const;

  /**
   * @brief Constructor by copy.
   * This function is private because, we don't want to allow copies.
   * @param pDb Other database.
   */
  StaticSynthesizerDictionary(const StaticSynthesizerDictionary& pDB);


  /**
   * @brief Copy of an other object.
   * This function is private because, we don't want to allow copies.
   * @param pDb Other database.
   * @return The resulting database.
   */
  StaticSynthesizerDictionary& operator=
  (const StaticSynthesizerDictionary& pDB);


  /// Load a binary file in memory.
  void xLoad(std::istream& pIStream);

  /// Deallocate all.
  void xUnload();

  signed char* xGetPtrToMeaningLinkedAndMeaningLinkedBefore(int pConceptId) const;

  int xGetMascSingularPtr(const signed char* pConjPtr) const;
  int xGetMascPluralPtr(const signed char* pConjPtr) const;
  int xGetFemSingularlPtr(const signed char* pConjPtr) const;
  int xGetFemPluralPtr(const signed char* pConjPtr) const;
  int xGetNeutralSingularPtr(const signed char* pConjPtr) const;
  int xGetneutralPluralPtr(const signed char* pConjPtr) const;
  int xGetEnglishComparativePtr(const signed char* pConjPtr) const;

  int xGetnounOrAdjForm(bool nounOrAdj,
                        const StaticLinguisticMeaning& pMeaning,
                        SemanticGenderType& pGender,
                        SemanticNumberType& pNumber,
                        ComparisonOperator pComparisonOperator) const;

  bool _beAuxIsPossibleForThisFrenchVerb(int pConjId) const;
};



} // End of namespace linguistics
} // End of namespace onsem

#include "detail/staticsynthesizerdictionary.hxx"


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICSYNTHESIZERDICTIONARY_HPP
