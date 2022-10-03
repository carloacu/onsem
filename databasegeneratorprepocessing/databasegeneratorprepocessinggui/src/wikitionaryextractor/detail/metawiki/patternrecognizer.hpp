#ifndef METAPATTERNRECOGNIZER_H
#define METAPATTERNRECOGNIZER_H

#include <string>
#include <list>

enum PatternRecognizerSeparatorType
{
  PATTERNRECO_SEP_SYNONYMS,
  PATTERNRECO_SEP_UNKNOWN
};


enum PatternRecognizerLanguageEnum
{
  PATTERNRECO_LANG_INLANG,
  PATTERNRECO_LANG_OUTLANG,
  PATTERNRECO_LANG_OTHERLANG
};


class PatternRecognizer
{
public:
  virtual ~PatternRecognizer() {}

  static bool isTitle
  (const std::string& pLine);

  virtual bool isSeparator
  (const std::string& pLine) const = 0;

  virtual bool isLanguage
  (const std::string& pLine) const = 0;

  virtual PatternRecognizerLanguageEnum getLanguage
  (const std::string& pLine) const = 0;

  virtual bool isBeginLineOfTrads
  (const std::string& pLine) const = 0;

  virtual bool isGramInLineType
  (const std::string& pLine) const = 0;

  virtual void getSperator
  (std::string& pSeparatorName,
   PatternRecognizerSeparatorType& pSeparatorType,
   const std::string& pLine) const = 0;


  static void getStradsStr
  (std::list<std::string>& pTradsStr,
   const std::string& pLine);
};


inline bool PatternRecognizer::isTitle
(const std::string& pLine)
{
  return pLine.compare(0, 11, "    <title>") == 0;
}



#endif // METAPATTERNRECOGNIZER_H
