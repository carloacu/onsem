#ifndef ONSEM_COMPILERMODEL_LOADERS_ANYDATABASELOADER_HPP
#define ONSEM_COMPILERMODEL_LOADERS_ANYDATABASELOADER_HPP

#include <string>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class LingdbTree;


class AnyDatabaseLoader
{
public:
  void open
  (const std::string& pFilename,
   const std::string& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pLingdb,
   const LingdbTree& pLingdbTree,
   std::size_t pImbricationLevel = 0) const;

  void mergeWith
  (const std::string& pFilename,
   const std::string& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pLingdb,
   const LingdbTree& pLingdbTree,
   std::size_t pImbricationLevel = 0) const;

  void exec
  (const std::string& pFilename,
   const LingdbTree& pLingdbTree) const;
};



} // End of namespace onsem

#endif // ONSEM_COMPILERMODEL_LOADERS_ANYDATABASELOADER_HPP
