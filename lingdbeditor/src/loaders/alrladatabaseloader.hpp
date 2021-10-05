#ifndef ALRLADATABASELOADER_H
#define ALRLADATABASELOADER_H

#include <string>
#include <boost/filesystem/path.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALLingdbTree;
class ALAnyDatabaseLoader;


class ALRlaDatabaseLoader
{
public:
  static void merge
  (const boost::filesystem::path& pFilename,
   const boost::filesystem::path& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pCurrLingdb,
   const ALLingdbTree& pLingdbTree,
   const ALAnyDatabaseLoader& pAnyLoader,
   std::size_t pImbricationLevel);

private:
  static boost::filesystem::path _getPath
  (const std::string& pLine,
   const std::string& pInstruction,
   const boost::filesystem::path& pHoldingFolder,
   const boost::filesystem::path& pInputResourcesDir);
};



} // End of namespace onsem

#endif // ALRLADATABASELOADER_H
