#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICTREECONVERTER_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICTREECONVERTER_HPP

#include <memory>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/semtreeconversiondatabase.hpp>


namespace onsem
{
class SemExpTreeConversionDatabase;
struct SemanticExpression;
struct GroundedExpression;
struct SemLineToPrint;
struct SemanticMemory;


class StaticTreeConverter : public SemExpTreeConversionDatabase
{
public:
  StaticTreeConverter(linguistics::LinguisticDatabaseStreams& pIStreams);

  void refactorSemExp(UniqueSemanticExpression& pUSemExp,
                      TreePatternConventionEnum pFromConvention,
                      TreePatternConventionEnum pToConvention,
                      SemanticLanguageEnum pLanguage,
                      std::list<std::list<SemLineToPrint> >* pDebugOutput = nullptr) const;

  void addDifferentForms(UniqueSemanticExpression& pSemExp,
                                     SemanticLanguageEnum pLanguage,
                                     bool pAllOrJustTheOneThatAreInBothDirections,
                                     std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  GrammaticalType getChildThatShouldBeUnique(mystd::unique_propagate_const<UniqueSemanticExpression>& pChildThatShouldBeUnique,
                                             const GroundedExpression& pGrdExp) const;

private:
  static const std::string _formalism;

  void _extractNewForms
  (std::list<std::pair<int, UniqueSemanticExpression>>& pNewFormsOfTheExp,
   UniqueSemanticExpression& pSemExp,
   std::set<std::string>& pAlreadyDoneConvIds,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   int pCurrPrio,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  void _searchRootOfSplitPossibilities
  (UniqueSemanticExpression& pSemExp,
   SemanticLanguageEnum pLanguage,
   bool pAllOrJustTheOneThatAreInBothDirections,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  void _addDifferentFormsOfExpressionForALanguage
  (UniqueSemanticExpression& pSemExp,
   bool pAllOrJustTheOneThatAreInBothDirections,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  void _printAConversionInfos
  (std::list<std::list<SemLineToPrint> >& pDebugOutput,
   const ConversionRule& pConvInfos) const;

  void _printAPatternNode
  (std::list<SemLineToPrint>& pOutLines,
   std::stringstream& pSs,
   std::size_t pOffsetNewLine,
   const SemExpTreePatternNode& pRootPattern) const;

  void _recurssivelyAddForms
  (std::list<std::pair<int, UniqueSemanticExpression> >& pNewFormsOfTheExp,
   UniqueSemanticExpression& pSemExp,
   std::set<std::string>& pAlreadyDoneConvIds,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   int pCurrPrio,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  static void _addExpsToDebugOutput
  (std::list<std::list<SemLineToPrint> >& pDebugOutput,
   const SemanticExpression& pSemExp);

  void _refactorSemExpForALanguage
  (UniqueSemanticExpression& pSemExp,
   TreePatternConventionEnum pFromConvention,
   TreePatternConventionEnum pToConvention,
   SemanticLanguageEnum pLanguage,
   std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  void _applyModifsOnSemExp
  (SemanticExpression& pRootInSemExp,
   const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pLinks,
   const SemExpTreePatternNode& pRootPattern) const;

  void _applyModifsOnGrdExp
  (GroundedExpression& pRootInGrExp,
   const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
   const SemExpTreePatternNode& pRootPattern,
   bool pSetMotherConceptOfConceptsAny = false) const;

  bool _aSubSemExpIsInTheList
  (const ListExpression& pListExp,
   const std::list<std::shared_ptr<SemanticExpression> >& pSubSemExp) const;

  void _applyModifsForAChild
  (GroundedExpression& pRootInGrdExp,
   const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
   GrammaticalType pChildGramType,
   const SemExpTreePatternNode& pchildPattern) const;

  void _convertFindLinksToReplacementLinks
  (std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
   const std::map<std::string, std::list<UniqueSemanticExpression*> >& pLinks) const;

  void _refactSemExpRec
  (UniqueSemanticExpression& pSemExp,
   std::map<const ConversionRule*, std::set<const SemanticExpression*> >& pAlreadyDoneConv,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  void _refactSemExp
  (UniqueSemanticExpression& pSemExp,
   std::map<const ConversionRule*, std::set<const SemanticExpression*> >& pAlreadyDoneConv,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<SemLineToPrint> >* pDebugOutput) const;

  UniqueSemanticExpression _patternNodeToSemExp(const SemExpTreePatternNode& pTreePattern,
                                                const std::map<std::string, std::list<std::shared_ptr<SemanticExpression>>>& pRepLinks,
                                                bool pSetMotherConceptOfConceptsAny = false) const;

  template <typename RULE>
  void _getRulesThatHaveTheseConcepts
  (std::list<const RULE*>& pPossiblesConv,
   const ConceptTreeOfRules<RULE>& pTreeOfConvs,
   const std::set<std::string>& pSemExpConcepts) const;

  template <typename SEMEXPCONTAINER>
  bool _doesASemExpMatchAPatternTree
  (SEMEXPCONTAINER*& pRootInSemExp,
   std::map<std::string, std::list<SEMEXPCONTAINER*> >& pLinks,
   SEMEXPCONTAINER& pSemExp,
   const std::set<const SemanticExpression*>& pAlreadyTreatedRoots,
   const SemExpTreePatternNode& pRootNode,
   const SemExpTreePatternNode& pCurrNode,
   bool pCanMatchWithAChildNode) const;
};



} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICTREECONVERTER_HPP
