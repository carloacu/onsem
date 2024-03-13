#ifndef ENPATTERNRECOGNIZER_H
#define ENPATTERNRECOGNIZER_H

#include "../metawiki/patternrecognizer.hpp"

class EnPatternRecognizer : public PatternRecognizer {
public:
    EnPatternRecognizer();

    virtual bool isSeparator(const std::string& pLine) const;

    virtual bool isLanguage(const std::string& pLine) const;

    virtual PatternRecognizerLanguageEnum getLanguage(const std::string& pLine) const;

    virtual bool isBeginLineOfTrads(const std::string& pLine) const;

    virtual bool isGramInLineType(const std::string& pLine) const;

    virtual void getSperator(std::string& pSeparatorName,
                             PatternRecognizerSeparatorType& pSeparatorType,
                             const std::string& pLine) const;

private:
    std::string fBeginLineOutLang;
    std::size_t fBeginLineOutLang_size;
    std::string fBeginLineOutLang2;
    std::size_t fBeginLineOutLang_size2;
};

inline bool EnPatternRecognizer::isSeparator(const std::string& pLine) const {
    return pLine.size() > 6 && pLine.compare(pLine.size() - 2, 2, "==") == 0;
}

inline bool EnPatternRecognizer::isLanguage(const std::string& pLine) const {
    return pLine[pLine.size() - 3] != '=';
}

inline PatternRecognizerLanguageEnum EnPatternRecognizer::getLanguage(const std::string& pLine) const {
    if (pLine.find("English") != std::string::npos) {
        return PATTERNRECO_LANG_INLANG;
    }
    if (pLine.find("French") != std::string::npos) {
        return PATTERNRECO_LANG_OUTLANG;
    }
    return PATTERNRECO_LANG_OTHERLANG;
}

inline bool EnPatternRecognizer::isBeginLineOfTrads(const std::string& pLine) const {
    return pLine.compare(0, fBeginLineOutLang_size, fBeginLineOutLang) == 0
        || pLine.compare(0, fBeginLineOutLang_size2, fBeginLineOutLang2) == 0;
}

inline bool EnPatternRecognizer::isGramInLineType(const std::string& pLine) const {
    return pLine.compare(0, 3, "===") == 0 && pLine.size() > 6 && pLine[4] != '=';
}

#endif    // ENPATTERNRECOGNIZER_H
