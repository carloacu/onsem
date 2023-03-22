#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_DETAIL_SEMTREECONVERSIONDATABASE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_DETAIL_SEMTREECONVERSIONDATABASE_HPP

#include <map>
#include <set>
#include <sstream>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/enum/semanticentitytype.hpp>
#include <onsem/common/enum/treepatternconventionenum.hpp>
#include <onsem/common/enum/semanticrelativetimetype.hpp>
#include <onsem/common/keytostreams.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/virtualsembinarydatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include "../../../api.hpp"


namespace onsem
{

class ONSEM_TEXTTOSEMANTIC_API SemExpTreeConversionDatabase : public VirtualSemBinaryDatabase
{
public:
  virtual ~SemExpTreeConversionDatabase() {}


protected:
  struct SemExpTreePatternNode
  {
    SemExpTreePatternNode()
      : id(),
        groundingType(),
        concepts(),
        notConcepts(),
        beginOfConcepts(),
        conceptsOrHyponyms(),
        notConceptsOrHyponyms(),
        nb(),
        requests(),
        notRequests(),
        time(),
        type(),
        notTypes(),
        reference(),
        word(),
        removeWord(),
        timeType(),
        hasToBeCompletedFromContext(),
        children(),
        notChildren()
    {
    }

    std::string id;
    mystd::optional<SemanticGroundingType> groundingType;
    std::map<std::string, std::list<char> > concepts;
    std::set<std::string> notConcepts;
    std::set<std::string> beginOfConcepts;
    std::set<std::string> conceptsOrHyponyms;
    std::set<std::string> notConceptsOrHyponyms;
    mystd::optional<int> nb;
    SemanticRequests requests;
    SemanticRequests notRequests;
    mystd::optional<SemanticVerbTense> time;
    mystd::optional<SemanticEntityType> type;
    std::set<SemanticEntityType> notTypes;
    mystd::optional<SemanticReferenceType> reference;
    mystd::optional<SemanticWord> word;
    mystd::optional<bool> removeWord;
    mystd::optional<SemanticRelativeTimeType> timeType;
    mystd::optional<bool> hasToBeCompletedFromContext;
    std::map<GrammaticalType, SemExpTreePatternNode> children;
    std::set<GrammaticalType> notChildren;
  };
  struct ConversionRule
  {
    ConversionRule
    (const std::string& pFilename,
     int pConvesionNb,
     int pPriorityOfOutPattern,
     const std::shared_ptr<SemExpTreePatternNode>& pTreePatternIn,
     const std::shared_ptr<SemExpTreePatternNode>& pTreePatternOut)
      : id(pFilename),
        filename(pFilename),
        convesionNb(pConvesionNb),
        priorityOfOutPattern(pPriorityOfOutPattern),
        treePatternIn(pTreePatternIn),
        treePatternOut(pTreePatternOut)
    {
      std::stringstream ss;
      ss << " (" << pConvesionNb << ")";
      id += ss.str();
    }

    std::string id; // /!\ 2 elts can have the same id if they are the same but inverted
    std::string filename;
    int convesionNb;
    /// come from <addPossibleTrees> <to addStrength="priorityOfOutPattern">
    int priorityOfOutPattern;
    std::shared_ptr<SemExpTreePatternNode> treePatternIn;
    std::shared_ptr<SemExpTreePatternNode> treePatternOut;
  };
  struct UniqueInformationRule
  {
    GrammaticalType childThatIsUnique = GrammaticalType::UNKNOWN;
    std::shared_ptr<SemExpTreePatternNode> treePattern{};
  };
  template<typename RULE>
  struct ConceptTreeOfRules
  {
    ConceptTreeOfRules()
      : rules(),
        children()
    {
    }

    // list of rules
    //   with all the concepts of the father nodes
    //   without the concepts of the children nodes
    std::list<RULE> rules;
    // map of concept -> child node
    std::map<std::string, ConceptTreeOfRules<RULE>> children;
  };
  struct TreePatternEnumPair
  {
    TreePatternEnumPair
    (TreePatternConventionEnum pEnumIn,
     TreePatternConventionEnum pEnumOut)
      : enumIn(pEnumIn),
        enumOut(pEnumOut)
    {
    }

    bool operator<
    (const TreePatternEnumPair& pEnumPair) const
    {
      if (enumIn != pEnumPair.enumIn)
      {
        return enumIn < pEnumPair.enumIn;
      }
      return enumOut < pEnumPair.enumOut;
    }

    TreePatternConventionEnum enumIn;
    TreePatternConventionEnum enumOut;
  };
  struct RootNodeWithRelatedCpts
  {
    RootNodeWithRelatedCpts()
      : rootNode(),
        concepts()
    {
    }

    std::shared_ptr<SemExpTreePatternNode> rootNode;
    std::list<std::string> concepts;
  };
  typedef std::map<TreePatternConventionEnum,
                   std::list<RootNodeWithRelatedCpts> > PatternWorkStruct;


  bool isLoaded;
  std::map<SemanticLanguageEnum, std::map<TreePatternEnumPair, ConceptTreeOfRules<ConversionRule>> > fConversions;
  ConceptTreeOfRules<ConversionRule> _semanticFormsBothDirections;
  ConceptTreeOfRules<ConversionRule> _semanticForms;
  ConceptTreeOfRules<UniqueInformationRule> fTreesOfSemUniquePattern;

  static const std::string fFormalism;

  SemExpTreeConversionDatabase(linguistics::LinguisticDatabaseStreams& pIStreams);

  virtual bool xIsLoaded() const
  { return isLoaded; }

  void xWriteErrorMsg
  (const std::string& pMsg) const;

  void xLoadUniquePatterns
  (const boost::property_tree::ptree& pTree,
   std::list<std::pair<std::list<std::string>, UniqueInformationRule>>& pSemanticUniquePatterns) const;

  void xLoadAddPossibleTreesBeacon
  (const boost::property_tree::ptree& pTree,
   std::list<std::pair<std::list<std::string>, ConversionRule>>& pQuestionsPossTreesBothDirectionList,
   std::list<std::pair<std::list<std::string>, ConversionRule>>& pQuestionsPossTreesList) const;

  void xAddSemExp
  (RootNodeWithRelatedCpts& pRootNodeWithRelatedCpts,
   SemExpTreePatternNode*& pCurrNode,
   const std::string& pBeaconName,
   const boost::property_tree::ptree& pTree,
   SemExpTreePatternNode* pFatherNode) const;

  void xFillNode
  (RootNodeWithRelatedCpts& pRootNodeWithRelatedCpts,
   SemExpTreePatternNode& pCurrNode,
   const boost::property_tree::ptree& pTree) const;

  template <typename RULE>
  void xFillTreeOfConvs
  (ConceptTreeOfRules<RULE>& pTreeOfConvs,
   std::list<std::pair<std::list<std::string>, RULE> >& pCptToCvs) const;

  void xAddConversionTreePattern
  (PatternWorkStruct& pPatternWkStruct,
   const boost::property_tree::ptree& pTree) const;

  void xAddTreePattern
  (RootNodeWithRelatedCpts& pPatternToFill,
   const boost::property_tree::ptree& pTree) const;

  void xGetGroupsOfConversions
  (std::map<TreePatternEnumPair, std::list<std::pair<std::list<std::string>, ConversionRule> > >& pRes,
   PatternWorkStruct& pPatternWkStruct) const;

  void xLoadConvFile
  (SemanticLanguageEnum pLangEnum,
   std::istream& pIStream,
   const std::string& pLocalPath);

  template<typename T1, typename T2>
  bool xIsASubSetOnMapLabel
  (const T1& pConceptsSubSet,
   const T2& pAllConcepts) const;


private:
  std::string fCurrFilename;
  int fCurrConversionNb;

  void xLoad(linguistics::LinguisticDatabaseStreams& pIStreams);
};



template<typename T1, typename T2>
bool SemExpTreeConversionDatabase::xIsASubSetOnMapLabel
(const T1& pConceptsSubSet,
 const T2& pAllConcepts) const
{
  for (typename T1::const_iterator
       itConvCpt = pConceptsSubSet.begin();
       itConvCpt != pConceptsSubSet.end(); ++itConvCpt)
  {
    if (pAllConcepts.find(itConvCpt->first) == pAllConcepts.end())
    {
      return false;
    }
  }
  return true;
}



} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_DETAIL_SEMTREECONVERSIONDATABASE_HPP
