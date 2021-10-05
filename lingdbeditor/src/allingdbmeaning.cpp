#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/lingdbeditor/allingdbstring.hpp>
#include <sstream>
#include "concept/allingdblinktoaconcept.hpp"
#include "concept/allingdbconcept.hpp"

namespace onsem
{

void ALLingdbMeaning::getAllContextInfos
(std::list<char>& pAllContextInfos) const
{
  ForwardPtrList<char>* contInfos = fContextInfos;
  while (contInfos != nullptr)
  {
    pAllContextInfos.push_back(*contInfos->elt);
    contInfos = contInfos->next;
  }
}


void ALLingdbMeaning::getLinkToConceptsStr
(std::list<std::string>& pConceptsStr) const
{
  ForwardPtrList<ALLingdbLinkToAConcept> const* lkToCpts = fLinkToConcepts;
  while (lkToCpts != nullptr)
  {
    std::stringstream ss;
    ss << lkToCpts->elt->getConcept()->getName()->toStr();
    ss << ", " << static_cast<int>(lkToCpts->elt->getRelatedToConcept());
    pConceptsStr.emplace_back(ss.str());
    lkToCpts = lkToCpts->next;
  }
}


ForwardPtrList<char>* ALLingdbMeaning::pushBackContextInfo
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


void ALLingdbMeaning::removeContextInfo
(LinguisticIntermediaryDatabase& pLingDatabase,
 WordContextualInfos pContextInfo)
{
  if (fContextInfos != nullptr)
  {
    char contextInfoChar = static_cast<char>(pContextInfo);
    ALCompositePoolAllocator& typeAlloc = pLingDatabase.xGetFPAlloc();
    fContextInfos = fContextInfos->remove(contextInfoChar, typeAlloc);
  }
}


ForwardPtrList<char>* ALLingdbMeaning::xPushBackContextInfo
(LinguisticIntermediaryDatabase& pLingDatabase,
 ForwardPtrList<char>*& pContInfos,
 char pContextInfo)
{
  // Add the new link to context info
  ALCompositePoolAllocator& typeAlloc = pLingDatabase.xGetFPAlloc();
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


void ALLingdbMeaning::getLinkToConceptsWithoutNullRelation
(std::vector<const ALLingdbLinkToAConcept*>& pLinkToConcepts) const
{
  if (fLinkToConcepts == nullptr)
  {
    return;
  }
  pLinkToConcepts.reserve(fLinkToConcepts->length());
  const ForwardPtrList<ALLingdbLinkToAConcept>* linkToConcepts = fLinkToConcepts;
  while (linkToConcepts != nullptr)
  {
    if (linkToConcepts->elt->getRelatedToConcept() != 0)
    {
      pLinkToConcepts.push_back(linkToConcepts->elt);
    }
    linkToConcepts = linkToConcepts->next;
  }
}


void ALLingdbMeaning::addLinkToConcept
(LinguisticIntermediaryDatabase& pLingDatabase,
 const ALLingdbConcept* pNewConcept,
 const std::string& pNewConceptStr,
 char pRelatedToConcept,
 bool pReplaceIfAlreadyExist)
{
  ALCompositePoolAllocator& typeAlloc = pLingDatabase.xGetFPAlloc();
  {
    // Check if the link to concept don't already exist for this meaning
    ForwardPtrList<ALLingdbLinkToAConcept>* prevConcept = nullptr;
    ForwardPtrList<ALLingdbLinkToAConcept>* existingLinkToConcepts = fLinkToConcepts;
    while (existingLinkToConcepts != nullptr)
    {
      bool needToReplace = false;
      const ALLingdbConcept* existingConcept = existingLinkToConcepts->elt->getConcept();
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
          ForwardPtrList<ALLingdbLinkToAConcept>::clearNextElt
              (fLinkToConcepts, typeAlloc);
        else
          ForwardPtrList<ALLingdbLinkToAConcept>::clearNextElt
              (prevConcept->next, typeAlloc);
      }
      prevConcept = existingLinkToConcepts;
      existingLinkToConcepts = existingLinkToConcepts->next;
    }
  }

  // Add the new link to concept
  ALLingdbLinkToAConcept* newLinkToConcept = typeAlloc.allocate<ALLingdbLinkToAConcept>(1);
  newLinkToConcept->xInit(pNewConcept, pRelatedToConcept);

  // Add the new link to concept to the list of links to concept for this meaning
  // they are sorted by relationToConcept and by name
  ForwardPtrList<ALLingdbLinkToAConcept>* newLinkToConceptList =
      typeAlloc.allocate<ForwardPtrList<ALLingdbLinkToAConcept> >(1);
  newLinkToConceptList->init(newLinkToConcept);

  ForwardPtrList<ALLingdbLinkToAConcept>* prevConcept = nullptr;
  ForwardPtrList<ALLingdbLinkToAConcept>* existingLinkToConcepts = fLinkToConcepts;
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


bool ALLingdbMeaning::xIsSecondConceptMorePrecise
(const std::string& pConceptName1,
 const std::string& pConceptName2) const
{
  if (ALLingdbConcept::conceptNameFinishWithAStar(pConceptName1))
  {
    std::string cpt1Root = pConceptName1.substr(0, pConceptName1.size() - 1);
    return pConceptName2.compare(0, cpt1Root.size(), cpt1Root) == 0;
  }
  return false;
}



void ALLingdbMeaning::xDeallocate
(ALCompositePoolAllocator& pFPAlloc)
{
  fLemma->xRemoveMeaning(pFPAlloc, getPartOfSpeech());
  if (fLinkToConcepts != nullptr)
  {
    fLinkToConcepts->clear(pFPAlloc);
    fLinkToConcepts = nullptr;
  }
  pFPAlloc.deallocate<ALLingdbMeaning>(this);
}


} // End of namespace onsem
