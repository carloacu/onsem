#ifndef ONSEM_COMMON_IO_SAVEMODEL_HPP
#define ONSEM_COMMON_IO_SAVEMODEL_HPP

#include <boost/property_tree/ptree.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemory;
struct SemanticExpression;

namespace serializationprivate
{

void saveSemExp(boost::property_tree::ptree& pTree,
                const SemanticExpression& pSemExp);

void saveSemMemory(boost::property_tree::ptree& pTree,
                   const SemanticMemory& pSemMemory);

void saveLingDatabase(boost::property_tree::ptree& pTree,
                      const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace serializationprivate
} // End of namespace onsem

#endif // ONSEM_COMMON_IO_SAVEMODEL_HPP
