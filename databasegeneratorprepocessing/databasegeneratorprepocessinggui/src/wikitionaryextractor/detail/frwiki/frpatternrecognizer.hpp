#ifndef FRPATTERNRECOGNIZER_H
#define FRPATTERNRECOGNIZER_H

#include "../metawiki/patternrecognizer.hpp"

class FrPatternRecognizer : public PatternRecognizer
{
public:
  FrPatternRecognizer();

  virtual bool isSeparator
  (const std::string& pLine) const;

  virtual bool isLanguage
  (const std::string& pLine) const;

  virtual PatternRecognizerLanguageEnum getLanguage
  (const std::string& pLine) const;

  virtual bool isBeginLineOfTrads
  (const std::string& pLine) const;

  virtual bool isGramInLineType
  (const std::string& pLine) const;

  virtual void getSperator
  (std::string& pSeparatorName,
   PatternRecognizerSeparatorType& pSeparatorType,
   const std::string& pLine) const;


private:
  std::string fBeginLineOutLang;
  std::size_t fBeginLineOutLang_size;
  std::string fBeginLineOutLang2;
  std::size_t fBeginLineOutLang_size2;
};



inline bool FrPatternRecognizer::isSeparator
(const std::string& pLine) const
{
  return pLine.size() > 6 &&
      pLine.compare(pLine.size() - 2, 2, "==") == 0;
}


inline bool FrPatternRecognizer::isLanguage
(const std::string& pLine) const
{
  return pLine[pLine.size() - 3] == ' ' &&
      pLine.find("langue") != std::string::npos;
}

inline PatternRecognizerLanguageEnum FrPatternRecognizer::getLanguage
(const std::string& pLine) const
{
  if (pLine.find("|fr") != std::string::npos)
  {
    return PATTERNRECO_LANG_INLANG;
  }
  if (pLine.find("|en") != std::string::npos)
  {
    return PATTERNRECO_LANG_OUTLANG;
  }
  return PATTERNRECO_LANG_OTHERLANG;
}



inline bool FrPatternRecognizer::isGramInLineType
(const std::string& pLine) const
{
  return pLine.compare(0, 8, "=== {{S|") == 0 &&
      pLine.find("fr", 9) != std::string::npos;
}



inline bool FrPatternRecognizer::isBeginLineOfTrads
(const std::string& pLine) const
{
  return pLine.compare(0, fBeginLineOutLang_size, fBeginLineOutLang) == 0 ||
      pLine.compare(0, fBeginLineOutLang_size2, fBeginLineOutLang2) == 0;
}






#endif // FRPATTERNRECOGNIZER_H
