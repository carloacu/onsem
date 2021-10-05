#ifndef ALSEMEXPDATABASES_GROUNDEDEXPRESSION_H
#define ALSEMEXPDATABASES_GROUNDEDEXPRESSION_H

#include "semanticexpression.hpp"
#include <memory>
#include "../../api.hpp"

namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API GroundedExpressionContainer
{
  virtual ~GroundedExpressionContainer() {}

  virtual const GroundedExpression& getGrdExp() const = 0;
};


struct ONSEM_TEXTTOSEMANTIC_API GroundedExpression : public SemanticExpression,
    public GroundedExpressionContainer
{
  // Constructors
  GroundedExpression();
  template<typename TGROUDING>
  GroundedExpression(std::unique_ptr<TGROUDING> pGrounding);

  GroundedExpression(const GroundedExpression&) = delete;
  GroundedExpression& operator=(const GroundedExpression&) = delete;

  GroundedExpression& getGrdExp() override { return *this; }
  const GroundedExpression& getGrdExp() const override { return *this; }
  GroundedExpression* getGrdExpPtr() override { return this; }
  const GroundedExpression* getGrdExpPtr() const override { return this; }

  bool operator==(const GroundedExpression& pOther) const;
  bool isEqual(const GroundedExpression& pOther) const;
  void assertEltsEqual(const GroundedExpression& pOther) const;

  // Setters
  /// Sets the current grounding to the provided grounding.
  template<typename TGROUDING>
  void moveGrounding(std::unique_ptr<TGROUDING> pGrounding);


  // Getters
  const SemanticGrounding& grounding() const;
  SemanticGrounding& grounding();

  SemanticGrounding* operator->();
  const SemanticGrounding* operator->() const;
  SemanticGrounding& operator*();
  const SemanticGrounding& operator*() const;


  // Copiers
  std::unique_ptr<GroundedExpression> clone(const IndexToSubNameToParameterValue* pParams = nullptr,
                                            bool pRemoveRecentContextInterpretations = false,
                                            const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;
  std::unique_ptr<SemanticGrounding> cloneGrounding(const IndexToSubNameToParameterValue* pParams = nullptr) const;



  std::map<GrammaticalType, UniqueSemanticExpression> children;

private:
  std::unique_ptr<SemanticGrounding> _grounding;
};



struct ONSEM_TEXTTOSEMANTIC_API GroundedExpressionRef : public GroundedExpressionContainer
{
  GroundedExpressionRef(const GroundedExpression& pGrdExp)
    : GroundedExpressionContainer(),
      _grdExp(pGrdExp)
  {
  }

  const GroundedExpression& getGrdExp() const override { return _grdExp; }

private:
  const GroundedExpression& _grdExp;
};


struct ONSEM_TEXTTOSEMANTIC_API GroundedExpressionFromSemExp : public GroundedExpressionContainer
{
  GroundedExpressionFromSemExp(UniqueSemanticExpression pSemExp)
    : GroundedExpressionContainer(),
      _semExp(std::move(pSemExp))
  {
    assert(_semExp->type == SemanticExpressionType::GROUNDED);
  }

  const GroundedExpression& getGrdExp() const override { return _semExp->getGrdExp(); }

private:
  UniqueSemanticExpression _semExp;
};


} // End of namespace onsem

#include "details/groundedexpression.hxx"


#endif // ALSEMEXPDATABASES_GROUNDEDEXPRESSION_H
