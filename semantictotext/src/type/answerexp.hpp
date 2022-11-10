#ifndef ONSEM_SEMANTICTOTEXT_SRC_TYPE_ANSWEREXP_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_TYPE_ANSWEREXP_HPP

#include <list>
#include <memory>
#include <assert.h>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/semanticmemory/referencesgetter.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SentenceWithLinks;
struct SemanticMemoryBlock;
struct ExpressionWithLinks;
struct SemanticMemoryGrdExp;


struct RelatedContextAxiom : public ReferencesGetter
{
  void add(const RelatedContextAxiom& pOther);
  void add(const SemanticMemoryGrdExp& pSemMemoryGrdExp);
  bool haveThisExpHandleInMemory(const ExpressionWithLinks* pExpHandleInMemory) const;
  void getReferences(std::list<std::string>& pReferences) const override;
  bool isAnAssertion() const;
  bool isEmpty() const;

  std::list<SentenceWithLinks*> elts;
  std::list<const SentenceWithLinks*> constElts;
};

struct AnswerExpGenerated
{
  AnswerExpGenerated(UniqueSemanticExpression pGenSemExp,
                     const RelatedContextAxiom* pRelatedContextAxiomsPtr = nullptr)
    : relatedContextAxioms(pRelatedContextAxiomsPtr != nullptr ? *pRelatedContextAxiomsPtr : RelatedContextAxiom{}),
      genSemExp(std::move(pGenSemExp))
  {
  }

  RelatedContextAxiom relatedContextAxioms;
  UniqueSemanticExpression genSemExp;
};


struct AnswerExp
{
public:
  AnswerExp
  (const RelatedContextAxiom& pRelatedContextAxioms,
   std::unique_ptr<GroundedExpressionContainer> pGrdExp,
   const SemanticExpression* pEqualitySemExp,
   std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer)
    : relatedContextAxioms(pRelatedContextAxioms),
      equalitySemExp(pEqualitySemExp),
      annotationsOfTheAnswer(),
      _grdExp(std::move(pGrdExp))
  {
    annotationsOfTheAnswer.swap(pAnnotationsOfTheAnswer);
  }

  AnswerExp
  (const SentenceWithLinks& pContextAxiom,
   std::unique_ptr<GroundedExpressionContainer> pGrdExp,
   const SemanticExpression* pEqualitySemExp,
   std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer)
    : relatedContextAxioms(),
      equalitySemExp(pEqualitySemExp),
      annotationsOfTheAnswer(),
      _grdExp(std::move(pGrdExp))
  {
    relatedContextAxioms.constElts.emplace_back(&pContextAxiom);
    annotationsOfTheAnswer.swap(pAnnotationsOfTheAnswer);
  }

  const GroundedExpression& getGrdExp() const
  { return _grdExp->getGrdExp(); }

  RelatedContextAxiom relatedContextAxioms;
  const SemanticExpression* equalitySemExp;
  std::map<GrammaticalType, const SemanticExpression*> annotationsOfTheAnswer;

private:
  std::unique_ptr<GroundedExpressionContainer> _grdExp;
};



bool answerExpAreEqual(const AnswerExp& pAnswerExp1,
                       const AnswerExp& pAnswerExp2,
                       const SemanticMemoryBlock& pMemBlock,
                       const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_TYPE_ANSWEREXP_HPP
