#ifndef ALLINGDBTREE_HXX
#define ALLINGDBTREE_HXX

#include "../allingdbtree.hpp"

namespace onsem
{


inline std::string ALLingdbTree::getExtDynDatabase
() const
{
  if (fIn32Bits)
  {
    return "ddb32";
  }
  return "ddb64";
}



inline std::string ALLingdbTree::getExtXmlDatabase() const
{
  return "xml";
}


inline std::string ALLingdbTree::getExtDelaDatabase
() const
{
  return "dela";
}



inline std::string ALLingdbTree::getExtGfsDatabase
() const
{
  return "gfs";
}



inline std::string ALLingdbTree::getExtBinaryDatabase
() const
{
  return "bdb";
}


inline std::string ALLingdbTree::getRlaDatabase
() const
{
  return "rla";
}


inline std::string ALLingdbTree::getCptsDatabase
() const
{
  return "cpts";
}



inline std::string ALLingdbTree::getWlksDatabase
() const
{
  return "wlks";
}



inline std::string ALLingdbTree::getDynamicDatabasesFolder() const
{
  return fDynamicDatabasesFolder;
}



} // End of namespace onsem

#endif // ALLINGDBTREE_HXX
