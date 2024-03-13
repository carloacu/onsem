#ifndef WEBTRANSLATOR_WEBTRANSLATOR_HPP
#define WEBTRANSLATOR_WEBTRANSLATOR_HPP

#include <string>

namespace onsem {
class LingdbTree;
namespace webTranslator {

void run(const LingdbTree& pLingDbTree,
         const std::string& pTmpFolder,
         const std::string& pShareDbFolder,
         const std::string& pInputResourcesDir);

}    // End of namespace webTranslator
}    // End of namespace onsem

#endif    // WEBTRANSLATOR_WEBTRANSLATOR_HPP
