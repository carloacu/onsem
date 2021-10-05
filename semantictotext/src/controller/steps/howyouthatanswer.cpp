#include "howyouthatanswer.hpp"
#include <map>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticcontextaxiom.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include "../../utility/semexpcreator.hpp"
#include "../../type/semanticdetailledanswer.hpp"


namespace onsem
{
namespace howYouThatAnswer
{

struct InformationProvider
{
  explicit InformationProvider(SemanticSourceEnum pFrom);

  InformationProvider(InformationProvider&& pOther);
  InformationProvider& operator=(InformationProvider&& pOther);

  InformationProvider(const InformationProvider&) = delete;
  InformationProvider& operator=(const InformationProvider&) = delete;

  bool operator<(const InformationProvider& pOther) const;

  SemanticSourceEnum from;
  std::unique_ptr<SemanticAgentGrounding> author;
  std::string eventName;
  std::string eventValue;
};


InformationProvider::InformationProvider(SemanticSourceEnum pFrom)
 : from(pFrom),
   author(),
   eventName(),
   eventValue()
{
}

InformationProvider::InformationProvider(InformationProvider&& pOther)
  : from(std::move(pOther.from)),
    author(std::move(pOther.author)),
    eventName(std::move(pOther.eventName)),
    eventValue(std::move(pOther.eventValue))
{
}

InformationProvider&
InformationProvider::operator=(InformationProvider&& pOther)
{
  from = std::move(pOther.from);
  author = std::move(pOther.author);
  eventName = std::move(pOther.eventName);
  eventValue = std::move(pOther.eventValue);
  return *this;
}


bool InformationProvider::operator<(const InformationProvider& pOther) const
{
  if (from != pOther.from)
  {
    return from < pOther.from;
  }
  if (&*author != &*pOther.author)
  {
    return &*author < &*pOther.author;
  }
  if (eventName != pOther.eventName)
  {
    return eventName < pOther.eventName;
  }
  return eventValue < pOther.eventValue;
}



void _addProvider(std::set<InformationProvider>& pInfosProv,
                  const RelatedContextAxiom& pRelatedContextAxioms)
{
  auto considerAxiom = [&](const SemanticContextAxiom* pAxiomPtr)
  {
    if (pAxiomPtr == nullptr)
      return false;
    const auto& currAxiom = *pAxiomPtr;

    const MetadataExpression* metadataExpPtr = currAxiom.getSemExpWrappedForMemory().semExp->getMetadataPtr_SkipWrapperPtrs();
    if (metadataExpPtr != nullptr)
    {
      const MetadataExpression& metadataExp = *metadataExpPtr;
      InformationProvider newInfoProv(metadataExp.from);
      if (newInfoProv.from == SemanticSourceEnum::EVENT)
      {
        const auto& semExpWrappedForMemory = currAxiom.getSemExpWrappedForMemory();
        auto itEventName = semExpWrappedForMemory.linkedInfos.find(linkedInfoLabel_eventName);
        if (itEventName != semExpWrappedForMemory.linkedInfos.end())
        {
          newInfoProv.eventName = itEventName->second;
          auto itEventValue = semExpWrappedForMemory.linkedInfos.find(linkedInfoLabel_eventValue);
          if (itEventValue != semExpWrappedForMemory.linkedInfos.end())
          {
            newInfoProv.eventValue = itEventValue->second;
          }
        }
      }
      else
      {
        const SemanticAgentGrounding* sourceSubjectPtr = metadataExp.getAuthorPtr();
        if (sourceSubjectPtr != nullptr)
          newInfoProv.author = mystd::make_unique<SemanticAgentGrounding>(*sourceSubjectPtr);
      }
      pInfosProv.emplace(std::move(newInfoProv));
      return true;
    }
    return false;
  };

  bool infoProvFilled = false;
  for (const auto& currAxiomPtr : pRelatedContextAxioms.elts)
  {
    if (considerAxiom(currAxiomPtr))
    {
      infoProvFilled = true;
      break;
    }
  }
  if (!infoProvFilled)
    for (const auto& currAxiomPtr : pRelatedContextAxioms.constElts)
      if (considerAxiom(currAxiomPtr))
        break;
}


void process(std::unique_ptr<LeafSemAnswer>& pLeafAnswer)
{
  std::set<InformationProvider> infosProv;
  for (const auto& currElt : pLeafAnswer->answerElts)
  {
    const AllAnswerElts& answElts = currElt.second;
    for (const auto& currKnowGrdExp : answElts.answersFromMemory)
      _addProvider(infosProv, currKnowGrdExp.relatedContextAxioms);
    for (const auto& currEltGen : answElts.answersGenerated)
      _addProvider(infosProv, currEltGen.relatedContextAxioms);
  }

  pLeafAnswer.reset();
  for (const InformationProvider& pInfProvider : infosProv)
  {
    std::unique_ptr<SemanticExpression> howAnswerSemExp;
    if (pInfProvider.from == SemanticSourceEnum::ASR)
    {
      howAnswerSemExp = SemExpCreator::getSemExpThatSomebodyToldMeThat(*pInfProvider.author);
    }
    else if (pInfProvider.from == SemanticSourceEnum::EVENT)
    {
      howAnswerSemExp = SemExpCreator::getSemExpOfEventValue
          (pInfProvider.eventName, pInfProvider.eventValue);
    }
    else if (pInfProvider.from == SemanticSourceEnum::WRITTENTEXT)
    {
      howAnswerSemExp = SemExpCreator::sayThatOpNotifyInformedMeThat();
    }

    if (howAnswerSemExp)
    {
      pLeafAnswer = mystd::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
      pLeafAnswer->answerElts[SemanticRequestType::MANNER].answersGenerated.emplace_back(std::move(howAnswerSemExp));
    }
  }
}



} // End of namespace howYouThatAnswer
} // End of namespace onsem
