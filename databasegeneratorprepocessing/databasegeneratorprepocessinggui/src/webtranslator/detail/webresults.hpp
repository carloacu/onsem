#ifndef WEBTRANSLATOR_WEBRESULTS_H
#define WEBTRANSLATOR_WEBRESULTS_H

#include <string>

namespace onsem
{
class LingdbTree;


void process_webresults(const LingdbTree& pLingDbTree,
                        const std::string& pTmpFolder,
                        const std::string &pShareDbFolder);


} // End of namespace onsem

#endif // WEBTRANSLATOR_WEBRESULTS_H
