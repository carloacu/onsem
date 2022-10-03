#ifndef FRGRAMMARBOOKEXTRACTOR_HELPERS_H
#define FRGRAMMARBOOKEXTRACTOR_HELPERS_H

#include <set>
#include <list>
#include <string>
#include <boost/optional.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/common/enum/chunklinktype.hpp>


namespace onsem
{
class LingdbTree;
namespace linguistics
{
class StaticLinguisticDictionary;
}

static const std::string arrowStr = "►";
static const std::string pageSeparatorStr = "-------";


#define FRGRAMMARBOOK_OBJECTTYPE_TABLE                              \
  ADD_FRGRAMMARBOOK_OBJECTTYPE(OBJECTTYPE_AGENT, "agent")           \
  ADD_FRGRAMMARBOOK_OBJECTTYPE(OBJECTTYPE_LOCATION, "location")     \
  ADD_FRGRAMMARBOOK_OBJECTTYPE(OBJECTTYPE_OBJECT, "object")

#define ADD_FRGRAMMARBOOK_OBJECTTYPE(a, b) a,
enum FrGrammarBookObjectType
{
  FRGRAMMARBOOK_OBJECTTYPE_TABLE
  FRGRAMMARBOOK_OBJECTTYPE_TABLE_ENDFORNOCOMPILWARNING
};
#undef ADD_FRGRAMMARBOOK_OBJECTTYPE

#define ADD_FRGRAMMARBOOK_OBJECTTYPE(a, b) b,
static std::string FrGrammarBookObjectType_toStr[] = {
  FRGRAMMARBOOK_OBJECTTYPE_TABLE
};
#undef ADD_FRGRAMMARBOOK_OBJECTTYPE


class FrGrammarBookExtHelpers
{
public:
  FrGrammarBookExtHelpers
  (const linguistics::StaticLinguisticDictionary& pBinDico,
   const LingdbTree &pLingDbTree);

  static void removeAllSpacesAndSplit
  (std::list<std::string>& pRes,
   const std::string& pStr);

  static void replaceSomeCharsToBlanck
  (std::string& pStr);

  static void removeAfterDoublePoint
  (std::string& pStr);

  static void removeBackSlashRAtEnd
  (std::string& pStr);

  static void toLowerCase
  (std::string& pStr);

  static void removeArrowAtBeginning
  (std::string& pStr);

  static bool removeBlanksAtBeginning
  (std::string& pStr,
   std::size_t pMaxNbToRemove);

  bool findVerb
  (std::list<std::string>& pVerbs,
   const std::string& pLine) const;


  struct PrepToObjectType
  {
    PrepToObjectType
    (const std::string& pPerp,
     FrGrammarBookObjectType pObjType)
      : prep(pPerp),
        objType(pObjType)
    {
    }

    bool operator<
    (const PrepToObjectType& pOther) const
    {
      if (prep != pOther.prep)
      {
        return prep < pOther.prep;
      }
      return objType < pOther.objType;
    }

    std::string prep;
    FrGrammarBookObjectType objType;
  };

  void extractPrepsToObjectType
  (std::set<PrepToObjectType>& pPrepsToObjectType,
   const std::string& pStr) const;


  struct VerbToPreps
  {
    VerbToPreps()
      : currVerbs(),
        prepsToObjectType()
    {
    }

    std::list<std::string> currVerbs;
    std::set<FrGrammarBookExtHelpers::PrepToObjectType> prepsToObjectType;
  };

  static void removeVerbsThatOccur2Times
  (std::list<VerbToPreps>& pVerbToPreps);


  void writeMostFrequentAfterPrep
  (std::ofstream& pInfosFile,
   const std::list<FrGrammarBookExtHelpers::VerbToPreps>& pVbToPrepsList);

  void writeVerbsPrepsXmlFile
  (const std::string& pVerbsPrepsXmlFilename,
   const std::list<FrGrammarBookExtHelpers::VerbToPreps>& pVbToPrepsList) const;


private:
  struct StrSortedBySize
  {
    StrSortedBySize()
      : str()
    {
    }

    explicit StrSortedBySize
    (const std::string& pStr)
      : str(pStr)
    {
    }

    bool operator<
    (const StrSortedBySize& pOther) const
    {
      if (str.size() != pOther.str.size())
      {
        return str.size() > pOther.str.size();
      }
      return str < pOther.str;
    }

    std::string str;
  };

  const linguistics::StaticLinguisticDictionary& fBinDico;
  SemanticWord fSePronCompWord;
  const std::string fQuelqueChoseStr;
  const std::string fQuelquePartStr;
  const std::string fQuelquUnStr;
  std::map<StrSortedBySize, boost::optional<FrGrammarBookObjectType> > fPreps;
  std::map<std::string, linguistics::ChunkLinkType> fPrepToChunkLinkType;

  bool xTryToDetectAnyObjectType
  (std::set<PrepToObjectType>& pPrepsToObjectType,
   std::list<std::string>& pCurrPreps,
   std::size_t& pStrPos,
   const std::string& pStr) const;

  bool xTryToDetectObjectType
  (std::set<PrepToObjectType>& pPrepsToObjectType,
   std::list<std::string>& pCurrPreps,
   std::size_t& pStrPos,
   const std::string& pStr,
   const std::string& pObjectStr,
   FrGrammarBookObjectType pObjectType) const;

  bool xTryToDetectPrepsToAnObjectType
  (std::set<PrepToObjectType>& pPrepsToObjectType,
   std::size_t& pStrPos,
   const std::string& pStr,
   FrGrammarBookObjectType pObjectType) const;

  bool xTryToDetectAPrep
  (std::list<std::string>& pCurrPreps,
   std::size_t& pStrPos,
   const std::string& pStr) const;

  bool xTryToAddVerb
  (std::list<std::string>& pVerbs,
   const std::string& pLine,
   bool pIsAPronominalVerb) const;

  void xWritePrepStats
  (std::ofstream& pInfosFile,
   const std::string& pPrep,
   const std::map<FrGrammarBookObjectType, int>& pStats);

  static bool xHas
  (const std::list<std::string>& pList,
   const std::string& pStr);
};


} // End of namespace onsem

#endif // FRGRAMMARBOOKEXTRACTOR_HELPERS_H
