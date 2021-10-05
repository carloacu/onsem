#ifndef ALSEMEXPMODIFIER_H
#define ALSEMEXPMODIFIER_H

#include <functional>
#include <list>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/common/utility/optional.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include "../api.hpp"

namespace onsem
{
struct SemanticQuantity;
struct Coreference;
namespace linguistics
{
class LinguisticDatabase;
}


namespace SemExpModifier
{
/// Applies the given function to the top level grounded expressions.
/// Wrappers are skipped to find the top level ground expression if there
/// resolved by skipping wrappers, but it is not recursive.
ONSEM_TEXTTOSEMANTIC_API
std::list<GroundedExpression*> listTopGroundedExpressionsPtr(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void removeSpecificationsNotNecessaryForAnAnswer(GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void removeSpecificationsNotNecessaryForAnAnswerFromSemExp(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void setNumberFromSemExp(UniqueSemanticExpression& pSemExp, int pNumber);

ONSEM_TEXTTOSEMANTIC_API
void setAtPluralFromSemExp(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void removeChild(GroundedExpression& pGrdExp,
                 GrammaticalType pChildType);

ONSEM_TEXTTOSEMANTIC_API
void removeChildFromSemExp(SemanticExpression& pSemExp,
                           GrammaticalType pChildType);

ONSEM_TEXTTOSEMANTIC_API
void setChild(GroundedExpression& pGrdExp,
              GrammaticalType pChildGramType,
              UniqueSemanticExpression pChildSemExp);

ONSEM_TEXTTOSEMANTIC_API
void clearRequestList(GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void swapRequests(GroundedExpression& pGrdExp,
                  SemanticRequests& pRequests);

ONSEM_TEXTTOSEMANTIC_API
void clearRequestListOfSemExp(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void modifyRequestIfAtPassiveForm(SemanticRequestType& pRequest);

ONSEM_TEXTTOSEMANTIC_API
void invertSubjectAndObjectGrdExp(GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void invertPolarityFromGrdExp(GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void invertPolarity(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void removeEmptyListElements(UniqueSemanticExpression& pListExp);


ONSEM_TEXTTOSEMANTIC_API
void applyVerbTenseModifOfSemExp(SemanticExpression& pSemExp,
                                 std::function<void(SemanticVerbTense&)> pVerbTenseModif);


ONSEM_TEXTTOSEMANTIC_API
void modifyVerbTense(GroundedExpression& pGrdExp,
                     SemanticVerbTense pVerbTense);

ONSEM_TEXTTOSEMANTIC_API
void modifyVerbTenseOfSemExp(SemanticExpression& pSemExp,
                             SemanticVerbTense pVerbTense);

ONSEM_TEXTTOSEMANTIC_API
void putInPastWithTimeAnnotation(UniqueSemanticExpression& pSemExp,
                                 std::unique_ptr<SemanticTimeGrounding> pTimeGrd);

ONSEM_TEXTTOSEMANTIC_API
void replaceSayByAskToRobot(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
void setReference(GroundedExpression& pGrdExp,
                   SemanticReferenceType pRefType);

ONSEM_TEXTTOSEMANTIC_API
void setReferenceTypeOfSemExp(SemanticExpression& pSemExp,
                              SemanticReferenceType pRefType);

/// Add a request type to the given grounded expression, if it is a statement.
ONSEM_TEXTTOSEMANTIC_API
void addRequest(GroundedExpression& pGrdExp,
                SemanticRequestType pRequestType);

/// Finds the highest-level statements and add the given request type to them.
ONSEM_TEXTTOSEMANTIC_API
void addRequest(SemanticExpression& pSemExp,
                SemanticRequestType pRequestType);

ONSEM_TEXTTOSEMANTIC_API
void setRequests(SemanticExpression& pSemExp,
                 const SemanticRequests& pRequests);

ONSEM_TEXTTOSEMANTIC_API
void setRequest(SemanticExpression& pSemExp,
                SemanticRequestType pRequestType);

ONSEM_TEXTTOSEMANTIC_API
void addCoreferenceMotherSemExp(UniqueSemanticExpression& pSemExp,
                                GrammaticalType pGrammaticalType);


ONSEM_TEXTTOSEMANTIC_API
void addReferences(UniqueSemanticExpression& pSemExp,
                   const std::list<std::string>& pReferences);


ONSEM_TEXTTOSEMANTIC_API
void fillVerbGoal(GroundedExpression& pGrdExp,
                  VerbGoalEnum pVerbGoal);

ONSEM_TEXTTOSEMANTIC_API
bool putItAdjectival(SemanticExpression& pSemExp,
                     const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
void replaceAgentOfSemExp(SemanticExpression& pSemExp,
                          const std::string& pNewUserId,
                          const std::string& pUserIdToReplace);


template<typename TCHILD>
void addChild(GroundedExpression& pGrdExp,
              GrammaticalType pGramType,
              TCHILD&& pNewExp,
              ListExpressionType pPoorList = ListExpressionType::UNRELATED,
              bool pTryToMergeWithExistingChild = false,
              bool pPushBackOrFront = true);

ONSEM_TEXTTOSEMANTIC_API
void addNewSemExp(UniqueSemanticExpression& pRootSemExp,
                  UniqueSemanticExpression&& pSemExpToAdd,
                  ListExpressionType pListType = ListExpressionType::UNRELATED);

ONSEM_TEXTTOSEMANTIC_API
void addNewChildWithConcept(std::map<GrammaticalType, UniqueSemanticExpression>& pChildren,
                            GrammaticalType pGramOfChild,
                            const std::string& pConceptName);


ONSEM_TEXTTOSEMANTIC_API
void addASemExp(UniqueSemanticExpression& pSemExp,
                UniqueSemanticExpression pSemExpToAdd);

ONSEM_TEXTTOSEMANTIC_API
void moveChildrenOfAGrdExp(GroundedExpression& pGrdExpToFill,
                           GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression grdExpsToUniqueSemExp(const std::list<const GroundedExpression*>& pGrdExps);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression fromImperativeToActionDescription(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression fromActionDescriptionToSentenceInPresentTense(const GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void addChildrenOfAnotherSemExp(SemanticExpression& pSemExpToFill,
                                const SemanticExpression& pSemExp,
                                ListExpressionType pListType);

ONSEM_TEXTTOSEMANTIC_API
void addEmptyIntroductingWord(GroundedExpression& pGrdExp);

ONSEM_TEXTTOSEMANTIC_API
void addChildFromSemExp(SemanticExpression& pSemExp,
                        GrammaticalType pGramType,
                        UniqueSemanticExpression pNewChild,
                        ListExpressionType pListType = ListExpressionType::UNRELATED);

ONSEM_TEXTTOSEMANTIC_API
bool fillReferenceFromConcepts(SemanticReferenceType& pReferenceType,
                               const std::map<std::string, char>& pConcepts);

ONSEM_TEXTTOSEMANTIC_API
bool fillQuantityAndReferenceFromConcepts(SemanticQuantity& pQuantity,
                                          SemanticReferenceType& pReferenceType,
                                          const std::map<std::string, char>& pConcepts);

ONSEM_TEXTTOSEMANTIC_API
void fillConceptsForPronouns(std::map<std::string, char>& pSemConcepts,
                                                  const std::map<std::string, char>& pLingConcepts);

ONSEM_TEXTTOSEMANTIC_API
void fillSemanticConcepts(std::map<std::string, char>& pConcepts,
                          const std::map<std::string, char>& pConceptsToAdd);

ONSEM_TEXTTOSEMANTIC_API
void fillCoreference(mystd::optional<Coreference>& pCoreference,
                     const std::map<std::string, char>& pConcepts);

ONSEM_TEXTTOSEMANTIC_API
void removeSemExpPartsThatDoesntHaveAnAgent(UniqueSemanticExpression& pSemExp,
                                            const SemanticAgentGrounding& pAgentGrd);


ONSEM_TEXTTOSEMANTIC_API
bool removeYearInformation(SemanticExpression& pSemExp);

ONSEM_TEXTTOSEMANTIC_API
bool removeDayInformation(SemanticExpression& pSemExp);


template<typename TCHILD>
void addListElt(UniqueSemanticExpression& pUSemExp,
                ListExpressionType pListType,
                TCHILD&& pNewExp,
                bool pPushBackOrFront = true)
{
  ListExpression* listExp = pUSemExp->getListExpPtr();
  if (listExp != nullptr &&
      listExp->listType == pListType)
  {
    listExp->elts.emplace_back(std::move(pNewExp));
    return;
  }

  auto newListExp = mystd::make_unique<ListExpression>(pListType);
  if (pPushBackOrFront)
  {
    newListExp->elts.emplace_back(std::move(pUSemExp));
    newListExp->elts.emplace_back(std::move(pNewExp));
  }
  else
  {
    newListExp->elts.emplace_back(std::move(pNewExp));
    newListExp->elts.emplace_back(std::move(pUSemExp));
  }
  pUSemExp = std::move(newListExp);
}


template<typename TSEMEXP>
bool _tryToMergeGrdExpAndSemExp(GroundedExpression& pGrdExp1,
                                const TSEMEXP& pSemExp2)
{
  SemanticTimeGrounding* timeGrd1Ptr = pGrdExp1->getTimeGroundingPtr();
  if (timeGrd1Ptr == nullptr)
    return false;
  const GroundedExpression* grdExp2Ptr = pSemExp2->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp2Ptr == nullptr)
    return false;
  const SemanticTimeGrounding* timeGrd2Ptr = grdExp2Ptr->grounding().getTimeGroundingPtr();
  if (timeGrd2Ptr == nullptr)
    return false;
  timeGrd1Ptr->mergeWith(*timeGrd2Ptr);
  return true;
}


template<typename TSEMEXP>
bool _tryToMergeSemExps(UniqueSemanticExpression& pSemExp1,
                        const TSEMEXP& pSemExp2)
{
  GroundedExpression* grdExp1Ptr = pSemExp1->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp1Ptr != nullptr)
    return _tryToMergeGrdExpAndSemExp(*grdExp1Ptr, pSemExp2);
  return false;
}

template<typename TCHILD>
void addChild(GroundedExpression& pGrdExp,
              GrammaticalType pGramType,
              TCHILD&& pNewExp,
              ListExpressionType pListType,
              bool pTryToMergeWithExistingChild,
              bool pPushBackOrFront)
{
  auto itGramChild = pGrdExp.children.find(pGramType);
  if (itGramChild != pGrdExp.children.end())
  {
    if (!pTryToMergeWithExistingChild || !_tryToMergeSemExps(itGramChild->second, pNewExp))
      addListElt(itGramChild->second, pListType, std::move(pNewExp), pPushBackOrFront);
    return;
  }
  if (!pTryToMergeWithExistingChild || !_tryToMergeGrdExpAndSemExp(pGrdExp, pNewExp))
    pGrdExp.children.emplace(pGramType, std::move(pNewExp));
}


} // End of namespace SemExpModifier

} // End of namespace onsem

#endif // ALSEMEXPMODIFIER_H
