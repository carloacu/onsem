#ifndef ONSEM_TEXTTOSEMANTIC_CONCEPTSET_HPP
#define ONSEM_TEXTTOSEMANTIC_CONCEPTSET_HPP

#include <map>
#include <set>
#include <list>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/relativeperson.hpp>
#include <onsem/common/utility/optional.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>
#include "../../api.hpp"


namespace onsem
{



class ONSEM_TEXTTOSEMANTIC_API ConceptSet
{
public:
  ConceptSet(std::istream& pIStream);

  void addConcept(const std::string& pConceptName);
  void notifyOppositeConcepts(const std::string& pConceptName1,
                              const std::string& pConceptName2);
  void reset();

  void getOppositeConcepts(std::set<std::string>& pOppositeConcepts,
                           const std::string& pConcept) const;
  bool areOppositeConcepts(const std::string& pConcept1,
                           const std::string& pConcept2) const;
  bool haveOppositeConcepts(const std::map<std::string, char>& pConcepts1,
                            const std::map<std::string, char>& pConcepts2) const;
  void conceptToChildConcepts(std::vector<std::string>& pResConcepts,
                              const std::string& pConceptName) const;
  void conceptToNearlyEqualConcepts(std::vector<std::string>& pResConcepts,
                                    const std::string& pConceptName) const;

  // ex: "location_planet_earth" => "location_planet", "location"
  static void conceptToParentConcepts(std::vector<std::string>& pResConcepts,
                                      const std::string& pConceptName);

  static bool cpts1HaveAParentCptOf2(const std::map<std::string, char>& pConcepts1,
                                     const std::map<std::string, char>& pConcepts2);

  const std::unordered_set<std::string>& getLocalConcepts() const
  { return _localConcepts; }
  const std::map<std::string, std::set<std::string>>& getOppositeConcepts() const
  { return _oppositeConcepts; }

  std::size_t getStaticConceptsSize(std::string& pErrorStr,
                                    bool& pIsLoaded) const;

  mystd::optional<bool> areConceptsCompatibles
  (const std::map<std::string, char>& pConcepts1,
   const std::map<std::string, char>& pConcepts2) const;

  static bool haveAConcept
  (const std::map<std::string, char>& pInputConcepts,
   const std::string& pConceptName);

  static bool haveAnyOfConcepts
  (const std::map<std::string, char>& pInputConcepts,
   const std::vector<std::string>& pConceptNames);

  static bool haveAConceptThatBeginWith
  (const std::map<std::string, char>& pInputConcepts,
   const std::string& pBeginOfConceptName);

  static void removeConceptsOrHyponyms
  (std::map<std::string, char>& pInputConcepts,
   const std::string& pConcept);

  static bool haveAConceptOrAHyponym
  (const std::map<std::string, char>& pInputConcepts,
   const std::string& pConcept);

  static bool haveAConceptThatBeginWithAnyOf
  (const std::map<std::string, char>& pInputConcepts,
   const std::vector<std::string>& pExistingConcepts);

  static bool doesConceptBeginWith
  (const std::string& pConceptName,
   const std::string& pBeginOfConceptName);

  static bool doesConceptBeginWithWithAnyOf
  (const std::string& pConceptName,
   const std::vector<std::string>& pBeginOfConcepts);

  static void extractConceptsThatBeginWith
  (std::map<std::string, char>& pExtractedConcepts,
   const std::map<std::string, char>& pInputConcepts,
   const std::string& pBeginOfConceptName);

  static bool extractUserId(std::string& pUserId,
                            const std::map<std::string, char>& pInputConcepts);

  static bool haveAConceptIncompatibleWithSomethingCountable
  (const std::map<std::string, char>& pInputConcepts);

  static char getConfidenceOfConcept
  (const std::map<std::string, char>& pInputConcepts,
   const std::string& pConceptName);

  static bool isAConceptAny(const std::string& pConceptName);
  static bool haveAConceptNotAny(const std::map<std::string, char>& pInputConcepts);

  static void getMotherConceptOfConceptAny(std::string& pConceptName);

  static void sortConceptsSortedByConfidence
  (std::map<char, std::map<std::string, char> >& pSortedConcepts,
   const std::map<std::string, char>& pConcepts);

  static bool haveAConceptParentOf(const std::map<std::string, char>& pConcepts1,
                                   const std::map<std::string, char>& pConcepts2);

  static void sortAndPrintConcepts
  (std::list<std::string>& pPrintedConcepts,
   const std::map<std::string, char>& pConcepts);

  static bool verbSyntesisIsFollowedByAnInfinitiveThatMeansAnImperative(const std::map<std::string, char>& pConcepts);
  static RelativePerson conceptsToRelativePerson(const std::map<std::string, char>& pConcepts);

  static bool rankConceptToNumberStr(std::string& pNumberStr,
                                     const std::map<std::string, char>& pConcepts);

  static const std::string conceptVerbEquality;
  static const std::string conceptAccordanceAgreementOk;

  const StaticConceptSet& statDb;


private:
  std::unordered_set<std::string> _localConcepts;
  std::map<std::string, std::set<std::string>> _oppositeConcepts;

  static std::mutex _pathToStatConceptsMutex;
  static std::unique_ptr<StaticConceptSet> _statConcepts;
  static const StaticConceptSet& _getStatDbInstance(std::istream& pIStream);

  ConceptSet(const ConceptSet&);
  ConceptSet& operator=(const ConceptSet&){ return *this; }
};



inline bool ConceptSet::doesConceptBeginWith
(const std::string& pConceptName,
 const std::string& pBeginOfConceptName)
{
  return pConceptName.compare(0, pBeginOfConceptName.size(),
                              pBeginOfConceptName) == 0;
}

} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_CONCEPTSET_HPP
