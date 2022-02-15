#include "synthesizerchunksmergerenglish.hpp"
#include "../synthesizertypes.hpp"

namespace onsem
{

void SynthesizerChunksMergerEnglish::formulateNominalGroup(std::list<WordToSynthesize>& pOut,
                                                           OutNominalGroup& pOutSentence) const
{
  bool puproseAtBeginning = pOutSentence.noun.empty();
  if (puproseAtBeginning)
    pOut.splice(pOut.end(), pOutSentence.purpose.out);
  pOut.splice(pOut.end(), pOutSentence.modifiersBeforeDeterminer.out);
  pOut.splice(pOut.end(), pOutSentence.determiner.out);
  pOut.splice(pOut.end(), pOutSentence.ownerBeforeMainWord.out);
  pOut.splice(pOut.end(), pOutSentence.modifiersBeforeNoun.out);
  pOut.splice(pOut.end(), pOutSentence.subConceptsBeforeNoun.out);
  pOut.splice(pOut.end(), pOutSentence.noun.out);
  pOut.splice(pOut.end(), pOutSentence.subConceptsAfterNoun.out);
  pOut.splice(pOut.end(), pOutSentence.modifiersAfterNoun.out);
  pOut.splice(pOut.end(), pOutSentence.ownerAfterMainWord.out);
  pOut.splice(pOut.end(), pOutSentence.time.out);
  pOut.splice(pOut.end(), pOutSentence.location.out);
  if (!puproseAtBeginning)
    pOut.splice(pOut.end(), pOutSentence.purpose.out);
  pOut.splice(pOut.end(), pOutSentence.subordinate.out);
}


void SynthesizerChunksMergerEnglish::formulateSentence
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  if (!pOutSentence.requests.empty())
  {
    if (pOutSentence.requests.has(SemanticRequestType::ACTION) &&
        !pOutSentence.verb.out.empty() &&
        pOutSentence.verb.out.front().word.lemma == "let" &&
        !pOutSentence.objectAfterVerb.out.empty() &&
        pOutSentence.objectAfterVerb.out.front().word.partOfSpeech == PartOfSpeech::VERB &&
        !pOutSentence.verb.out.front().inflections.empty())
      pOutSentence.verb.out.front().inflections.front().str = "let's";

    if (pOutSentence.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC)
      _formulation_objectQuestionWithSubjetAfterTheVerb(pOut, pOutSentence);
    else
      _formulation_objectQuestionWithSubjetBeforeTheVerb(pOut, pOutSentence);
  }
  else
  {
    _formulation_default(pOut, pOutSentence);
  }
}


void SynthesizerChunksMergerEnglish::_formulation_objectQuestionWithSubjetAfterTheVerb
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  pOut.splice(pOut.end(), pOutSentence.objectBeforeSubject.out);
  pOut.splice(pOut.end(), pOutSentence.negation1.out);
  pOut.splice(pOut.end(), pOutSentence.verbGoal.out);
  pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
  pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
  if (pOutSentence.aux.out.empty())
  {
    pOut.splice(pOut.end(), pOutSentence.verb.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.subject.out);
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.verb2.out);
  }
  else
  {
    if (pOutSentence.requests.has(SemanticRequestType::SUBJECT) &&
        !pOutSentence.aux.out.empty() &&
        pOutSentence.aux.out.front().word.lemma != "do")
    {
      pOut.splice(pOut.end(), pOutSentence.subject.out);
      pOut.splice(pOut.end(), pOutSentence.aux.out);
    }
    else
    {
      if (!pOutSentence.aux.out.empty())
        pOut.splice(pOut.end(), pOutSentence.aux.out, pOutSentence.aux.out.begin());
      pOut.splice(pOut.end(), pOutSentence.subject.out);
      pOut.splice(pOut.end(), pOutSentence.aux.out);
    }
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.verb.out);
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.verb2.out);
  }
  _writEndOfSentence(pOut, pOutSentence);
}


void SynthesizerChunksMergerEnglish::_formulation_objectQuestionWithSubjetBeforeTheVerb
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
  pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
  pOut.splice(pOut.end(), pOutSentence.subject.out);
  pOut.splice(pOut.end(), pOutSentence.negation1.out);
  pOut.splice(pOut.end(), pOutSentence.aux.out);
  pOut.splice(pOut.end(), pOutSentence.negation2.out);
  pOut.splice(pOut.end(), pOutSentence.verb.out);
  pOut.splice(pOut.end(), pOutSentence.verb2.out);
  pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
  _writEndOfSentence(pOut, pOutSentence);
}


void SynthesizerChunksMergerEnglish::_formulation_default
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  if (pOutSentence.aux.out.empty())
  {
    pOut.splice(pOut.end(), pOutSentence.subject.out);
    if (pOutSentence.equVerb)
    {
      pOut.splice(pOut.end(), pOutSentence.verb.out);
      pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
      pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    }
    else
    {
      pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
      pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
      pOut.splice(pOut.end(), pOutSentence.verb.out);
    }
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
  }
  else
  {
    pOut.splice(pOut.end(), pOutSentence.subject.out);
    pOut.splice(pOut.end(), pOutSentence.aux.out);
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.verb.out);
  }
  pOut.splice(pOut.end(), pOutSentence.verb2.out);
  _writEndOfSentence(pOut, pOutSentence);
}


void SynthesizerChunksMergerEnglish::_writEndOfSentence
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  pOut.splice(pOut.end(), pOutSentence.inCaseOf.out);

  const bool needToPopLocationPreposition =
      pOutSentence.verb2.getLastPartOfSpeech() == PartOfSpeech::PREPOSITION &&
      pOutSentence.manner.empty() &&
      pOutSentence.receiverAfterVerb.empty() &&
      pOutSentence.objectAfterVerb.empty() &&
      !pOutSentence.location.out.empty() &&
      pOutSentence.location.getFirstPartOfSpeech() == PartOfSpeech::PREPOSITION;

  pOut.splice(pOut.end(), pOutSentence.receiverAfterVerb.out);
  pOut.splice(pOut.end(), pOutSentence.objectAfterVerb.out);
  pOut.splice(pOut.end(), pOutSentence.manner.out);

  if (needToPopLocationPreposition)
    pOutSentence.location.out.pop_front();

  pOut.splice(pOut.end(), pOutSentence.occurrenceRank.out);
  pOut.splice(pOut.end(), pOutSentence.startingPoint.out);
  _writeDurationLocationAndTimeInGoodOrder(pOut, pOutSentence);
}


} // End of namespace onsem
