#include "tokenstostringconverter.hpp"
#include "../tool/synthesizerconditions.hpp"
#include <onsem/common/utility/uppercasehandler.hpp>

namespace onsem
{

TokensToStringConverter::MergedWordInfos::MergedWordInfos(const std::string& pResultWord,
                                                          PartOfSpeech pPartOfSpeechWord1,
                                                          PartOfSpeech pPartOfSpeechWord2)
  : resultWord(pResultWord),
    partOfSpeechWord1(pPartOfSpeechWord1),
    partOfSpeechWord2(pPartOfSpeechWord2)
{
}


TokensToStringConverterEnglish::TokensToStringConverterEnglish()
  : TokensToStringConverter()
{
  // TODO: remove the following workarounds required to prevent infinitive "to"
  // to appear after the preposition "to", or after some specific cases.
  _add2WordsToMerge("to", "to", "to");
  _add2WordsToMerge("in order to", "to", "in order to");
  _add2WordsToMerge("can", "to", "can");
  _add2WordsToMerge("can't", "to", "can't");
  _add2WordsToMerge("cannot", "to", "cannot");
  // end TODO remove

  _add2WordsToMerge("an", "other", "another");
  _add2WordsToMerge("any", "person", "anybody");
  _add2WordsToMerge("it", "is", "it's");
  _add2WordsToMerge("don't", "can", "can't");
  _add2WordsToMerge("this", "night", "tonight");
}

TokensToStringConverterFrench::TokensToStringConverterFrench()
  : TokensToStringConverter()
{
  _add2WordsToMerge("peux-", "je", "puis-je");
  _add2WordsToMerge("à", "le", "au");
  _add2WordsToMerge("à", "les", "aux");
  _add2WordsToMerge("à", "ce", "à");
  _add2WordsToMerge("de", "d'", "d'");
  _add2WordsToMerge("de", "de", "de");
  _2WordsToMerge["de"].emplace("le", MergedWordInfos("du", PartOfSpeech::UNKNOWN, PartOfSpeech::DETERMINER));
  _2WordsToMerge["de"].emplace("les", MergedWordInfos("des", PartOfSpeech::UNKNOWN, PartOfSpeech::DETERMINER));
}



TokensToStringConverter::TokensToStringConverter()
 : _2WordsToMerge()
{
}


std::list<WordToSynthesize>::iterator _tryMergeNextWords
(const TokensToStringConverter::ReplacementForWordCouple& wordCoupleReplacements,
 std::list<WordToSynthesize>::iterator begin,
 std::list<WordToSynthesize>::iterator end)
{
  while (begin != end)
  {
    auto nextIt = begin;
    ++nextIt;
    if (nextIt == end)
      return begin;

    // only the first element of the list of synthesize output blocks is interesting here
    auto& firstStr = begin->inflections.begin()->str;

    auto firstWordToReplaceIt = wordCoupleReplacements.find(firstStr);
    if (firstWordToReplaceIt == wordCoupleReplacements.end())
      return begin;

    auto& secondStr = nextIt->inflections.begin()->str;
    auto secondWordToReplaceIt = firstWordToReplaceIt->second.find(secondStr);
    if (secondWordToReplaceIt == firstWordToReplaceIt->second.end())
      return begin;

    if ((secondWordToReplaceIt->second.partOfSpeechWord1 == PartOfSpeech::UNKNOWN ||
         secondWordToReplaceIt->second.partOfSpeechWord1 == begin->word.partOfSpeech) &&
        (secondWordToReplaceIt->second.partOfSpeechWord2 == PartOfSpeech::UNKNOWN ||
         secondWordToReplaceIt->second.partOfSpeechWord2 == nextIt->word.partOfSpeech))
    {
      ++begin;
      begin->inflections.begin()->str = secondWordToReplaceIt->second.resultWord;
//      begin->inflections.begin()->ifCanHaveSpaceAfter = nextIt->inflections.begin()->ifCanHaveSpaceAfter;
    }
    else
    {
      return begin;
    }
  }
  return begin;
}

void TokensToStringConverter::writeOutputText
(std::list<std::unique_ptr<SynthesizerResult>>& pNaturalLanguageResult,
 std::list<WordToSynthesize>& pOutBlocs,
 bool pOneLinePerSentence) const
{
  static const WordToSynthesize endOfTextWord(SemanticWord(SemanticLanguageEnum::UNKNOWN, "", PartOfSpeech::PUNCTUATION),
                                              InflectionToSynthesize("", false, false, alwaysTrue));
  {
    // remove bad conditions
    WordToSynthesize* nextWordPtr = nullptr;
    auto itBlocs = pOutBlocs.end();
    while (itBlocs != pOutBlocs.begin())
    {
      --itBlocs;
      // find the good possibility
      auto itGoodPossibility = itBlocs->inflections.end();
      for (auto itPossibility = itBlocs->inflections.begin();
           itPossibility != itBlocs->inflections.end(); ++itPossibility)
      {
        bool conditionIsSatisfied = false;
        if (nextWordPtr != nullptr)
          conditionIsSatisfied = itPossibility->conditions(*nextWordPtr);
        else
          conditionIsSatisfied = itPossibility->conditions(endOfTextWord);
        if (conditionIsSatisfied)
        {
          itGoodPossibility = itPossibility;
          break;
        }
      }

      if (itGoodPossibility == itBlocs->inflections.end())
      {
        itBlocs = pOutBlocs.erase(itBlocs);
      }
      else
      {
        while (itBlocs->inflections.begin() != itGoodPossibility)
          itBlocs->inflections.pop_front();
        nextWordPtr = &*itBlocs;
      }
    }
  }

  // print text result
  bool ifCanHaveSpaceAfter = false;
  bool beginOfASentence = true;
  std::string currentText;
  for (auto itBlocs = pOutBlocs.begin();
       itBlocs != pOutBlocs.end(); ++itBlocs)
  {
    if (ifCanHaveSpaceAfter)
    {
      if (beginOfASentence)
        currentText += pOneLinePerSentence ? "\n" : " ";
      else if (itBlocs->inflections.begin()->ifCanHaveSpaceBefore)
        currentText += " ";
    }

    itBlocs = _tryMergeNextWords(_2WordsToMerge, itBlocs, pOutBlocs.end());
    const InflectionToSynthesize& inflToSynt = itBlocs->inflections.front();
    if (inflToSynt.fromResourcePtr != nullptr)
    {
      if (!currentText.empty())
      {
        pNaturalLanguageResult.emplace_back(std::make_unique<SynthesizerText>(currentText));
        currentText.clear();
      }
      pNaturalLanguageResult.emplace_back(std::make_unique<SynthesizerTask>(inflToSynt.fromResourcePtr->resource));
    }
    else
    {
      _addAWord(currentText, inflToSynt.str, beginOfASentence);
    }
    ifCanHaveSpaceAfter = inflToSynt.ifCanHaveSpaceAfter;
    beginOfASentence = itBlocs->word.partOfSpeech == PartOfSpeech::PUNCTUATION;
  }
  if (!currentText.empty())
    pNaturalLanguageResult.emplace_back(std::make_unique<SynthesizerText>(currentText));
}

void TokensToStringConverter::_addAWord
(std::string& pNaturalLanguageText,
 const std::string& pStrToAdd,
 bool pBeginOfASentence)
{
  if (pBeginOfASentence)
    pNaturalLanguageText += getFirstLetterInUpperCase(pStrToAdd);
  else
    pNaturalLanguageText += pStrToAdd;
}


void TokensToStringConverter::_add2WordsToMerge(const std::string& pWord1,
                                                const std::string& pWord2,
                                                const std::string& pWordResult)
{
  _2WordsToMerge[pWord1].emplace(pWord2, MergedWordInfos(pWordResult));
}


} // End of namespace onsem
