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
struct ALSemLineToPrint;
struct SemanticMemory;


class StaticTreeConverter : public SemExpTreeConversionDatabase
{
public:
  StaticTreeConverter(linguistics::LinguisticDatabaseStreams& pIStreams);

  void refactorSemExp(UniqueSemanticExpression& pUSemExp,
                      TreePatternConventionEnum pFromConvention,
                      TreePatternConventionEnum pToConvention,
                      SemanticLanguageEnum pLanguage,
                      std::list<std::list<ALSemLineToPrint> >* pDebugOutput = nullptr) const;

  void splitPossibilitiesOfQuestions(UniqueSemanticExpression& pSemExp,
                                     SemanticLanguageEnum pLanguage,
                                     bool pAllOrJustTheOneThatAreInBothDirections,
                                     std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  GrammaticalType getChildThatShouldBeUnique(mystd::unique_propagate_const<UniqueSemanticExpression>& pChildThatShouldBeUnique,
                                             const GroundedExpression& pGrdExp) const;

private:
  static const std::string fFormalism;

  void xExtractNewQuestionForms
  (std::list<std::pair<int, UniqueSemanticExpression>>& pNewFormsOfTheExp,
   UniqueSemanticExpression& pSemExp,
   std::set<std::string>& pAlreadyDoneConvIds,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   int pCurrPrio,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  void xSearchRootOfSplitPossOfQuestions
  (UniqueSemanticExpression& pSemExp,
   SemanticLanguageEnum pLanguage,
   bool pAllOrJustTheOneThatAreInBothDirections,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  void xRefactorQuestionsOfExpressionForALanguage
  (UniqueSemanticExpression& pSemExp,
   bool pAllOrJustTheOneThatAreInBothDirections,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  void xPrintAConversionInfos
  (std::list<std::list<ALSemLineToPrint> >& pDebugOutput,
   const ConversionRule& pConvInfos) const;

  void xPrintAPatternNode
  (std::list<ALSemLineToPrint>& pOutLines,
   std::stringstream& pSs,
   std::size_t pOffsetNewLine,
   const SemExpTreePatternNode& pRootPattern) const;

  void xRecurssivelyAddQuestionForm
  (std::list<std::pair<int, UniqueSemanticExpression> >& pNewFormsOfTheExp,
   UniqueSemanticExpression& pSemExp,
   std::set<std::string>& pAlreadyDoneConvIds,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   int pCurrPrio,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  static void xAddExpsToDebugOutput
  (std::list<std::list<ALSemLineToPrint> >& pDebugOutput,
   const SemanticExpression& pSemExp);

  void xRefactorSemExpForALanguage
  (UniqueSemanticExpression& pSemExp,
   TreePatternConventionEnum pFromConvention,
   TreePatternConventionEnum pToConvention,
   SemanticLanguageEnum pLanguage,
   std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  void xApplyModifsOnSemExp
  (SemanticExpression& pRootInSemExp,
   const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pLinks,
   const SemExpTreePatternNode& pRootPattern) const;

  void xApplyModifsOnGrdExp
  (GroundedExpression& pRootInGrExp,
   const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
   const SemExpTreePatternNode& pRootPattern,
   bool pSetMotherConceptOfConceptsAny = false) const;

  bool xASubSemExpIsInTheList
  (const ListExpression& pListExp,
   const std::list<std::shared_ptr<SemanticExpression> >& pSubSemExp) const;

  void xApplyModifsForAChild
  (GroundedExpression& pRootInGrdExp,
   const std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
   GrammaticalType pChildGramType,
   const SemExpTreePatternNode& pchildPattern) const;

  void xConvertFindLinksToReplacementLinks
  (std::map<std::string, std::list<std::shared_ptr<SemanticExpression> > >& pRepLinks,
   const std::map<std::string, std::list<UniqueSemanticExpression*> >& pLinks) const;

  void xRefactSemExpRec
  (UniqueSemanticExpression& pSemExp,
   std::map<const ConversionRule*, std::set<const SemanticExpression*> >& pAlreadyDoneConv,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  void xRefactSemExp
  (UniqueSemanticExpression& pSemExp,
   std::map<const ConversionRule*, std::set<const SemanticExpression*> >& pAlreadyDoneConv,
   const ConceptTreeOfRules<ConversionRule>& pTreeOfConvs,
   const SemanticExpression& pRootSemExpForDebug,
   std::list<std::list<ALSemLineToPrint> >* pDebugOutput) const;

  UniqueSemanticExpression xPatternNodeToSemExp(const SemExpTreePatternNode& pTreePattern,
                                                const std::map<std::string, std::list<std::shared_ptr<SemanticExpression>>>& pRepLinks,
                                                bool pSetMotherConceptOfConceptsAny = false) const;

  template <typename RULE>
  void xGetRulesThatHaveTheseConcepts
  (std::list<const RULE*>& pPossiblesConv,
   const ConceptTreeOfRules<RULE>& pTreeOfConvs,
   const std::set<std::string>& pSemExpConcepts) const;

  template <typename SEMEXPCONTAINER>
  bool xDoesASemExpMatchAPatternTree
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
