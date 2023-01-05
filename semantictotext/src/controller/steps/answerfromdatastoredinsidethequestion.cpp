#include "answerfromdatastoredinsidethequestion.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>

namespace onsem
{
namespace answerFromDataStoredInsideTheQuestion
{

void getAnswers(std::map<QuestionAskedInformation, AllAnswerElts>& pAllAnswers,
                const GroundedExpression& pGrdExp,
                SemanticRequestType pRootRequest,
                const linguistics::LinguisticDatabase& pLingDb)
{
  const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
  {
    const SemanticStatementGrounding& statGrd = *statGrdPtr;

    SemanticRequestType request = statGrd.requests.firstOrNothing();
    switch (request)
    {
    case SemanticRequestType::TIME:
    {
      auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
      if (itSubject != pGrdExp.children.end())
      {
        const SemanticExpression* timeSemExpPtr = SemExpGetter::getGrammTypeInfo(*itSubject->second, GrammaticalType::TIME);
        if (timeSemExpPtr != nullptr)
          pAllAnswers[request].answersGenerated.emplace_back(timeSemExpPtr->clone());
      }
      break;
    }
    case SemanticRequestType::SUBJECT:
    {
      if (ConceptSet::haveAConcept(pGrdExp->concepts, "verb_equal_be"))
      {
        auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
        if (itSubject != pGrdExp.children.end())
        {
          const GroundedExpression* subjectGrdExpPtr = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
          if (subjectGrdExpPtr != nullptr)
          {
            auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
            if (itObject != pGrdExp.children.end())
            {
              const GroundedExpression* objectGrdExpPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
              if (objectGrdExpPtr != nullptr)
              {
                auto itSpecificChild = objectGrdExpPtr->children.find(GrammaticalType::SPECIFIER);
                if (itSpecificChild != objectGrdExpPtr->children.end())
                {
                  const GroundedExpression* specificChildGrdExpPtr = itSpecificChild->second->getGrdExpPtr_SkipWrapperPtrs();
                  if (specificChildGrdExpPtr != nullptr &&
                      SemExpComparator::isAnInstanceOf(*specificChildGrdExpPtr, *subjectGrdExpPtr, pLingDb))
                  {
                    pAllAnswers[pRootRequest].answersGenerated.emplace_back(specificChildGrdExpPtr->clone());
                  }
                }
              }
            }
          }
        }
      }
      break;
    }
    default:
      break;
    }
  }
}


} // End of namespace answerFromDataStoredInsideTheQuestion
} // End of namespace onsem
