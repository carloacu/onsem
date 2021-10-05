#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_TOKENSTOSTRINGCONVERTER_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_TOKENSTOSTRINGCONVERTER_HPP

#include "../synthesizertypes.hpp"
#include "../synthesizerresulttypes.hpp"

namespace onsem
{


class TokensToStringConverter
{
public:
  virtual ~TokensToStringConverter() {}


  void writeOutputText
  (std::list<std::unique_ptr<SynthesizerResult> >& pNaturalLanguageResult,
   std::list<WordToSynthesize>& outBlocs,
   bool pOneLinePerSentence) const;

  struct MergedWordInfos
  {
    MergedWordInfos(const std::string& pResultWord,
                    PartOfSpeech pPartOfSpeechWord1 = PartOfSpeech::UNKNOWN,
                    PartOfSpeech pPartOfSpeechWord2 = PartOfSpeech::UNKNOWN);
    std::string resultWord;
    PartOfSpeech partOfSpeechWord1;
    PartOfSpeech partOfSpeechWord2;
  };

  // first word -> (second word -> merged word)
  using ReplacementForWordCouple = std::map<std::string, std::map<std::string, MergedWordInfos> >;

protected:
  ReplacementForWordCouple _2WordsToMerge;

  TokensToStringConverter();

  static void _addAWord
  (std::string& pNaturalLanguageText,
   const std::string& pStrToAdd,
   bool pBeginOfASentence);

  void _add2WordsToMerge(const std::string& pWord1,
                         const std::string& pWord2,
                         const std::string& pWordResult);
};



class TokensToStringConverterEnglish : public TokensToStringConverter
{
public:
  TokensToStringConverterEnglish();
};

class TokensToStringConverterFrench : public TokensToStringConverter
{
public:
  TokensToStringConverterFrench();
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_TOKENSTOSTRINGCONVERTER_HPP
