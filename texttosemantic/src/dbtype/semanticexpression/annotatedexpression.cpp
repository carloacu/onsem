#include <onsem/texttosemantic/dbtype/semanticexpression/annotatedexpression.hpp>

namespace onsem
{


AnnotatedExpression::AnnotatedExpression
(UniqueSemanticExpression&& pSemExp)
  : SemanticExpression(SemanticExpressionType::ANNOTATED),
    synthesizeAnnotations(false),
    annotations(),
    semExp(std::move(pSemExp))
{
}


void AnnotatedExpression::assertEltsEqual(const AnnotatedExpression& pOther) const
{
  assert(synthesizeAnnotations == pOther.synthesizeAnnotations);
  _assertChildrenEqual(annotations, pOther.annotations);
  semExp->assertEqual(*pOther.semExp);
}


} // End of namespace onsem
