#include <onsem/common/utility/uppercasehandler.hpp>

namespace onsem
{

namespace
{

std::size_t _nbOfCharactersLoweredForTheFirstLetter
(std::string& pStrToModify,
 std::size_t pPos = 0)
{
  if (pPos >= pStrToModify.size())
    return 0;

  if (pStrToModify[pPos] >= 'A' && pStrToModify[pPos] <= 'Z')
  {
    pStrToModify[pPos] = static_cast<char>(pStrToModify[pPos] + 'a' - 'A');
    return 1;
  }
  switch (pStrToModify[pPos])
  {
  case "É"[0]:
  {
    if (pStrToModify.compare(pPos, _capitalAGrave_size, "À") == 0)
    {
      pStrToModify.replace(pPos, _capitalAGrave_size, "à");
      return _capitalAGrave_size;
    }
    if (pStrToModify.compare(pPos, _capitalEAcute_size, "É") == 0)
    {
      pStrToModify.replace(pPos, _capitalEAcute_size, "é");
      return _capitalEAcute_size;
    }
    if (pStrToModify.compare(pPos, _capitalECirconflex_size, "Ê") == 0)
    {
      pStrToModify.replace(pPos, _capitalECirconflex_size, "ê");
      return _capitalECirconflex_size;
    }
    if (pStrToModify.compare(pPos, _capitalCCedilla_size, "Ç") == 0)
    {
      pStrToModify.replace(pPos, _capitalCCedilla_size, "ç");
      return _capitalCCedilla_size;
    }
    if (pStrToModify.compare(pPos, _capitalOTrema_size, "Ö") == 0)
    {
      pStrToModify.replace(pPos, _capitalOTrema_size, "ö");
      return _capitalOTrema_size;
    }
    break;
  }
  case "Ş"[0]:
  {
    if (pStrToModify.compare(pPos, _capitalSCedilla_size, "Ş") == 0)
    {
      pStrToModify.replace(pPos, _capitalSCedilla_size, "ș");
      return _capitalSCedilla_size;
    }
    break;
  }
  }
  return 0;
}


bool _isLowerCaseOf
(const std::string& pStr1,
 std::size_t& pPos1,
 const std::string& pStr2,
 std::size_t& pPos2)
{
  if (pStr1[pPos1] >= 'A' && pStr1[pPos1] <= 'Z')
  {
    if (pStr2[pPos2] == static_cast<char>(pStr1[pPos1] + 'a' - 'A'))
    {
      ++pPos1;
      ++pPos2;
      return true;
    }
    return false;
  }
  switch (pStr1[pPos1])
  {
  case "É"[0]:
  {
    if (pStr1.compare(pPos1, _capitalAGrave_size, "À") == 0)
    {
      if (pStr2.compare(pPos2, _aGrave_size, "à") == 0)
      {
        pPos1 += _capitalAGrave_size;
        pPos2 += _aGrave_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _capitalEAcute_size, "É") == 0)
    {
      if (pStr2.compare(pPos2, _eAcute_size, "é") == 0)
      {
        pPos1 += _capitalEAcute_size;
        pPos2 += _eAcute_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _capitalECirconflex_size, "Ê") == 0)
    {
      if (pStr2.compare(pPos2, _eCirconflex_size, "ê") == 0)
      {
        pPos1 += _capitalECirconflex_size;
        pPos2 += _eCirconflex_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _capitalCCedilla_size, "Ç") == 0)
    {
      if (pStr2.compare(pPos2, _cCedilla_size, "ç") == 0)
      {
        pPos1 += _capitalCCedilla_size;
        pPos2 += _cCedilla_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _capitalOTrema_size, "Ö") == 0)
    {
      if (pStr2.compare(pPos2, _oTrema_size, "ö") == 0)
      {
        pPos1 += _capitalOTrema_size;
        pPos2 += _oTrema_size;
        return true;
      }
    }
    break;
  }
  case "Ş"[0]:
  {
    if (pStr1.compare(pPos1, _capitalSCedilla_size, "Ş") == 0)
    {
      if (pStr2.compare(pPos2, _sCedilla_size, "ș") == 0)
      {
        pPos1 += _capitalSCedilla_size;
        pPos2 += _sCedilla_size;
        return true;
      }
    }
    break;
  }
  }
  return false;
}


bool _isUpperCaseOf
(const std::string& pStr1,
 std::size_t& pPos1,
 const std::string& pStr2,
 std::size_t& pPos2)
{
  if (pStr1[pPos1] >= 'a' && pStr1[pPos1] <= 'z')
  {
    if (pStr2[pPos2] == static_cast<char>(pStr1[pPos1] - ('a' - 'A')))
    {
      ++pPos1;
      ++pPos2;
      return true;
    }
    return false;
  }
  switch (pStr1[pPos1])
  {
  case "é"[0]:
  {
    if (pStr1.compare(pPos1, _aGrave_size, "à") == 0)
    {
      if (pStr2.compare(pPos2, _capitalAGrave_size, "À") == 0)
      {
        pPos1 += _aGrave_size;
        pPos2 += _capitalAGrave_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _eAcute_size, "é") == 0)
    {
      if (pStr2.compare(pPos2, _capitalEAcute_size, "É") == 0)
      {
        pPos1 += _eAcute_size;
        pPos2 += _capitalEAcute_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _eCirconflex_size, "ê") == 0)
    {
      if (pStr2.compare(pPos2, _capitalECirconflex_size, "Ê") == 0)
      {
        pPos1 += _eCirconflex_size;
        pPos2 += _capitalECirconflex_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _cCedilla_size, "ç") == 0)
    {
      if (pStr2.compare(pPos2, _capitalCCedilla_size, "Ç") == 0)
      {
        pPos1 += _cCedilla_size;
        pPos2 += _capitalCCedilla_size;
        return true;
      }
    }
    if (pStr1.compare(pPos1, _oTrema_size, "ö") == 0)
    {
      if (pStr2.compare(pPos2, _capitalOTrema_size, "Ö") == 0)
      {
        pPos1 += _oTrema_size;
        pPos2 += _capitalOTrema_size;
        return true;
      }
    }
    break;
  }
  case "ș"[0]:
  {
    if (pStr1.compare(pPos1, _sCedilla_size, "ș") == 0)
    {
      if (pStr2.compare(pPos2, _capitalSCedilla_size, "Ş") == 0)
      {
        pPos1 += _sCedilla_size;
        pPos2 += _capitalSCedilla_size;
        return true;
      }
    }
    break;
  }
  }
  return false;
}


}


bool lowerCaseFirstLetter(std::string& pStrToModify,
                          std::size_t pPos)
{
  return _nbOfCharactersLoweredForTheFirstLetter(pStrToModify, pPos) > 0;
}


bool lowerCaseText(std::string& pStrToModify,
                   std::size_t pPos)
{
  bool res = false;
  while (pPos < pStrToModify.size())
  {
    auto newPos = _nbOfCharactersLoweredForTheFirstLetter(pStrToModify, pPos);
    if (newPos > 0)
    {
      pPos += newPos;
      res = true;
    }
    else
    {
      ++pPos;
    }
  }
  return res;
}


bool areTextEqualWithoutCaseSensitivity(const std::string& pText1,
                                        const std::string& pText2)
{
  if (pText1.size() != pText2.size())
    return false;
  for (std::size_t i = 0; i < pText1.size(); ++i)
  {
    if (pText1[i] == pText2[i])
    {
      ++i;
      continue;
    }
    if (_isLowerCaseOf(pText1, i, pText2, i) ||
        _isUpperCaseOf(pText1, i, pText2, i))
    {
      continue;
    }
    return false;
  }
  return true;
}



} // End of namespace onsem
