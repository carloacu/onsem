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


} // End of namespace onsem
