#include <onsem/semantictotext/semanticmemory/semanticcontextaxiom.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semantictracker.hpp>

namespace onsem
{

SemanticTriggerAxiomId::SemanticTriggerAxiomId()
  : nbOfAxioms(1),
    idOfAxiom(0),
    semExpType(SemanticExpressionType::LIST),
    listExpType(ListExpressionType::UNRELATED)
{
}

SemanticTriggerAxiomId::SemanticTriggerAxiomId(std::size_t pNbOfAxioms,
                                               std::size_t pIdOfAxiom,
                                               SemanticExpressionType pSemExpType)
  : nbOfAxioms(pNbOfAxioms),
    idOfAxiom(pIdOfAxiom),
    semExpType(pSemExpType),
    listExpType(ListExpressionType::UNRELATED)
{
}

SemanticTriggerAxiomId::SemanticTriggerAxiomId(std::size_t pNbOfAxioms,
                                               std::size_t pIdOfAxiom,
                                               ListExpressionType pListExpType)
  : nbOfAxioms(pNbOfAxioms),
    idOfAxiom(pIdOfAxiom),
    semExpType(SemanticExpressionType::LIST),
    listExpType(pListExpType)
{
}

bool SemanticTriggerAxiomId::isEmpty() const
{
  return nbOfAxioms == 1 && idOfAxiom == 0 &&
      semExpType == SemanticExpressionType::LIST &&
      listExpType == ListExpressionType::UNRELATED;
}

bool SemanticTriggerAxiomId::operator<(const SemanticTriggerAxiomId& pOther) const
{
  if (nbOfAxioms != pOther.nbOfAxioms)
    return nbOfAxioms < pOther.nbOfAxioms;
  if (idOfAxiom != pOther.idOfAxiom)
    return idOfAxiom < pOther.idOfAxiom;
  if (semExpType != pOther.semExpType)
    return semExpType < pOther.semExpType;
  return listExpType < pOther.listExpType;
}



SemanticContextAxiom::SemanticContextAxiom
(InformationType pInformationType,
 ExpressionWithLinks& pSemExpWrappedForMemory)
  : semTracker(),
    informationType(pInformationType),
    triggerAxiomId(),
    semExpToDoIsAlwaysActive(false),
    semExpToDo(nullptr),
    semExpToDoElse(nullptr),
    infCommandToDo(nullptr),
    memorySentences(),
    _semExpWrappedForMemory(pSemExpWrappedForMemory)
{
}


void SemanticContextAxiom::setEnabled(bool pEnabled)
{
  memorySentences.setEnabled(pEnabled);
}

void SemanticContextAxiom::clear()
{
  memorySentences.clear();

  semTracker.reset();
  semExpToDo = nullptr;
  semExpToDoElse = nullptr;
  infCommandToDo = nullptr;
}

bool SemanticContextAxiom::isAnActionLinked() const
{
  return semExpToDo != nullptr || semExpToDoElse != nullptr;
}

bool SemanticContextAxiom::canOtherInformationTypeBeMoreRevelant(InformationType pInformationType) const
{
  return pInformationType == InformationType::ASSERTION ||
      informationType != InformationType::ASSERTION;
}

void SemanticContextAxiom::getReferences(std::list<std::string>& pReferences) const
{
  SemExpGetter::extractReferences(pReferences, *_semExpWrappedForMemory.semExp);
}



} // End of namespace onsem

