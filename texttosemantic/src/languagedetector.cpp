#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>


namespace onsem
{
namespace linguistics
{
namespace
{

std::size_t _getMaxLength(std::string& pLowerCaseText,
                          std::size_t pCurrPos,
                          const StaticLinguisticDictionary& pBinDico,
                          bool pCanLowerCase)
{
  std::size_t res = pBinDico.getLengthOfLongestWord
      (pLowerCaseText, pCurrPos);
  if (res > 0 || !pCanLowerCase)
  {
    return res;
  }
  lowerCaseFirstLetter(pLowerCaseText, pCurrPos);
  return pBinDico.getLengthOfLongestWord
      (pLowerCaseText, pCurrPos);
}


bool _moveUntilNextSeparator
(std::size_t& pCurrPos,
 const std::string& pLowerCaseText,
 const StaticLinguisticDictionary& pBinDico)
{
  bool directlyASeparator = true;
  while (pCurrPos < pLowerCaseText.size())
  {
    const std::size_t longestSep = pBinDico.getLengthOfLongestWord
        (pLowerCaseText, pCurrPos);
    if (longestSep == 0)
    {
      ++pCurrPos;
    }
    PartOfSpeech mainPartOfSpeech = PartOfSpeech::UNKNOWN;
    pBinDico.getConfidenceOfWordInThatLanguage
        (mainPartOfSpeech, pLowerCaseText, pCurrPos, longestSep);
    pCurrPos += longestSep;
    if (!partOfSpeech_isAWord(mainPartOfSpeech))
      break;
    directlyASeparator = false;
  }
  return directlyASeparator;
}
}


SemanticLanguageEnum getLanguage(const std::string& pText,
                                 const LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum moreProbableLang = SemanticLanguageEnum::UNKNOWN;
  int maxconfidenceOfCurrentLanguageForTheText = 0;

  std::size_t langToSpec_size = pLingDb.langToSpec.size();
  for (std::size_t i = 0; i < langToSpec_size; ++i)
  {
    SemanticLanguageEnum language = semanticLanguageEnum_fromChar(static_cast<char>(i));
    if (language == SemanticLanguageEnum::UNKNOWN)
      continue;
    std::size_t currPos = 0;
    int confidenceOfCurrentLanguageForTheText = 0;
    bool canLowerCase = true;
    bool aWordFound = false;
    std::string lowerCaseText = pText;

    while (currPos < lowerCaseText.size())
    {
      const auto& binDico = pLingDb.langToSpec[i].lingDico.statDb;
      std::size_t length = _getMaxLength(lowerCaseText, currPos, binDico,
                                         canLowerCase);
      canLowerCase = false;

      if (length > 0)
      {
        PartOfSpeech mainPartOfSpeech = PartOfSpeech::UNKNOWN;
        std::size_t nbOfMeanings = binDico.getConfidenceOfWordInThatLanguage
            (mainPartOfSpeech, lowerCaseText, currPos, length);
        currPos += length;

        // if it's a word AND
        // the word is followed by a separator
        if (nbOfMeanings > 0)
        {
          if (partOfSpeech_isAWord(mainPartOfSpeech))
          {
            if (!binDico.haveSeparatorBetweenWords() ||
                _moveUntilNextSeparator(currPos, lowerCaseText, binDico))
            {
              confidenceOfCurrentLanguageForTheText += static_cast<int>(length * nbOfMeanings);
              aWordFound = true;
            }
          }
          else if (mainPartOfSpeech == PartOfSpeech::PUNCTUATION)
          {
            canLowerCase = true;
          }
        }
      }
      else
      {
        std::size_t beginPos = currPos;
        ++currPos;
        if (binDico.haveSeparatorBetweenWords())
        {
          _moveUntilNextSeparator(currPos, lowerCaseText, binDico);
        }
        std::size_t lengthOfUnknownCharacters = currPos - beginPos;
        static const int CONFIDENCE_TO_REMOVE_PER_CHARACTER_OF_A_WORD_WITHOUT_PART_OF_SPEECH = 5;
        confidenceOfCurrentLanguageForTheText -= static_cast<int>(lengthOfUnknownCharacters *
                                                                  CONFIDENCE_TO_REMOVE_PER_CHARACTER_OF_A_WORD_WITHOUT_PART_OF_SPEECH);
      }
    }

    if (confidenceOfCurrentLanguageForTheText > maxconfidenceOfCurrentLanguageForTheText ||
        (moreProbableLang == SemanticLanguageEnum::UNKNOWN && aWordFound))
    {
      moreProbableLang = language;
      maxconfidenceOfCurrentLanguageForTheText = confidenceOfCurrentLanguageForTheText;
    }
  }

  return moreProbableLang;
}


} // End of namespace linguistics
} // End of namespace onsem
