#ifndef ONSEM_DATABASEGENERATORPREPROCESSINGGUI_FRGRAMMARBOOKEXTRACTOR_HPP
#define ONSEM_DATABASEGENERATORPREPROCESSINGGUI_FRGRAMMARBOOKEXTRACTOR_HPP

#include <string>

namespace onsem
{
class LingdbTree;
namespace linguistics
{
struct LinguisticDatabase;
}

namespace frgrammarbookextractor
{

void run(const LingdbTree& pLingDbTree,
         const std::string& pShareDbFolder,
         const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace frgrammarbookextractor
} // End of namespace onsem


#endif // ONSEM_DATABASEGENERATORPREPROCESSINGGUI_FRGRAMMARBOOKEXTRACTOR_HPP
