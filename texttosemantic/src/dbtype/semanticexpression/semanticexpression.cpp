#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>

namespace onsem
{

SemanticExpression::SemanticExpression(SemanticExpressionType pSemExpType)
  : type(pSemExpType)
{
}



SemanticExpression::~SemanticExpression()
{
}


GroundedExpression& SemanticExpression::getGrdExp()
{
  assert(false);
  return *dynamic_cast<GroundedExpression*>(this);
}

const GroundedExpression& SemanticExpression::getGrdExp() const
{
  assert(false);
  return *dynamic_cast<const GroundedExpression*>(this);
}


ListExpression& SemanticExpression::getListExp()
{
  assert(false);
  return *dynamic_cast<ListExpression*>(this);
}

const ListExpression& SemanticExpression::getListExp() const
{
  assert(false);
  return *dynamic_cast<const ListExpression*>(this);
}


ConditionExpression& SemanticExpression::getCondExp()
{
  assert(false);
  return *dynamic_cast<ConditionExpression*>(this);
}

const ConditionExpression& SemanticExpression::getCondExp() const
{
  assert(false);
  return *dynamic_cast<const ConditionExpression*>(this);
}


ComparisonExpression& SemanticExpression::getCompExp()
{
  assert(false);
  return *dynamic_cast<ComparisonExpression*>(this);
}

const ComparisonExpression& SemanticExpression::getCompExp() const
{
  assert(false);
  return *dynamic_cast<const ComparisonExpression*>(this);
}


InterpretationExpression& SemanticExpression::getIntExp()
{
  assert(false);
  return *dynamic_cast<InterpretationExpression*>(this);
}

const InterpretationExpression& SemanticExpression::getIntExp() const
{
  assert(false);
  return *dynamic_cast<const InterpretationExpression*>(this);
}


template <typename ReturnType, typename SemExpType>
ReturnType getIntExpPtrSkipWrapperPtrs(SemExpType& self)
{
  switch (self.type)
  {
  case SemanticExpressionType::INTERPRETATION:
  {
    return self.getIntExpPtr();
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return self.getFdkExp().concernedExp->getIntExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return self.getAnnExp().semExp->getIntExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::METADATA:
  {
    return self.getMetadataExp().semExp->getIntExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::COMMAND:
  {
    return self.getCmdExp().semExp->getIntExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = self.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return (*originalFrom)->getIntExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = self.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      return semExp->getIntExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::LIST:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::GROUNDED:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


InterpretationExpression* SemanticExpression::getIntExpPtr_SkipWrapperPtrs()
{
  return getIntExpPtrSkipWrapperPtrs<InterpretationExpression*>(*this);
}

const InterpretationExpression* SemanticExpression::getIntExpPtr_SkipWrapperPtrs() const
{
  return getIntExpPtrSkipWrapperPtrs<const InterpretationExpression*>(*this);
}


SetOfFormsExpression& SemanticExpression::getSetOfFormsExp()
{
  assert(false);
  return *dynamic_cast<SetOfFormsExpression*>(this);
}

const SetOfFormsExpression& SemanticExpression::getSetOfFormsExp() const
{
  assert(false);
  return *dynamic_cast<const SetOfFormsExpression*>(this);
}


FeedbackExpression& SemanticExpression::getFdkExp()
{
  assert(false);
  return *dynamic_cast<FeedbackExpression*>(this);
}

const FeedbackExpression& SemanticExpression::getFdkExp() const
{
  assert(false);
  return *dynamic_cast<const FeedbackExpression*>(this);
}


AnnotatedExpression& SemanticExpression::getAnnExp()
{
  assert(false);
  return *dynamic_cast<AnnotatedExpression*>(this);
}

const AnnotatedExpression& SemanticExpression::getAnnExp() const
{
  assert(false);
  return *dynamic_cast<const AnnotatedExpression*>(this);
}


MetadataExpression& SemanticExpression::getMetadataExp()
{
  assert(false);
  return *dynamic_cast<MetadataExpression*>(this);
}

const MetadataExpression& SemanticExpression::getMetadataExp() const
{
  assert(false);
  return *dynamic_cast<const MetadataExpression*>(this);
}


CommandExpression& SemanticExpression::getCmdExp()
{
  assert(false);
  return *dynamic_cast<CommandExpression*>(this);
}

const CommandExpression& SemanticExpression::getCmdExp() const
{
  assert(false);
  return *dynamic_cast<const CommandExpression*>(this);
}


FixedSynthesisExpression& SemanticExpression::getFSynthExp()
{
  assert(false);
  return *dynamic_cast<FixedSynthesisExpression*>(this);
}

const FixedSynthesisExpression& SemanticExpression::getFSynthExp() const
{
  assert(false);
  return *dynamic_cast<const FixedSynthesisExpression*>(this);
}




template <typename ReturnType, typename SemExpType>
ReturnType getGrdExpPtrSkipWrapperPtrs(SemExpType& self,
                                       bool pFollowInterpretations)
{
  switch (self.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    return self.getGrdExpPtr();
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = self.getIntExp();
    if (pFollowInterpretations)
      return intExp.interpretedExp->getGrdExpPtr_SkipWrapperPtrs();
    return intExp.originalExp->getGrdExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return self.getFdkExp().concernedExp->getGrdExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return self.getAnnExp().semExp->getGrdExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::METADATA:
  {
    return self.getMetadataExp().semExp->getGrdExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::COMMAND:
  {
    return self.getCmdExp().semExp->getGrdExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = self.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return (*originalFrom)->getGrdExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = self.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      return semExp->getGrdExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::LIST:
    return nullptr;
  }
  assert(false);
  return nullptr;
}

GroundedExpression* SemanticExpression::getGrdExpPtr_SkipWrapperPtrs(bool pFollowInterpretations)
{
  return getGrdExpPtrSkipWrapperPtrs<GroundedExpression*>(*this, pFollowInterpretations);
}

const GroundedExpression* SemanticExpression::getGrdExpPtr_SkipWrapperPtrs(bool pFollowInterpretations) const
{
  return getGrdExpPtrSkipWrapperPtrs<const GroundedExpression*>(*this, pFollowInterpretations);
}

template <typename ReturnType, typename SemExpType>
ReturnType getListExpPtrSkipWrapperPtrs(SemExpType& self, bool pFollowInterpretations)
{
  switch (self.type)
  {
  case SemanticExpressionType::LIST:
  {
    return self.getListExpPtr();
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    if (pFollowInterpretations)
      return self.getIntExp().interpretedExp->getListExpPtr_SkipWrapperPtrs();
    return self.getIntExp().originalExp->getListExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return self.getFdkExp().concernedExp->getListExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return self.getAnnExp().semExp->getListExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::METADATA:
  {
    return self.getMetadataExp().semExp->getListExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::COMMAND:
  {
    return self.getCmdExp().semExp->getListExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = self.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return (*originalFrom)->getListExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = self.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      return semExp->getListExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::GROUNDED:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


ListExpressionType SemanticExpression::getGrdExpPtrs_SkipWrapperLists(std::list<GroundedExpression*>& pRes,
                                                                      bool pFollowInterpretations)
{
  GroundedExpression* grdExpPtr = getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
  if (grdExpPtr != nullptr)
  {
    pRes.emplace_back(grdExpPtr);
    return ListExpressionType::UNRELATED;
  }

  ListExpression* listExpPtr = getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    for (auto& currElt : listExpPtr->elts)
      currElt->getGrdExpPtrs_SkipWrapperLists(pRes, pFollowInterpretations);
    return listExpPtr->listType;
  }
  return ListExpressionType::UNRELATED;
}


ListExpressionType SemanticExpression::getGrdExpPtrs_SkipWrapperLists(std::list<const GroundedExpression*>& pRes,
                                                                      bool pFollowInterpretations) const
{
  const GroundedExpression* grdExpPtr = getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
  if (grdExpPtr != nullptr)
  {
    pRes.emplace_back(grdExpPtr);
    return ListExpressionType::UNRELATED;
  }

  const ListExpression* listExpPtr = getListExpPtr_SkipWrapperPtrs(pFollowInterpretations);
  if (listExpPtr != nullptr)
  {
    for (auto& currElt : listExpPtr->elts)
      currElt->getGrdExpPtrs_SkipWrapperLists(pRes, pFollowInterpretations);
    return listExpPtr->listType;
  }
  return ListExpressionType::UNRELATED;
}


ListExpression* SemanticExpression::getListExpPtr_SkipWrapperPtrs(bool pFollowInterpretations)
{
  return getListExpPtrSkipWrapperPtrs<ListExpression*>(*this, pFollowInterpretations);
}

const ListExpression* SemanticExpression::getListExpPtr_SkipWrapperPtrs(bool pFollowInterpretations) const
{
  return getListExpPtrSkipWrapperPtrs<const ListExpression*>(*this, pFollowInterpretations);
}


template <typename ReturnType, typename SemExpType>
ReturnType getCondExpPtrSkipWrapperPtrs(SemExpType& self, bool pFollowInterpretations)
{
  switch (self.type)
  {
  case SemanticExpressionType::CONDITION:
  {
    return self.getCondExpPtr();
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    if (pFollowInterpretations)
      return self.getIntExp().interpretedExp->getCondExpPtr_SkipWrapperPtrs();
    return self.getIntExp().originalExp->getCondExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::FEEDBACK:
  {
    return self.getFdkExp().concernedExp->getCondExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return self.getAnnExp().semExp->getCondExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::METADATA:
  {
    return self.getMetadataExp().semExp->getCondExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::COMMAND:
  {
    return self.getCmdExp().semExp->getCondExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = self.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return (*originalFrom)->getCondExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = self.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      return semExp->getCondExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::LIST:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::GROUNDED:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


ConditionExpression* SemanticExpression::getCondExpPtr_SkipWrapperPtrs(bool pFollowInterpretations)
{
  return getCondExpPtrSkipWrapperPtrs<ConditionExpression*>(*this, pFollowInterpretations);
}

const ConditionExpression* SemanticExpression::getCondExpPtr_SkipWrapperPtrs(bool pFollowInterpretations) const
{
  return getCondExpPtrSkipWrapperPtrs<const ConditionExpression*>(*this, pFollowInterpretations);
}


template <typename ReturnType, typename SemExpType>
ReturnType getFdkExpPtrSkipWrapperPtrs(SemExpType& self, bool pFollowInterpretations)
{
  switch (self.type)
  {
  case SemanticExpressionType::FEEDBACK:
  {
    return self.getFdkExpPtr();
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    if (pFollowInterpretations)
      return self.getIntExp().interpretedExp->getFdkExpPtr_SkipWrapperPtrs();
    return self.getIntExp().originalExp->getFdkExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return self.getAnnExp().semExp->getFdkExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::METADATA:
  {
    return self.getMetadataExp().semExp->getFdkExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::COMMAND:
  {
    return self.getCmdExp().semExp->getFdkExpPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = self.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return (*originalFrom)->getFdkExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = self.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      return semExp->getFdkExpPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::LIST:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::GROUNDED:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


FeedbackExpression* SemanticExpression::getFdkExpPtr_SkipWrapperPtrs(bool pFollowInterpretations)
{
  return getFdkExpPtrSkipWrapperPtrs<FeedbackExpression*>(*this, pFollowInterpretations);
}

const FeedbackExpression* SemanticExpression::getFdkExpPtr_SkipWrapperPtrs(bool pFollowInterpretations) const
{
  return getFdkExpPtrSkipWrapperPtrs<const FeedbackExpression*>(*this, pFollowInterpretations);
}



template <typename ReturnType, typename SemExpType>
ReturnType getMetadataPtrSkipWrapperPtrs(SemExpType& self)
{
  switch (self.type)
  {
  case SemanticExpressionType::FEEDBACK:
  {
    return self.getFdkExp().concernedExp->getMetadataPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    return self.getIntExp().interpretedExp->getMetadataPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return self.getAnnExp().semExp->getMetadataPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::METADATA:
  {
    return self.getMetadataExpPtr();
  }
  case SemanticExpressionType::COMMAND:
  {
    return self.getCmdExp().semExp->getMetadataPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = self.getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return (*originalFrom)->getMetadataPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = self.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      return semExp->getMetadataPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::LIST:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::GROUNDED:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


MetadataExpression* SemanticExpression::getMetadataPtr_SkipWrapperPtrs()
{
  return getMetadataPtrSkipWrapperPtrs<MetadataExpression*>(*this);
}

const MetadataExpression* SemanticExpression::getMetadataPtr_SkipWrapperPtrs() const
{
  return getMetadataPtrSkipWrapperPtrs<const MetadataExpression*>(*this);
}

void SemanticExpression::getMetadataPtr_SkipWrapperAndLists(std::list<const MetadataExpression*>& pRes) const
{
  const MetadataExpression* metadataExpPtr = getMetadataPtr_SkipWrapperPtrs();
  if (metadataExpPtr != nullptr)
  {
    pRes.emplace_back(metadataExpPtr);
    return;
  }

  const GroundedExpression* grdExpPtr = getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    for (const auto& currElt : grdExpPtr->children)
      currElt.second->getMetadataPtr_SkipWrapperAndLists(pRes);

  const ListExpression* listExpPtr = getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
    for (const auto& currElt : listExpPtr->elts)
      currElt->getMetadataPtr_SkipWrapperAndLists(pRes);
}




template <typename ReturnType, typename SemExpType>
ReturnType geSetOfFormsPtrSkipWrapperPtrs(SemExpType& self, bool pFollowInterpretations)
{
  switch (self.type)
  {
  case SemanticExpressionType::FEEDBACK:
  {
    return self.getFdkExp().concernedExp->getSetOfFormsPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    if (pFollowInterpretations)
      return self.getIntExp().interpretedExp->getSetOfFormsPtr_SkipWrapperPtrs();
    return self.getIntExp().originalExp->getSetOfFormsPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::ANNOTATED:
  {
    return self.getAnnExp().semExp->getSetOfFormsPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::METADATA:
  {
    return self.getMetadataExp().semExp->getSetOfFormsPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::COMMAND:
  {
    return self.getCmdExp().semExp->getSetOfFormsPtr_SkipWrapperPtrs();
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    return self.getSetOfFormsExpPtr();
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto* semExp = self.getFSynthExp().getSemExpPtr();
    if (semExp != nullptr)
      return semExp->getSetOfFormsPtr_SkipWrapperPtrs();
    return nullptr;
  }
  case SemanticExpressionType::LIST:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::GROUNDED:
    return nullptr;
  }
  assert(false);
  return nullptr;
}


SetOfFormsExpression* SemanticExpression::getSetOfFormsPtr_SkipWrapperPtrs(bool pFollowInterpretations)
{
  return geSetOfFormsPtrSkipWrapperPtrs<SetOfFormsExpression*>(*this, pFollowInterpretations);
}

const SetOfFormsExpression* SemanticExpression::getSetOfFormsPtr_SkipWrapperPtrs(bool pFollowInterpretations) const
{
  return geSetOfFormsPtrSkipWrapperPtrs<const SetOfFormsExpression*>(*this, pFollowInterpretations);
}


std::unique_ptr<SemanticExpression> SemanticExpression::clone
(const IndexToSubNameToParameterValue* pParams,
 bool pRemoveRecentContextInterpretations,
 const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const
{
  switch (type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const auto& grdExp = getGrdExp();
    // Fill the parameters
    if (pParams != nullptr &&
        grdExp.grounding().type == SemanticGroundingType::META)
    {
      const SemanticMetaGrounding* metaGr = dynamic_cast<const SemanticMetaGrounding*>
          (&grdExp.grounding());
      for (const auto& currParam : *pParams)
        if (metaGr->paramId == currParam.first)
          for (const auto& currAttrName : currParam.second)
            if (metaGr->attibuteName == currAttrName.first &&
                currAttrName.second)
              return currAttrName.second->getSemExp().clone(nullptr, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);
    }
    return grdExp.clone(pParams, pRemoveRecentContextInterpretations,
                        pExpressionTypesToSkip);
  }
  case SemanticExpressionType::LIST:
  {
    const auto& listExp = getListExp();
    return listExp.clone(pParams, pRemoveRecentContextInterpretations,
                         pExpressionTypesToSkip);
  }
  case SemanticExpressionType::CONDITION:
  {
    const auto& condExp = getCondExp();
    return condExp.clone(pParams, pRemoveRecentContextInterpretations,
                         pExpressionTypesToSkip);
  }
  case SemanticExpressionType::COMPARISON:
  {
    const auto& compExp = getCompExp();
    auto res = std::make_unique<ComparisonExpression>
        (compExp.op,
         compExp.leftOperandExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    res->tense = compExp.tense;
    res->request = compExp.request;
    if (compExp.whatIsComparedExp)
      res->whatIsComparedExp.emplace((*compExp.whatIsComparedExp)->clone(pParams));
    if (compExp.rightOperandExp)
      res->rightOperandExp.emplace((*compExp.rightOperandExp)->clone(pParams));
    return std::move(res);
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const auto& intExp = getIntExp();
    if (pRemoveRecentContextInterpretations &&
        intExp.source == InterpretationSource::RECENTCONTEXT)
      return intExp.originalExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);
    if (pExpressionTypesToSkip != nullptr &&
        pExpressionTypesToSkip->count(SemanticExpressionType::INTERPRETATION) > 0)
      return intExp.originalExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);
    return std::make_unique<InterpretationExpression>
        (intExp.source,
         intExp.interpretedExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip),
         intExp.originalExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const auto& fdkExp = getFdkExp();
    return std::make_unique<FeedbackExpression>
        (fdkExp.feedbackExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip),
         fdkExp.concernedExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const auto& annExp = getAnnExp();
    return annExp.clone(pParams, pRemoveRecentContextInterpretations,
                        pExpressionTypesToSkip);
  }
  case SemanticExpressionType::METADATA:
  {
    const auto& metadataExp = getMetadataExp();
    if (pExpressionTypesToSkip != nullptr &&
        pExpressionTypesToSkip->count(SemanticExpressionType::METADATA) > 0)
      return metadataExp.semExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);
    return metadataExp.clone(pParams, pRemoveRecentContextInterpretations,
                             pExpressionTypesToSkip);
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    const auto& setOfFromsExp = getSetOfFormsExp();
    return setOfFromsExp.clone(pParams, pRemoveRecentContextInterpretations,
                               pExpressionTypesToSkip);
  }
  case SemanticExpressionType::COMMAND:
  {
    const auto& cmdExp = getCmdExp();
    return cmdExp.clone(pParams, pRemoveRecentContextInterpretations,
                        pExpressionTypesToSkip);
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const auto& fSynthExp = getFSynthExp();
    if (pExpressionTypesToSkip != nullptr &&
        pExpressionTypesToSkip->count(SemanticExpressionType::FIXEDSYNTHESIS) > 0)
      return fSynthExp.getSemExp().clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);
    return fSynthExp.clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);
  }
  }

  assert(false);
  return std::unique_ptr<SemanticExpression>();
}



bool SemanticExpression::operator==(const SemanticExpression& pOther) const
{
  if (type != pOther.type)
    return false;

  switch (type)
  {
  case SemanticExpressionType::GROUNDED:
    return getGrdExp().isEqual(pOther.getGrdExp()); // use isEqual() instead of == to be sure that the child class has implemented the equality comparison
  case SemanticExpressionType::LIST:
    return getListExp().isEqual(pOther.getListExp());
  case SemanticExpressionType::CONDITION:
    return getCondExp().isEqual(pOther.getCondExp());
  case SemanticExpressionType::COMPARISON:
    return getCompExp().isEqual(pOther.getCompExp());
  case SemanticExpressionType::INTERPRETATION:
    return getIntExp().isEqual(pOther.getIntExp());
  case SemanticExpressionType::FEEDBACK:
    return getFdkExp().isEqual(pOther.getFdkExp());
  case SemanticExpressionType::ANNOTATED:
    return getAnnExp().isEqual(pOther.getAnnExp());
  case SemanticExpressionType::METADATA:
    return getMetadataExp().isEqual(pOther.getMetadataExp());
  case SemanticExpressionType::SETOFFORMS:
    return getSetOfFormsExp().isEqual(pOther.getSetOfFormsExp());
  case SemanticExpressionType::COMMAND:
    return getCmdExp().isEqual(pOther.getCmdExp());
  case SemanticExpressionType::FIXEDSYNTHESIS:
    return getFSynthExp().isEqual(pOther.getFSynthExp());
  }

  assert(false);
  return false;
}


void SemanticExpression::assertEqual(const SemanticExpression& pOther) const
{
  assert(type == pOther.type);
  switch (type)
  {
  case SemanticExpressionType::GROUNDED:
    return getGrdExp().assertEltsEqual(pOther.getGrdExp());
  case SemanticExpressionType::LIST:
    return getListExp().assertEltsEqual(pOther.getListExp());
  case SemanticExpressionType::CONDITION:
    return getCondExp().assertEltsEqual(pOther.getCondExp());
  case SemanticExpressionType::COMPARISON:
    return getCompExp().assertEltsEqual(pOther.getCompExp());
  case SemanticExpressionType::INTERPRETATION:
    return getIntExp().assertEltsEqual(pOther.getIntExp());
  case SemanticExpressionType::FEEDBACK:
    return getFdkExp().assertEltsEqual(pOther.getFdkExp());
  case SemanticExpressionType::ANNOTATED:
    return getAnnExp().assertEltsEqual(pOther.getAnnExp());
  case SemanticExpressionType::METADATA:
    return getMetadataExp().assertEltsEqual(pOther.getMetadataExp());
  case SemanticExpressionType::SETOFFORMS:
    return getSetOfFormsExp().assertEltsEqual(pOther.getSetOfFormsExp());
  case SemanticExpressionType::COMMAND:
    return getCmdExp().assertEltsEqual(pOther.getCmdExp());
  case SemanticExpressionType::FIXEDSYNTHESIS:
    return getFSynthExp().assertEltsEqual(pOther.getFSynthExp());
  }
  assert(false);
}


bool SemanticExpression::operator!=(const SemanticExpression& pOther) const
{
  return !(*this == pOther);
}


bool SemanticExpression::isEmpty() const
{
  const GroundedExpression* grdExp = getGrdExpPtr();
  return grdExp != nullptr && grdExp->grounding().isEmpty();
}

void SemanticExpression::_assertSemExpOptsEqual(const mystd::unique_propagate_const<UniqueSemanticExpression>& pSemExpOpt1,
                                                const mystd::unique_propagate_const<UniqueSemanticExpression>& pSemExpOpt2)
{
  assert(pSemExpOpt1.has_value() == pSemExpOpt2.has_value());
  if (pSemExpOpt1)
    (*pSemExpOpt1)->assertEqual(**pSemExpOpt2);
}

void SemanticExpression::_assertChildrenEqual(const std::map<GrammaticalType, UniqueSemanticExpression>& pChildren1,
                                              const std::map<GrammaticalType, UniqueSemanticExpression>& pChildren2)
{
  assert(pChildren1.size() == pChildren2.size());
  auto it = pChildren1.begin();
  auto itOther = pChildren2.begin();
  while (it != pChildren1.end())
  {
    assert(it->first == itOther->first);
    it->second->assertEqual(*itOther->second);
    ++it;
    ++itOther;
  }
}


UniqueSemanticExpression::UniqueSemanticExpression()
  : _semanticExpression(std::make_unique<GroundedExpression>())
{
}

void UniqueSemanticExpression::clear()
{
  _semanticExpression = std::make_unique<GroundedExpression>();
}


std::unique_ptr<GroundedExpressionContainer> makeGrdExpContainer(
    UniqueSemanticExpression& pUSemExp)
{
  return std::make_unique<GroundedExpressionFromSemExp>(std::move(pUSemExp));
}

std::unique_ptr<GroundedExpressionContainer> makeGrdExpContainer(
    const ReferenceOfSemanticExpressionContainer& pSemExpRef)
{
  return std::make_unique<GroundedExpressionRef>(pSemExpRef.getSemExp().getGrdExp());
}

} // End of namespace onsem
