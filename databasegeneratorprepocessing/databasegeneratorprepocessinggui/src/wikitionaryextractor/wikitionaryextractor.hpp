#ifndef WIKITIONARYEXTRACTOR_WIKITIONARYEXTRACTOR_HPP
#define WIKITIONARYEXTRACTOR_WIKITIONARYEXTRACTOR_HPP

#include <string>


namespace onsem
{
class LingdbTree;


namespace wikitionaryExtractor
{

void runTraductions(const LingdbTree& pLingDbTree,
                    const std::string& pMyDataMiningPath,
                    const std::string& pTmpFolder);

void addComposedWords(const LingdbTree& pLingDbTree,
                      const std::string& pMyDataMiningPath,
                      const std::string& pInputResourcePath);


void addTransitiveVerbs(const LingdbTree& pLingDbTree,
                        const std::string& pMyDataMiningPath,
                        const std::string& pInputResourcePath);


void addConcepts(const LingdbTree& pLingDbTree,
                 const std::string& pMyDataMiningPath,
                 const std::string& pInputResourcePath);

} // End of namespace wikitionaryExtractor

} // End of namespace onsem



#endif // WIKITIONARYEXTRACTOR_WIKITIONARYEXTRACTOR_HPP
