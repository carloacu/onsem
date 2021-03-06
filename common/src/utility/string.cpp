#include <onsem/common/utility/string.hpp>

namespace onsem
{
namespace mystd
{
namespace
{
const std::string _capitalAGrave_str = "À";
const std::size_t _capitalAGrave_size = _capitalAGrave_str.size();
const std::string _capitalEAcute_str = "É";
const std::size_t _capitalEAcute_size = _capitalEAcute_str.size();
const std::string _capitalECirconflex_str = "Ê";
const std::size_t _capitalECirconflex_size = _capitalECirconflex_str.size();
const std::string _capitalICirconflex_str = "Î";
const std::size_t _capitalICirconflex_size = _capitalICirconflex_str.size();
const std::string _capitalOCirconflex_str = "Ô";
const std::size_t _capitalOCirconflex_size = _capitalOCirconflex_str.size();
const std::string _capitalCCedilla_str = "Ç";
const std::size_t _capitalCCedilla_size = _capitalCCedilla_str.size();
const std::string _aGrave_str = "à";
const std::size_t _aGrave_size = _aGrave_str.size();
const std::string _eAcute_str = "é";
const std::size_t _eAcute_size = _eAcute_str.size();
const std::string _eGrave_str = "è";
const std::size_t _eGrave_size = _eGrave_str.size();
const std::string _eTrema_str = "ë";
const std::size_t _eTrema_size = _eTrema_str.size();
const std::string _aCirconflex_str = "â";
const std::size_t _aCirconflex_size = _aCirconflex_str.size();
const std::string _eCirconflex_str = "ê";
const std::size_t _eCirconflex_size = _eCirconflex_str.size();
const std::string _iCirconflex_str = "î";
const std::size_t _iCirconflex_size = _iCirconflex_str.size();
const std::string _oCirconflex_str = "ô";
const std::size_t _oCirconflex_size = _oCirconflex_str.size();
const std::string _iTrema_str = "ï";
const std::size_t _iTrema_size = _iTrema_str.size();
const std::string _cCedilla_str = "ç";
const std::size_t _cCedilla_size = _cCedilla_str.size();
const std::string _apos_str = "’";
const std::size_t _apos_size = _apos_str.size();
const std::string _bigSep_str = "–";
const std::size_t _bigSep_size = _bigSep_str.size();
}

static_assert('A' < 'Z', "Wrong assumption: A is not inferior to Z");
static_assert("É"[0] == "À"[0], "Wrong assumption: É & À doesn't begin with same character");
static_assert("É"[0] == "Ê"[0], "Wrong assumption: É & Ê doesn't begin with same character");
static_assert("É"[0] == "Î"[0], "Wrong assumption: É & Î doesn't begin with same character");
static_assert("É"[0] == "Ô"[0], "Wrong assumption: É & Ô doesn't begin with same character");
static_assert("É"[0] == "Ç"[0], "Wrong assumption: É & Ç doesn't begin with same character");
static_assert('a' < 'z', "Wrong assumption: a is not inferior to z");
static_assert("é"[0] == "à"[0], "Wrong assumption: é & à doesn't begin with same character");
static_assert("é"[0] == "è"[0], "Wrong assumption: é & è doesn't begin with same character");
static_assert("é"[0] == "â"[0], "Wrong assumption: é & â doesn't begin with same character");
static_assert("é"[0] == "ê"[0], "Wrong assumption: é & ê doesn't begin with same character");
static_assert("é"[0] == "î"[0], "Wrong assumption: é & î doesn't begin with same character");
static_assert("é"[0] == "ô"[0], "Wrong assumption: é & ô doesn't begin with same character");
static_assert("é"[0] == "ï"[0], "Wrong assumption: é & î doesn't begin with same character");
static_assert("é"[0] == "ç"[0], "Wrong assumption: é & ç doesn't begin with same character");
static_assert("É"[0] == "é"[0], "Wrong assumption: É & é doesn't begin with same character");


std::string urlizeText(const std::string& pText)
{
  std::string res;

  for (std::size_t i = 0; i < pText.size(); )
  {
    if (pText[i] >= 'a' && pText[i] <= 'z')
    {
      res += pText[i++];
      continue;
    }
    if (pText[i] >= '0' && pText[i] <= '9')
    {
      res += pText[i++];
      continue;
    }
    if (pText[i] >= 'A' && pText[i] <= 'Z')
    {
      res += static_cast<char>(pText[i++] + 'a' - 'A');
      continue;
    }
    if (pText[i] == '-' || pText[i] == ' ' || pText[i] == '\'' || pText[i] == '.')
    {
      if (!res.empty() && res[res.size()-1] != '-')
        res += '-';
      ++i;
      continue;
    }
    if (pText.compare(i, _apos_size, "’") == 0)
    {
      res += '-';
      i += _apos_size;
      continue;
    }
    if (pText.compare(i, _bigSep_size, "–") == 0)
    {
      res += '-';
      i += _bigSep_size;
      continue;
    }

    if (pText[i] == "É"[0])
    {
      if (pText.compare(i, _capitalAGrave_size, "À") == 0)
      {
        res += 'a';
        i += _capitalAGrave_size;
        continue;
      }
      if (pText.compare(i, _aGrave_size, "à") == 0)
      {
        res += 'a';
        i += _aGrave_size;
        continue;
      }
      if (pText.compare(i, _capitalEAcute_size, "É") == 0)
      {
        res += 'e';
        i += _capitalEAcute_size;
        continue;
      }
      if (pText.compare(i, _eAcute_size, "é") == 0)
      {
        res += 'e';
        i += _eAcute_size;
        continue;
      }
      if (pText.compare(i, _capitalECirconflex_size, "Ê") == 0)
      {
        res += 'e';
        i += _capitalECirconflex_size;
        continue;
      }
      if (pText.compare(i, _eCirconflex_size, "ê") == 0)
      {
        res += 'e';
        i += _eCirconflex_size;
        continue;
      }
      if (pText.compare(i, _eTrema_size, "ë") == 0)
      {
        res += 'e';
        i += _eTrema_size;
        continue;
      }
      if (pText.compare(i, _capitalCCedilla_size, "Ç") == 0)
      {
        res += 'c';
        i += _capitalCCedilla_size;
        continue;
      }
      if (pText.compare(i, _cCedilla_size, "ç") == 0)
      {
        res += 'c';
        i += _cCedilla_size;
        continue;
      }
      if (pText.compare(i, _capitalICirconflex_size, "Î") == 0)
      {
        res += 'i';
        i += _capitalICirconflex_size;
        continue;
      }
      if (pText.compare(i, _iCirconflex_size, "î") == 0)
      {
        res += 'i';
        i += _iCirconflex_size;
        continue;
      }
      if (pText.compare(i, _iTrema_size, "ï") == 0)
      {
        res += 'i';
        i += _iTrema_size;
        continue;
      }
      if (pText.compare(i, _capitalOCirconflex_size, "Ô") == 0)
      {
        res += 'o';
        i += _capitalOCirconflex_size;
        continue;
      }
      if (pText.compare(i, _oCirconflex_size, "ô") == 0)
      {
        res += 'o';
        i += _oCirconflex_size;
        continue;
      }
      if (pText.compare(i, _aCirconflex_size, "â") == 0)
      {
        res += 'a';
        i += _aCirconflex_size;
        continue;
      }
      if (pText.compare(i, _eGrave_size, "è") == 0)
      {
        res += 'e';
        i += _eGrave_size;
        continue;
      }
    }

    ++i;
  }
  if (!res.empty() && res[res.size()-1] == '-')
    res = res.substr(0, res.size() - 1);
  return res;
}



std::size_t findFirstOf(const std::string& pStr,
                        const std::string& pChars,
                        std::size_t pBegin,
                        std::size_t pEnd)
{
  for (auto i = pBegin; i < pEnd; ++i)
    if (pChars.find_first_of(pStr[i]) != std::string::npos)
      return i;
  return pEnd;
}


void replace_all(std::string& str,
                 const std::string& oldStr,
                 const std::string& newStr)
{
  std::string::size_type pos = 0u;
  while ((pos = str.find(oldStr, pos)) != std::string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}



void split(std::vector<std::string>& pStrs,
           const std::string& pStr,
           const std::string& pSeparator)
{
  std::string::size_type lastPos = 0u;
  std::string::size_type pos = lastPos;
  std::size_t separatorSize = pSeparator.size();
  while ((pos = pStr.find(pSeparator, pos)) != std::string::npos)
  {
    pStrs.emplace_back(pStr.substr(lastPos, pos - lastPos));
    pos += separatorSize;
    lastPos = pos;
  }
  pStrs.emplace_back(pStr.substr(lastPos, pos - lastPos));
}


std::string filenameToSuffix(const std::string& pFileName)
{
  auto i = pFileName.size();
  while (i > 0)
  {
    --i;
    if (pFileName[i] == '.')
    {
      auto pos = i + 1;
      return pFileName.substr(pos, pFileName.size() - pos);
    }
  }
  return "";
}


} // End of namespace mystd
} // End of namespace onsem
