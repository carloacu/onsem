#include "occurrencerankconverter.hpp"
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>

namespace onsem
{
namespace semanticOccurrenceRankConverter
{

void process(UniqueSemanticExpression& pSemExp)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (grdExp->type == SemanticGroudingType::STATEMENT)
    {
      SemanticStatementGrounding& rootStatementGrd = grdExp->getStatementGrounding();
      if (rootStatementGrd.concepts.count("verb_equal_be") > 0 &&
          grdExp.children.size() == 1)
      {
        auto itSubject = grdExp.children.find(GrammaticalType::SUBJECT);
        if (itSubject != grdExp.children.end())
        {
          SemanticExpression& subjectSemExp = *itSubject->second;
          GroundedExpression* subjectGrdExpPtr = subjectSemExp.getGrdExpPtr_SkipWrapperPtrs();
          if (subjectGrdExpPtr != nullptr)
          {
            GroundedExpression& subjectGrdExp = *subjectGrdExpPtr;
            if (subjectGrdExp.children.size() == 2)
            {
              auto itSpecifierOfTheSubject = subjectGrdExp.children.find(GrammaticalType::SPECIFIER);
              if (itSpecifierOfTheSubject != subjectGrdExp.children.end() &&
                  SemExpGetter::getRank(*itSpecifierOfTheSubject->second) != 0)
              {
                auto itObjectOfTheSubject = subjectGrdExp.children.find(GrammaticalType::OBJECT);
                if (itObjectOfTheSubject != subjectGrdExp.children.end())
                {
                  UniqueSemanticExpression& objectUSemExp = itObjectOfTheSubject->second;
                  GroundedExpression* objectGrdExpPtr = objectUSemExp->getGrdExpPtr_SkipWrapperPtrs();
                  if (objectGrdExpPtr != nullptr)
                  {
                    GroundedExpression& objectGrdExp = *objectGrdExpPtr;
                    if (objectGrdExp->type == SemanticGroudingType::STATEMENT)
                    {
                      // Pattern of the semantic expression found we add the new question form
                      UniqueSemanticExpression cloneOfOldSemExp(pSemExp->clone());

                      SemanticStatementGrounding& objectStatementGrd = objectGrdExp->getStatementGrounding();
                      if (rootStatementGrd.verbTense != SemanticVerbTense::PRESENT)
                        objectStatementGrd.verbTense = rootStatementGrd.verbTense;
                      if (subjectGrdExp->concepts.find("occurrenceRank") != subjectGrdExp->concepts.end())
                      {
                        objectStatementGrd.requests.set(SemanticRequestType::TIME);
                      }
                      else
                      {
                        if (objectGrdExp.children.find(GrammaticalType::OBJECT) == objectGrdExp.children.end())
                        {
                          objectGrdExp.children.emplace(GrammaticalType::OBJECT,
                                                        mystd::make_unique<GroundedExpression>(subjectGrdExp.cloneGrounding()));
                        }
                        subjectGrdExp.moveGrounding([]
                        {
                          auto genGrd = mystd::make_unique<SemanticGenericGrounding>(SemanticReferenceType::DEFINITE,
                                                                                     SemanticEntityType::THING);
                          genGrd->concepts.emplace("occurrenceRank", 4);
                          return genGrd;
                        }());
                      }

                      UniqueSemanticExpression newRoot(std::move(objectUSemExp));
                      subjectGrdExp.children.erase(itObjectOfTheSubject);

                      GroundedExpression* newRootGrdExpPtr = newRoot->getGrdExpPtr_SkipWrapperPtrs();
                      assert(newRootGrdExpPtr != nullptr);
                      if (newRootGrdExpPtr != nullptr)
                      {
                        SemExpModifier::addChild(*newRootGrdExpPtr, GrammaticalType::OCCURRENCE_RANK,
                                                 std::move(itSubject->second));
                        auto res = mystd::make_unique<SetOfFormsExpression>();
                        res->prioToForms[-10].emplace_back(mystd::make_unique<QuestExpressionFrom>(std::move(cloneOfOldSemExp), true));
                        res->prioToForms[-11].emplace_back(mystd::make_unique<QuestExpressionFrom>(std::move(newRoot), false));
                        pSemExp = std::move(res);
                      }
                    }
                  }
                }
              }
            }
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
      process(currElt);
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



} // End of namespace semanticOccurrenceRankConverter
} // End of namespace onsem
