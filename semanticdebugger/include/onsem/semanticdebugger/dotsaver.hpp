#ifndef ONSEM_SEMANTICDEBUGGER_DOTSAVER_HPP
#define ONSEM_SEMANTICDEBUGGER_DOTSAVER_HPP

#include <list>
#include <string>
#include "api.hpp"

namespace onsem
{
namespace linguistics
{
struct ChunkLink;

namespace DotSaver
{

ONSEMSEMANTICDEBUGGER_API
void writeChunkLinks(std::ostream& pSs,
                     const std::list<ChunkLink>& pSyntTree);

ONSEMSEMANTICDEBUGGER_API
/**
 * @brief save Generate a .png image corresponding to the dot content.
 * Note: It will also generate a .dot file.
 * @param pInFilename Filename of the intermediary .dot file that will be generated.
 * @param pOutFilename Filename of the .png file.
 * @param pDotContent String to write into the .dot intermediary file.
 */
void save(const std::string& pInFilename,
          const std::string& pOutFilename,
          const std::string& pDotContent);


} // End of namespace DotSaver


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_SEMANTICDEBUGGER_DOTSAVER_HPP
