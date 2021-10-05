#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include "../utility/semexpcreator.hpp"

namespace onsem
{
namespace sentimentDetector
{



std::unique_ptr<SentimentContext> _selectMainSentiment
(std::map<std::string, std::unique_ptr<SentimentContext>>& pSentTypeToSpec)
{
  std::unique_ptr<SentimentContext>* maxElt = nullptr;
  for (auto& currSentTypeToSpec : pSentTypeToSpec)
  {
    if (currSentTypeToSpec.second->sentimentStrengh == 0)
      continue;
    if (maxElt == nullptr ||
        std::abs(currSentTypeToSpec.second->sentimentStrengh) > std::abs((*maxElt)->sentimentStrengh))
      maxElt = &currSentTypeToSpec.second;
  }

  if (maxElt != nullptr)
    return std::move(*maxElt);
  return {};
}


bool _invertConcept(std::string& pCptName,
                    const ConceptSet& pConceptSet)
{
  std::set<std::string> oppositeConcepts;
  pConceptSet.getOppositeConcepts(oppositeConcepts, pCptName);
  if (!oppositeConcepts.empty())
  {
    pCptName = *oppositeConcepts.begin();
    return true;
  }
  if (ConceptSet::doesConceptBeginWith(pCptName, "sentiment_positive_"))
  {
    pCptName = "sentiment_negative_*";
    return true;
  }
  if (ConceptSet::doesConceptBeginWith(pCptName, "sentiment_negative_"))
  {
    pCptName = "sentiment_positive_*";
    return true;
  }
  return false;
}

void _sentimentFromAuthorToSubject
(std::unique_ptr<SemanticExpression>& pAuthor,
 std::unique_ptr<SemanticExpression>& pReceiver,
 const GroundedExpression& pGrdExp,
 const SemanticAgentGrounding& pAuthorGrd)
{
  pAuthor = mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticAgentGrounding>(pAuthorGrd));
  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject != pGrdExp.children.end())
  {
    pReceiver = itSubject->second->clone();
  }
}

void _sentimentFromAuthorToContext
(std::unique_ptr<SemanticExpression>& pAuthor,
 std::unique_ptr<SemanticExpression>& pReceiver,
 const SemanticAgentGrounding& pAuthorGrd)
{
  pAuthor = mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticAgentGrounding>(pAuthorGrd));
  pReceiver = SemExpCreator::sayThat();
}


void _sentimentFromSubjectToObject
(std::unique_ptr<SemanticExpression>& pAuthor,
 std::unique_ptr<SemanticExpression>& pReceiver,
 const GroundedExpression& pGrdExp)
{
  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject != pGrdExp.children.end())
  {
    pAuthor = itSubject->second->clone();
  }

  auto itReceiver = pGrdExp.children.find(GrammaticalType::RECEIVER);
  if (itReceiver != pGrdExp.children.end())
  {
    pReceiver = itReceiver->second->clone();
  }
  else
  {
    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject != pGrdExp.children.end())
    {
      pReceiver = itObject->second->clone();
    }
  }
}



void _addSent
(std::map<std::string, std::unique_ptr<SentimentContext>>& pSentTypeToSpec,
 std::unique_ptr<SemanticExpression> pAuthor,
 std::unique_ptr<SemanticExpression> pReceiver,
 const std::string& pCptName,
 int pConfidence)
{
  if (!pAuthor)
    pAuthor = mystd::make_unique<GroundedExpression>
        (mystd::make_unique<SemanticGenericGrounding>());
  if (!pReceiver)
    pReceiver = pAuthor->clone();

  std::unique_ptr<SentimentContext>& sentContextPtr = pSentTypeToSpec[pCptName];
  if (!sentContextPtr ||
      sentContextPtr->sentimentStrengh <= pConfidence)
    sentContextPtr = mystd::make_unique<SentimentContext>
        (std::move(pAuthor), pCptName, pConfidence, std::move(pReceiver));
}


void _extractSentFromStatementGrounding
(std::map<std::string, std::unique_ptr<SentimentContext>>& pSentTypeToSpec,
 const std::map<std::string, char>& pSentimentConcepts,
 const GroundedExpression& pActionRootGrdExp,
 bool pAuthorIsTheSubject,
 float pGlobalConfidence,
 const SemanticAgentGrounding& pAuthor,
 const ConceptSet& pConceptSet)
{
  for (const auto& currConcept : pSentimentConcepts)
  {
    std::string cptName = currConcept.first;
    int confidence = static_cast<int>(currConcept.second * pGlobalConfidence);
    if (confidence < 0)
    {
      if (_invertConcept(cptName, pConceptSet))
        confidence = -confidence;
      else
        continue;
    }

    std::unique_ptr<SemanticExpression> author;
    std::unique_ptr<SemanticExpression> receiver;
    if (pAuthorIsTheSubject)
    {
      _sentimentFromAuthorToSubject(author, receiver, pActionRootGrdExp, pAuthor);
    }
    else
    {
      _sentimentFromSubjectToObject(author, receiver, pActionRootGrdExp);
    }
    _addSent(pSentTypeToSpec, std::move(author), std::move(receiver),
             cptName, confidence);
  }
}


void _sentimentFromSubjectToSubject
(std::unique_ptr<SemanticExpression>& pAuthor,
 std::unique_ptr<SemanticExpression>& pReceiver,
 const GroundedExpression& pGrdExp)
{
  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject != pGrdExp.children.end())
  {
    pAuthor = itSubject->second->clone();
    pReceiver = itSubject->second->clone();
  }
}


void _extractSentFromGenGrounding
(std::map<std::string, std::unique_ptr<SentimentContext>>& pSentTypeToSpec,
 const GroundedExpression& pGrdExp,
 const GroundedExpression* pRootGrdExpPtr,
 const GroundedExpression* pStatementGrdExpPtr,
 bool pVerbIsEquality,
 bool pRootVerbAtInfinitive,
 float pGlobalConfidence,
 const SemanticAgentGrounding& pAuthor,
 const ConceptSet& pConceptSet)
{
  std::map<std::string, char> sentimentConcepts;
  const auto& concepts = pGrdExp.grounding().concepts;
  ConceptSet::extractConceptsThatBeginWith(sentimentConcepts, concepts, "sentiment_");
  for (const auto& currConcept : sentimentConcepts)
  {
    std::string cptName = currConcept.first;
    int confidence = static_cast<int>(currConcept.second * pGlobalConfidence);
    if (confidence < 0)
    {
      if (_invertConcept(cptName, pConceptSet))
        confidence = -confidence;
      else
        continue;
    }

    std::unique_ptr<SemanticExpression> author;
    std::unique_ptr<SemanticExpression> receiver;
    if (pRootGrdExpPtr != nullptr)
    {
      if (pVerbIsEquality && !pRootVerbAtInfinitive && pStatementGrdExpPtr != nullptr)
      {
        if (cptName == "sentiment_positive_*" || cptName == "sentiment_positive_funny" ||
            cptName == "sentiment_positive_kind" || cptName == "sentiment_positive_joy" ||
            cptName == "sentiment_negative_*" || cptName == "sentiment_negative_bad" ||
            cptName == "sentiment_negative_disgust")
        {
          _sentimentFromAuthorToSubject(author, receiver, *pStatementGrdExpPtr, pAuthor);
        }
        else
        {
          _sentimentFromSubjectToSubject(author, receiver, *pStatementGrdExpPtr);
        }
      }
      else if (pRootGrdExpPtr->grounding().getStatementGroundingPtr() != nullptr)
      {
        _sentimentFromSubjectToObject(author, receiver, *pRootGrdExpPtr);
      }
      else
      {
        author = pRootGrdExpPtr->clone();
        receiver = pRootGrdExpPtr->clone();
      }
    }
    else if (SemExpGetter::getReferenceTypeFromGrd(pGrdExp.grounding()) == SemanticReferenceType::UNDEFINED)
    {
      _sentimentFromAuthorToContext(author, receiver, pAuthor);
    }
    _addSent(pSentTypeToSpec, std::move(author), std::move(receiver),
             cptName, confidence);
  }
}

void _extractSentFromSemExp
(std::map<std::string, std::unique_ptr<SentimentContext>>& pSentTypeToSpec,
 const GroundedExpression& pGrdExp,
 const GroundedExpression* pRootGrdExp,
 const GroundedExpression* pStatementGrdExpPtr,
 bool pVerbIsEquality,
 bool pRootVerbAtInfinitive,
 float pGlobalConfidence,
 const SemanticAgentGrounding& pAuthor,
 const ConceptSet& pConceptSet)
{
  SemanticGroudingType grdGroundingType = pGrdExp.grounding().type;

  bool askSomething = false;
  float newGlobalConfidence = pGlobalConfidence;
  if (grdGroundingType == SemanticGroudingType::STATEMENT)
  {
    const SemanticStatementGrounding& statementGrd = pGrdExp->getStatementGrounding();
    bool askAction = statementGrd.requests.has(SemanticRequestType::ACTION);
    askSomething = !statementGrd.requests.empty();
    std::map<std::string, char> sentimentConcepts;
    bool authorIsTheSubject = askAction;
    if (!askAction &&
        isAPresentTense(statementGrd.verbTense) &&
        statementGrd.concepts.count("verb_go") != 0 &&
        (statementGrd.requests.empty() ||
         statementGrd.requests.has(SemanticRequestType::YESORNO) ||
         statementGrd.requests.has(SemanticRequestType::MANNER)) &&
        pGrdExp.children.count(GrammaticalType::OBJECT) == 0)
    {
      authorIsTheSubject = true;
      sentimentConcepts.emplace("sentiment_positive_*", 3);
    }
    if (askSomething && !askAction && sentimentConcepts.empty())
      return;
    ConceptSet::extractConceptsThatBeginWith(sentimentConcepts, statementGrd.concepts, "sentiment_");

    float verbConfidence = statementGrd.polarity ? pGlobalConfidence : -pGlobalConfidence;
    if (pStatementGrdExpPtr == nullptr || pVerbIsEquality)
      pRootVerbAtInfinitive = statementGrd.isAtInfinitive() &&
          !SemExpGetter::isACoreferenceFromStatementGrounding(statementGrd, CoreferenceDirectionEnum::UNKNOWN, true);
    pVerbIsEquality = ConceptSet::haveAConceptThatBeginWith(statementGrd.concepts, "verb_equal_");
    if (pVerbIsEquality)
    {
      newGlobalConfidence = verbConfidence;
    }
    pStatementGrdExpPtr = &pGrdExp;

    _extractSentFromStatementGrounding
        (pSentTypeToSpec, sentimentConcepts,
         pGrdExp, authorIsTheSubject, verbConfidence, pAuthor, pConceptSet);
  }
  else if (grdGroundingType == SemanticGroudingType::GENERIC ||
           grdGroundingType == SemanticGroudingType::NAME ||
           grdGroundingType == SemanticGroudingType::CONCEPTUAL)
  {
    float wordConfidence = pGlobalConfidence;
    if (!pGrdExp.grounding().polarity)
      wordConfidence = -wordConfidence;
    _extractSentFromGenGrounding(pSentTypeToSpec, pGrdExp, pRootGrdExp, pStatementGrdExpPtr,
                                 pVerbIsEquality, pRootVerbAtInfinitive, wordConfidence,
                                 pAuthor, pConceptSet);
  }

  if (!askSomething)
  {
    // iterate on the children
    for (const auto& currChild : pGrdExp.children)
    {
      const GroundedExpression* childGrdExp = currChild.second->getGrdExpPtr_SkipWrapperPtrs();
      if (childGrdExp != nullptr)
      {
        if (currChild.first == GrammaticalType::MITIGATION)
        {
          newGlobalConfidence = pGlobalConfidence;
        }
        _extractSentFromSemExp(pSentTypeToSpec, *childGrdExp, &pGrdExp, pStatementGrdExpPtr,
                               pVerbIsEquality, pRootVerbAtInfinitive, newGlobalConfidence, pAuthor,
                               pConceptSet);
      }
    }
  }
}




std::unique_ptr<SentimentContext> extractMainSentiment
(const GroundedExpression& pGrdExp,
 const SemanticAgentGrounding& pAuthor,
 const ConceptSet& pConceptSet)
{
  std::map<std::string, std::unique_ptr<SentimentContext>> sentTypeToSpec;
  _extractSentFromSemExp(sentTypeToSpec, pGrdExp, nullptr, nullptr, false, false, 1, pAuthor, pConceptSet);
  return _selectMainSentiment(sentTypeToSpec);
}


// TODO: change std::list<std::unique_ptr<SentimentContext>> to std::list<SentimentContext>
void semExpToSentimentInfos(std::list<std::unique_ptr<SentimentContext>>& pSentContexts,
                            const SemanticExpression& pSemExp,
                            const SemanticAgentGrounding& pAuthor,
                            const ConceptSet& pConceptSet)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const GroundedExpression& grdExp = pSemExp.getGrdExp();
    auto sentSpec = extractMainSentiment(grdExp, pAuthor, pConceptSet);
    if (sentSpec)
      pSentContexts.emplace_back(std::move(sentSpec));
    break;
  }
  case SemanticExpressionType::LIST:
  {
    const ListExpression& listExp = pSemExp.getListExp();
    for (const auto& currExp : listExp.elts)
    {
      semExpToSentimentInfos(pSentContexts, *currExp, pAuthor, pConceptSet);
    }
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const InterpretationExpression& intExp = pSemExp.getIntExp();
    semExpToSentimentInfos(pSentContexts, *intExp.interpretedExp, pAuthor, pConceptSet);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const FeedbackExpression& fdkExp = pSemExp.getFdkExp();
    semExpToSentimentInfos(pSentContexts, *fdkExp.concernedExp, pAuthor, pConceptSet);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const AnnotatedExpression& annExp = pSemExp.getAnnExp();
    semExpToSentimentInfos(pSentContexts, *annExp.semExp, pAuthor, pConceptSet);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    const MetadataExpression& metadataExp = pSemExp.getMetadataExp();
    semExpToSentimentInfos(pSentContexts, *metadataExp.semExp, pAuthor, pConceptSet);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const FixedSynthesisExpression& fSynthExp = pSemExp.getFSynthExp();
    semExpToSentimentInfos(pSentContexts, fSynthExp.getSemExp(), pAuthor, pConceptSet);
    break;
  }
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}



} // End of namespace sentimentDetector
} // End of namespace onsem
