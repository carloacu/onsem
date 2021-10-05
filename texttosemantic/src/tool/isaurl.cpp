#include "isaurl.hpp"


namespace onsem
{

bool isAUrl(const std::string& pStr,
            std::size_t pStrBeginPos)
{
  if (pStr.size() <= pStrBeginPos)
    return false;
  if (pStr.compare(pStrBeginPos, 4, "http") == 0)
    return true;
  return false;
}


/**
 * TODO: There are so many more!
 * What actually makes an URI (and not just URLs) is the : character preceded with a single word (sequence of unreserved characters unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"). See https://datatracker.ietf.org/doc/html/rfc3986#page-6
 * Until you hit a space character, you can safely say it is a URI, regardless that extension, which is utterly specific to the web.
 */
bool canBeTheExtensionOfAnUrl(const std::string& pStr)
{
  if (pStr.empty())
    return false;
  if (pStr.compare(0, 2, "fr") == 0 ||
      pStr.compare(0, 2, "ai") == 0 ||
      pStr.compare(0, 2, "uk") == 0 ||
      pStr.compare(0, 3, "com") == 0 ||
      pStr.compare(0, 3, "lan") == 0)
    return true;
  return false;
}



} // End of namespace onsem
