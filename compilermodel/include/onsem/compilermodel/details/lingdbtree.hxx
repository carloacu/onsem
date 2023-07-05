#ifndef ONSEM_COMPILERMODEL_LINGDBTREE_HXX
#define ONSEM_COMPILERMODEL_LINGDBTREE_HXX

#include "../lingdbtree.hpp"

namespace onsem
{


inline std::string LingdbTree::getExtDynDatabase
() const
{
  if (fIn32Bits)
  {
    return "ddb32";
  }
  return "ddb64";
}



inline std::string LingdbTree::getExtXmlDatabase() const
{
  return "xml";
}


inline std::string LingdbTree::getExtDelaDatabase
() const
{
  return "dela";
}

inline std::string LingdbTree::getExtOmld() const
{
  return "omld";
}


inline std::string LingdbTree::getExtGfsDatabase
() const
{
  return "gfs";
}



inline std::string LingdbTree::getExtBinaryDatabase
() const
{
  return "bdb";
}


inline std::string LingdbTree::getRlaDatabase
() const
{
  return "rla";
}


inline std::string LingdbTree::getCptsDatabase
() const
{
  return "cpts";
}



inline std::string LingdbTree::getWlksDatabase
() const
{
  return "wlks";
}



inline std::string LingdbTree::getDynamicDatabasesFolder() const
{
  return fDynamicDatabasesFolder;
}



} // End of namespace onsem

#endif // ONSEM_COMPILERMODEL_LINGDBTREE_HXX
