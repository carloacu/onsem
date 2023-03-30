#include <onsem/texttosemantic/type/syntacticgraph.hpp>


namespace onsem
{
namespace linguistics
{

unsigned char ParsingConfidence::toPercentage() const
{
  unsigned long res = 0;
  res += nbOfNotUnderstood * 70;
  res += nbOfProblematicRetries * 40;
  res += nbOfSuspiciousChunks * 30;
  if (res > 100)
    return 0;
  return static_cast<unsigned char>(100 - res);
}

} // End of namespace linguistics
} // End of namespace onsem
