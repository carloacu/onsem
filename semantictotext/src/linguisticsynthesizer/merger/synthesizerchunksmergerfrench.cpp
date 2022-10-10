#include "synthesizerchunksmergerfrench.hpp"
#include "../tool/synthesizeradder.hpp"
#include "../tool/synthesizerconditions.hpp"

namespace onsem
{

namespace
{


bool _shouldWriteSubjectAfterTheVerb
(const std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence)
{
  bool requestisYesOrNoOrCause = pOutSentence.requests.has(SemanticRequestType::YESORNO) || pOutSentence.requests.has(SemanticRequestType::CAUSE);
  if (pOutSentence.requests.empty() ||
      pOutSentence.requests.has(SemanticRequestType::SUBJECT) ||
      pOutSentence.requests.has(SemanticRequestType::ACTION) ||
      (pOutSentence.verb.empty() && pOutSentence.verbGoal.empty()) ||
      (!requestisYesOrNoOrCause && !pOutSentence.negation1.empty()))
    return false;

  if (pOutSentence.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC)
  {
    if (pOutSentence.equVerb ||
        (!requestisYesOrNoOrCause && !pOutSentence.aux.out.empty() && pOutSentence.aux.out.front().word.lemma == "être"))
    {
      if (pOutSentence.subject.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT)
      {
        if (pOut.empty() || pOut.back().word.lemma != "qui" ||
            pOutSentence.subject.out.empty() ||
            pOutSentence.subject.out.front().word.lemma != "ce")
          return true;
      }
      else if (pOutSentence.objectAfterVerb.empty() && pOutSentence.verbGoal.empty() &&
               !pOutSentence.subject.out.empty() &&
                pOutSentence.subject.out.front().word.lemma != "ça")
        return true;
    }
    else
    {
      if (!pOut.empty() && pOut.back().word.lemma == "qui" &&
          (pOutSentence.subject.out.empty() ||
           pOutSentence.subject.out.front().word.lemma != "je"))
        return pOutSentence.requests.has(SemanticRequestType::SUBJECT);

      if ((pOutSentence.subject.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT ||
           pOutSentence.requests.has(SemanticRequestType::QUANTITY) ||
           pOutSentence.requests.has(SemanticRequestType::OBJECT)) &&
          !pOutSentence.subject.out.empty() &&
          (pOutSentence.subject.out.front().word.lemma != "je" || !pOutSentence.verbGoal.empty()))
        return true;
    }
  }
  else if (pOutSentence.equVerb &&
           (pOutSentence.requests.has(SemanticRequestType::OBJECT) ||
            pOutSentence.requests.has(SemanticRequestType::LOCATION)) &&
           !partOfSpeech_isPronominal(pOutSentence.subject.partOfSpeech))
  {
    return true;
  }

  return false;
}


void _tryToAddEstCeQue(std::list<WordToSynthesize>& pOut,
                       OutSentence& pOutSentence)
{
  if (pOutSentence.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC &&
      !pOutSentence.requests.empty() &&
      !pOutSentence.requests.has(SemanticRequestType::ACTION) &&
      !pOutSentence.requests.has(SemanticRequestType::CAUSE) &&
      !pOutSentence.requests.has(SemanticRequestType::LOCATION) &&
      !pOutSentence.requests.has(SemanticRequestType::MANNER) &&
      !pOutSentence.requests.has(SemanticRequestType::QUANTITY) &&
      !pOutSentence.requests.has(SemanticRequestType::WAY) &&
      (!pOutSentence.requests.has(SemanticRequestType::OBJECT) ||
       pOut.empty() || (pOut.back().word.lemma != "qui" && pOut.back().word.lemma != "à quoi")) &&
      (!pOutSentence.requests.has(SemanticRequestType::SUBJECT) ||
       pOut.empty() || (pOut.back().word.lemma != "qui" && pOut.back().word.lemma != "quel")))
  {
    if (pOutSentence.requests.has(SemanticRequestType::ABOUT))
    {
      synthTool::strWithApostropheToOut(pOut, PartOfSpeech::DETERMINER,
                                        "d'", "de", SemanticLanguageEnum::FRENCH);
    }
    else if (pOutSentence.subject.out.empty())
    {
      if (pOutSentence.verbTense != SemanticVerbTense::UNKNOWN)
        synthTool::strToOut(pOut, PartOfSpeech::ADVERB,
                            "est-ce qui", SemanticLanguageEnum::FRENCH);
    }
    else
    {
      synthTool::strWithApostropheToOut(pOut, PartOfSpeech::ADVERB,
                                        "est-ce qu'", "est-ce que", SemanticLanguageEnum::FRENCH);
    }
  }
}


void _tryToAddHyphen(std::list<WordToSynthesize>& pOut,
                     OutSentence& pOutSentence)
{
  if (!pOutSentence.subject.out.empty() &&
      partOfSpeech_isPronominal(pOutSentence.subject.partOfSpeech) &&
      !pOut.empty())
  {
    WordToSynthesize& wordToSynth = pOut.back();
    std::string hyphenStr = "-";
    if (pOutSentence.subject.relativePerson == RelativePerson::THIRD_SING &&
        !wordToSynth.inflections.empty())
    {
      const std::string& lastWordInflStr = wordToSynth.inflections.begin()->str;
      if (!lastWordInflStr.empty() && lastWordInflStr[lastWordInflStr.size() - 1] != 't')
        hyphenStr = "-t-";
    }

    for (auto& currInfl : wordToSynth.inflections)
    {
      currInfl.str += hyphenStr;
      currInfl.ifCanHaveSpaceAfter = false;
    }

    // subject cannot be in the contracted form if it is after the verb
    pOutSentence.subject.out.front().keepOnlyLastInflection();
  }
}

}


void SynthesizerChunksMergerFrench::formulateNominalGroup(std::list<WordToSynthesize>& pOut,
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


void SynthesizerChunksMergerFrench::formulateSentence
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  if (_shouldWriteSubjectAfterTheVerb(pOut, pOutSentence))
    _formulation_objectQuestionWithSubjetAfterTheVerb(pOut, pOutSentence);
  else
    _formulation_default(pOut, pOutSentence);
}


void SynthesizerChunksMergerFrench::_formulation_objectQuestionWithSubjetAfterTheVerb
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  pOut.splice(pOut.end(), pOutSentence.negation1.out);
  if (!pOutSentence.verbGoal.out.empty())
  {
    pOut.splice(pOut.end(), pOutSentence.objectBeforeSubject.out);
    pOut.splice(pOut.end(), pOutSentence.verbGoal.out);
    _tryToAddHyphen(pOut, pOutSentence);
    pOut.splice(pOut.end(), pOutSentence.subject.out);
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.verb.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.mannerBetweenAuxAndVerb.out);
  }
  else if (pOutSentence.aux.out.empty())
  {
    pOut.splice(pOut.end(), pOutSentence.objectBeforeSubject.out);
    pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.verb.out);
    _tryToAddHyphen(pOut, pOutSentence);
    pOut.splice(pOut.end(), pOutSentence.subject.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.mannerBetweenAuxAndVerb.out);
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
  }
  else
  {
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeSubject.out);
    pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.aux.out);
    if (partOfSpeech_isNominal(pOutSentence.subject.partOfSpeech))
    {
      pOut.splice(pOut.end(), pOutSentence.verb.out);
      pOut.splice(pOut.end(), pOutSentence.subject.out);
    }
    else
    {
      _tryToAddHyphen(pOut, pOutSentence);
      pOut.splice(pOut.end(), pOutSentence.subject.out);
      pOut.splice(pOut.end(), pOutSentence.verb.out);
    }
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.mannerBetweenAuxAndVerb.out);
  }
  _writEndOfSentence(pOut, pOutSentence);
}


void SynthesizerChunksMergerFrench::_formulation_default
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  pOut.splice(pOut.end(), pOutSentence.objectBeforeSubject.out);
  _tryToAddEstCeQue(pOut, pOutSentence);
  pOut.splice(pOut.end(), pOutSentence.subject.out);
  pOut.splice(pOut.end(), pOutSentence.negation1.out);

  if (!pOutSentence.verbGoal.out.empty())
  {
    if (!pOutSentence.verbGoal.out.empty())
      pOut.splice(pOut.end(), pOutSentence.verbGoal.out, pOutSentence.verbGoal.out.begin());
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.verbGoal.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.mannerBetweenAuxAndVerb.out);
    pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.aux.out);
    pOut.splice(pOut.end(), pOutSentence.verb.out);
  }
  else if (pOutSentence.aux.out.empty())
  {
    pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.verb.out);
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.mannerBetweenAuxAndVerb.out);
  }
  else
  {
    pOut.splice(pOut.end(), pOutSentence.receiverBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectBeforeVerb.out);
    pOut.splice(pOut.end(), pOutSentence.aux.out);
    pOut.splice(pOut.end(), pOutSentence.negation2.out);
    pOut.splice(pOut.end(), pOutSentence.equalityAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.mannerBetweenAuxAndVerb.out);
    pOut.splice(pOut.end(), pOutSentence.verb.out);
  }
  _writEndOfSentence(pOut, pOutSentence);
}


void SynthesizerChunksMergerFrench::_writEndOfSentence
(std::list<WordToSynthesize>& pOut,
 OutSentence& pOutSentence) const
{
  pOut.splice(pOut.end(), pOutSentence.verb2.out);
  pOut.splice(pOut.end(), pOutSentence.manner.out);
  if (pOutSentence.objectAfterVerb.partOfSpeech == PartOfSpeech::VERB ||
      pOutSentence.receiverAfterVerb.partOfSpeech == PartOfSpeech::PRONOUN)
  {
    pOut.splice(pOut.end(), pOutSentence.receiverAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.objectAfterVerb.out);
  }
  else
  {
    pOut.splice(pOut.end(), pOutSentence.objectAfterVerb.out);
    pOut.splice(pOut.end(), pOutSentence.receiverAfterVerb.out);
  }
  pOut.splice(pOut.end(), pOutSentence.inCaseOf.out);
  pOut.splice(pOut.end(), pOutSentence.occurrenceRank.out);
  pOut.splice(pOut.end(), pOutSentence.startingPoint.out);
  _writeDurationLocationAndTimeInGoodOrder(pOut, pOutSentence);
}


} // End of namespace onsem
