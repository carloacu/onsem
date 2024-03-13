#ifndef TRADUCTIONGENERATOR_TRADUCTIONGENRATOR_HPP
#define TRADUCTIONGENERATOR_TRADUCTIONGENRATOR_HPP

#include <string>

namespace onsem {
class LingdbTree;
namespace traductiongenerator {

void run(const LingdbTree& pLingDbTree, const std::string& pTmpFolder, const std::string& pInputResourcePath);

}    // End of namespace traductiongenerator
}    // End of namespace onsem

#endif    // TRADUCTIONGENERATOR_TRADUCTIONGENRATOR_HPP
