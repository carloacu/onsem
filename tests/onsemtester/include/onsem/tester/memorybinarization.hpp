#ifndef ONSEM_TESTER_MEMORYBINARIZATION_HPP
#define ONSEM_TESTER_MEMORYBINARIZATION_HPP

#include "api.hpp"


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticMemoryBlock;


ONSEMTESTER_API
bool checkMemBlocBinarization(const SemanticMemoryBlock& pMemBloc,
                              const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem

#endif // ONSEM_TESTER_MEMORYBINARIZATION_HPP
