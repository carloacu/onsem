#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>
#include <onsem/common/binary/binaryloader.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>

namespace onsem
{


StaticConceptSet::StaticConceptSet(std::istream& pIStream)
  : MetaWordTreeDb()
{
  xLoad(pIStream);
}


StaticConceptSet::~StaticConceptSet()
{
  xUnload();
}

bool StaticConceptSet::isUpToDate
(const std::string& pFilename,
 int pCurrentVersion)
{
  std::ifstream binDatabaseFile(pFilename.c_str(), std::ifstream::binary);
  if (!binDatabaseFile.is_open())
  {
    return false;
  }
  ConceptsDatabaseHeader header;
  binDatabaseFile.read(header.charValues, sizeof(ConceptsDatabaseHeader));
  return header.intValues[0] == fFormalism &&
      header.intValues[1] == pCurrentVersion;
}


void StaticConceptSet::xUnload()
{
  binaryloader::deallocMemZone(&fPtrPatriciaTrie);
  fTotalSize = 0;
  fErrorMessage = "NOT_LOADED";
}


void StaticConceptSet::xLoad(std::istream& pIStream)
{
  assert(!xIsLoaded());

  // Read the 3 first int of the file, which correspond to:
  // -> Version of the database
  // -> Offset of the begin of the trie
  // -> Size between the begin of the trie and the end of the file
  ConceptsDatabaseHeader header;
  pIStream.read(header.charValues, sizeof(ConceptsDatabaseHeader));
  if (header.intValues[0] != fFormalism)
  {
    fErrorMessage = "BAD_FORMALISM";
    return;
  }

  fTotalSize = binaryloader::alignedDecToInt(header.intValues[2]);

  // Read all the database and put it in RAM
  if (!binaryloader::allocMemZone(&fPtrPatriciaTrie, pIStream, fTotalSize))
  {
    xUnload();
    fErrorMessage = "BAD_ALLOC";
    return;
  }

  // Close database file
  fErrorMessage = "";
}



// ex: "agent" => "agent_human", "agent_robot", ...
void StaticConceptSet::conceptToChildConcepts
(std::vector<std::string>& pResConcepts,
 const std::string& pConceptName) const
{
  assert(xIsLoaded());
  std::list<const signed char*> conceptsPtrs;

  xGetWordsThatBeginWith(conceptsPtrs, pConceptName + "_");
  std::size_t nbOfCpts = conceptsPtrs.size();
  if (nbOfCpts > 0)
  {
    pResConcepts.resize(nbOfCpts);
    std::size_t idRes = 0;
    for (const auto& cptPtr : conceptsPtrs)
      xGetWord(pResConcepts[idRes++], xNodePtrToNodeId(cptPtr));
  }
}


void StaticConceptSet::getConceptName
(std::string& pConceptName,
 int pConceptId) const
{
  assert(xIsLoaded());
  if (pConceptId == noConcept)
  {
    return;
  }

  xGetWord(pConceptName, pConceptId);
}


std::string StaticConceptSet::conceptName
(int pConceptId) const
{
  assert(xIsLoaded());
  std::string conceptName;
  xGetWord(conceptName, pConceptId);
  return conceptName;
}



int StaticConceptSet::getConceptToMeaningId
(int pConceptId) const
{
  assert(xIsLoaded());
  if (pConceptId == noConcept)
  {
    return LinguisticMeaning_noMeaningId;
  }
  return binaryloader::alignedDecToInt(*xGetPtrOfConceptToMeaningId
                                       (fPtrPatriciaTrie + pConceptId));
}



void StaticConceptSet::getOppositeConcepts
(std::set<std::string>& pOppositeConcepts,
 const std::string& pConcept) const
{
  assert(xIsLoaded());
  int conceptId = getConceptId(pConcept);
  if (conceptId != noConcept)
  {
    auto* endOfNode = fPtrPatriciaTrie + conceptId;
    unsigned char nbOfOppCpts = xGetNbOfOppositeConcepts(endOfNode);
    if (nbOfOppCpts > 0)
    {
      const int* currOppConcept = xGetFirstOppositeConcept(endOfNode);
      for (unsigned char i = 0; i < nbOfOppCpts; ++i, ++currOppConcept)
        pOppositeConcepts.insert(conceptName(binaryloader::alignedDecToInt(*currOppConcept)));
    }
  }
}



void StaticConceptSet::conceptsToNearlyEqualConcepts
(std::vector<std::string>& pResConcepts,
 const std::string& pConceptName) const
{
  assert(xIsLoaded());
  int conceptId = getConceptId(pConceptName);
  if (conceptId != noConcept)
  {
    auto* endOfNode = fPtrPatriciaTrie + conceptId;
    unsigned char nbOfEquCpts = xGetNbOfNearlyEqualConcepts(endOfNode);
    if (nbOfEquCpts > 0)
    {
      pResConcepts.resize(nbOfEquCpts);
      const int* currEquConcept = xGetFirstNearlyEqualConcept(endOfNode);
      for (unsigned char i = 0; i < nbOfEquCpts; ++i, ++currEquConcept)
        xGetWord(pResConcepts[i], binaryloader::alignedDecToInt(*currEquConcept));
    }
  }
}



bool StaticConceptSet::areConceptsNearlyEqual
(const std::string& pConcept1,
 const std::string& pConcept2) const
{
  assert(xIsLoaded());
  int concept1Id = getConceptId(pConcept1);
  if (concept1Id != noConcept)
  {
    auto* endOfNode = fPtrPatriciaTrie + concept1Id;
    unsigned char nbOfEquCpts = xGetNbOfNearlyEqualConcepts(endOfNode);
    if (nbOfEquCpts > 0)
    {
      const int* currEquConcept = xGetFirstNearlyEqualConcept(endOfNode);
      for (unsigned char i = 0; i < nbOfEquCpts; ++i, ++currEquConcept)
        if (conceptName(binaryloader::alignedDecToInt(*currEquConcept)) == pConcept2)
          return true;
    }
  }
  return false;
}


bool StaticConceptSet::hasConcept(const std::string& pConceptName) const
{
  return xGetConceptPtr(pConceptName) != nullptr;
}

int StaticConceptSet::getConceptId
(const std::string& pConceptName) const
{
  assert(xIsLoaded());
  auto* conceptPtr = xGetConceptPtr(pConceptName);
  if (conceptPtr != nullptr)
  {
    return xNodePtrToNodeId(conceptPtr);
  }
  return noConcept;
}


const signed char* StaticConceptSet::xGetConceptPtr(const std::string& pConceptName) const
{
  return xGetNode(pConceptName, 0, pConceptName.size(), false);
}


int StaticConceptSet::xNodePtrToNodeId
(const signed char* pNode) const
{
  if (pNode == nullptr)
  {
    return noConcept;
  }
  assert((pNode - fPtrPatriciaTrie) % 4 == 0);
  return binaryloader::alignedDecToInt(static_cast<int>(pNode - fPtrPatriciaTrie) / 4);
}



const int* StaticConceptSet::xGetPtrOfConceptToMeaningId
(const signed char* pNode) const
{
  return xGetBeginOfEndingStruct(pNode);
}


unsigned char StaticConceptSet::xGetNbOfOppositeConcepts
(const signed char* pNode) const
{
  return *(reinterpret_cast<const signed char*>
           (xGetPtrOfConceptToMeaningId(pNode) + 1));
}

unsigned char StaticConceptSet::xGetNbOfNearlyEqualConcepts
(const signed char* pNode) const
{
  return *(reinterpret_cast<const signed char*>
           (xGetPtrOfConceptToMeaningId(pNode) + 1) + 1);
}

const int* StaticConceptSet::xGetFirstOppositeConcept
(const signed char* pNode) const
{
  return xGetPtrOfConceptToMeaningId(pNode) + 2;
}

const int* StaticConceptSet::xGetFirstNearlyEqualConcept
(const signed char* pNode) const
{
  return xGetPtrOfConceptToMeaningId(pNode) + 2 +
      xGetNbOfOppositeConcepts(pNode);
}


} // End of namespace onsem
