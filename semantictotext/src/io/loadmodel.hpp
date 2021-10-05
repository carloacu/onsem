#ifndef ONSEM_COMMON_IO_LOADMODEL_HPP
#define ONSEM_COMMON_IO_LOADMODEL_HPP

#include <boost/property_tree/ptree.hpp>
#include <memory>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;
struct UniqueSemanticExpression;

namespace serializationprivate
{

UniqueSemanticExpression loadSemExp(const boost::property_tree::ptree& pTree);

void loadSemMemory(const boost::property_tree::ptree& pTree,
                   SemanticMemory& pSemMemory,
                   const linguistics::LinguisticDatabase& pLingDb);

void loadLingDatabase(const boost::property_tree::ptree& pTree,
                      linguistics::LinguisticDatabase& pLingDb);

} // End of namespace serializationprivate
} // End of namespace onsem

#endif // ONSEM_COMMON_IO_LOADMODEL_HPP
