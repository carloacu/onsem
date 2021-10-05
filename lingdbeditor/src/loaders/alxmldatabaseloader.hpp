#ifndef ALXMLDATABASELOADER_H
#define ALXMLDATABASELOADER_H

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>

namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALCompositePoolAllocator;
class ALLingdbTree;
class ALLingdbMeaning;


/// This class load a database from a xml database file.
class ALXmlDatabaseLoader
{

public:
  /**
   * @brief Get the version stored in a xml database file.
   * @param pFilename Filename of the xml database file.
   * @return The version stored in the xml database file.
   */
  static int getVersion
  (const std::string& pFilename);

  /**
   * @brief Merge a xml database file into a database.
   * @param pFilename Filename of the xml database file.
   * @param pLingDatabase The database to merge.
   * @param pLingdbTree Tree of the databases.
   * @return True if no error, False if an error.
   */
  static void merge
  (const boost::filesystem::path& pFilename,
   LinguisticIntermediaryDatabase& pLingDatabase,
   const ALLingdbTree& pLingdbTree);

private:
  /**
   * @brief Load a xml database.
   * @param pDatabaseXml Folder of the xml database file.
   * @param pFilename Filename of the xml database file.
   * @return True if no error, False if an error.
   */
  /*
  static bool xLoad
  (QDomDocument& pDatabaseXml,
   const boost::filesystem::path& pFilename);
  */

  /**
   * @brief Split a string by each delimiter into a vector.
   * @param pItems The resulting vector.
   * @param pStr The input string.
   * @param pDelim The delimiter.
   */
  static void xSplit
  (std::vector<std::string>& pItems,
   const std::string& pStr,
   char pDelim);

  static ALLingdbMeaning* xExtractMeaning
  (const boost::property_tree::ptree &pTree,
   const LinguisticIntermediaryDatabase& pLingDatabase);
};

} // End of namespace onsem


#endif // ALXMLDATABASELOADER_H
