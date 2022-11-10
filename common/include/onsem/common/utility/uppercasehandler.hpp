#ifndef ONSEM_COMMON_UTILITY_UPPERCASEHANDLER_HPP
#define ONSEM_COMMON_UTILITY_UPPERCASEHANDLER_HPP

#include <ostream>
#include <assert.h>

namespace onsem
{
namespace
{
const std::string _capitalAGrave_str = "À";
const std::size_t _capitalAGrave_size = _capitalAGrave_str.size();
const std::string _capitalEAcute_str = "É";
const std::size_t _capitalEAcute_size = _capitalEAcute_str.size();
const std::string _capitalECirconflex_str = "Ê";
const std::size_t _capitalECirconflex_size = _capitalECirconflex_str.size();
const std::string _capitalCCedilla_str = "Ç";
const std::size_t _capitalCCedilla_size = _capitalCCedilla_str.size();
const std::string _capitalSCedilla_str = "Ş";
const std::size_t _capitalSCedilla_size = _capitalSCedilla_str.size();
const std::string _capitalOTrema_str = "Ö";
const std::size_t _capitalOTrema_size = _capitalOTrema_str.size();
const std::string _aGrave_str = "à";
const std::size_t _aGrave_size = _aGrave_str.size();
const std::string _eAcute_str = "é";
const std::size_t _eAcute_size = _eAcute_str.size();
const std::string _eCirconflex_str = "ê";
const std::size_t _eCirconflex_size = _eCirconflex_str.size();
const std::string _iCirconflex_str = "î";
const std::size_t _iCirconflex_size = _iCirconflex_str.size();
const std::string _cCedilla_str = "ç";
const std::size_t _cCedilla_size = _cCedilla_str.size();
const std::string _sCedilla_str = "ș";
const std::size_t _sCedilla_size = _sCedilla_str.size();
const std::string _oTrema_str = "ö";
const std::size_t _oTrema_size = _oTrema_str.size();
}

static_assert('A' < 'Z', "Wrong assumption: A is not inferior to Z");
static_assert("É"[0] == "À"[0], "Wrong assumption: É & À doesn't begin with same character");
static_assert("É"[0] == "Ê"[0], "Wrong assumption: É & Ê doesn't begin with same character");
static_assert("É"[0] == "Î"[0], "Wrong assumption: É & Î doesn't begin with same character");
static_assert("É"[0] == "Ç"[0], "Wrong assumption: É & Ç doesn't begin with same character");
static_assert("É"[0] == "Ö"[0], "Wrong assumption: É & Ö doesn't begin with same character");
static_assert('a' < 'z', "Wrong assumption: a is not inferior to z");
static_assert("é"[0] == "à"[0], "Wrong assumption: é & à doesn't begin with same character");
static_assert("é"[0] == "ê"[0], "Wrong assumption: é & ê doesn't begin with same character");
static_assert("é"[0] == "î"[0], "Wrong assumption: é & î doesn't begin with same character");
static_assert("é"[0] == "ç"[0], "Wrong assumption: é & ç doesn't begin with same character");
static_assert("é"[0] == "ö"[0], "Wrong assumption: é & ö doesn't begin with same character");
static_assert("É"[0] == "é"[0], "Wrong assumption: É & é doesn't begin with same character");


/**
 * @brief If the char if a digit.
 * @param pChar The char.
 * @return True if the char is a digit, False otherwise.
 */
inline static bool isDigit(char pChar)
{
  return pChar >= '0' && pChar <= '9';
}

// /!\ If pBegin is greater or equal to the size of pStr, it causes undefined behavior.
inline static bool beginWithUppercase(const std::string& pStr,
                                      std::size_t pBegin = 0)
{
  assert(pBegin < pStr.size());
  if ((pStr[pBegin] >= 'A' && pStr[pBegin] <= 'Z'))
    return true;
  switch (pStr[pBegin])
  {
  case "É"[0]:
  {
    if (pStr.compare(pBegin, _capitalAGrave_size, "À") == 0 ||
        pStr.compare(pBegin, _capitalEAcute_size, "É") == 0 ||
        pStr.compare(pBegin, _capitalECirconflex_size, "Ê") == 0 ||
        pStr.compare(pBegin, _capitalCCedilla_size, "Ç") == 0 ||
        pStr.compare(pBegin, _capitalOTrema_size, "Ö") == 0)
      return true;
    break;
  }
  case "Ş"[0]:
  {
    if (pStr.compare(pBegin, _capitalSCedilla_size, "Ş") == 0)
      return true;
    break;
  }
  }
  return false;
}


// /!\ If pBegin is greater or equal to the size of pStr, it causes undefined behavior.
inline static bool beginWithLowerCase(const std::string& pStr,
                                      std::size_t pBegin = 0)
{
  assert(pBegin < pStr.size());
  if (pStr[pBegin] >= 'a' && pStr[pBegin] <= 'z')
    return true;
  switch (pStr[pBegin])
  {
  case "é"[0]:
  {
    if (pStr.compare(pBegin, _aGrave_size, "à") == 0 ||
        pStr.compare(pBegin, _eAcute_size, "é") == 0 ||
        pStr.compare(pBegin, _eCirconflex_size, "ê") == 0 ||
        pStr.compare(pBegin, _cCedilla_size, "ç") == 0 ||
        pStr.compare(pBegin, _oTrema_size, "ö") == 0)
      return true;
    break;
  }
  case "Ş"[0]:
  {
    if (pStr.compare(pBegin, _sCedilla_size, "ș") == 0)
      return true;
    break;
  }
  }
  return false;
}


bool lowerCaseFirstLetter(std::string& pStrToModify,
                          std::size_t pPos = 0);

bool lowerCaseText(std::string& pStrToModify,
                   std::size_t pPos = 0);

inline static std::string getFirstLetterInUpperCase(const std::string& pText)
{
  if (pText.empty())
    return "";

  static const std::size_t pos = 0;
  std::string firstLetter;
  std::size_t nbCharEaten = 0;
  if (pText[pos] >= 'a' && pText[pos] <= 'z')
  {
    nbCharEaten = 1;
    firstLetter = std::string(1, static_cast<char>(pText[pos] - ('a' - 'A')));
  }
  else
  {
    switch (pText[pos])
    {
    case "é"[0]:
    {
      if (pText.compare(pos, _aGrave_size, "à") == 0)
      {
        nbCharEaten = _aGrave_size;
        firstLetter = "À";
      }
      if (pText.compare(pos, _eAcute_size, "é") == 0)
      {
        nbCharEaten = _eAcute_size;
        firstLetter = "É";
      }
      if (pText.compare(pos, _eCirconflex_size, "ê") == 0)
      {
        nbCharEaten = _eCirconflex_size;
        firstLetter = "Ê";
      }
      if (pText.compare(pos, _cCedilla_size, "ç") == 0)
      {
        nbCharEaten = _cCedilla_size;
        firstLetter = "Ç";
      }
      if (pText.compare(pos, _oTrema_size, "ö") == 0)
      {
        nbCharEaten = _oTrema_size;
        firstLetter = "Ö";
      }
      break;
    }
    case "ș"[0]:
    {
      if (pText.compare(pos, _sCedilla_size, "ș") == 0)
      {
        nbCharEaten = _sCedilla_size;
        firstLetter = "Ş";
      }
      break;
    }
    }
  }
  if (nbCharEaten > 0 && pText.size() >= nbCharEaten)
    return firstLetter + pText.substr(nbCharEaten, pText.size() - nbCharEaten);
  return pText;
}


inline bool isFirstLetterAVowel(const std::string& pText,
                                std::size_t pPos = 0)
{
  if (pText[pPos] >= 'a' && pText[pPos] <= 'z')
  {
    return pText[pPos] == 'a' || pText[pPos] == 'e' || pText[pPos] == 'i' ||
        pText[pPos] == 'o' || pText[pPos] == 'u' || pText[pPos] == 'y';
  }

  if (pText[pPos] >= 'A' && pText[pPos] <= 'Z')
  {
    return pText[pPos] == 'A' || pText[pPos] == 'E' || pText[pPos] == 'I' ||
        pText[pPos] == 'O' || pText[pPos] == 'U' || pText[pPos] == 'Y';
  }


  if (pText[pPos] == "É"[0])
  {
    return pText.compare(pPos, _capitalAGrave_size, "À") == 0 ||
        pText.compare(pPos, _capitalEAcute_size, "É") == 0 ||
        pText.compare(pPos, _capitalECirconflex_size, "Ê") == 0 ||
        pText.compare(pPos, _capitalECirconflex_size, "Î") == 0 ||
        pText.compare(pPos, _capitalOTrema_size, "Ö") == 0 ||
        pText.compare(pPos, _aGrave_size, "à") == 0 ||
        pText.compare(pPos, _eAcute_size, "é") == 0 ||
        pText.compare(pPos, _eCirconflex_size, "ê") == 0 ||
        pText.compare(pPos, _iCirconflex_size, "î") == 0 ||
        pText.compare(pPos, _oTrema_size, "ö") == 0;
  }
  return false;
}

bool areTextEqualWithoutCaseSensitivity(const std::string& pText1,
                                        const std::string& pText2);

} // End of namespace onsem

#endif // ONSEM_COMMON_UTILITY_UPPERCASEHANDLER_HPP
