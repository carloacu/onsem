#ifndef ALANYDATABASELOADER_H
#define ALANYDATABASELOADER_H

#include <string>
#include <boost/filesystem/path.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALLingdbTree;


class ALAnyDatabaseLoader
{
public:
  void open
  (const boost::filesystem::path& pFilename,
   const boost::filesystem::path& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pLingdb,
   const ALLingdbTree& pLingdbTree,
   std::size_t pImbricationLevel = 0) const;

  void mergeWith
  (const boost::filesystem::path& pFilename,
   const boost::filesystem::path& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pLingdb,
   const ALLingdbTree& pLingdbTree,
   std::size_t pImbricationLevel = 0) const;

  void exec
  (const boost::filesystem::path& pFilename,
   const ALLingdbTree& pLingdbTree) const;
};



} // End of namespace onsem

#endif // ALANYDATABASELOADER_H
