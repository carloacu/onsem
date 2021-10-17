#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_GROUNDEDEXPRESSION_HXX
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_GROUNDEDEXPRESSION_HXX

#include "../groundedexpression.hpp"


namespace onsem
{


template<typename TGROUDING>
GroundedExpression::GroundedExpression
(std::unique_ptr<TGROUDING> pGrounding)
  : SemanticExpression(SemanticExpressionType::GROUNDED),
    GroundedExpressionContainer(),
    children(),
    _grounding(std::move(pGrounding))
{
  assert(_grounding);
}


inline bool GroundedExpression::operator==(const GroundedExpression& pOther) const
{
  return isEqual(pOther);
}

inline bool GroundedExpression::isEqual(const GroundedExpression& pOther) const
{
  return children == pOther.children &&
      areEquals(_grounding, pOther._grounding);
}


template<typename TGROUDING>
void GroundedExpression::moveGrounding
(std::unique_ptr<TGROUDING> pGrounding)
{
  _grounding = std::move(pGrounding);
  assert(_grounding);
}


inline const SemanticGrounding& GroundedExpression::grounding() const
{
  return *_grounding;
}


inline SemanticGrounding& GroundedExpression::grounding()
{
  return *_grounding;
}

inline SemanticGrounding* GroundedExpression::operator->()
{
  return &*_grounding;
}

inline const SemanticGrounding* GroundedExpression::operator->() const
{
  return &*_grounding;
}

inline SemanticGrounding& GroundedExpression::operator*()
{
  return *_grounding;
}

inline const SemanticGrounding& GroundedExpression::operator*() const
{
  return *_grounding;
}




} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_GROUNDEDEXPRESSION_HXX
