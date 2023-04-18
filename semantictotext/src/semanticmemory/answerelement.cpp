#include "answerelement.hpp"
#include "../type/answerexp.hpp"
#include <onsem/common/binary/enummapreader.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexploader.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include "semanticmemoryblockbinaryreader.hpp"

namespace onsem
{


std::unique_ptr<SemanticExpressionContainer> AnswerElementDynamic::getSemExpForGrammaticalType(
    GrammaticalType pGrammType,
    const GroundedExpression* pFromGrdExpQuestion,
    const linguistics::LinguisticDatabase* pLingDb,
    bool* pHasSamePolarityPtr) const
{
  auto* rawRes = _memSentPtr->getSemExpForGrammaticalType(pGrammType);
  if (rawRes != nullptr)
  {
    if (pHasSamePolarityPtr != nullptr && pFromGrdExpQuestion != nullptr && pLingDb != nullptr)
      *pHasSamePolarityPtr = SemExpComparator::haveSamePolarity(_memSentPtr->grdExp, *pFromGrdExpQuestion, pLingDb->conceptSet, true);
    return std::make_unique<ReferenceOfSemanticExpressionContainer>(*rawRes);
  }
  return {};
}



AnswerElementStatic::AnswerElementStatic(const unsigned char* pMemSentPtr,
                                         const linguistics::LinguisticDatabase& pLingDb)
  : AnswerElement(),
    _memSentPtr(pMemSentPtr),
    _grdExpPtr(),
    _annotations(),
    _memSents(),
    _lingDb(pLingDb)
{
}

void AnswerElementStatic::_fillGrdExp() const
{
  const unsigned char* grdExpPtr = SemanticMemoryBlockBinaryReader::memorySentenceToGrdExpPtr(_memSentPtr);
  _grdExpPtr = semexploader::loadGrdExp(grdExpPtr, _lingDb);
}

void AnswerElementStatic::_fillAnnotations() const
{
  _annotations = std::make_unique<SemanticAnnotationsInstances>();
  auto* annotationsPtr = SemanticMemoryBlockBinaryReader::memorySentenceToAnnotationsPtr(_memSentPtr);
  readAllEnumMapValues(annotationsPtr,
                       grammaticalType_size, [&]
                       (unsigned char pKey,
                       const unsigned char* pValue)
  {
    auto* semExpPtr = SemanticMemoryBlockBinaryReader::readExpPtr(pValue);
    _annotations->addAnnotation(grammaticalType_fromChar(pKey), semexploader::loadSemExp(semExpPtr, _lingDb));
  });
}


std::unique_ptr<SemanticExpressionContainer> AnswerElementStatic::getSemExpForGrammaticalType(
    GrammaticalType pGrammType,
    const GroundedExpression* pFromGrdExpQuestion,
    const linguistics::LinguisticDatabase* pLingDb,
    bool* pHasSamePolarityPtr) const
{
  if (!_grdExpPtr)
    _fillGrdExp();
  const auto& grdExp = *_grdExpPtr;
  if (!_annotations)
    _fillAnnotations();
  auto* semExpPtr = _annotations->grammaticalTypeToSemExpPtr(pGrammType);
  if (semExpPtr != nullptr)
  {
    if (pHasSamePolarityPtr != nullptr && pFromGrdExpQuestion != nullptr && pLingDb != nullptr)
      *pHasSamePolarityPtr = SemExpComparator::haveSamePolarity(grdExp, *pFromGrdExpQuestion, pLingDb->conceptSet, true);
    return std::make_unique<UniqueSemanticExpression>(semExpPtr->clone());
  }

  const auto itChild = grdExp.children.find(pGrammType);
  if (itChild != grdExp.children.end())
  {
    if (pHasSamePolarityPtr != nullptr && pFromGrdExpQuestion != nullptr && pLingDb != nullptr)
      *pHasSamePolarityPtr = SemExpComparator::haveSamePolarity(grdExp, *pFromGrdExpQuestion, pLingDb->conceptSet, true);
    return std::make_unique<UniqueSemanticExpression>(itChild->second->clone());
  }
  return {};
}



} // End of namespace onsem

