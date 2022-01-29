#include "verbaltomoninalchunkslinker.hpp"
#include <iostream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/enumsconvertions.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include "questionwords.hpp"
#include "entityrecognizer.hpp"
#include "chunkslinker.hpp"
#include "../tool/chunkshandler.hpp"


namespace onsem
{
namespace linguistics
{
namespace
{
bool _doesSubjectCanBeCompletedWithAPronounSubjectFr(const SemanticRequests& pRequests)
{
  for (const auto& currRequest : pRequests.types)
    if (currRequest == SemanticRequestType::SUBJECT ||
        currRequest == SemanticRequestType::OBJECT ||
        currRequest == SemanticRequestType::QUANTITY ||
        currRequest == SemanticRequestType::LOCATION ||
        currRequest == SemanticRequestType::TIME)
      return false;
  return true;
}

void _modifyRequestOfChunkThatWillHaveASubject(Chunk& pVerbChunk,
                                               const Chunk& pSubVerbChunk,
                                               const Chunk& pSubjectChunk)
{
  if (pSubVerbChunk.requestWordTokenPosOpt)
  {
    if (pSubjectChunk.tokRange.doesContain(*pSubVerbChunk.requestWordTokenPosOpt))
      pVerbChunk.requests.set(SemanticRequestType::SUBJECT);
  }
  else if (pSubVerbChunk.requestCanBeObject &&
           pSubVerbChunk.requests.has(SemanticRequestType::SUBJECT))
  {
    pVerbChunk.requests.set(SemanticRequestType::OBJECT);
  }
}

}

VerbalToNominalChunksLinker::VerbalToNominalChunksLinker
(const AlgorithmSetForALanguage& pConfiguration)
  : fConf(pConfiguration),
    fSpecLingDb(fConf.getSpecifcLingDb()),
    fLingDico(fConf.getLingDico())
{
}



void VerbalToNominalChunksLinker::process
(std::list<ChunkLink>& pFirstChildren) const
{
  auto endChild = pFirstChildren.end();
  auto prevVerb = endChild;
  for (auto it = pFirstChildren.begin(); it != endChild; ++it)
  {
    ChunkType chunkType = it->chunk->type;
    if (chunkTypeIsVerbal(chunkType))
    {
      _constructASyntGraphBetween2VerbChunks(pFirstChildren, prevVerb, it);
      prevVerb = it;
    }
  }
  _constructASyntGraphBetween2VerbChunks(pFirstChildren, prevVerb, endChild);
}



void VerbalToNominalChunksLinker::extractEnglishSubjectOf
(std::list<ChunkLink>& pFirstChildren) const
{
  auto endChild = pFirstChildren.end();
  for (auto it = pFirstChildren.begin(); it != endChild; ++it)
  {
    if (it->chunk->getHeadPartOfSpeech() == PartOfSpeech::DETERMINER)
    {
      auto itVerb = it;
      ++itVerb;
      if (itVerb == endChild)
        break;
      if (!chunkTypeIsVerbal(itVerb->chunk->type))
      {
        it = itVerb;
        continue;
      }
      auto itNominalGroup = itVerb;
      ++itNominalGroup;
      if (itNominalGroup == endChild)
        break;
      if (itNominalGroup->chunk->getHeadPartOfSpeech() != PartOfSpeech::NOUN)
      {
        it = itNominalGroup;
        continue;
      }
      itVerb->type = ChunkLinkType::SUBJECT_OF;
      itNominalGroup->chunk->children.splice(itNominalGroup->chunk->children.end(), pFirstChildren, it);
      itNominalGroup->chunk->children.splice(itNominalGroup->chunk->children.end(), pFirstChildren, itVerb);
      it = itNominalGroup;
    }
  }
}



void VerbalToNominalChunksLinker::_splitAdverbBeforeNouns(ChunkLinkIter& pChunkLinkIter) const
{
  Chunk& currChunk = *pChunkLinkIter->chunk;
  bool isAnAdverbBefore = false;
  for (TokIt currIt = currChunk.tokRange.getItBegin();
       currIt != currChunk.tokRange.getItEnd();
       currIt = getNextToken(currIt, currChunk.tokRange.getItEnd(), SkipPartOfWord::YES))
  {
    if (isAnAdverbBefore)
    {
      SemanticLanguageEnum language = fConf.getLanguageType();
      separateBeginOfAChunk(pChunkLinkIter, currIt,
                            currChunk.type, language);
      _splitAdverbBeforeNouns(pChunkLinkIter);
      return;
    }
    PartOfSpeech currentPartOfSpeech = currIt->inflWords.begin()->word.partOfSpeech;
    if (currentPartOfSpeech != PartOfSpeech::ADVERB)
      return;
    isAnAdverbBefore = true;
  }
}


void VerbalToNominalChunksLinker::_constructASyntGraphBetween2VerbChunks
(std::list<ChunkLink>& pRootList,
 std::list<ChunkLink>::iterator pItFirstVerbChunk,
 std::list<ChunkLink>::iterator pItSecondVerbChunk) const
{
  auto afterFirstVerb = [&]
  {
    if (pItFirstVerbChunk == pRootList.end()) // if there is no first verb
      return pRootList.begin();
    return std::next(pItFirstVerbChunk);
  }();
  ChunkLinkWorkingZone workingZone(pRootList, afterFirstVerb, pItSecondVerbChunk);
  if (workingZone.empty())
    return;
  Chunk* firstVerbChunk =
      pItFirstVerbChunk == pRootList.end() ? nullptr : &*pItFirstVerbChunk->chunk;
  Chunk* secondVerbChunk =
      pItSecondVerbChunk == pRootList.end() ? nullptr : &*pItSecondVerbChunk->chunk;

  const InflectionsChecker& inflsCheker = fConf.getFlsChecker();
  SemanticLanguageEnum language = fConf.getLanguageType();

  Chunk* subSecondVerbChunk = nullptr;
  if (secondVerbChunk != nullptr)
    subSecondVerbChunk = linguistics::whereToLinkTheSubjectPtr(secondVerbChunk, inflsCheker);

  // extract question words
  if (!workingZone.empty())
    _extractQuestionWords(workingZone, firstVerbChunk, secondVerbChunk,
                          subSecondVerbChunk);

  // handle first verb children in case of a question
  if (!workingZone.empty() && workingZone.begin()->chunk->type != ChunkType::SEPARATOR_CHUNK &&
      firstVerbChunk != nullptr && firstVerbChunk->type == ChunkType::VERB_CHUNK)
  {
    auto itFirstChunkLk = workingZone.begin();
    auto newFirstChunkHead = itFirstChunkLk->chunk->tokRange.getItBegin();
    while (newFirstChunkHead != itFirstChunkLk->chunk->tokRange.getItEnd())
    {
      PartOfSpeech headPartOfSpeech = newFirstChunkHead->inflWords.front().word.partOfSpeech;
      if (headPartOfSpeech != PartOfSpeech::DETERMINER && headPartOfSpeech != PartOfSpeech::PRONOUN_COMPLEMENT)
        break;
      newFirstChunkHead = getNextToken(newFirstChunkHead, itFirstChunkLk->chunk->tokRange.getItEnd());
    }
    if (newFirstChunkHead != itFirstChunkLk->chunk->tokRange.getItEnd() &&
        _canLinkNextChunkAsTheSubject(workingZone, firstVerbChunk, newFirstChunkHead))
    {
      setSubjectPronounInflectionsAccordingToTheVerb(newFirstChunkHead->inflWords.front(),
                                                     firstVerbChunk->head->inflWords.front(), inflsCheker);
      TokIt newEnd = newFirstChunkHead;
      if (haveChild(*firstVerbChunk, ChunkLinkType::QUESTIONWORD))
        newEnd = getEndOfNominalGroup(itFirstChunkLk->chunk->tokRange, newFirstChunkHead);
      else
        newEnd = getNextToken(newFirstChunkHead, itFirstChunkLk->chunk->tokRange.getItEnd());
      if (newEnd != itFirstChunkLk->chunk->tokRange.getItEnd())
      {
        auto itSecondChunkLk = itFirstChunkLk;
        ++itSecondChunkLk;
        if (itSecondChunkLk == workingZone.end() ||
            itSecondChunkLk->chunk->type != ChunkType::NOMINAL_CHUNK ||
            itSecondChunkLk->chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::ADVERB)
        {
          separateBeginOfAChunk(workingZone.syntTree(), itFirstChunkLk, newEnd,
                                ChunkType::NOMINAL_CHUNK, newFirstChunkHead, language);
        }
      }
      bool isTheSubjectInserted = false;
      if (firstVerbChunk->form != LingVerbForm::INTERROGATIVE)
      {
        firstVerbChunk->form = LingVerbForm::INTERROGATIVE;
        firstVerbChunk->requests.set(SemanticRequestType::YESORNO);
      }
      else
      {
        ChunkLink* oldSubjectLink = getSubjectChunkLink(*firstVerbChunk);
        if (oldSubjectLink != nullptr)
        {
          if (language == SemanticLanguageEnum::FRENCH &&
              _doesSubjectCanBeCompletedWithAPronounSubjectFr(firstVerbChunk->requests))
          {
            oldSubjectLink->tokRange = workingZone.begin()->chunk->tokRange;
            workingZone.syntTree().erase(workingZone.begin());
            isTheSubjectInserted = true;
          }
          else
          {
            if (firstVerbChunk->requestWordTokenPosOpt &&
                oldSubjectLink->chunk->tokRange.doesContain(*firstVerbChunk->requestWordTokenPosOpt))
               firstVerbChunk->requests.set(SemanticRequestType::OBJECT);
            if (chunkCanBeAnObject(*oldSubjectLink->chunk))
              oldSubjectLink->type = ChunkLinkType::DIRECTOBJECT;
            else
              oldSubjectLink->type = ChunkLinkType::SPECIFICATION;
          }
        }
      }
      if (!isTheSubjectInserted)
      {
        auto itChLkSubject = workingZone.begin();
        _modifyRequestOfChunkThatWillHaveASubject(*firstVerbChunk, *firstVerbChunk, *itChLkSubject->chunk);
        itChLkSubject->type = ChunkLinkType::SUBJECT;
        firstVerbChunk->children.splice(firstVerbChunk->children.end(),
                                        workingZone.syntTree(), itChLkSubject);
      }
      if (!workingZone.empty())
      {
        auto beginListIter = workingZone.beginListIter();
        _splitAdverbBeforeNouns(beginListIter);
      }
    }
  }

  const auto& entityRecognizer = fConf.getEntityRecognizer();
  bool codAlreadyLinkToFirstVerb = false;
  if (firstVerbChunk != nullptr &&
      !workingZone.empty())
  {
    ChunkLink* doCkLkPtr = getDOChunkLink(*firstVerbChunk);
    if (doCkLkPtr != nullptr)
    {
      auto itToPotentialPartitiveChunk = workingZone.beginListIter();
      linkPartitives(*doCkLkPtr->chunk, itToPotentialPartitiveChunk, entityRecognizer);
    }
    codAlreadyLinkToFirstVerb = doCkLkPtr != nullptr;

    // link cod of the first verb if the first verb:
    // * doesn't have an auxiliary
    // * is at past participle or infinitive
    if (!codAlreadyLinkToFirstVerb && !workingZone.empty() &&
        !haveAnAuxiliary(*firstVerbChunk) &&
        ((language == SemanticLanguageEnum::ENGLISH && firstVerbChunk->type == ChunkType::INFINITVE_VERB_CHUNK) ||
         firstVerbChunk->head->inflWords.front().infos.hasContextualInfo(WordContextualInfos::TRANSITIVEVERB) ||
         inflsCheker.verbIsOnlyAtPastParticiple(firstVerbChunk->head->inflWords.front())))
    {
      codAlreadyLinkToFirstVerb |= _linkAVerbGroupToHisCOD(workingZone, firstVerbChunk);
    }

    if (!codAlreadyLinkToFirstVerb &&
        firstVerbChunk->type == ChunkType::INFINITVE_VERB_CHUNK)
    {
      auto beforeFirstVerb = workingZone.begin();
      --beforeFirstVerb;
      if (beforeFirstVerb != workingZone.syntTree().begin())
      {
        --beforeFirstVerb;
        if (beforeFirstVerb != workingZone.syntTree().begin() &&
            beforeFirstVerb->chunk->type != ChunkType::VERB_CHUNK &&
            beforeFirstVerb->chunk->type != ChunkType::SEPARATOR_CHUNK)
        {
          codAlreadyLinkToFirstVerb |= _linkAVerbGroupToHisCOD(workingZone, firstVerbChunk);
        }
      }
    }

    if (language == SemanticLanguageEnum::FRENCH &&
        !codAlreadyLinkToFirstVerb &&
        subSecondVerbChunk != nullptr &&
        inflsCheker.verbCanBeAtImperative(subSecondVerbChunk->head->inflWords.front()) &&
        workingZone.size() == 1)
    {
      codAlreadyLinkToFirstVerb |= _linkAVerbGroupToHisCOD(workingZone, firstVerbChunk);
    }
  }


  if (!workingZone.empty())
  {
    if (subSecondVerbChunk != nullptr)
    {
      ChunkLink* subjectChunkLkPtr = getSubjectChunkLink(*subSecondVerbChunk);
      if (subjectChunkLkPtr == nullptr)
      {
        auto potentialSubject = workingZone.end();
        --potentialSubject;
        const InflectedWord& firstWordIGram = potentialSubject->chunk->tokRange.getItBegin()->inflWords.front();
        if (firstWordIGram.word.partOfSpeech != PartOfSpeech::PARTITIVE &&
            _linkAVerbChunkToHisSubject(*subSecondVerbChunk, *secondVerbChunk, *potentialSubject))
          workingZone.syntTree().erase(potentialSubject);
      }
      else
      {
        auto potentialSubject = workingZone.end();
        --potentialSubject;
        if ((fConf.getLinker().canSubstituteAChunkByAnother(*subjectChunkLkPtr, *potentialSubject->chunk, true)))
        {
          subjectChunkLkPtr->tokRange = subjectChunkLkPtr->chunk->tokRange;
          subjectChunkLkPtr->chunk = potentialSubject->chunk;
          workingZone.syntTree().erase(potentialSubject);
        }
      }
    }
    else if (secondVerbChunk != nullptr &&
             secondVerbChunk->type == ChunkType::INFINITVE_VERB_CHUNK &&
             secondVerbChunk->requests.has(SemanticRequestType::OBJECT))
    {
      auto potentialDO = workingZone.end();
      --potentialDO;
      auto& potentialDOChunk = potentialDO->chunk;
      if (potentialDOChunk->type != ChunkType::SEPARATOR_CHUNK)
      {
        secondVerbChunk->children.emplace_back(ChunkLinkType::DIRECTOBJECT, potentialDOChunk);
        workingZone.syntTree().erase(potentialDO);
      }
    }
  }


  if (firstVerbChunk != nullptr &&
      !workingZone.empty())
  {
    if (firstVerbChunk->form == LingVerbForm::INTERROGATIVE &&
        !firstVerbChunk->requests.has(SemanticRequestType::SUBJECT) &&
        !haveASubjectExceptQuestionWord(*firstVerbChunk))
    {
      Chunk* subFirstVerbChunk = linguistics::whereToLinkTheSubjectPtr(firstVerbChunk, inflsCheker);
      if (subFirstVerbChunk != nullptr &&
          (!ConceptSet::haveAConceptThatBeginWith(workingZone.begin()->chunk->getHeadConcepts(), "time_") ||
           ConceptSet::haveAConcept(firstVerbChunk->getHeadConcepts(), "verb_equal_be")) &&
          _linkAVerbChunkToHisSubject(*subFirstVerbChunk, *firstVerbChunk, *workingZone.begin()))
      {
        workingZone.syntTree().erase(workingZone.begin());
      }
      else
      {
        bool questWordIsSubject = false;
        // know if the question word is subject
        for (auto it = firstVerbChunk->children.begin(); it != firstVerbChunk->children.end(); ++it)
        {
          if (it->type == ChunkLinkType::QUESTIONWORD &&
              canLinkVerbToASubject(*firstVerbChunk, *it->chunk, inflsCheker, false))
          {
            firstVerbChunk->requests.set(SemanticRequestType::SUBJECT);
            questWordIsSubject = true;
            break;
          }
        }
        if (questWordIsSubject && !codAlreadyLinkToFirstVerb)
        {
          _linkAVerbGroupToHisCOD(workingZone, firstVerbChunk);
        }
      }
    }
    else if (!codAlreadyLinkToFirstVerb)
    {
      _linkAVerbGroupToHisCOD(workingZone, firstVerbChunk);
    }
  }

  if (subSecondVerbChunk != nullptr && secondVerbChunk != nullptr &&
      !workingZone.empty() &&
      (secondVerbChunk->form == LingVerbForm::INTERROGATIVE ||
       (workingZone.begin()->chunk->type == ChunkType::SEPARATOR_CHUNK &&
        fLingDico.statDb.wordToSubordinateRequest(workingZone.begin()->chunk->head->inflWords.front().word) != SemanticRequestType::NOTHING)))
  {
    _linkObjectBeforeAnInterrogativeVerb(workingZone, *secondVerbChunk,
                                         *subSecondVerbChunk);
  }

  if (firstVerbChunk != nullptr)
  {
    mystd::optional<ChunkLinkIter> itSeparator;
    auto putSeparatorBefore = [&itSeparator](ChunkLinkIter& pCurrIt)
    {
      if (itSeparator)
      {
        if (!itSeparator->atEnd())
        {
          pCurrIt->tokRange = std::move((*itSeparator)->chunk->tokRange);
          itSeparator->eraseIt();
        }
        itSeparator.reset();
      }
    };

    auto itChunkLink = workingZone.beginListIter();
    while (!itChunkLink.atEnd())
    {
      auto itNext = itChunkLink;
      ++itNext;
      const Chunk& currChunk = *itChunkLink->chunk;
      if (!itSeparator || itSeparator->atEnd())
      {
        if (currChunk.type == ChunkType::SEPARATOR_CHUNK &&
            currChunk.head->inflWords.front().word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS)
        {
          itSeparator = itChunkLink;
          itChunkLink = itNext;
          continue;
        }
        if (!chunkCanBeAnObject(currChunk))
        {
          break;
        }
      }
      else if (currChunk.type != ChunkType::PREPOSITIONAL_CHUNK ||
               currChunk.getHeadPartOfSpeech() == PartOfSpeech::PREPOSITION)
      {
        if (currChunk.type == ChunkType::NOMINAL_CHUNK &&
            currChunk.head->inflWords.front().word.partOfSpeech == PartOfSpeech::PROPER_NOUN &&
            !firstVerbChunk->children.empty())
        {
          auto& firstVerbLastChildChLk = firstVerbChunk->children.back();
          if (firstVerbLastChildChLk.type != ChunkLinkType::QUESTIONWORD &&
              firstVerbLastChildChLk.type != ChunkLinkType::AUXILIARY &&
              firstVerbLastChildChLk.chunk->getHeadPartOfSpeech() == PartOfSpeech::PROPER_NOUN)
          {
            putSeparatorBefore(itChunkLink);
            itChunkLink->type = ChunkLinkType::SPECIFICATION;
            firstVerbLastChildChLk.chunk->children.splice(firstVerbLastChildChLk.chunk->children.end(),
                                                          workingZone.syntTree(), itChunkLink.getIt());
          }
        }
        break;
      }
      if (canBeTheHeadOfASubordinate(itChunkLink->chunk->head->inflWords.front()))
      {
        itChunkLink->type = entityRecognizer.findNatureOfAChunkLink(*itChunkLink, firstVerbChunk);
        putSeparatorBefore(itChunkLink);
        firstVerbChunk->children.splice(firstVerbChunk->children.end(),
                                        workingZone.syntTree(), itChunkLink.getIt());
      }
      itChunkLink = itNext;
    }

    if (ConceptSet::haveAConcept(firstVerbChunk->getHeadConcepts(), "verb_action_say"))
    {
      // move the interjection at the other objects children
      auto itInterjection = firstVerbChunk->children.end();
      for (auto it = firstVerbChunk->children.begin(); it != firstVerbChunk->children.end(); ++it)
      {
        if (it->type == ChunkLinkType::DIRECTOBJECT ||
            it->type == ChunkLinkType::SUBORDINATE)
        {
          if (itInterjection == firstVerbChunk->children.end())
          {
            if (it->chunk->type == ChunkType::INTERJECTION_CHUNK)
              itInterjection = it;
            else
              break;
          }
          else
          {
            itInterjection->type = ChunkLinkType::INTERJECTION;
            it->type = ChunkLinkType::DIRECTOBJECT;
            it->chunk->children.splice(it->chunk->children.begin(),
                                       firstVerbChunk->children, itInterjection);
          }
        }
      }
    }
  }
}


bool VerbalToNominalChunksLinker::_canLinkNextChunkAsTheSubject(ChunkLinkWorkingZone& pWorkingZone,
                                                                const Chunk* firstVerbChunk,
                                                                TokIt newFirstChunkHead) const
{
  const InflectedWord& headInflWord = newFirstChunkHead->inflWords.front();
  if (headInflWord.infos.hasContextualInfo(WordContextualInfos::CANNOTBEASUBJECT))
    return false;
  const InflectedWord& firstVerbInflWord = firstVerbChunk->head->inflWords.front();
  PartOfSpeech headPartOfSpeech = headInflWord.word.partOfSpeech;
  const InflectionsChecker& inflsCheker = fConf.getFlsChecker();

  switch (fConf.getLanguageType())
  {
  case SemanticLanguageEnum::ENGLISH:
  {
    if (pWorkingZone.sizeUntilNextNotNominal() > 1 &&
        firstVerbChunk->head->inflWords.front().word == SemanticWord(SemanticLanguageEnum::ENGLISH, "be", PartOfSpeech::VERB) &&
        !haveASubject(*firstVerbChunk) &&
        canLinkVerbToASubject(*firstVerbChunk, *pWorkingZone.begin()->chunk, inflsCheker, false))
      return true;
    bool canBeASubjectAfterVerb = headPartOfSpeech == PartOfSpeech::PRONOUN_SUBJECT ||
        (headPartOfSpeech == PartOfSpeech::PRONOUN ||
         (partOfSpeech_isNominal(headPartOfSpeech) && !headInflWord.infos.hasContextualInfo(WordContextualInfos::CANBEBEFORENOUN)));
    if (!canBeASubjectAfterVerb || firstVerbInflWord.word != fLingDico.getBeVerb().word)
      return false;
    break;
  }
  case SemanticLanguageEnum::FRENCH:
  {
    if (firstVerbChunk->requests.has(SemanticRequestType::OBJECT) && !haveAnAuxiliary(*firstVerbChunk) && !haveASubject(*firstVerbChunk))
      return true;
    if (headPartOfSpeech != PartOfSpeech::PRONOUN_SUBJECT ||
        firstVerbInflWord.word == fLingDico.getSayVerb().word)
      return false;
    break;
  }
  default:
    return false;
  }

  return ((firstVerbChunk->form == LingVerbForm::INTERROGATIVE && !haveAnAuxiliary(*firstVerbChunk)) ||
       !haveASubject(*firstVerbChunk)) &&
      inflsCheker.areCompatibles(firstVerbInflWord, newFirstChunkHead->inflWords.front());
}


void VerbalToNominalChunksLinker::_extractQuestionWords
(ChunkLinkWorkingZone& pWorkingZone,
 Chunk* pFirstVerbChunk,
 Chunk* pSecondVerbChunk,
 Chunk* pSubSecondVerbChunk) const
{
  Chunk* secondVerbChunkForQuestionWord = pSubSecondVerbChunk != nullptr ? pSubSecondVerbChunk : pSecondVerbChunk;

  if (pFirstVerbChunk != nullptr || secondVerbChunkForQuestionWord != nullptr)
  {
    questionWords::addQuestionWords(pWorkingZone, pFirstVerbChunk,
                                    secondVerbChunkForQuestionWord, fSpecLingDb);
    if (pSecondVerbChunk != nullptr && pSubSecondVerbChunk != nullptr)
    {
      if (!pSubSecondVerbChunk->requests.empty())
        pSecondVerbChunk->requests = pSubSecondVerbChunk->requests;
      if (!pSecondVerbChunk->requests.empty())
        pSecondVerbChunk->form = LingVerbForm::INTERROGATIVE;
    }
  }
}


void VerbalToNominalChunksLinker::_tryAddDirectObjectToImperativeVerb
(Chunk& pVerbRoot,
 ChunkLinkWorkingZone& pWorkingZone) const
{
  if (pVerbRoot.requests.empty() &&
      !haveASubject(pVerbRoot))
  {
    const InflectedWord& iGramVerb = pVerbRoot.head->inflWords.front();
    if (fConf.getFlsChecker().verbCanBeAtImperative(iGramVerb))
    {
      auto itSubordonatesRootLink = pWorkingZone.begin();
      ChunkLink& subordonatesRootLink = *itSubordonatesRootLink;
      const InflectedWord& iGramPotPron = subordonatesRootLink.chunk->tokRange.getItBegin()->inflWords.front();
      if (partOfSpeech_isPronominal(iGramPotPron.word.partOfSpeech))
      {
        pVerbRoot.requests.set(SemanticRequestType::ACTION);
        subordonatesRootLink.type = ChunkLinkType::DIRECTOBJECT;
        pVerbRoot.children.splice(pVerbRoot.children.end(), pWorkingZone.syntTree(), itSubordonatesRootLink);
      }
    }
  }
}


void VerbalToNominalChunksLinker::_linkObjectBeforeAnInterrogativeVerb
(ChunkLinkWorkingZone& pWorkingZone,
 Chunk& pSecondVerbChunk,
 Chunk& pSubSecondVerbChunk) const
{
  const Chunk* qWordChunkPtr = getChildChunkPtr(pSubSecondVerbChunk, ChunkLinkType::QUESTIONWORD);
  auto itSubordinate = pWorkingZone.begin();
  advanceBeforeLastSeparator(itSubordinate, pWorkingZone);
  while (itSubordinate != pWorkingZone.end())
  {
    auto itNext = itSubordinate;
    ++itNext;
    Chunk& subordonateChunk = *itSubordinate->chunk;
    if ((qWordChunkPtr == nullptr || checkOrder(*qWordChunkPtr, subordonateChunk)))
    {
      if (fConf.getLanguageType() == SemanticLanguageEnum::FRENCH &&
          itNext == pWorkingZone.end() &&
          _doesSubjectCanBeCompletedWithAPronounSubjectFr(pSecondVerbChunk.requests))
      {
        ChunkLink* subjectChunkLink = getSubjectChunkLink(pSecondVerbChunk);
        if (subjectChunkLink != nullptr)
        {
          subjectChunkLink->tokRange = subjectChunkLink->chunk->tokRange;
          subjectChunkLink->chunk = itSubordinate->chunk;
          pWorkingZone.syntTree().erase(itSubordinate);
          return;
        }
      }

      if (chunkCanBeAnObject(subordonateChunk))
      {
        bool isChunkLinkTypeSet = false;
        if (pSubSecondVerbChunk.requests.has(SemanticRequestType::QUANTITY))
        {
          const auto& subCpts = subordonateChunk.head->inflWords.front().infos.concepts;
          if (ConceptSet::haveAConcept(subCpts, "time") ||
              ConceptSet::haveAConceptThatBeginWith(subCpts, "duration_"))
          {
            pSecondVerbChunk.requests.set(SemanticRequestType::DURATION);
            pSubSecondVerbChunk.requests.set(SemanticRequestType::DURATION);
            itSubordinate->type = ChunkLinkType::DURATION;
            isChunkLinkTypeSet = true;
          }
        }

        if (!isChunkLinkTypeSet)
        {
          auto chkLkType = linguistics::requestTypeToChunkType(pSubSecondVerbChunk.requests.firstOrNothing());
          if (chkLkType)
            itSubordinate->type = *chkLkType;
          else
            itSubordinate->type = ChunkLinkType::DIRECTOBJECT;
          if (itSubordinate->type == ChunkLinkType::DIRECTOBJECT)
          {
            itSubordinate->type = fConf.getEntityRecognizer().findNatureOfAChunkLink(*itSubordinate, &pSubSecondVerbChunk);
            if (itSubordinate->type == ChunkLinkType::OWNER)
              itSubordinate->type = ChunkLinkType::DIRECTOBJECT;
          }
          if (itSubordinate->type == ChunkLinkType::SPECIFICATION)
            itSubordinate->type = ChunkLinkType::DIRECTOBJECT;
        }
        pSubSecondVerbChunk.children.splice(pSubSecondVerbChunk.children.end(),
                                            pWorkingZone.syntTree(), itSubordinate);
      }
    }
    itSubordinate = itNext;
  }
}


bool VerbalToNominalChunksLinker::_linkAVerbGroupToHisCOD
(ChunkLinkWorkingZone& pWorkingZone,
 Chunk* pVerbChunk) const
{
  if (pWorkingZone.empty())
    return false;

  {
    Chunk& subordonateChunk = *pWorkingZone.begin()->chunk;
    const auto& subordinateInflWord = subordonateChunk.head->inflWords.front();
    if (!canBeTheHeadOfASubordinate(subordinateInflWord))
      return false;
    PartOfSpeech codPartOfSpeech = subordinateInflWord.word.partOfSpeech;
    if (codPartOfSpeech == PartOfSpeech::PRONOUN_SUBJECT)
    {
      if (fConf.getLanguageType() == SemanticLanguageEnum::FRENCH &&
          pVerbChunk != nullptr)
      {
        ChunkLink* subjectChunkLinkPtr = getSubjectChunkLink(*pVerbChunk);
        if (subjectChunkLinkPtr != nullptr)
        {
          pVerbChunk->requests.set(SemanticRequestType::YESORNO);
          subjectChunkLinkPtr->tokRange = subordonateChunk.tokRange;
          pWorkingZone.beginListIter().eraseIt();
        }
      }
      return false;
    }
    if (codPartOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION ||
        codPartOfSpeech == PartOfSpeech::PREPOSITION)
      return false;
  }

  _tryAddDirectObjectToImperativeVerb(*pVerbChunk, pWorkingZone);
  while (!pWorkingZone.empty())
  {
    auto& subordonateChunkLk = *pWorkingZone.begin();
    Chunk& subordonateChunk = *subordonateChunkLk.chunk;
    if (subordonateChunk.type == ChunkType::INFINITVE_VERB_CHUNK)
    {
      subordonateChunkLk.type = ChunkLinkType::DIRECTOBJECT;
      pVerbChunk->children.splice(pVerbChunk->children.end(),
                                  pWorkingZone.syntTree(), pWorkingZone.begin());
      break;
    }
    if (subordonateChunk.type != ChunkType::PREPOSITIONAL_CHUNK &&
        pVerbChunk->requests.isAQuestionNotAskingAboutTheObject() &&
        haveAQuestionWordChildAfter(*pVerbChunk))
    {
      auto itSubjectChunkLk = getSubjectChunkLkIterator(*pVerbChunk);
      if (!itSubjectChunkLk.atEnd() &&
          (fConf.getLinker().canSubstituteAChunkByAnother(*itSubjectChunkLk, subordonateChunk, true)))
      {
        subordonateChunkLk.type = ChunkLinkType::SUBJECT;
        subordonateChunkLk.tokRange = itSubjectChunkLk->chunk->tokRange;
        itSubjectChunkLk.eraseIt();
        pVerbChunk->children.splice(pVerbChunk->children.end(),
                                    pWorkingZone.syntTree(), pWorkingZone.begin());
        break;
      }
    }

    if (chunkCanBeAnObject(subordonateChunk))
    {
      if (fConf.getEntityRecognizer().addSubordonatesToAVerb(*pVerbChunk, pWorkingZone))
        break; // TODO: replace break by continue
    }
    break;
  }
  return true;
}


bool VerbalToNominalChunksLinker::_linkAVerbChunkToHisSubject
(Chunk& pSubVerbChunk,
 Chunk& pVerbChunk,
 ChunkLink& pPotentialSubject) const
{
  if (canLinkVerbToASubject(pSubVerbChunk, *pPotentialSubject.chunk,
                            fConf.getFlsChecker(), false))
  {
    _modifyRequestOfChunkThatWillHaveASubject(pVerbChunk, pSubVerbChunk, *pPotentialSubject.chunk);
    pSubVerbChunk.children.emplace_back(ChunkLinkType::SUBJECT, pPotentialSubject.chunk);
    return true;
  }
  return false;
}



} // End of namespace linguistics
} // End of namespace onsem
