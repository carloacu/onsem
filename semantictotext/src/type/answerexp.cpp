#include "answerexp.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include "../semanticmemory/semanticmemorygrdexp.hpp"
#include "semanticdetailledanswer.hpp"

namespace onsem
{

void RelatedContextAxiom::add(const RelatedContextAxiom& pOther)
{
  for (const auto& currElt : pOther.elts)
    elts.emplace_back(currElt);
  for (const auto& currElt : pOther.constElts)
    constElts.emplace_back(currElt);
}

void RelatedContextAxiom::add(const SemanticMemoryGrdExp& pSemMemoryGrdExp)
{
  constElts.emplace_back(&pSemMemoryGrdExp.getMemSentence().getContextAxiom());
}

bool RelatedContextAxiom::haveThisExpHandleInMemory(const ExpressionHandleInMemory* pExpHandleInMemory) const
{
  for (const auto& currElt : elts)
    if (pExpHandleInMemory == &currElt->getSemExpWrappedForMemory())
      return true;
  for (const auto& currElt : constElts)
    if (pExpHandleInMemory == &currElt->getSemExpWrappedForMemory())
      return true;
  return false;
}

void RelatedContextAxiom::getReferences(std::list<std::string>& pReferences) const
{
  for (const auto& currAxiom : elts)
    currAxiom->getReferences(pReferences);
  for (const auto& currAxiom : constElts)
    currAxiom->getReferences(pReferences);
}

bool RelatedContextAxiom::isAnAssertion() const
{
  for (const auto& currAxiom : elts)
    if (currAxiom != nullptr &&
        currAxiom->informationType == InformationType::ASSERTION)
      return true;
  for (const auto& currAxiom : constElts)
    if (currAxiom != nullptr &&
        currAxiom->informationType == InformationType::ASSERTION)
      return true;
  return false;
}

bool RelatedContextAxiom::isEmpty() const
{
  return elts.empty() && constElts.empty();
}


bool answerExpAreEqual(const AnswerExp& pAnswerExp1,
                       const AnswerExp& pAnswerExp2,
                       const SemanticMemoryBlock& pMemBlock,
                       const linguistics::LinguisticDatabase& pLingDb)
{
  return SemExpComparator::grdExpsReferToSameInstance(pAnswerExp1.getGrdExp(), pAnswerExp2.getGrdExp(), pMemBlock, pLingDb);
}


} // End of namespace onsem
