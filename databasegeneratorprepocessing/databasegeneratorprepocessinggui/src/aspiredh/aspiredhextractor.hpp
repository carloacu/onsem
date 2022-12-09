#ifndef ASPIREDH_ASPIREDHEXTRACTOR_HPP
#define ASPIREDH_ASPIREDHEXTRACTOR_HPP

#include <string>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
namespace aspiredHExtractor
{

void run(const std::string& pMyDataMiningPath,
         const std::string& pInputResourcesFolder,
         const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace aspiredHExtractor
} // End of namespace onsem


#endif // ASPIREDH_ASPIREDHEXTRACTOR_HPP
