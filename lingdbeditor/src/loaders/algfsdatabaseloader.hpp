#ifndef ALGFSDATABASELOADER_H
#define ALGFSDATABASELOADER_H

#include <string>
#include <boost/filesystem/path.hpp>

namespace onsem
{
class LinguisticIntermediaryDatabase;

/// This class load a database from a dela dictionnary file.
class ALGFSDatabaseLoader
{
public:

  static void merge
  (const boost::filesystem::path& pFilename,
   LinguisticIntermediaryDatabase& pLingdb);

};



} // End of namespace onsem

#endif // ALGFSDATABASELOADER_H
