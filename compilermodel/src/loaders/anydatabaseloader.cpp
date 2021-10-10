#include <onsem/compilermodel/loaders/anydatabaseloader.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/loaders/deladatabaseloader.hpp>
#include <onsem/compilermodel/loaders/wlksdatabaseloader.hpp>
#include "xmldatabaseloader.hpp"
#include "rladatabaseloader.hpp"
#include "gfsdatabaseloader.hpp"
#include "cptsdatabaseloader.hpp"


namespace onsem
{


void AnyDatabaseLoader::open
(const std::string& pFilename,
 const std::string& pInputResourcesDir,
 LinguisticIntermediaryDatabase& pCurrLingdb,
 const LingdbTree& pLingdbTree,
 std::size_t pImbricationLevel) const
{
  std::string suffixStr = mystd::filenameToSuffix(pFilename);

  if (suffixStr == pLingdbTree.getExtDynDatabase())
  {
    pCurrLingdb.load(pFilename);
  }
  else if (suffixStr == pLingdbTree.getExtXmlDatabase())
  {
    pCurrLingdb.clear();
    XmlDatabaseLoader::merge(pFilename, pCurrLingdb, pLingdbTree);
  }
  else if (suffixStr == pLingdbTree.getExtDelaDatabase())
  {
    pCurrLingdb.clear();
    DelaDatabaseLoader ddl;
    ddl.merge(pFilename, pCurrLingdb);
  }
  else if (suffixStr == pLingdbTree.getRlaDatabase())
  {
    pCurrLingdb.clear();
    RlaDatabaseLoader::merge(pFilename, pInputResourcesDir, pCurrLingdb,
                               pLingdbTree, *this, pImbricationLevel);
  }
  else if (suffixStr == pLingdbTree.getCptsDatabase())
  {
    pCurrLingdb.clear();
    CptsDatabaseLoader cptsLoader;
    cptsLoader.merge(pFilename, pCurrLingdb);
  }
}


void AnyDatabaseLoader::mergeWith
(const std::string& pFilename,
 const std::string& pInputResourcesDir,
 LinguisticIntermediaryDatabase& pCurrLingdb,
 const LingdbTree& pLingdbTree,
 std::size_t pImbricationLevel) const
{
  std::string suffixStr = mystd::filenameToSuffix(pFilename);

  if (suffixStr == pLingdbTree.getExtXmlDatabase())
  {
    XmlDatabaseLoader::merge(pFilename, pCurrLingdb, pLingdbTree);
  }
  else if (suffixStr == pLingdbTree.getExtDelaDatabase())
  {
    DelaDatabaseLoader ddl;
    ddl.merge(pFilename, pCurrLingdb);
  }
  else if (suffixStr == pLingdbTree.getExtGfsDatabase())
  {
    GFSDatabaseLoader::merge(pFilename, pCurrLingdb);
  }
  else if (suffixStr == pLingdbTree.getRlaDatabase())
  {
    RlaDatabaseLoader::merge(pFilename, pInputResourcesDir, pCurrLingdb,
                               pLingdbTree, *this, pImbricationLevel);
  }
  else if (suffixStr == pLingdbTree.getCptsDatabase())
  {
    CptsDatabaseLoader cptsLoader;
    cptsLoader.merge(pFilename, pCurrLingdb);
  }
}


void AnyDatabaseLoader::exec
(const std::string& pFilename,
 const LingdbTree& pLingdbTree) const
{
  std::string suffixStr = mystd::filenameToSuffix(pFilename);

  if (suffixStr == pLingdbTree.getWlksDatabase())
  {
    WlksDatabaseLoader wlksLoader;
    wlksLoader.loadAndSave(pFilename, pLingdbTree);
    return;
  }
  throw std::runtime_error("Cannot exec file: \"" + pFilename + "\".");
}


} // End of namespace onsem
