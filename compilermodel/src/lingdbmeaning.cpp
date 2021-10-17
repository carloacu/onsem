#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbstring.hpp>
#include <sstream>
#include "concept/lingdblinktoaconcept.hpp"
#include "concept/lingdbconcept.hpp"

namespace onsem
{

void LingdbMeaning::getAllContextInfos
(std::list<char>& pAllContextInfos) const
{
  ForwardPtrList<char>* contInfos = fContextInfos;
  while (contInfos != nullptr)
  {
    pAllContextInfos.push_back(*contInfos->elt);
    contInfos = contInfos->next;
  }
}


void LingdbMeaning::getLinkToConceptsStr
(std::list<std::string>& pConceptsStr) const
{
  ForwardPtrList<LingdbLinkToAConcept> const* lkToCpts = fLinkToConcepts;
  while (lkToCpts != nullptr)
  {
    std::stringstream ss;
    ss << lkToCpts->elt->getConcept()->getName()->toStr();
    ss << ", " << static_cast<int>(lkToCpts->elt->getRelatedToConcept());
    pConceptsStr.emplace_back(ss.str());
    lkToCpts = lkToCpts->next;
  }
}


ForwardPtrList<char>* LingdbMeaning::pushBackContextInfo
(LinguisticIntermediaryDatabase& pLingDatabase,
 WordContextualInfos pContextInfo)
{
  char contextInfoChar = static_cast<char>(pContextInfo);
  // dont add if already exist
  if (fContextInfos != nullptr)
  {
    ForwardPtrList<char>* oldElt = fContextInfos->find(contextInfoChar);
    if (oldElt != nullptr)
    {
      return oldElt;
    }
  }
  return xPushBackContextInfo(pLingDatabase, fContextInfos, contextInfoChar);
}


void LingdbMeaning::removeContextInfo
(LinguisticIntermediaryDatabase& pLingDatabase,
 WordContextualInfos pContextInfo)
{
  if (fContextInfos != nullptr)
  {
    char contextInfoChar = static_cast<char>(pContextInfo);
    CompositePoolAllocator& typeAlloc = pLingDatabase.xGetFPAlloc();
    fContextInfos = fContextInfos->remove(contextInfoChar, typeAlloc);
  }
}


ForwardPtrList<char>* LingdbMeaning::xPushBackContextInfo
(LinguisticIntermediaryDatabase& pLingDatabase,
 ForwardPtrList<char>*& pContInfos,
 char pContextInfo)
{
  // Add the new link to context info
  CompositePoolAllocator& typeAlloc = pLingDatabase.xGetFPAlloc();
  char* newContextInfo = typeAlloc.allocate<char>(1);
  *newContextInfo = pContextInfo;

  // Add the new link to the list of links for this meaning
  ForwardPtrList<char>* newLinkToContextInfoList = typeAlloc.allocate<ForwardPtrList<char> >(1);
  newLinkToContextInfoList->init(newContextInfo);
  newLinkToContextInfoList->next = nullptr;

  if (pContInfos == nullptr)
  {
    pContInfos = newLinkToContextInfoList;
  }
  else
  {
    pContInfos->back()->next = newLinkToContextInfoList;
  }
  return newLinkToContextInfoList;
}


void LingdbMeaning::getLinkToConceptsWithoutNullRelation
(std::vector<const LingdbLinkToAConcept*>& pLinkToConcepts) const
{
  if (fLinkToConcepts == nullptr)
  {
    return;
  }
  pLinkToConcepts.reserve(fLinkToConcepts->length());
  const ForwardPtrList<LingdbLinkToAConcept>* linkToConcepts = fLinkToConcepts;
  while (linkToConcepts != nullptr)
  {
    if (linkToConcepts->elt->getRelatedToConcept() != 0)
    {
      pLinkToConcepts.push_back(linkToConcepts->elt);
    }
    linkToConcepts = linkToConcepts->next;
  }
}


void LingdbMeaning::addLinkToConcept
(LinguisticIntermediaryDatabase& pLingDatabase,
 const LingdbConcept* pNewConcept,
 const std::string& pNewConceptStr,
 char pRelatedToConcept,
 bool pReplaceIfAlreadyExist)
{
  CompositePoolAllocator& typeAlloc = pLingDatabase.xGetFPAlloc();
  {
    // Check if the link to concept don't already exist for this meaning
    ForwardPtrList<LingdbLinkToAConcept>* prevConcept = nullptr;
    ForwardPtrList<LingdbLinkToAConcept>* existingLinkToConcepts = fLinkToConcepts;
    while (existingLinkToConcepts != nullptr)
    {
      bool needToReplace = false;
      const LingdbConcept* existingConcept = existingLinkToConcepts->elt->getConcept();
      if (existingConcept == pNewConcept)
      {
        if (pReplaceIfAlreadyExist)
        {
          existingLinkToConcepts->elt->setRelatedToConcept(pRelatedToConcept);
        }
        return;
      }
      else
      {
        std::string existingCptStr = existingConcept->getName()->toStr();
        if (xIsSecondConceptMorePrecise(existingCptStr, pNewConceptStr))
        {
          needToReplace = true;
        }
        else if (xIsSecondConceptMorePrecise(pNewConceptStr, existingCptStr))
        {
          return;
        }
      }

      if (needToReplace)
      {
        // if we have only one link, we replace directly his value
        // instead of desallocating the old elt and allocate the new one
        // (it's for optimization)
        if (fLinkToConcepts->length() == 1)
        {
          existingLinkToConcepts->elt->setConcept(pNewConcept);
          existingLinkToConcepts->elt->setRelatedToConcept(pRelatedToConcept);
          return;
        }
        if (prevConcept == nullptr)
          ForwardPtrList<LingdbLinkToAConcept>::clearNextElt
              (fLinkToConcepts, typeAlloc);
        else
          ForwardPtrList<LingdbLinkToAConcept>::clearNextElt
              (prevConcept->next, typeAlloc);
      }
      prevConcept = existingLinkToConcepts;
      existingLinkToConcepts = existingLinkToConcepts->next;
    }
  }

  // Add the new link to concept
  LingdbLinkToAConcept* newLinkToConcept = typeAlloc.allocate<LingdbLinkToAConcept>(1);
  newLinkToConcept->xInit(pNewConcept, pRelatedToConcept);

  // Add the new link to concept to the list of links to concept for this meaning
  // they are sorted by relationToConcept and by name
  ForwardPtrList<LingdbLinkToAConcept>* newLinkToConceptList =
      typeAlloc.allocate<ForwardPtrList<LingdbLinkToAConcept> >(1);
  newLinkToConceptList->init(newLinkToConcept);

  ForwardPtrList<LingdbLinkToAConcept>* prevConcept = nullptr;
  ForwardPtrList<LingdbLinkToAConcept>* existingLinkToConcepts = fLinkToConcepts;
  while (existingLinkToConcepts != nullptr)
  {
    if (*existingLinkToConcepts->elt < *newLinkToConcept)
    {
      break;
    }
    prevConcept = existingLinkToConcepts;
    existingLinkToConcepts = existingLinkToConcepts->next;
  }

  if (prevConcept == nullptr)
  {
    newLinkToConceptList->next = fLinkToConcepts;
    fLinkToConcepts = newLinkToConceptList;
  }
  else
  {
    newLinkToConceptList->next = prevConcept->next;
    prevConcept->next = newLinkToConceptList;
  }
}


bool LingdbMeaning::xIsSecondConceptMorePrecise
(const std::string& pConceptName1,
 const std::string& pConceptName2) const
{
  if (LingdbConcept::conceptNameFinishWithAStar(pConceptName1))
  {
    std::string cpt1Root = pConceptName1.substr(0, pConceptName1.size() - 1);
    return pConceptName2.compare(0, cpt1Root.size(), cpt1Root) == 0;
  }
  return false;
}



void LingdbMeaning::xDeallocate
(CompositePoolAllocator& pFPAlloc)
{
  fLemma->xRemoveMeaning(pFPAlloc, getPartOfSpeech());
  if (fLinkToConcepts != nullptr)
  {
    fLinkToConcepts->clear(pFPAlloc);
    fLinkToConcepts = nullptr;
  }
  pFPAlloc.deallocate<LingdbMeaning>(this);
}


} // End of namespace onsem
