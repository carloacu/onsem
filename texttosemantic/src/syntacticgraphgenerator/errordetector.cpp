#include "errordetector.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include "../tool/listiter.hpp"


namespace onsem
{
namespace linguistics
{
void _updateCarryOnValue(CarryOnFrom& pCurrentValue,
                         CarryOnFrom pNewValue)
{
  switch (pNewValue)
  {
  case CarryOnFrom::PARTOFSPEECH_FILTERS:
    pCurrentValue = pNewValue;
    return;
  case CarryOnFrom::SYNTACTIC_TREE:
    if (pCurrentValue != CarryOnFrom::PARTOFSPEECH_FILTERS)
      pCurrentValue = pNewValue;
    return;
  case CarryOnFrom::HERE:
    return;
  }
}


ErrorDetector::ErrorDetector
(const AlgorithmSetForALanguage& pConfiguration)
  : fConfiguration(pConfiguration),
    fSemFrameDict(fConfiguration.getSpecifcLingDb().getSemFrameDict()),
    fLingDico(fConfiguration.getLingDico()),
    fPossNewHeadGram({PartOfSpeech::NOUN, PartOfSpeech::PRONOUN})
{
}

CarryOnFrom _adjNounLists(std::list<ChunkLink>& pSyntTree)
{
  CarryOnFrom res = CarryOnFrom::HERE;
  Chunk* prevChunk = nullptr;
  for (auto itChkLk = pSyntTree.rbegin();
       itChkLk != pSyntTree.rend(); ++itChkLk)
  {
    Chunk& currChk = *itChkLk->chunk;
    if (prevChunk != nullptr &&
        currChk.type == ChunkType::SEPARATOR_CHUNK &&
        ConceptSet::haveAConceptThatBeginWith(currChk.head->inflWords.front().infos.concepts,
                                              "list_"))
    {
      auto nextIt = itChkLk;
      ++nextIt;
      if (nextIt != pSyntTree.rend())
      {
        Token& headTok1 = *prevChunk->head;
        Token& headTok2 = *nextIt->chunk->head;
        std::list<InflectedWord>& inflWords1 = headTok1.inflWords;
        std::list<InflectedWord>& inflWords2 = headTok2.inflWords;
        if (inflWords1.front().word.partOfSpeech != inflWords2.front().word.partOfSpeech)
        {
          if (tokenIsMoreProbablyAType(headTok1, PartOfSpeech::NOUN) &&
              hasAPartOfSpeech(inflWords2, PartOfSpeech::NOUN))
          {
            if (delAllExept(inflWords2, PartOfSpeech::NOUN))
              res = CarryOnFrom::PARTOFSPEECH_FILTERS;
          }
          else if (tokenIsMoreProbablyAType(headTok2, PartOfSpeech::NOUN) &&
                   hasAPartOfSpeech(inflWords1, PartOfSpeech::NOUN))
          {
            if (delAllExept(inflWords1, PartOfSpeech::NOUN))
              res = CarryOnFrom::PARTOFSPEECH_FILTERS;
          }
        }
      }
    }
    prevChunk = &currChk;
  }
  return res;
}


CarryOnFrom ErrorDetector::falseGramPossibilitiesRemoved
(std::list<ChunkLink>& pSyntTree) const
{
  CarryOnFrom res = CarryOnFrom::HERE;
  ChunkType previousChunkType = ChunkType::SEPARATOR_CHUNK;
  bool firstChunk = true;
  for (ChunkLinkIter chkLkIter(pSyntTree, pSyntTree.begin());
       !chkLkIter.atEnd(); ++chkLkIter)
  {
    ChunkLinkIter nextIt = chkLkIter;
    ++nextIt;
    auto currChunk = chkLkIter.getIt()->chunk;
    ChunkType currChunkType = currChunk->type;

    xPutRepetitionChildToTheFatherNode(*currChunk);
    _updateCarryOnValue(res,
                        xSolveConjunctionUnlinked(chkLkIter.getIt()));

    if (nextIt.atEnd())
    {
      _updateCarryOnValue(res,
                          xRemoveSubordinatingConjonctionUnliked(*chkLkIter.getIt(), nullptr));
    }
    else
    {
      _updateCarryOnValue(res,
                          xRemoveSubordinatingConjonctionUnliked(*chkLkIter.getIt(), &*nextIt.getIt()));
    }

    _updateCarryOnValue(res,
                        xCheckThatNominalGroupHaveAValidHead(chkLkIter.getIt()));
    if (currChunkType == ChunkType::PREPOSITIONAL_CHUNK)
      _updateCarryOnValue(res,
                          xCorrectFalsePrepositionalChunk(chkLkIter.getIt()));
    if (res == CarryOnFrom::HERE)
    {
      _updateCarryOnValue(res, xRemoveInvalidPronouns(*chkLkIter.getIt()->chunk));
      _updateCarryOnValue(res,
                          xSolveBadVerbChunks(chkLkIter, chkLkIter->type, previousChunkType, firstChunk));
      if (chkLkIter.atEnd())
        return res;
    }
    previousChunkType = currChunkType;
    firstChunk = false;
  }

  _updateCarryOnValue(res, _adjNounLists(pSyntTree));
  return res;
}


bool ErrorDetector::tryToConvertNounToImperativeVerbs
(std::list<ChunkLink>& pSyntTree) const
{
  bool rem = false;
  for (auto itChunkLink = pSyntTree.begin(); itChunkLink != pSyntTree.end(); ++itChunkLink)
  {
    Chunk& chunk = *itChunkLink->chunk;
    if (chunk.type == ChunkType::SEPARATOR_CHUNK)
    {
      const InflectedWord& headInflWorld = chunk.head->inflWords.front();
      if (headInflWorld.infos.hasContextualInfo(WordContextualInfos::CONDITION))
      {
        auto itNextChunk = itChunkLink;
        ++itNextChunk;
        if (itNextChunk != pSyntTree.end() &&
            itNextChunk->chunk->type == ChunkType::VERB_CHUNK &&
            itNextChunk->chunk->requests.empty())
        {
          auto itNextNextChunk = itNextChunk;
          ++itNextNextChunk;
          if (itNextNextChunk == pSyntTree.end() ||
              itNextNextChunk->chunk->type == ChunkType::SEPARATOR_CHUNK)
          {
            Chunk& nextChunk = *itNextChunk->chunk;
            auto itObject = getChunkLink(nextChunk, ChunkLinkType::DIRECTOBJECT);
            if (itObject != nextChunk.children.end())
              rem |= xTryToConvertNounToImperativeVerbsForAChunk(*itObject->chunk);
          }
        }
      }
      // don't try to put infinitive verb after a relative time word
      else if (fSemFrameDict.doesIntroductionWordHasChunkLinkType(headInflWorld.word, ChunkLinkType::TIME))
      {
        ++itChunkLink;
        if (itChunkLink == pSyntTree.end())
          break;
      }
    }
    else
    {
      rem |= xTryToConvertNounToImperativeVerbsForAChunk(chunk);
    }
  }
  return rem;
}


bool ErrorDetector::xTryToConvertNounToImperativeVerbsForAChunk(Chunk& pChunk) const
{
  static const std::vector<PartOfSpeech> nounPartOfSpeeches =
  {PartOfSpeech::NOUN, PartOfSpeech::UNKNOWN};
  bool rem = false;
  auto& headInflWords = pChunk.head->inflWords;
  if (pChunk.type == ChunkType::NOMINAL_CHUNK &&
      headInflWords.front().word.partOfSpeech == PartOfSpeech::NOUN &&
      !haveChild(pChunk, ChunkLinkType::SIMPLE))
  {
    bool prevIsEmptyOrNominal = true;
    for (auto it = pChunk.tokRange.getItBegin();
         it != pChunk.tokRange.getItEnd(); it = getNextToken(it, pChunk.tokRange.getItEnd()))
    {
      auto& itInflWord = it->inflWords;
      bool isNominal = partOfSpeech_isNominal(itInflWord.front().word.partOfSpeech);
      if (prevIsEmptyOrNominal && isNominal)
      {
        std::list<InflectedWord>::iterator verbIGram =
            getInflWordWithASpecificPartOfSpeech(itInflWord, PartOfSpeech::VERB);
        if (verbIGram != itInflWord.end() &&
            fConfiguration.getFlsChecker().verbCanBeAtImperative(*verbIGram))
        {
          auto itNext = it;
          itNext = getNextWord(itNext, pChunk.tokRange.getItEnd());
          bool isNextCompatible = true;
          if (itNext != pChunk.tokRange.getItEnd() &&
              itNext->inflWords.front().infos.hasContextualInfo(WordContextualInfos::EN_TIMEWORD))
            isNextCompatible = false;

          if (isNextCompatible)
          {
            verbIGram->moveInflections(VerbalInflections::get_inflections_imperative());
            rem |= delAllBefore(itInflWord, verbIGram);
          }
        }
      }
      prevIsEmptyOrNominal = isNominal;
    }
  }
  return rem;
}



void ErrorDetector::frFixOfVerbalChunks
(std::list<ChunkLink>& pSyntTree) const
{
  // if we have a pronominal verb that the pronoun has not the same gender than the subject, then
  // we go back to the simple verb meaning
  for (auto& currChunkLink : pSyntTree)
  {
    auto& currChunk = *currChunkLink.chunk;
    if (chunkTypeIsVerbal(currChunk.type))
    {
      if (getReflexiveChunk(currChunk) == nullptr)
      {
        Token& verbTok = *currChunk.head;

        // if he is linked to a pronoun complement, it means it's a pronominal verb
        // so it's a false positive, and reput the simple word meaning
        bool firstLoop = true;
        for (const auto& currLinkedTok : verbTok.linkedTokens)
        {
          if (!firstLoop && // the elt is the root himself
              currLinkedTok->inflWords.front().word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT)
          {
            xBreakLinkedTokens(verbTok);
            break;
          }
          firstLoop = false;
        }
      }

      // add negation if necessary
      if (currChunk.positive &&
          currChunk.tokRange.getItBegin()->inflWords.front().infos.hasContextualInfo(WordContextualInfos::NEGATION))
      {
        currChunk.positive = false;

        auto nextTokenIt = getNextToken(currChunk.head, currChunk.tokRange.getItEnd());
        if (nextTokenIt != currChunk.tokRange.getItEnd() &&
            nextTokenIt->inflWords.front().word.lemma == "que")
        {
          currChunk.positive = true;
        }
        else
        {
          auto* doChunkLinkPtr = getDOChunkLink(currChunk);
          if (doChunkLinkPtr != nullptr)
          {
            if (!doChunkLinkPtr->tokRange.isEmpty() &&
                doChunkLinkPtr->tokRange.getItBegin()->inflWords.front().word.lemma == "que")
              currChunk.positive = true;
            else if (ConceptSet::haveAnyOfConcepts(doChunkLinkPtr->chunk->tokRange.getItBegin()->inflWords.front().infos.concepts, {"quantity_nothing", "number_0"}))
              currChunk.positive = true;
          }
        }

        if (!currChunk.positive)
        {
          auto* subjectChunkLinkPtr = getSubjectChunk(currChunk);
          if (subjectChunkLinkPtr != nullptr &&
              ConceptSet::haveAnyOfConcepts(subjectChunkLinkPtr->tokRange.getItBegin()->inflWords.front().infos.concepts, {"quantity_nothing", "number_0"}))
            currChunk.positive = true;
        }
      }
    }

    if (!currChunk.children.empty())
    {
      frFixOfVerbalChunks(currChunk.children);
    }
  }
}



void ErrorDetector::addYesOrNoRequestForVerbsBeforeInterrogationPunctuation
(std::list<ChunkLink>& pSyntTree)
{
  std::list<ChunkLink>::iterator itLastChunk = pSyntTree.end();
  for (std::list<ChunkLink>::iterator
       it = pSyntTree.begin(); it != pSyntTree.end(); ++it)
  {
    auto& currChunk = *it->chunk;
    if (currChunk.type == ChunkType::VERB_CHUNK)
    {
      if (haveASubject(currChunk))
        itLastChunk = it;
      else
        itLastChunk = pSyntTree.end();
    }
    else if (currChunk.type == ChunkType::INFINITVE_VERB_CHUNK)
    {
      itLastChunk = pSyntTree.end();
    }
    else if (chunkTypeIsAList(currChunk.type))
    {
      itLastChunk = it;
    }
    else if (currChunk.type == ChunkType::SEPARATOR_CHUNK)
    {
      if (currChunk.head->str.find_first_of('?') != std::string::npos &&
          itLastChunk != pSyntTree.end() &&
          itLastChunk->chunk->requests.empty() &&
          !haveAQuestionInDirectObject(*itLastChunk->chunk))
      {
        setChunkAtInterrogativeForm(*itLastChunk->chunk);
        itLastChunk = pSyntTree.end();
      }
    }
  }
}



CarryOnFrom ErrorDetector::xSolveConjunctionUnlinked
(std::list<ChunkLink>::iterator pItChLk) const
{
  SemanticLanguageEnum language = fConfiguration.getLanguageType();
  if (language == SemanticLanguageEnum::ENGLISH &&
      pItChLk->chunk->type == ChunkType::SEPARATOR_CHUNK)
  {
    Token& headTok = *pItChLk->chunk->head;
    InflectedWord& headIGram =  headTok.inflWords.front();

    if (headIGram.word.partOfSpeech == PartOfSpeech::CONJUNCTIVE &&
        headIGram.word.language == SemanticLanguageEnum::ENGLISH)
    {
      // if we encounter a word "then" unlinked and
      //    it has not been considered as a succession of different steps
      // => we restart the syntactic tree construction, with the word "then"
      //    considered as a succession of different steps
      if (headIGram.word.lemma == "then")
      {
        if (headIGram.infos.concepts.count("list_then") == 0 &&
            !headTok.thisTokenAlreadyCausedARestart)
        {
          headTok.thisTokenAlreadyCausedARestart = true;
          headIGram.infos.concepts.emplace("list_then", 4);
          return CarryOnFrom::SYNTACTIC_TREE;
        }
      }
      // remove for as conjunction
      else if (headIGram.word.lemma == "for" &&
               headTok.inflWords.size() > 1 &&
               delAPartOfSpeech(headTok.inflWords, PartOfSpeech::CONJUNCTIVE))
      {
        return CarryOnFrom::PARTOFSPEECH_FILTERS;
      }
    }
  }
  return CarryOnFrom::HERE;
}



/**
 * ex: "Ils viennent de Paris, comme moi."
 *              venir            comme       moi
 *             /     \
 *           ils   de Paris
 * Because the more probable grammatical type of "comme" is subordinating conjuntion.
 *
 * This Algo remove the subordinating conjuntion grammatical type of "comme".
 * (if this word have other grammatical types)
 * To have:
 *              venir
 *             /     \
 *           ils   de Paris
 *                     \
 *                   comme moi
 */
CarryOnFrom ErrorDetector::xRemoveSubordinatingConjonctionUnliked
(ChunkLink& pItChLk,
 ChunkLink* pNextItChLk) const
{
  if (pItChLk.chunk->type == ChunkType::SEPARATOR_CHUNK &&
      pItChLk.chunk->head->inflWords.size() > 1)
  {
    auto& inflWordsHead = pItChLk.chunk->head->inflWords;
    auto itFirstInflWordHead = inflWordsHead.begin();
    auto headGram = itFirstInflWordHead->word.partOfSpeech;
    if (headGram == PartOfSpeech::SUBORDINATING_CONJONCTION)
    {
      auto itInflWordHead = itFirstInflWordHead;
      ++itInflWordHead;
      while (itInflWordHead != inflWordsHead.end())
      {
        if (itInflWordHead->word.partOfSpeech == PartOfSpeech::PRONOUN ||
            fLingDico.statDb.wordToQuestionWord(itInflWordHead->word, false, true) != nullptr)
        {
          delAPartOfSpeech(inflWordsHead, itFirstInflWordHead);
          return CarryOnFrom::PARTOFSPEECH_FILTERS;
        }
        ++itInflWordHead;
      }
      return CarryOnFrom::HERE;
    }
    if (headGram == PartOfSpeech::CONJUNCTIVE &&
        pNextItChLk != nullptr &&
        (pNextItChLk->chunk->type == ChunkType::NOMINAL_CHUNK ||
         chunkTypeIsAList(pNextItChLk->chunk->type)) &&
        (++inflWordsHead.begin())->word.partOfSpeech == PartOfSpeech::PREPOSITION)
    {
      delAPartOfSpeech(inflWordsHead, itFirstInflWordHead);
      return CarryOnFrom::PARTOFSPEECH_FILTERS;
    }
  }
  return CarryOnFrom::HERE;
}



/**
 * ex: "J'ai du mal"
 *         avoir
 *         /    \
 *        je    du mal
 *             (head: du)
 * Because "mal" grammatical type is adverb
 *
 * This algo see that the head of a grammatical type is not correct
 * (in ths ex a determiner), and try to find if another word of the Chunk
 * can have a correct grammatical type to be the head of the chunk.
 * (here "mal" can be a noun)
 * To have:
 *         avoir
 *         /    \
 *        je    du mal
 *             (head: mal)
 */
CarryOnFrom ErrorDetector::xCheckThatNominalGroupHaveAValidHead
(std::list<ChunkLink>::iterator pItChLk) const
{
  CarryOnFrom res = CarryOnFrom::HERE;
  Chunk& chunk = *pItChLk->chunk;
  if (chunk.type == ChunkType::NOMINAL_CHUNK || chunk.type == ChunkType::PREPOSITIONAL_CHUNK)
  {
    auto& headInflWords = chunk.head->inflWords;
    auto itFrontHeadInflWords = headInflWords.begin();
    PartOfSpeech headPartOfSpeech = itFrontHeadInflWords->word.partOfSpeech;
    if (headPartOfSpeech == PartOfSpeech::ADVERB || headPartOfSpeech == PartOfSpeech::DETERMINER ||
        headPartOfSpeech == PartOfSpeech::PREPOSITION)
    {
      for (TokIt itTok = chunk.tokRange.getItBegin();
           itTok != chunk.tokRange.getItEnd();
           itTok = getNextToken(itTok, chunk.tokRange.getItEnd()))
      {
         if (itTok != chunk.head)
         {
           auto itIgram = getInflWordWithAnyPartOfSeechOf(*itTok, fPossNewHeadGram);
           if (itIgram != itTok->inflWords.end())
           {
             if (delAllBefore(itTok->inflWords, itIgram))
               res = CarryOnFrom::PARTOFSPEECH_FILTERS;
             break;
           }
         }
      }
    }
  }
  for (auto itChild = chunk.children.begin(); itChild != chunk.children.end(); ++itChild)
  {
    _updateCarryOnValue(res,
                        xCheckThatNominalGroupHaveAValidHead(itChild));
  }
  return res;
}


CarryOnFrom ErrorDetector::xRemoveInvalidPronouns(Chunk& pChunk) const
{
  auto res = CarryOnFrom::HERE;
  if (pChunk.type == ChunkType::NOMINAL_CHUNK || pChunk.type == ChunkType::PREPOSITIONAL_CHUNK)
  {
    for (TokIt currIt = getTheNextestToken(pChunk.tokRange.getItBegin(), pChunk.tokRange.getItEnd(), SkipPartOfWord::YES);
         currIt != pChunk.tokRange.getItEnd();
         currIt = getNextToken(currIt, pChunk.tokRange.getItEnd(), SkipPartOfWord::YES))
    {
      auto& inflWords = currIt->inflWords;
      auto itFrontHeadInflWords = inflWords.begin();
      PartOfSpeech headPartOfSpeech = itFrontHeadInflWords->word.partOfSpeech;
      if (headPartOfSpeech == PartOfSpeech::PRONOUN &&
          inflWords.size() > 1 &&
          itFrontHeadInflWords->infos.hasContextualInfo(WordContextualInfos::CANNOTBEACOREFRENCE))
      {
        delAPartOfSpeech(inflWords, itFrontHeadInflWords);
        res = CarryOnFrom::PARTOFSPEECH_FILTERS;
        break;
      }
    }
  }
  for (auto& currChild : pChunk.children)
  {
    if (currChild.type != ChunkLinkType::QUESTIONWORD)
    {
      auto subRes = xRemoveInvalidPronouns(*currChild.chunk);
      if (subRes != CarryOnFrom::HERE)
        return subRes;
    }
  }
  return res;
}


CarryOnFrom ErrorDetector::xCorrectFalsePrepositionalChunk
(std::list<ChunkLink>::iterator pItChLk) const
{
  Chunk& chunk = *pItChLk->chunk;
  Token& firstToken = *chunk.tokRange.getItBegin();
  auto firstTokenInflWord = firstToken.inflWords.begin();
  PartOfSpeech firstTokenPartOfSpeech = firstTokenInflWord->word.partOfSpeech;
  if (firstTokenPartOfSpeech == PartOfSpeech::PREPOSITION)
  {
    if (hasAPartOfSpeech(firstToken.inflWords, PartOfSpeech::PARTITIVE))
    {
      delAPartOfSpeech(firstToken.inflWords, firstTokenInflWord);
      return CarryOnFrom::PARTOFSPEECH_FILTERS;
    }
    else
    {
      auto itSubClause = getChunkLink(chunk, ChunkLinkType::SUBORDINATE_CLAUSE);
      if (itSubClause != chunk.children.end())
      {
        for (auto it = itSubClause->tokRange.getItBegin();
             it != itSubClause->tokRange.getItEnd(); ++it)
        {
          if (it->inflWords.size() > 1)
          {
            auto itFirstInflsWord = it->inflWords.begin();
            if (itFirstInflsWord->word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION)
            {
              delAPartOfSpeech(it->inflWords, itFirstInflsWord);
              return CarryOnFrom::PARTOFSPEECH_FILTERS;
            }
          }
        }
      }
    }
  }
  return CarryOnFrom::HERE;
}


void ErrorDetector::xPutRepetitionChildToTheFatherNode(Chunk& pChunk) const
{
  for (auto& currChild : pChunk.children)
  {
    Chunk& subChildChunk = *currChild.chunk;
    xPutRepetitionChildToTheFatherNode(subChildChunk);

    if (currChild.type == ChunkLinkType::SUBORDINATE_CLAUSE)
    {
      auto repetitionChild = getChunkLink(subChildChunk,
                                          ChunkLinkType::REPETITION);
      if (repetitionChild != subChildChunk.children.end())
        pChunk.children.splice(pChunk.children.end(),
                               subChildChunk.children, repetitionChild);
    }
  }
}


CarryOnFrom ErrorDetector::xSolveBadVerbChunks
(ChunkLinkIter& pChkLkIter,
 ChunkLinkType pParentChkLk,
 ChunkType pPreviousChunkType,
 bool pFirstChunk) const
{
  CarryOnFrom res = CarryOnFrom::HERE;
  // remove participle without auxiliary
  bool isAList = chunkTypeIsAList(pChkLkIter->chunk->type);
  if (isAList ||
      fConfiguration.getLanguageType() == SemanticLanguageEnum::FRENCH) // TODO: remove french filter
  {
    for (ChunkLinkIter it(pChkLkIter->chunk->children, pChkLkIter->chunk->children.begin());
         !it.atEnd(); ++it)
    {
      if (isAList ||
          it->tokRange.isEmpty() ||
          fLingDico.statDb.wordToSubordinateRequest(it->tokRange.getItBegin()->inflWords.front().word) != SemanticRequestType::OBJECT)
      {
        ChunkLinkType subParentChkLk =
            it->type == ChunkLinkType::SIMPLE ? pParentChkLk : it->type;
        res = xSolveBadVerbChunks(it, subParentChkLk, ChunkType::SEPARATOR_CHUNK, false);
        if (res != CarryOnFrom::HERE)
          break;
      }
    }
  }

  if (pChkLkIter->chunk->type == ChunkType::VERB_CHUNK)
  {
    Chunk& verbChunk = *pChkLkIter->chunk;
    if (verbChunk.requests.has(SemanticRequestType::ACTION) &&
        (pPreviousChunkType != ChunkType::SEPARATOR_CHUNK &&
         pPreviousChunkType != ChunkType::INTERJECTION_CHUNK))
      verbChunk.requests.clear();

    if (pParentChkLk != ChunkLinkType::SUBJECT_OF &&
        xSolveVerbThatShouldHaveAnAuxiliary(verbChunk))
    {
      res = CarryOnFrom::PARTOFSPEECH_FILTERS;
    }
    else if (haveASubject(verbChunk) ||
             pParentChkLk == ChunkLinkType::SUBJECT_OF)
    {
      if (xSolveVerbThatHaveASubjectThatBeginsWithAPrep(verbChunk))
      {
        res = CarryOnFrom::PARTOFSPEECH_FILTERS;
      }
    }
    else if (xTryToCorrectVerbsWithoutSubject(pChkLkIter, pFirstChunk))
    {
      res = CarryOnFrom::PARTOFSPEECH_FILTERS;
    }
  }

  return res;
}


/**
 * ex: "Il est un homme engagé qui a compté"
 *          être              engagé
 *         /    \                \
 *       il    un homme          compté
 *                                  \
 *                                   a
 * Because "engagé" is more likelyto be a verb.
 *
 * This algo will detect "engagé" is not linked to an auxiliary,
 * but it should. So it try to remove the verb grammatical type of "engagé".
 * To have:
 *          être
 *         /    \
 *       il    un homme engagé
 *                 \
 *                compté
 *                   \
 *                    a
 */
bool ErrorDetector::xSolveVerbThatShouldHaveAnAuxiliary
(Chunk& pVerbChunk) const
{
  return pVerbChunk.head->inflWords.size() > 1 &&
      fConfiguration.getFlsChecker().verbHasToHaveAnAuxiliary(pVerbChunk.head->inflWords.front()) &&
      !haveAnAuxiliary(pVerbChunk) &&
      delAPartOfSpeech(pVerbChunk.head->inflWords, PartOfSpeech::VERB);
}


bool ErrorDetector::xSolveVerbThatHaveASubjectThatBeginsWithAPrep
(Chunk& pVerbChunk) const
{
  if (pVerbChunk.head->inflWords.size() > 1)
  {
    ChunkLink* subjectChLk = getSubjectChunkLink(pVerbChunk);
    if (subjectChLk != nullptr &&
        subjectChLk->chunk->head->inflWords.front().word.partOfSpeech == PartOfSpeech::PROPER_NOUN &&
        !fConfiguration.getFlsChecker().verbCanBeSingular(pVerbChunk.head->inflWords.front()) &&
        delAPartOfSpeech(pVerbChunk.head->inflWords, PartOfSpeech::VERB))
    {
      return true;
    }
  }
  return false;
}


bool ErrorDetector::xTryToCorrectVerbsWithoutSubject
(ChunkLinkIter& pChkLkIter,
 bool pFirstChunk) const
{
  Chunk& verbChunk = *pChkLkIter->chunk;
  auto language = fConfiguration.getLanguageType();
  // TODO: hack solve common error "dit" instead of "dis"
  // This hack will be removed when we will integrate the dictionnary of homophones words
  if (verbChunk.requests.empty() &&
      language == SemanticLanguageEnum::FRENCH &&
      verbChunk.head->inflWords.front().word.lemma == "dire")
    verbChunk.requests.set(SemanticRequestType::ACTION);

  if ((verbChunk.requests.empty() ||
       (verbChunk.requests.has(SemanticRequestType::OBJECT) && language == SemanticLanguageEnum::FRENCH)) &&
      verbChunk.head->inflWords.size() > 1)
  {
    auto itVerbBefore = pChkLkIter;
    --itVerbBefore;
    while (!itVerbBefore.atEnd())
    {
      if (itVerbBefore->chunk->type == ChunkType::VERB_CHUNK)
      {
        Chunk& verbChunkBefore = *itVerbBefore->chunk;
        if (haveASubject(verbChunkBefore))
        {
          auto& verbInflWord = verbChunkBefore.head->inflWords.front();
          if (InflectionsChecker::verbIsAtPastParticiple(verbInflWord) &&
              InflectionsChecker::verbRemoveAllInflectionsThatAreNotAtPastParticiple(verbInflWord))
            return true;
        }
        break;
      }
      if (itVerbBefore->chunk->type != ChunkType::SEPARATOR_CHUNK ||
          itVerbBefore->chunk->getHeadPartOfSpeech() != PartOfSpeech::LINKBETWEENWORDS)
        break;
      --itVerbBefore;
    }

    if (!InflectionsChecker::verbCanHaveNoSubject(verbChunk.head->inflWords.front()))
    {
      // remove this verb if another verb can have no subject
      auto it = verbChunk.head->inflWords.begin();
      assert(it != verbChunk.head->inflWords.end()); // because "verbChunk.head->inflWords.size() > 1" on previous condition
      ++it;
      while (it != verbChunk.head->inflWords.end())
      {
        if (partOfSpeech_isVerbal(it->word.partOfSpeech) &&
            InflectionsChecker::verbCanHaveNoSubject(*it))
          return delAPartOfSpeech(verbChunk.head->inflWords, PartOfSpeech::VERB);
        ++it;
      }
    }

    // if it's the first token maybe the subject was in a previous text
    bool canHaveASubjectBefore =
        pFirstChunk && !haveAChildBefore(verbChunk) &&
        !fConfiguration.getFlsChecker().verbIsOnlyAtPresentOrPastParticiple(verbChunk.head->inflWords.front());
    if (linguistics::hasAPartOfSpeech(verbChunk.head->inflWords, PartOfSpeech::CONJUNCTIVE) ||
        !canHaveASubjectBefore)
      return delAPartOfSpeech(verbChunk.head->inflWords, PartOfSpeech::VERB);
  }
  return false;
}




void ErrorDetector::xBreakLinkedTokens
(Token& pToken) const
{
  for (std::list<TokIt>::iterator itLinkedTok = pToken.linkedTokens.begin();
       itLinkedTok != pToken.linkedTokens.end(); ++itLinkedTok)
  {
    if (itLinkedTok != pToken.linkedTokens.begin()) // the elt is the root himself
    {
      (*itLinkedTok)->linkedTokens.clear();
    }
  }
  pToken.linkedTokens.clear();
  InflectedWord& tokenIgram = pToken.inflWords.front();
  fConfiguration.getLingDico().statDb.putRootMeaning(tokenIgram);
}


} // End of namespace linguistics
} // End of namespace onsem
