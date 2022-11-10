#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_ANSWERELEMENT_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_ANSWERELEMENT_HPP

#include <list>
#include <memory>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentence.hpp>
#include <onsem/semantictotext/semanticmemory/sentencewithlinks.hpp>
#include "../type/answerexp.hpp"
#include "semanticannotation.hpp"


namespace onsem
{


struct AnswerElement
{
  AnswerElement()
    : relatedContextAxioms()
  {
  }

  virtual const GroundedExpression& getGrdExpRef() const = 0;
  virtual std::unique_ptr<GroundedExpressionContainer> getGrdExpContainer() = 0;
  virtual const SemanticAnnotations& getAnnotations() const = 0;
  virtual std::unique_ptr<SemanticExpressionContainer> getSemExpForGrammaticalType(GrammaticalType pGrammType,
                                                                                   const GroundedExpression* pFromGrdExpQuestion,
                                                                                   const linguistics::LinguisticDatabase* pLingDb,
                                                                                   bool* pHasSamePolarityPtr) = 0;
  virtual const std::list<SemanticMemorySentence>& getMemorySentences() const = 0;

  RelatedContextAxiom relatedContextAxioms;
};


struct AnswerElementDynamic : public AnswerElement
{
  AnswerElementDynamic()
    : AnswerElement(),
      _memSentPtr(nullptr),
      _annotations(std::make_unique<SemanticAnnotationsPtrs>())
  {
  }

  AnswerElementDynamic
  (const SemanticMemorySentence* pMemSentPtr)
    : AnswerElement(),
      _memSentPtr(pMemSentPtr),
      _annotations(std::make_unique<SemanticAnnotationsPtrs>(&pMemSentPtr->getAnnotations()))
  {
  }

  const GroundedExpression& getGrdExpRef() const override { return _memSentPtr->grdExp; }
  std::unique_ptr<GroundedExpressionContainer> getGrdExpContainer() override { return std::make_unique<GroundedExpressionRef>(_memSentPtr->grdExp); }
  const SemanticAnnotations& getAnnotations() const override { return *_annotations; }
  std::unique_ptr<SemanticExpressionContainer> getSemExpForGrammaticalType(GrammaticalType pGrammType,
                                                                           const GroundedExpression* pFromGrdExpQuestion,
                                                                           const linguistics::LinguisticDatabase* pLingDb,
                                                                           bool* pHasSamePolarityPtr) override;
  const std::list<SemanticMemorySentence>& getMemorySentences() const override
  { return _memSentPtr->getContextAxiom().memorySentences.elts; }

private:
  const SemanticMemorySentence* _memSentPtr;
  std::unique_ptr<SemanticAnnotationsPtrs> _annotations;
};



struct AnswerElementStatic : public AnswerElement
{
  AnswerElementStatic(const unsigned char* pMemSentPtr,
                      const linguistics::LinguisticDatabase& pLingDb);

  const GroundedExpression& getGrdExpRef() const override
  {
    if (!_grdExpPtr)
      _fillGrdExp();
    return _grdExpPtr->getGrdExp();
  }
  std::unique_ptr<GroundedExpressionContainer> getGrdExpContainer() override
  {
    if (!_grdExpPtr)
      _fillGrdExp();
    return std::move(_grdExpPtr);
  }
  const SemanticAnnotations& getAnnotations() const override
  {
    if (!_annotations)
      _fillAnnotations();
    return *_annotations;
  }
  std::unique_ptr<SemanticExpressionContainer> getSemExpForGrammaticalType(GrammaticalType pGrammType,
                                                                           const GroundedExpression* pFromGrdExpQuestion,
                                                                           const linguistics::LinguisticDatabase* pLingDb,
                                                                           bool* pHasSamePolarityPtr) override;
  const std::list<SemanticMemorySentence>& getMemorySentences() const override { return _memSents; }

private:
  const unsigned char* _memSentPtr;
  mutable std::unique_ptr<GroundedExpression> _grdExpPtr;
  mutable std::unique_ptr<SemanticAnnotationsInstances> _annotations;
  std::list<SemanticMemorySentence> _memSents;
  const linguistics::LinguisticDatabase& _lingDb;

  void _fillGrdExp() const;
  void _fillAnnotations() const;
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_ANSWERELEMENT_HPP
