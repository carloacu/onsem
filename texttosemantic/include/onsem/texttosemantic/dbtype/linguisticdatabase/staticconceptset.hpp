#ifndef ONSEM_TEXTTOSEMANTIC_STATICCONCEPTSET_HPP
#define ONSEM_TEXTTOSEMANTIC_STATICCONCEPTSET_HPP

#include <list>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/metawordtreedb.hpp>
#include <onsem/common/enum/grammaticaltype.hpp>


namespace onsem
{


class StaticConceptSet : public MetaWordTreeDb
{
public:
  /// Constructor.
  StaticConceptSet(std::istream& pIStream);
  ~StaticConceptSet();

  /**
   * @brief Check is the database is up to date according to the code and
   * to the given version of the database.
   * @param pFilename Filename of the database.
   * @param pCurrentVersion Version that has to have the database.
   * @return True if the database is up to date, False otherwise.
   */
  static bool isUpToDate
  (const std::string& pFilename,
   int pCurrentVersion);

  void getConceptName(std::string& pConceptName,
                      int pConceptId) const;

  std::string conceptName(int pConceptId) const;

  int getConceptToMeaningId(int pConceptId) const;

  bool hasConcept(const std::string& pConceptName) const;
  int getConceptId(const std::string& pConceptName) const;

  void conceptToChildConcepts
  (std::vector<std::string>& pResConcepts,
   const std::string& pConceptName) const;

  void getOppositeConcepts
  (std::set<std::string>& pOppositeConcepts,
   const std::string& pConcept) const;

  void conceptsToNearlyEqualConcepts(std::vector<std::string>& pResConcepts,
                                     const std::string& pConceptName) const;

  bool areConceptsNearlyEqual(const std::string& pConcept1,
                              const std::string& pConcept2) const;

  static const int noConcept = 0;



private:
  union ConceptsDatabaseHeader
  {
    int intValues[3];
    char charValues[12];
  };


  /**
   * @brief Constructor by copy.
   * This function is private because, we don't want to allow copies.
   * @param pDb Other database.
   */
  StaticConceptSet(const StaticConceptSet&);


  /**
   * @brief Copy of an other object.
   * This function is private because, we don't want to allow copies.
   * @param pDb Other database.
   * @return The resulting database.
   */
  StaticConceptSet& operator=
  (const StaticConceptSet&){ return *this; }


  /// Load a binary file in memory.
  void xLoad(std::istream& pIStream);

  /// Deallocate all.
  void xUnload();


  const signed char* xGetConceptPtr(const std::string& pConceptName) const;

  int xNodePtrToNodeId(const signed char* pNode) const;
  const int* xGetPtrOfConceptToMeaningId(const signed char* pNode) const;

  unsigned char xGetNbOfOppositeConcepts(const signed char* pNode) const;
  unsigned char xGetNbOfNearlyEqualConcepts(const signed char* pNode) const;

  const int* xGetFirstOppositeConcept(const signed char* pNode) const;
  const int* xGetFirstNearlyEqualConcept(const signed char* pNode) const;
};



} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_STATICCONCEPTSET_HPP
