#include <onsem/common/utility/string.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>

namespace onsem {
namespace mystd {
namespace {


bool _isASeparator(char pChar) {
    return pChar == ' ' || pChar == '\'' || pChar == '-' || pChar == ',' || pChar == ';' || pChar == ':' || pChar == '.'
        || pChar == '!' || pChar == '?' || pChar == '^' || pChar == '$' || pChar == '&' || pChar == '|';
}
}

static_assert('A' < 'Z', "Wrong assumption: A is not inferior to Z");
static_assert("É"[0] == "À"[0], "Wrong assumption: É & À doesn't begin with same character");
static_assert("É"[0] == "È"[0], "Wrong assumption: É & È doesn't begin with same character");
static_assert("É"[0] == "Ê"[0], "Wrong assumption: É & Ê doesn't begin with same character");
static_assert("É"[0] == "Î"[0], "Wrong assumption: É & Î doesn't begin with same character");
static_assert("É"[0] == "Ô"[0], "Wrong assumption: É & Ô doesn't begin with same character");
static_assert("É"[0] == "Ç"[0], "Wrong assumption: É & Ç doesn't begin with same character");
static_assert("É"[0] == "Â"[0], "Wrong assumption: É & Â doesn't begin with same character");
static_assert("É"[0] == "Ù"[0], "Wrong assumption: É & Ù doesn't begin with same character");
static_assert("É"[0] == "Û"[0], "Wrong assumption: É & Û doesn't begin with same character");
static_assert('a' < 'z', "Wrong assumption: a is not inferior to z");
static_assert("É"[0] == "à"[0], "Wrong assumption: É & à doesn't begin with same character");
static_assert("É"[0] == "ä"[0], "Wrong assumption: É & ä doesn't begin with same character");
static_assert("É"[0] == "á"[0], "Wrong assumption: É & á doesn't begin with same character");
static_assert("É"[0] == "æ"[0], "Wrong assumption: É & æ doesn't begin with same character");
static_assert("É"[0] == "é"[0], "Wrong assumption: É & é doesn't begin with same character");
static_assert("É"[0] == "è"[0], "Wrong assumption: É & è doesn't begin with same character");
static_assert("É"[0] == "ë"[0], "Wrong assumption: É & ë doesn't begin with same character");
static_assert("É"[0] == "â"[0], "Wrong assumption: É & â doesn't begin with same character");
static_assert("É"[0] == "ê"[0], "Wrong assumption: É & ê doesn't begin with same character");
static_assert("É"[0] == "î"[0], "Wrong assumption: É & î doesn't begin with same character");
static_assert("É"[0] == "í"[0], "Wrong assumption: É & í doesn't begin with same character");
static_assert("É"[0] == "ó"[0], "Wrong assumption: É & ó doesn't begin with same character");
static_assert("É"[0] == "ô"[0], "Wrong assumption: É & ô doesn't begin with same character");
static_assert("É"[0] == "ö"[0], "Wrong assumption: É & ö doesn't begin with same character");
static_assert("É"[0] == "ú"[0], "Wrong assumption: É & ú doesn't begin with same character");
static_assert("É"[0] == "û"[0], "Wrong assumption: É & û doesn't begin with same character");
static_assert("É"[0] == "ü"[0], "Wrong assumption: É & ü doesn't begin with same character");
static_assert("É"[0] == "ï"[0], "Wrong assumption: É & ï doesn't begin with same character");
static_assert("É"[0] == "ç"[0], "Wrong assumption: É & ç doesn't begin with same character");
static_assert("É"[0] == "ñ"[0], "Wrong assumption: É & ñ doesn't begin with same character");
static_assert("É"[0] == "ß"[0], "Wrong assumption: É & ß doesn't begin with same character");

static_assert("œ"[0] == "œ"[0], "Wrong assumption: É & œ doesn't begin with same character");



std::string urlizeText(const std::string& pText, bool pMergeTokens) {
  static const std::string _capitalAGrave_str = "À";
  static const std::size_t _capitalAGrave_size = _capitalAGrave_str.size();
  static const std::string _capitalEAcute_str = "É";
  static const std::size_t _capitalEAcute_size = _capitalEAcute_str.size();
  static const std::string _capitalECirconflex_str = "Ê";
  static const std::size_t _capitalECirconflex_size = _capitalECirconflex_str.size();
  static const std::string _capitalICirconflex_str = "Î";
  static const std::size_t _capitalICirconflex_size = _capitalICirconflex_str.size();
  static const std::string _capitalOCirconflex_str = "Ô";
  static const std::size_t _capitalOCirconflex_size = _capitalOCirconflex_str.size();
  static const std::string _capitalUCirconflex_str = "Û";
  static const std::size_t _capitalUCirconflex_size = _capitalUCirconflex_str.size();
  static const std::string _capitalACirconflex_str = "Â";
  static const std::size_t _capitalACirconflex_size = _capitalACirconflex_str.size();
  static const std::string _capitalCCedilla_str = "Ç";
  static const std::size_t _capitalCCedilla_size = _capitalCCedilla_str.size();
  static const std::string _aGrave_str = "à";
  static const std::size_t _aGrave_size = _aGrave_str.size();
  static const std::string _eAcute_str = "é";
  static const std::size_t _eAcute_size = _eAcute_str.size();
  static const std::string _eGrave_str = "è";
  static const std::size_t _eGrave_size = _eGrave_str.size();
  static const std::string _eTrema_str = "ë";
  static const std::size_t _eTrema_size = _eTrema_str.size();
  static const std::string _aCirconflex_str = "â";
  static const std::size_t _aCirconflex_size = _aCirconflex_str.size();
  static const std::string _eCirconflex_str = "ê";
  static const std::size_t _eCirconflex_size = _eCirconflex_str.size();
  static const std::string _iCirconflex_str = "î";
  static const std::size_t _iCirconflex_size = _iCirconflex_str.size();
  static const std::string _oCirconflex_str = "ô";
  static const std::size_t _oCirconflex_size = _oCirconflex_str.size();
  static const std::string _uCirconflex_str = "û";
  static const std::size_t _uCirconflex_size = _uCirconflex_str.size();
  static const std::string _iTrema_str = "ï";
  static const std::size_t _iTrema_size = _iTrema_str.size();
  static const std::string _cCedilla_str = "ç";
  static const std::size_t _cCedilla_size = _cCedilla_str.size();
  static const std::string _apos_str = "’";
  static const std::size_t _apos_size = _apos_str.size();
  static const std::string _bigSep_str = "–";
  static const std::size_t _bigSep_size = _bigSep_str.size();
  static const std::string _aAcute_str = "á";
  static const std::size_t _aAcute_size = _aAcute_str.size();
  static const std::string _iAcute_str = "í";
  static const std::size_t _iAcute_size = _iAcute_str.size();
  static const std::string _oAcute_str = "ó";
  static const std::size_t _oAcute_size = _oAcute_str.size();
  static const std::string _uAcute_str = "ú";
  static const std::size_t _uAcute_size = _uAcute_str.size();
  static const std::string _nTilde_str = "ñ";
  static const std::size_t _nTilde_size = _nTilde_str.size();
  static const std::string _uTrema_str = "ü";
  static const std::size_t _uTrema_size = _uTrema_str.size();
  static const std::string _aTrema_str = "ä";
  static const std::size_t _aTrema_size = _aTrema_str.size();
  static const std::string _oTrema_str = "ö";
  static const std::size_t _oTrema_size = _oTrema_str.size();
  static const std::string _eszett_str = "ß";
  static const std::size_t _eszett_size = _eszett_str.size();
  static const std::string _ae_str = "æ";
  static const std::size_t _ae_size = _ae_str.size();
  static const std::string _oe_str = "œ";
  static const std::size_t _oe_size = _oe_str.size();
  static const std::string _capitalEGrave_str = "È";
  static const std::size_t _capitalEGrave_size = _capitalEGrave_str.size();
  static const std::string _capitalUGrave_str = "Ù";
  static const std::size_t _capitalUGrave_size = _capitalUGrave_str.size();

  std::string res;

    for (std::size_t i = 0; i < pText.size();) {
        if (pText[i] >= 'a' && pText[i] <= 'z') {
            res += pText[i++];
            continue;
        }
        if (pText[i] >= '0' && pText[i] <= '9') {
            res += pText[i++];
            continue;
        }
        if (pText[i] >= 'A' && pText[i] <= 'Z') {
            res += static_cast<char>(pText[i++] + 'a' - 'A');
            continue;
        }
        if (!pMergeTokens && (pText[i] == '-' || pText[i] == ' ' || pText[i] == '\'' || pText[i] == '.')) {
            if (!res.empty() && res[res.size() - 1] != '-')
                res += '-';
            ++i;
            continue;
        }
        if (pText.compare(i, _apos_size, "’") == 0) {
            res += '-';
            i += _apos_size;
            continue;
        }
        if (pText.compare(i, _bigSep_size, "–") == 0) {
            res += '-';
            i += _bigSep_size;
            continue;
        }

        if (pText[i] == "É"[0]) {
            if (pText.compare(i, _capitalAGrave_size, "À") == 0) {
                res += 'a';
                i += _capitalAGrave_size;
                continue;
            }
            if (pText.compare(i, _capitalACirconflex_size, "Â") == 0) {
                res += 'a';
                i += _capitalACirconflex_size;
                continue;
            }
            if (pText.compare(i, _aGrave_size, "à") == 0) {
                res += 'a';
                i += _aGrave_size;
                continue;
            }
            if (pText.compare(i, _aCirconflex_size, "â") == 0) {
                res += 'a';
                i += _aCirconflex_size;
                continue;
            }
            if (pText.compare(i, _capitalEAcute_size, "É") == 0) {
                res += 'e';
                i += _capitalEAcute_size;
                continue;
            }
            if (pText.compare(i, _eAcute_size, "é") == 0) {
                res += 'e';
                i += _eAcute_size;
                continue;
            }
            if (pText.compare(i, _capitalECirconflex_size, "Ê") == 0) {
                res += 'e';
                i += _capitalECirconflex_size;
                continue;
            }
            if (pText.compare(i, _eCirconflex_size, "ê") == 0) {
                res += 'e';
                i += _eCirconflex_size;
                continue;
            }
            if (pText.compare(i, _eGrave_size, "è") == 0) {
                res += 'e';
                i += _eGrave_size;
                continue;
            }
            if (pText.compare(i, _eTrema_size, "ë") == 0) {
                res += 'e';
                i += _eTrema_size;
                continue;
            }
            if (pText.compare(i, _capitalCCedilla_size, "Ç") == 0) {
                res += 'c';
                i += _capitalCCedilla_size;
                continue;
            }
            if (pText.compare(i, _cCedilla_size, "ç") == 0) {
                res += 'c';
                i += _cCedilla_size;
                continue;
            }
            if (pText.compare(i, _capitalICirconflex_size, "Î") == 0) {
                res += 'i';
                i += _capitalICirconflex_size;
                continue;
            }
            if (pText.compare(i, _iCirconflex_size, "î") == 0) {
                res += 'i';
                i += _iCirconflex_size;
                continue;
            }
            if (pText.compare(i, _iTrema_size, "ï") == 0) {
                res += 'i';
                i += _iTrema_size;
                continue;
            }
            if (pText.compare(i, _capitalOCirconflex_size, "Ô") == 0) {
                res += 'o';
                i += _capitalOCirconflex_size;
                continue;
            }
            if (pText.compare(i, _oCirconflex_size, "ô") == 0) {
                res += 'o';
                i += _oCirconflex_size;
                continue;
            }
            if (pText.compare(i, _capitalUCirconflex_size, "Û") == 0) {
                res += 'u';
                i += _capitalUCirconflex_size;
                continue;
            }
            if (pText.compare(i, _uCirconflex_size, "û") == 0) {
                res += 'u';
                i += _uCirconflex_size;
                continue;
            }
            if (pText.compare(i, _aAcute_size, "á") == 0) {
                res += 'a';
                i += _aAcute_size;
                continue;
            }
            if (pText.compare(i, _iAcute_size, "í") == 0) {
                res += 'i';
                i += _iAcute_size;
                continue;
            }
            if (pText.compare(i, _oAcute_size, "ó") == 0) {
                res += 'o';
                i += _oAcute_size;
                continue;
            }
            if (pText.compare(i, _uAcute_size, "ú") == 0) {
                res += 'u';
                i += _uAcute_size;
                continue;
            }
            if (pText.compare(i, _nTilde_size, "ñ") == 0) {
                res += 'n';
                i += _nTilde_size;
                continue;
            }
            if (pText.compare(i, _uTrema_size, "ü") == 0) {
                res += 'u';
                i += _uTrema_size;
                continue;
            }
            if (pText.compare(i, _aTrema_size, "ä") == 0) {
                res += 'a';
                i += _aTrema_size;
                continue;
            }
            if (pText.compare(i, _oTrema_size, "ö") == 0) {
                res += 'o';
                i += _oTrema_size;
                continue;
            }
            if (pText.compare(i, _eszett_size, "ß") == 0) {
                res += "ss";
                i += _eszett_size;
                continue;
            }
            if (pText.compare(i, _ae_size, "æ") == 0) {
                res += "ae";
                i += _ae_size;
                continue;
            }
            if (pText.compare(i, _capitalEGrave_size, "È") == 0) {
                res += 'e';
                i += _capitalEGrave_size;
                continue;
            }
            if (pText.compare(i, _capitalUGrave_size, "Ù") == 0) {
                res += 'u';
                i += _capitalUGrave_size;
                continue;
            }
        }

        if (pText[i] == "œ"[0]) {
            if (pText.compare(i, _oe_size, "œ") == 0) {
                res += "oe";
                i += _oe_size;
                continue;
            }
        }

        ++i;
    }
    if (!res.empty() && res[res.size() - 1] == '-')
        res = res.substr(0, res.size() - 1);
    return res;
}

std::size_t findFirstOf(const std::string& pStr, const std::string& pChars, std::size_t pBegin, std::size_t pEnd) {
    for (auto i = pBegin; i < pEnd; ++i)
        if (pChars.find_first_of(pStr[i]) != std::string::npos)
            return i;
    return pEnd;
}

void replace_all(std::string& str, const std::string& oldStr, const std::string& newStr) {
    std::string::size_type pos = 0u;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

void removeFromStr(std::string& pStr, const std::string& pStrToRemove) {
    size_t pos = pStr.find(pStrToRemove);
    if (pos != std::string::npos)
        pStr.erase(pos, pStrToRemove.length());
}

Replacer::Replacer(bool pIsCaseSensitive, bool pHaveSeparatorBetweenWords)
    : _isCaseSensitive(pIsCaseSensitive)
    , _haveSeparatorBetweenWords(pHaveSeparatorBetweenWords)
    , _patternsToSearchToOutput() {}

void Replacer::addReplacementPattern(const std::string& pPatternToSearch, const std::string& pOutput) {
    if (_isCaseSensitive) {
        _patternsToSearchToOutput.emplace(pPatternToSearch, pOutput);
    } else {
        auto patternToSearch = pPatternToSearch;
        lowerCaseText(patternToSearch);
        _patternsToSearchToOutput.emplace(patternToSearch, pOutput);
    }
}

std::string Replacer::doReplacements(const std::string& pInput) const {
    std::string res;
    auto input = "^" + pInput + "$";
    if (!_isCaseSensitive)
        lowerCaseText(input);

    for (std::size_t i = 0; i < input.size();) {
        auto maxLength = _patternsToSearchToOutput.getMaxLength(input, i);
        if (maxLength > 0) {
            auto newPos = i + maxLength;
            if (!_haveSeparatorBetweenWords || newPos >= input.size() || _isASeparator(input[newPos])) {
                const auto* outputStr = _patternsToSearchToOutput.find_ptr(input, i, maxLength);
                if (outputStr != nullptr) {
                    res += *outputStr;
                    i = newPos;
                    continue;
                }
            }
        }

        if (i > 0 && i - 1 < pInput.size())
            res += pInput[i - 1];
        if (_haveSeparatorBetweenWords) {
            while (i < input.size() - 1 && !_isASeparator(input[i])) {
                if (i < pInput.size())
                    res += pInput[i];
                ++i;
            }
        }
        ++i;
    }
    bool removeFirstCharacter = !res.empty() && res[0] == '^';
    bool removeLastCharacter = !res.empty() && res[res.size() - 1] == '$';
    if (removeFirstCharacter || removeLastCharacter) {
        std::size_t beginPos = removeFirstCharacter ? 1 : 0;
        std::size_t subStrLen = res.size();
        if (removeFirstCharacter)
            --subStrLen;
        if (removeLastCharacter)
            --subStrLen;
        return res.substr(beginPos, subStrLen);
    }
    return res;
}

void split(std::vector<std::string>& pStrs, const std::string& pStr, const std::string& pSeparator) {
    std::string::size_type lastPos = 0u;
    std::string::size_type pos = lastPos;
    std::size_t separatorSize = pSeparator.size();
    while ((pos = pStr.find(pSeparator, pos)) != std::string::npos) {
        pStrs.emplace_back(pStr.substr(lastPos, pos - lastPos));
        pos += separatorSize;
        lastPos = pos;
    }
    pStrs.emplace_back(pStr.substr(lastPos, pStr.size() - lastPos));
}

void splitAnyOf(std::vector<std::string>& pStrs, const std::string& pStr, const std::set<char>& pChars) {
    std::size_t lastPos = 0u;
    for (std::size_t pos = 0u; pos < pStr.size();) {
        if (pChars.count(pStr[pos]) > 0) {
            pStrs.emplace_back(pStr.substr(lastPos, pos - lastPos));
            ++pos;
            lastPos = pos;
        } else {
            ++pos;
        }
    }
    pStrs.emplace_back(pStr.substr(lastPos, pStr.size() - lastPos));
}

void splitNotEmpty(std::vector<std::string>& pStrs, const std::string& pStr, const std::string& pSeparator) {
    std::string::size_type lastPos = 0u;
    std::string::size_type pos = lastPos;
    std::size_t separatorSize = pSeparator.size();
    while ((pos = pStr.find(pSeparator, pos)) != std::string::npos) {
        if (pos > lastPos)
            pStrs.emplace_back(pStr.substr(lastPos, pos - lastPos));
        pos += separatorSize;
        lastPos = pos;
    }

    std::size_t strSize = pStr.size();
    if (strSize > lastPos)
        pStrs.emplace_back(pStr.substr(lastPos, strSize - lastPos));
}

std::string filenameToSuffix(const std::string& pFileName) {
    auto i = pFileName.size();
    while (i > 0) {
        --i;
        if (pFileName[i] == '.') {
            auto pos = i + 1;
            return pFileName.substr(pos, pFileName.size() - pos);
        }
    }
    return "";
}

bool differentThanBoth(const std::string& pStr, const std::string& pPossibility1, const std::string& pPossibility2) {
    return pStr != pPossibility1 && pStr != pPossibility2;
}

}    // End of namespace mystd
}    // End of namespace onsem
