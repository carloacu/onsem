#include "reasonofrefactor.hpp"
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>

namespace onsem
{
namespace semanticReasonOfRefactor
{

void process(UniqueSemanticExpression& pSemExp)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (grdExp->type == SemanticGroundingType::STATEMENT)
    {
      auto itReasonOf = grdExp.children.find(GrammaticalType::REASONOF);
      if (itReasonOf != grdExp.children.end())
      {
        const SemanticStatementGrounding& statGrd = grdExp->getStatementGrounding();
        if (statGrd.concepts.count("verb_equal_be") > 0)
        {
          auto iSubject = grdExp.children.find(GrammaticalType::SUBJECT);
          if (iSubject != grdExp.children.end())
          {
            UniqueSemanticExpression reasonOfExp = std::move(itReasonOf->second);
            grdExp.children.erase(itReasonOf);
            SemExpModifier::removeChildFromSemExp(*reasonOfExp, GrammaticalType::INTRODUCTING_WORD);
            pSemExp = mystd::make_unique<ConditionExpression>
                (true, true, std::move(iSubject->second), std::move(reasonOfExp));
          }
        }
      }
    }
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    for (auto& currElt : listExp.elts)
    {
      process(currElt);
    }
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    process(pSemExp->getIntExp().interpretedExp);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    process(pSemExp->getFdkExp().concernedExp);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    process(pSemExp->getAnnExp().semExp);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    process(pSemExp->getMetadataExp().semExp);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}



} // End of namespace semanticReasonOfRefactor
} // End of namespace onsem
