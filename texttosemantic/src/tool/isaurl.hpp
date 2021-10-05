#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOOL_ISAURL_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOOL_ISAURL_HPP

#include <string>

namespace onsem
{

bool isAUrl(const std::string& pStr,
            std::size_t pStrBeginPos = 0);

bool canBeTheExtensionOfAnUrl(const std::string& pStr);


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOOL_ISAURL_HPP
