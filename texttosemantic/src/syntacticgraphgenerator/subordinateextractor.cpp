#include "subordinateextractor.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../tool/chunkshandler.hpp"
#include "listextractor.hpp"
#include "entityrecognizer.hpp"

#include "chunkslinker.hpp"

namespace onsem
{
namespace linguistics
{

SubordinateExtractor::SubordinateExtractor
(const AlgorithmSetForALanguage& pConfiguration)
  : fConf(pConfiguration),
    fSemFrameDict(fConf.getSpecifcLingDb().getSemFrameDict()),
    fLingDico(fConf.getLingDico())
{
}


bool SubordinateExtractor::process
(std::list<ChunkLink>& pFirstChildren,
 const SynthAnalEndingStepForDebug& pEndingStep) const
{
  ChunkLinkWorkingZone workingZone(pFirstChildren,
                                   pFirstChildren.begin(), pFirstChildren.end());
  const ListExtractor& listExtractor = fConf.getListExtractor();

  xLinkComplementaryNominalChunks(pFirstChildren);

  listExtractor.extractLists(workingZone, false, false, false, -1, ChunkType::THEN_CHUNK);
  listExtractor.extractLists(workingZone);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_1))
  { return false; }


  if (fConf.getLanguageType() == SemanticLanguageEnum::ENGLISH)
    xLinkSubordonateOfCondition(pFirstChildren);
  xLinkSubordinatesAtTheBeginning(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_2))
  { return false; }

  /**
   * Ex: "dis que je suis gentil"
   *      dis  que   suis
   *                /   \
   *              je    gentil
   *
   * To:
   *      dis
   *        \
   *        suis
   *       /    \
   *     je     gentil
   */
  listExtractor.extractSubordonates(workingZone);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_3))
  { return false; }

  xLinkSubjectInfinitiveVerbs(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_4))
  { return false; }

  xLinkComplementaryNominalChunksSeparatredByCommas(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_5))
  { return false; }

  /**
   * Ex: "je suis un robot créé par Honda"
   *        suis           créé
   *       /    \            \
   *      je    un robot     par Honda
   *
   * To:
   *        suis
   *       /    \
   *      je    un robot
   *               \
   *               créé
   *                 \
   *                par Honda
   */
  xLinkSubordonateVerbs(pFirstChildren);
  xResolveBadObjectRequests(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_6))
  { return false; }

  listExtractor.extractSubordonates(workingZone);
  xLinkSubordinatesThatBeginWithAPreposition(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_7))
  { return false; }

  listExtractor.extractLists(workingZone, false, true);
  xLinkSubjectAndAuxiliaryToVerbsInsideAList(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_8))
  { return false; }

  xModifyTheVerbsForList(pFirstChildren, ChunkLinkType::SIMPLE);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_9))
  { return false; }

  listExtractor.extractLists(workingZone, false, true);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_10))
  { return false; }

  xLinkSubordonateThatAreBeforeVerbs(pFirstChildren);
  xLinkElseSubordonates(pFirstChildren);
  if (pEndingStep.doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum::SUBORDONATEEXTRACTOR_11))
  { return false; }

  listExtractor.extractSubordonates(workingZone);


  xComparisonsFinder(pFirstChildren);
  return true;
}


bool _isAChunkLinkOkForVerbModification(ChunkLinkType pChunkLinkType)
{
  return pChunkLinkType == ChunkLinkType::SIMPLE ||
      pChunkLinkType == ChunkLinkType::DIRECTOBJECT ||
      pChunkLinkType == ChunkLinkType::SUBORDINATE_CLAUSE ||
      pChunkLinkType == ChunkLinkType::PURPOSE_OF;
}


void SubordinateExtractor::xModifyTheVerbsOfAChunk
(std::shared_ptr<Chunk>& pChunk,
 ChunkLinkType pFatherChunkLinkType,
 ChunkType pPrevChunkType) const
{
  Chunk& chunk = *pChunk;
  switch (chunk.type)
  {
  case ChunkType::VERB_CHUNK:
  {
    if (chunk.requests.empty() &&
        !haveASubject(chunk) &&
        pPrevChunkType != ChunkType::INFINITVE_VERB_CHUNK)
    {
      const InflectedWord& verbIGram = chunk.head->inflWords.front();
      if (fConf.getFlsChecker().verbCanBeAtImperative(verbIGram))
        chunk.requests.set(SemanticRequestType::ACTION);
    }
    xModifyTheVerbsForStatementChildren(chunk.children);
    break;
  }
  case ChunkType::INFINITVE_VERB_CHUNK:
  {
    xModifyTheVerbsForStatementChildren(chunk.children);
    break;
  }
  case ChunkType::AND_CHUNK:
  case ChunkType::OR_CHUNK:
  case ChunkType::THEN_CHUNK:
  case ChunkType::THEN_REVERSED_CHUNK:
  {
    auto beginItChild = chunk.children.begin();
    if (beginItChild->chunk->type == ChunkType::INFINITVE_VERB_CHUNK &&
        chunk.children.size() > 1)
    {
      SemanticLanguageEnum language = fConf.getLanguageType();

      auto itChild = beginItChild;
      ++itChild;
      {
        Chunk& currChunkChild = *itChild->chunk;
        if (currChunkChild.type == ChunkType::VERB_CHUNK &&
            pFatherChunkLinkType == ChunkLinkType::SIMPLE &&
            chunk.children.size() > 2 &&
            doesChunkIsOrCanBeAtImperative(currChunkChild))
        {
          if (language == SemanticLanguageEnum::ENGLISH)
          {
            auto localItChild = itChild;
            while (localItChild != chunk.children.end())
            {
              Chunk& currChunkChild = *localItChild->chunk;
              if (currChunkChild.type == ChunkType::VERB_CHUNK)
                currChunkChild.requests.set(SemanticRequestType::ACTION);
              ++localItChild;
            }
          }

          std::shared_ptr<Chunk> puposeChunkPtr = beginItChild->chunk;
          chunk.children.erase(beginItChild);
          puposeChunkPtr->children.emplace_back(ChunkLinkType::PURPOSE_OF, pChunk);
          pChunk = puposeChunkPtr;
          return;
        }
      }

      if (language == SemanticLanguageEnum::ENGLISH)
      {
        while (itChild != chunk.children.end())
        {
          Chunk& currChunkChild = *itChild->chunk;
          if (currChunkChild.type == ChunkType::VERB_CHUNK)
          {
            currChunkChild.type = ChunkType::INFINITVE_VERB_CHUNK;
            currChunkChild.requests.clear();
            InflectedWord& childHeadIGram = currChunkChild.head->inflWords.front();
            if (childHeadIGram.word.partOfSpeech == PartOfSpeech::VERB)
              childHeadIGram.moveInflections(VerbalInflections::get_inflections_infinitive());
          }
          ++itChild;
        }
      }
    }
    xModifyTheVerbsForList(chunk.children, pFatherChunkLinkType);
    break;
  }
  default:
    break;
  }
}


void SubordinateExtractor::xModifyTheVerbsForStatementChildren
(std::list<ChunkLink>& pChunkList) const
{
  for (ChunkLink& currChLk : pChunkList)
    if (_isAChunkLinkOkForVerbModification(currChLk.type))
      xModifyTheVerbsOfAChunk(currChLk.chunk, currChLk.type, ChunkType::SEPARATOR_CHUNK);
}


void SubordinateExtractor::xModifyTheVerbsForList
(std::list<ChunkLink>& pChunkList,
 ChunkLinkType pFatherChunkLinkType) const
{
  ChunkType prevChunkType = ChunkType::SEPARATOR_CHUNK;
  for (ChunkLink& currChLk : pChunkList)
  {
    if (_isAChunkLinkOkForVerbModification(currChLk.type))
      xModifyTheVerbsOfAChunk(currChLk.chunk, pFatherChunkLinkType, prevChunkType);
    prevChunkType = currChLk.chunk->type;
  }
}


struct SepAndNomnalChunk
{
  SepAndNomnalChunk(const std::list<ChunkLink>::iterator& pSepChunk,
                    const std::list<ChunkLink>::iterator& pNominalChunk)
    : sepChunk(pSepChunk),
      nominalChunk(pNominalChunk)
  {
  }

  std::list<ChunkLink>::iterator sepChunk;
  std::list<ChunkLink>::iterator nominalChunk;
};


void SubordinateExtractor::xLinkComplementaryNominalChunks
(std::list<ChunkLink>& pChunkLinks) const
{
  Chunk* prevChunkIfNominalPtr = nullptr;
  std::list<ChunkLink>::iterator prevIt = pChunkLinks.end();
  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); ++it)
  {
    if (it->chunk->type == ChunkType::PREPOSITIONAL_CHUNK &&
        chunkCanBeAnObject(*it->chunk))
    {
      auto linkType = ChunkLinkType::COMPLEMENT;
      if (ConceptSet::haveAConceptThatBeginWith(it->chunk->tokRange.getItBegin()->inflWords.front().infos.concepts, "location_"))
      {
         linkType = ChunkLinkType::LOCATION;
      }
      else
      {
        linkType = fConf.getEntityRecognizer().findNatureOfAChunkLink(*it, nullptr, false);
      }
      if (prevChunkIfNominalPtr != nullptr)
      {
        auto& prevChunkIfNominal = *prevChunkIfNominalPtr;
        xAddASubChunk(pChunkLinks, prevChunkIfNominal, it, it, linkType);
        it = prevIt;
      }
      else
      {
        it->type = linkType;
      }
    }

    ChunkLink& currChunkLink = *it;
    Chunk& currChunk = *currChunkLink.chunk;
    PartOfSpeech currHeadPartOfSpeech = currChunk.head->inflWords.front().word.partOfSpeech;
    if (currChunk.type == ChunkType::NOMINAL_CHUNK && partOfSpeech_isNominal(currHeadPartOfSpeech))
      prevChunkIfNominalPtr = &currChunk;
    else
      prevChunkIfNominalPtr = nullptr;
    prevIt = it;
  }
}



void SubordinateExtractor::xResolveBadObjectRequests(std::list<ChunkLink>& pChunkLinks) const
{
  for (const auto& currChkLk : pChunkLinks)
  {
    Chunk& currChunk = *currChkLk.chunk;
    if (currChunk.type == ChunkType::VERB_CHUNK &&
        currChunk.requests.has(SemanticRequestType::OBJECT))
    {
      // we spot the case where is both a subject and an object and the subject is first
      const Chunk* subjectChunkPtr = nullptr;
      const Chunk* objectChunkPtr = nullptr;
      ChunkLink* timeChunkLinkPtr = nullptr;
      for (auto& currChild : currChunk.children)
      {
        switch (currChild.type)
        {
        case ChunkLinkType::SUBJECT:
        {
          subjectChunkPtr = &*currChild.chunk;
          break;
        }
        case ChunkLinkType::AUXILIARY:
        {
          subjectChunkPtr = getSubjectChunk(*currChild.chunk);
          break;
        }
        case ChunkLinkType::DIRECTOBJECT:
        {
          objectChunkPtr = &*currChild.chunk;
          if (objectChunkPtr->type == ChunkType::INFINITVE_VERB_CHUNK)
          {
            const ChunkLink* subDoChunkLk = getDOChunkLink(*objectChunkPtr);
            if (subDoChunkLk == nullptr)
              objectChunkPtr = nullptr;
            else
              objectChunkPtr = &*subDoChunkLk->chunk;
          }
          break;
        }
        case ChunkLinkType::TIME:
        {
          timeChunkLinkPtr = &currChild;
          break;
        }
        default:
          break;
        }
        if (objectChunkPtr != nullptr)
          break;
      }

      if (subjectChunkPtr != nullptr &&
          checkOrder(*subjectChunkPtr, currChunk))
      {
        if (objectChunkPtr != nullptr &&
            checkOrder(*subjectChunkPtr, *objectChunkPtr) &&
            objectChunkPtr->type != ChunkType::OR_CHUNK)
        {
          currChunk.requests.set(SemanticRequestType::SUBJECT);
        }
        else if (timeChunkLinkPtr != nullptr &&
                 ConceptSet::haveAConceptThatBeginWith(currChunk.head->inflWords.begin()->infos.concepts, "verb_equal_"))
        {
          timeChunkLinkPtr->type = ChunkLinkType::DIRECTOBJECT;
          currChunk.requests.set(SemanticRequestType::SUBJECT);
        }
      }
    }
  }
}



void SubordinateExtractor::xLinkComplementaryNominalChunksSeparatredByCommas
(std::list<ChunkLink>& pChunkLinks) const
{
  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); ++it)
  {
    ChunkLink& currChunkLink = *it;
    Chunk& currChunk = *currChunkLink.chunk;
    PartOfSpeech currHeadPartOfSpeech = currChunk.head->inflWords.front().word.partOfSpeech;
    if ((currChunk.type == ChunkType::NOMINAL_CHUNK && partOfSpeech_isNominal(currHeadPartOfSpeech)) ||
        currChunk.type == ChunkType::VERB_CHUNK)
    {
      bool linkSubNominalChunks = false;
      std::list<SepAndNomnalChunk> subNominalChunks;

      auto itSeparator = it;
      ++itSeparator;
      while (itSeparator != pChunkLinks.end())
      {
        if (itSeparator->chunk->type == ChunkType::SEPARATOR_CHUNK)
        {
          PartOfSpeech sepPartOfSpeech = itSeparator->chunk->head->inflWords.front().word.partOfSpeech;
          if (sepPartOfSpeech == PartOfSpeech::LINKBETWEENWORDS)
          {
            auto itSecondChunk = itSeparator;
            ++itSecondChunk;
            if (itSecondChunk != pChunkLinks.end())
            {
              if (recInListConst([](const Chunk& pChunk)
              {
                if (pChunk.type == ChunkType::NOMINAL_CHUNK)
                {
                  return true;
                }
                else if (pChunk.type == ChunkType::VERB_CHUNK)
                {
                  return (InflectionsChecker::verbIsAtPresentParticiple(pChunk.head->inflWords.front()) &&
                          !haveASubject(pChunk));
                }
                return false;
            }, *itSecondChunk->chunk))
              {
                subNominalChunks.emplace_back(itSeparator, itSecondChunk);
                itSeparator = itSecondChunk;
              }
              else if (!subNominalChunks.empty() &&
                       (itSecondChunk->chunk->type == ChunkType::SEPARATOR_CHUNK ||
                        itSecondChunk->chunk->type == ChunkType::VERB_CHUNK))
              {
                linkSubNominalChunks = true;
                break;
              }
            }
          }
          else if (!subNominalChunks.empty())
          {
            linkSubNominalChunks = true;
            break;
          }
          ++itSeparator;
          continue;
        }
        break;
      }

      if (linkSubNominalChunks)
      {
        if (currChunk.type == ChunkType::NOMINAL_CHUNK)
        {
          if (subNominalChunks.size() == 1)
          {
            const SepAndNomnalChunk& frontElt = subNominalChunks.front();
            xAddASubChunk(pChunkLinks, currChunk, frontElt.sepChunk, frontElt.nominalChunk, ChunkLinkType::SPECIFICATION);
          }
        }
        else if (currChunk.type == ChunkType::VERB_CHUNK)
        {
          Chunk* doChunkPtr = getDOComplOrSubjClauseChunk(currChunk);
          if (doChunkPtr == nullptr)
          {
            for (auto& currSubNominalChunk : subNominalChunks)
              xAddASubChunk(pChunkLinks, currChunk, currSubNominalChunk.sepChunk, currSubNominalChunk.nominalChunk, ChunkLinkType::DIRECTOBJECT);
          }
          else
          {
            Chunk& doChunk = *doChunkPtr;
            if (doChunk.type == ChunkType::VERB_CHUNK)
              for (auto& currSubNominalChunk : subNominalChunks)
                xAddASubChunk(pChunkLinks, doChunk, currSubNominalChunk.sepChunk, currSubNominalChunk.nominalChunk, ChunkLinkType::DIRECTOBJECT);
            else
              for (auto& currSubNominalChunk : subNominalChunks)
                xAddASubChunk(pChunkLinks, doChunk, currSubNominalChunk.sepChunk, currSubNominalChunk.nominalChunk, ChunkLinkType::SPECIFICATION);
          }

        }
      }

    }
  }
}


void SubordinateExtractor::xLinkSubordonateVerbs
(std::list<ChunkLink>& pChunkLinks) const
{
  bool afterASeparator = false;
  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); )
  {
    // after a separator chunk
    if (afterASeparator)
    {
      auto prev = it;
      --prev;
      if (xTryToLinkASubordonateVerb(prev, it, pChunkLinks))
      {
        afterASeparator = false;
        continue;
      }
    }
    else if (xTryToLinkASubordonateVerb(it, it, pChunkLinks))
    {
      afterASeparator = false;
      continue;
    }

    const auto& headOfChkInflWords = it->chunk->head->inflWords;
    const auto& headOfChkWord = headOfChkInflWords.front().word;
    afterASeparator = it->chunk->type == ChunkType::SEPARATOR_CHUNK &&
        (headOfChkWord.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS ||
         (headOfChkWord.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION && fLingDico.statDb.inflWordsToQuestionWord(headOfChkInflWords, true, false) == nullptr));
    ++it;
  }
}

void SubordinateExtractor::xLinkSubordinatesThatBeginWithAPreposition
(std::list<ChunkLink>& pChunkLinks) const
{
  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); )
  {
    if (chunkTypeIsVerbal(it->chunk->type))
    {
      auto itAfter = it;
      ++itAfter;
      if (itAfter == pChunkLinks.end())
        return;
      if (itAfter->chunk->type == ChunkType::PREPOSITIONAL_CHUNK &&
          chunkCanBeAnObject(*itAfter->chunk))
      {
        itAfter->type = ChunkLinkType::SUBORDINATE;
        it->chunk->children.splice(it->chunk->children.end(), pChunkLinks, itAfter);
      }
    }
    ++it;
  }
}

void SubordinateExtractor::xLinkSubjectInfinitiveVerbs
(std::list<ChunkLink>& pChunkLinks) const
{
  auto language = fConf.getLanguageType();
  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); )
  {
    if (hasAChunkTypeOrAListOfChunkTypes(*it->chunk, ChunkType::INFINITVE_VERB_CHUNK))
    {
      auto itNext = it;
      ++itNext;
      if (itNext == pChunkLinks.end())
        break;
      Chunk& nextChunk = *itNext->chunk;
      if (nextChunk.type == ChunkType::VERB_CHUNK)
      {
        if (!haveASubject(nextChunk))
        {
          const auto& flsCheker = fConf.getFlsChecker();
          const auto& nextChunkInfls = nextChunk.head->inflWords.front();
          if (ConceptSet::haveAConceptThatBeginWith(nextChunkInfls.infos.concepts, "verb_equal_") &&
              !flsCheker.verbIsAtPresentParticiple(nextChunkInfls) &&
              flsCheker.verbIsConjAtPerson(nextChunkInfls, RelativePerson::THIRD_SING))
          {
            it->type = ChunkLinkType::SUBJECT;
            auto itToMove = it;
            ++it;
            nextChunk.children.splice(nextChunk.children.begin(), pChunkLinks, itToMove);
            continue;
          }
        }
      }
      else if (language == SemanticLanguageEnum::ENGLISH &&
               nextChunk.type == ChunkType::SEPARATOR_CHUNK &&
               nextChunk.head->inflWords.front().word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS)
      {
        auto itNextNext = itNext;
        ++itNextNext;
        if (itNextNext == pChunkLinks.end())
          break;
        Chunk& nextNextChunk = *itNextNext->chunk;
        if (recInListConst(chunkCanBeAtImperative, nextNextChunk))
        {
          itNextNext->type = ChunkLinkType::PURPOSE_OF;
          it->chunk->children.splice(it->chunk->children.end(), pChunkLinks, itNextNext);
        }
      }
    }
    ++it;
  }
}



void SubordinateExtractor::xLinkSubordonateOfCondition
(std::list<ChunkLink>& pChunkLinks) const
{
  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); ++it)
  {
    Chunk& currChunk = *it->chunk;
    if (currChunk.type == ChunkType::SEPARATOR_CHUNK)
    {
      const InflectedWord& headInflWorld = currChunk.head->inflWords.front();
      if (headInflWorld.infos.hasContextualInfo(WordContextualInfos::CONDITION))
      {
        ++it;
        if (it == pChunkLinks.end() ||
            !chunkTypeIsVerbal(it->chunk->type))
          return;

        auto itEnd = it;
        ++itEnd;
        if (itEnd == pChunkLinks.end() ||
            !chunkTypeIsVerbal(itEnd->chunk->type))
          return;
        ++itEnd;
        if (itEnd == pChunkLinks.end() ||
            !(chunkTypeIsVerbal(itEnd->chunk->type) ||
              (itEnd->chunk->type == ChunkType::SEPARATOR_CHUNK &&
               itEnd->chunk->head->inflWords.front().infos.hasContextualInfo(WordContextualInfos::THEN))))
          return;

        ChunkLinkWorkingZone subWorkingZone(pChunkLinks, it, itEnd);
        const ListExtractor& listExtractor = fConf.getListExtractor();
        listExtractor.extractSubordonates(subWorkingZone);
      }
    }
  }
}


void SubordinateExtractor::xLinkSubordinatesAtTheBeginning(std::list<ChunkLink>& pChunkLinks) const
{


  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); ++it)
  {
    ChunkLink& currChunkLk = *it;
    Chunk& currChunk = *currChunkLk.chunk;
    if (currChunk.type == ChunkType::PREPOSITIONAL_CHUNK &&
        chunkCanBeAnObject(currChunk))
    {
      auto itNext = it;
      ++itNext;
      if (itNext == pChunkLinks.end())
        break;

      ChunkType nextChunkType = itNext->chunk->type;
      auto itSeparator = pChunkLinks.end();
      if (nextChunkType == ChunkType::SEPARATOR_CHUNK &&
          itNext->chunk->getHeadPartOfSpeech() == PartOfSpeech::LINKBETWEENWORDS)
      {
        itSeparator = itNext;
        ++itNext;
        if (itNext == pChunkLinks.end())
          break;
        nextChunkType = itNext->chunk->type;
      }

      if (chunkTypeIsVerbal(nextChunkType))
      {
        Chunk& verbChunk = *itNext->chunk;
        ChunkLinkType newLkType = fConf.getEntityRecognizer().findNatureOfAChunkLink(currChunkLk, &verbChunk);
        if (newLkType != ChunkLinkType::SUBORDINATE)
        {
          currChunkLk.type = newLkType;
          if (itSeparator != pChunkLinks.end())
          {
            currChunkLk.tokRange = itSeparator->chunk->tokRange;
            pChunkLinks.erase(itSeparator);
            itSeparator = pChunkLinks.end();
          }
          verbChunk.children.splice(verbChunk.children.begin(), pChunkLinks, it);
          it = itNext;
        }
      }
    }
  }
}


void _addElseContent(std::list<ChunkLink>& pChunkLinks,
                     std::list<ChunkLink>::iterator pItThenContent,
                     std::list<ChunkLink>::iterator pItElseContent,
                     std::list<ChunkLink>::iterator pItElse)
{
  // move the "else content" node into the "then content" children
  pItThenContent->chunk->children.splice(pItThenContent->chunk->children.end(), pChunkLinks, pItElseContent);
  ChunkLink& elseContent = *(--pItThenContent->chunk->children.end());
  // tag the new link as a "else" link
  elseContent.type = ChunkLinkType::ELSE;
  // move "else" chunk into the "else content" link
  elseContent.tokRange = pItElse->chunk->tokRange;
  pChunkLinks.erase(pItElse);
}




void SubordinateExtractor::xLinkElseSubordonates
(std::list<ChunkLink>& pChunkLinks) const
{
  if (pChunkLinks.size() <= 2)
    return;

  auto it = pChunkLinks.end();
  --it;
  --it;
  while (it != pChunkLinks.begin())
  {
    // if we are at a separator node
    if (it->chunk->type == ChunkType::SEPARATOR_CHUNK)
    {
      // if the the separator node is a "else" node
      const InflectedWord& ifIGram = it->chunk->head->inflWords.front();
      if (ifIGram.word.partOfSpeech == PartOfSpeech::CONJUNCTIVE &&
          ifIGram.infos.hasContextualInfo(WordContextualInfos::ELSE))
      {
        auto itThenContent = it;
        --itThenContent;
        Chunk& prevChunk = *itThenContent->chunk;
        if (!haveChild(prevChunk, ChunkLinkType::ELSE))
        {
          // get "else content" node
          auto itElseContent = it;
          ++itElseContent;
          assert(itElseContent != pChunkLinks.end());
          // bad "else content" we restart from here
          if (itElseContent->chunk->type == ChunkType::SEPARATOR_CHUNK)
          {
            --it;
            continue;
          }

          auto itElse = it;
          it = itThenContent;
          _addElseContent(pChunkLinks, itThenContent, itElseContent, itElse);
          if (it == pChunkLinks.begin())
            return;
        }
      }
    }
    --it;
  }
}


void SubordinateExtractor::xLinkSubordonateThatAreBeforeVerbs
(std::list<ChunkLink>& pChunkLinks) const
{
  for (auto it = pChunkLinks.begin(); it != pChunkLinks.end(); ++it)
  {
    // if we are at a separator node
    switch (it->chunk->type)
    {
    case ChunkType::SEPARATOR_CHUNK:
    {
      // if the the separator node is a "if" node
      const InflectedWord& ifIGram = it->chunk->head->inflWords.front();
      if (ifIGram.word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION &&
          ifIGram.infos.hasContextualInfo(WordContextualInfos::SENTENCECANBEGINWITH) &&
          (ifIGram.infos.hasContextualInfo(WordContextualInfos::CONDITION) ||
           fSemFrameDict.doesIntroductionWordHasChunkLinkType(ifIGram.word, ChunkLinkType::TIME)))
      {
        // get "if" node
        auto itIf = it;
        ++it;
        if (it == pChunkLinks.end())
          return;

        // get "if content" node
        auto itIfContent = it;
        if (itIfContent->chunk->type == ChunkType::VERB_CHUNK &&
            !itIfContent->chunk->requests.empty())
          return;
        ++it;
        if (it == pChunkLinks.end())
          return;

        // get "then" node
        auto itThen = pChunkLinks.end();
        //if (ifIGram.infos.hasContextualInfo(WordContextualInfos::CONDITION))
        {
          const InflectedWord& iGramThenHead = it->chunk->head->inflWords.front();
          if (it->chunk->type == ChunkType::SEPARATOR_CHUNK &&
              (iGramThenHead.word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS ||
               (iGramThenHead.word.partOfSpeech == PartOfSpeech::CONJUNCTIVE &&
                iGramThenHead.infos.hasContextualInfo(WordContextualInfos::THEN))))
          {
            itThen = it;
            ++it;
            if (it == pChunkLinks.end())
              return;
          }
        }

        // get "then content" node
        if (it->chunk->type != ChunkType::SEPARATOR_CHUNK)
        {
          auto itThenContent = it;
          // tag the new link as a "if" link
          ChunkLinkType ifContentChunkLinkType = ChunkLinkType::IF;
          if (fSemFrameDict.doesIntroductionWordHasChunkLinkType(ifIGram.word, ChunkLinkType::TIME) &&
              !isACommandOrAListOfCommands(*itThenContent->chunk))
            ifContentChunkLinkType = ChunkLinkType::TIME;
          itThenContent->chunk->children.splice(itThenContent->chunk->children.begin(), pChunkLinks, itIfContent);
          ChunkLink& ifContent = *itThenContent->chunk->children.begin();
          ifContent.type = ifContentChunkLinkType;
          // move "if" chunk into the "if content" link
          ifContent.tokRange = itIf->chunk->tokRange;
          pChunkLinks.erase(itIf);
          // remove separator between subordonate clause and the principal verb group (if exists)
          if (itThen != pChunkLinks.end())
            pChunkLinks.erase(itThen);

          // get "else" node
          ++it;
          if (it == pChunkLinks.end())
            return;
          const InflectedWord& iGramElseHead = it->chunk->head->inflWords.front();
          if (it->chunk->type == ChunkType::SEPARATOR_CHUNK &&
              iGramElseHead.word.partOfSpeech == PartOfSpeech::CONJUNCTIVE &&
              iGramElseHead.infos.hasContextualInfo(WordContextualInfos::ELSE))
          {
            if (ifContent.type == ChunkLinkType::TIME)
              ifContent.type = ChunkLinkType::IF;

            auto itElse = it;

            // get "else content" node
            ++it;
            if (it == pChunkLinks.end())
              return;
            auto itElseContent = it;

            // bad "else content" we restart from here
            if (itElseContent->chunk->type == ChunkType::SEPARATOR_CHUNK)
            {
              --it;
              continue;
            }
            ++it;

            _addElseContent(pChunkLinks, itThenContent, itElseContent, itElse);
            if (it == pChunkLinks.end())
              return;
          }
          continue;
        }
      }
      break;
    }
    case ChunkType::NOMINAL_CHUNK:
    case ChunkType::PREPOSITIONAL_CHUNK:
    {
      if (ConceptSet::haveAConceptThatBeginWith(it->chunk->getHeadConcepts(), "time_"))
      {
        auto nextIt = it;
        ++nextIt;
        if (nextIt == pChunkLinks.end())
          return;
        if (nextIt->chunk->type == ChunkType::SEPARATOR_CHUNK)
        {
          auto nextNextIt = nextIt;
          ++nextNextIt;
          if (nextNextIt == pChunkLinks.end())
            return;
          if (nextNextIt->chunk->type == ChunkType::VERB_CHUNK)
          {
            auto timeChunkIt = it;
            it = nextNextIt;
            ++it;
            timeChunkIt->type = ChunkLinkType::TIME;
            timeChunkIt->tokRange = nextIt->chunk->tokRange;
            nextNextIt->chunk->children.splice(nextNextIt->chunk->children.begin(), pChunkLinks, timeChunkIt);
            pChunkLinks.erase(nextIt);
            if (it == pChunkLinks.end())
              return;
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




bool SubordinateExtractor::xTryToLinkASubordonateVerb
(std::list<ChunkLink>::iterator pChunkSep,
 std::list<ChunkLink>::iterator& pIt,
 std::list<ChunkLink>& pSyntTree) const
{
  // if we can look at chunk before the first separator
  if (pChunkSep != pSyntTree.begin())
  {
    const InflectionsChecker& fls = fConf.getFlsChecker();
    auto prevPrev = pChunkSep;
    --prevPrev;
    // if the current chunk is a verbal group
    if (pIt->chunk->type == ChunkType::VERB_CHUNK &&
        !haveASubject(*pIt->chunk))
    {
      const InflectedWord& verbIGram = pIt->chunk->head->inflWords.front();
      // if the verb is at the past participle
      if (fls.verbIsAtPastParticiple(verbIGram) ||
          fls.verbIsAtPresentParticiple(verbIGram))
      {
        return xAddOneOrAListofPastParticipleVerbs(pSyntTree, pIt, pChunkSep, *prevPrev->chunk);
      }
      // if the verb is not at the past participle AND
      // the prevPrev one is a nominal group
      if (prevPrev->chunk->type == ChunkType::NOMINAL_CHUNK &&
          canLinkVerbToASubject(*pIt->chunk, *prevPrev->chunk, fls, true))
      {
        if (pChunkSep != pIt)
        {
          prevPrev->tokRange = pChunkSep->chunk->tokRange;
          pSyntTree.erase(pChunkSep);
        }
        prevPrev->type = ChunkLinkType::SUBJECT;
        pIt->chunk->children.splice(pIt->chunk->children.begin(), pSyntTree, prevPrev);
        ++pIt;
        return true;
      }
    }
    // if we found a list of verbal groups AND
    // the first verb is conjugated at the past participle AND
    // the first verb is conjugated has no subject AND
    // if we succeed to link it to the previous chunk
    else if (chunkTypeIsAList(pIt->chunk->type) &&
             pIt->chunk->children.front().chunk->type == ChunkType::VERB_CHUNK &&
             fls.verbIsAtPastParticiple(pIt->chunk->children.front().chunk->head->inflWords.front()) &&
             !haveASubject(*pIt->chunk->children.front().chunk))
    {
      return xAddOneOrAListofPastParticipleVerbs(pSyntTree, pIt, pChunkSep, *prevPrev->chunk);
    }
    else if (hasAChunkTypeOrAListOfChunkTypes(*pIt->chunk, ChunkType::NOMINAL_CHUNK))
    {
      auto* parentVerbChunkPtr = getlatestVerbChunk(*prevPrev->chunk);
      if (parentVerbChunkPtr != nullptr)
      {
        Chunk& parentVerbChunk = *parentVerbChunkPtr;
        ChunkLink* doChkLk = getDOChunkLink(parentVerbChunk);
        if (doChkLk != nullptr)
        {
          if (pChunkSep->chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION)
          {
            if (pChunkSep != pIt)
            {
              pIt->tokRange = pChunkSep->chunk->tokRange;
              pSyntTree.erase(pChunkSep);
            }
            pIt->type = fConf.getEntityRecognizer().findNatureOfAChunkLink(*pIt, &parentVerbChunk);
            if (pIt->type == ChunkLinkType::SUBORDINATE_CLAUSE)
            {
              doChkLk->chunk->children.splice(doChkLk->chunk->children.begin(), pSyntTree, pIt++);
            }
            else
            {
              Chunk* verbChunk = &parentVerbChunk;
              auto itSubjectOf = getChunkLink(*doChkLk->chunk, ChunkLinkType::SUBJECT_OF);
              if (itSubjectOf != doChkLk->chunk->children.end() &&
                  itSubjectOf->chunk->type == ChunkType::VERB_CHUNK)
              {
                verbChunk = &*itSubjectOf->chunk;
              }
              verbChunk->children.splice(verbChunk->children.end(), pSyntTree, pIt++);
            }
            return true;
          }
        }
        else if (canBeTheHeadOfASubordinate(pIt->chunk->head->inflWords.front()))
        {
          if (pChunkSep != pIt)
          {
            pIt->tokRange = pChunkSep->chunk->tokRange;
            pSyntTree.erase(pChunkSep);
          }
          pIt->type = fConf.getEntityRecognizer().findNatureOfAChunkLink(*pIt, &parentVerbChunk);
          parentVerbChunk.children.splice(parentVerbChunk.children.end(), pSyntTree, pIt++);
          return true;
        }
      }
    }
  }
  return false;
}


bool SubordinateExtractor::xAddOneOrAListofPastParticipleVerbs
(std::list<ChunkLink>& pSyntTree,
 std::list<ChunkLink>::iterator& pIt,
 std::list<ChunkLink>::iterator pPrev,
 Chunk& pNewPotentialRootChunk) const
{
  std::list<ChunkLink>::iterator nexIt = pIt;
  ++nexIt;
  if (pNewPotentialRootChunk.type == ChunkType::NOMINAL_CHUNK &&
      canLinkVerbToASubject(*pIt->chunk, pNewPotentialRootChunk,
                            fConf.getFlsChecker(), true))
  {
    xAddASubChunk(pSyntTree, pNewPotentialRootChunk, pPrev, pIt,
                  ChunkLinkType::SUBJECT_OF);
    pIt = nexIt;
    return true;
  }
  else if (chunkTypeIsVerbal(pNewPotentialRootChunk.type))
  {
    Chunk* dObjPtr = getNoneVerbalDOComplOrSubjClauseChunk(pNewPotentialRootChunk);
    if (dObjPtr != nullptr)
    {
      Chunk& dObj = *dObjPtr;
      if (chunkTypeIsAList(dObj.type) &&
          dObj.children.size() >= 1)
      {
        return xAddOneOrAListofPastParticipleVerbs(pSyntTree, pIt, pPrev,
                                                   *dObj.children.back().chunk);
      }
      else if (canLinkVerbToASubject(*pIt->chunk, dObj,
                                     fConf.getFlsChecker(), true))
      {
        xAddASubChunk(pSyntTree, dObj, pPrev, pIt, ChunkLinkType::SUBJECT_OF);
        pIt = nexIt;
        return true;
      }
    }
    else
    {
      Chunk* subj = getSubjectChunk(pNewPotentialRootChunk);
      if (subj != nullptr &&
          checkOrder(pNewPotentialRootChunk, *subj) &&
          canLinkVerbToASubject(*pIt->chunk, *subj,
                                fConf.getFlsChecker(), true))
      {
        xAddASubChunk(pSyntTree, *subj, pPrev, pIt, ChunkLinkType::SUBJECT_OF);
        pIt = nexIt;
        return true;
      }
    }
  }
  return false;
}


void SubordinateExtractor::xAddASubChunk
(std::list<ChunkLink>& pSyntTree,
 Chunk& pMotherCunk,
 std::list<ChunkLink>::iterator pPrev,
 std::list<ChunkLink>::iterator pIt,
 ChunkLinkType pType)
{
  if (pPrev != pIt)
  {
    if (pIt->tokRange.isEmpty())
      pIt->tokRange = pPrev->chunk->tokRange;
    pSyntTree.erase(pPrev);
  }
  pIt->type = pType;
  pMotherCunk.children.emplace_back(*pIt);
  pSyntTree.erase(pIt);
}


void SubordinateExtractor::xLinkSubjectAndAuxiliaryToVerbsInsideAList
(std::list<ChunkLink>& pChunkLinks) const
{
  for (std::list<ChunkLink>::iterator it = pChunkLinks.begin();
       it != pChunkLinks.end(); ++it)
  {
    if (chunkTypeIsAList(it->chunk->type) &&
        !it->chunk->children.empty())
    {
      auto itFirstElt = it->chunk->children.begin();
      while (itFirstElt != it->chunk->children.end())
      {
        if (itFirstElt->type == ChunkLinkType::SIMPLE)
          break;
        ++itFirstElt;
      }
      if (itFirstElt == it->chunk->children.end())
        continue;
      Chunk& firstChunkOfTheList = *itFirstElt->chunk;
      if (firstChunkOfTheList.type != ChunkType::VERB_CHUNK)
        continue;
      ChunkLink* subjectChunkLink = nullptr;
      ChunkLink* auxiliaryChunkLink = nullptr;
      for (auto& currChildFirstVerb : firstChunkOfTheList.children)
      {
        if (currChildFirstVerb.type == ChunkLinkType::SUBJECT)
        {
          subjectChunkLink = &currChildFirstVerb;
          break;
        }
        if (currChildFirstVerb.type == ChunkLinkType::AUXILIARY)
        {
          auxiliaryChunkLink = &currChildFirstVerb;
          subjectChunkLink = getSubjectChunkLink(*currChildFirstVerb.chunk);
          break;
        }
      }
      if (subjectChunkLink == nullptr)
      {
        continue;
      }
      for (auto itList = ++itFirstElt; itList != it->chunk->children.end(); ++itList)
      {
        if (itList->chunk->type != ChunkType::VERB_CHUNK ||
            !itList->chunk->requests.empty() ||
            haveASubject(*itList->chunk))
        {
          break;
        }
        Chunk* auxChunkPtr = getAuxiliaryChunk(*itList->chunk);
        if (auxChunkPtr == nullptr)
        {
          if (auxiliaryChunkLink != nullptr)
          {
            const InflectionsChecker& fls = fConf.getFlsChecker();
            if (fls.verbCanHaveAnAuxiliary(itList->chunk->head->inflWords.front()) &&
                fls.areCompatibles(auxiliaryChunkLink->chunk->head->inflWords.front(),
                                   itList->chunk->head->inflWords.front()))
            {
              auxiliaryChunkLink->chunk->hasOnlyOneReference = false;
              itList->chunk->children.emplace_back(*auxiliaryChunkLink);
              itList->chunk->isPassive = isChunkAtPassiveForm(*itList->chunk, fConf.getLingDico(), fls);
              itList->chunk->positive = auxiliaryChunkLink->chunk->positive;
              itList->chunk->form = auxiliaryChunkLink->chunk->form;
              itList->chunk->requests = auxiliaryChunkLink->chunk->requests;
            }
          }
          else if (canLinkVerbToASubject(*itList->chunk, *subjectChunkLink->chunk,
                                         fConf.getFlsChecker(), true))
          {
            subjectChunkLink->chunk->hasOnlyOneReference = false;
            itList->chunk->children.emplace_back(*subjectChunkLink);
          }
        }
        else if (canLinkVerbToASubject(*auxChunkPtr, *subjectChunkLink->chunk,
                                       fConf.getFlsChecker(), true))
        {
          subjectChunkLink->chunk->hasOnlyOneReference = false;
          auxChunkPtr->children.emplace_back(*subjectChunkLink);
        }
      }
    }
  }
}



void SubordinateExtractor::xComparisonsFinder
(std::list<ChunkLink>& pChunkLinks) const
{
  SemanticLanguageEnum langType = fConf.getLanguageType();
  const InflectionsChecker& fls = fConf.getFlsChecker();

  for (const auto& currChunkLk : pChunkLinks)
  {
    Chunk& currChunk = *currChunkLk.chunk;
    if (currChunk.type == ChunkType::VERB_CHUNK &&
        currChunk.head->inflWords.front().word == fLingDico.getBeVerb().word)
    {
      auto itDO = getChunkLink(currChunk, ChunkLinkType::DIRECTOBJECT);
      if (itDO != currChunk.children.end())
      {
        auto itSub = getChunkLink(currChunk, ChunkLinkType::SUBORDINATE);
        if (itSub != currChunk.children.end())
        {
          bool isAComparison = false;
          ComparisonOperator compPolarity = ComparisonOperator::EQUAL;
          TokIt itEnd =currChunk.tokRange.getItEnd();
          for (TokIt itTokAfterVerb = getNextToken(currChunk.head, itEnd);
               itTokAfterVerb != itEnd; itTokAfterVerb = getNextToken(itTokAfterVerb, itEnd))
          {
            if (itTokAfterVerb->inflWords.front().word.partOfSpeech == PartOfSpeech::ADVERB)
            {
              const auto& advConcepts = itTokAfterVerb->inflWords.front().infos.concepts;
              if (ConceptSet::haveAConcept(advConcepts, "comparison_more"))
              {
                isAComparison = true;
                compPolarity = ComparisonOperator::MORE;
              }
              else if (ConceptSet::haveAConcept(advConcepts, "comparison_less"))
              {
                isAComparison = true;
                compPolarity = ComparisonOperator::LESS;
              }
            }
          }

          if (!isAComparison)
          {
            const InflectedWord& doInfosGram = itDO->chunk->head->inflWords.front();
            if (doInfosGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE)
            {
              if (langType == SemanticLanguageEnum::ENGLISH &&
                  fls.adjCanBeComparative(doInfosGram))
              {
                isAComparison = true;
                compPolarity = ComparisonOperator::MORE;
              }
              else if (ConceptSet::haveAConcept(doInfosGram.infos.concepts, "comparison_equal"))
              {
                isAComparison = true;
                compPolarity = currChunk.positive ? ComparisonOperator::EQUAL : ComparisonOperator::DIFFERENT;
              }
            }
          }

          if (isAComparison)
          {
            itDO->type = chunkLinkType_fromCompPolarity(compPolarity);
            itSub->type = ChunkLinkType::RIGHTOPCOMPARISON;
            itSub->chunk->introductingWordToSaveForSynthesis.reset();
          }
        }
      }
    }

    if (!currChunk.children.empty())
    {
      xComparisonsFinder(currChunk.children);
    }
  }
}

} // End of namespace linguistics
} // End of namespace onsem
