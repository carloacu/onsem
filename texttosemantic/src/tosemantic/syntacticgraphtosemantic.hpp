#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOSEMANTIC_SYNTACTICGRAPHTOSEMANTIC_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOSEMANTIC_SYNTACTICGRAPHTOSEMANTIC_HPP

#include <list>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticreferencetype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/common/enum/relativeperson.hpp>
#include <onsem/texttosemantic/type/chunklink.hpp>

namespace onsem
{
struct SemanticExpression;
struct SemanticGenericGrounding;
struct SemanticTimeGrounding;
struct Inflections;
struct SemanticStatementGrounding;
struct GroundedExpression;
struct SemanticDate;
struct UniqueSemanticExpression;
struct SemanticAgentGrounding;
namespace linguistics
{
struct Token;
struct TokensTree;
class SemanticFrameDictionary;
class AlgorithmSetForALanguage;
struct SyntacticGraph;
class LinguisticDictionary;
struct Chunk;
struct InflectedWord;
using TokIt = std::vector<Token>::iterator;


class SyntacticGraphToSemantic
{
public:
  SyntacticGraphToSemantic(const AlgorithmSetForALanguage& pConfiguration);

  ~SyntacticGraphToSemantic();

  UniqueSemanticExpression process(const SyntacticGraph& pSyntGraph,
                                   const TextProcessingContext& pTextProcContext,
                                   const SemanticTimeGrounding& pTimeGrd,
                                   std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout) const;

protected:
  struct ToGenRepContext
  {
    ToGenRepContext(const ChunkLink& pChLink)
      : chLink(pChLink),
        chunk(*pChLink.chunk),
        holdingSentenceRequests(),
        holdingSentenceVerbTense(SemanticVerbTense::UNKNOWN),
        holdingGrdExpPtr(nullptr),
        holdingVerbChunkPtr(nullptr),
        holdingVerbIsBe(false),
        grammTypeFromParent(GrammaticalType::UNKNOWN),
        localTextProcContextPtr(),
        requestToSet(SemanticRequestType::NOTHING),
        isAtRoot(true)
    {
    }

    ToGenRepContext(const ToGenRepContext& pOther,
                    const ChunkLink& pChunkLink,
                    Chunk& pChunk)
      : chLink(pChunkLink),
        chunk(pChunk),
        holdingSentenceRequests(pOther.holdingSentenceRequests),
        holdingSentenceVerbTense(pOther.holdingSentenceVerbTense),
        holdingGrdExpPtr(pOther.holdingGrdExpPtr),
        holdingVerbChunkPtr(pOther.holdingVerbChunkPtr),
        holdingVerbIsBe(pOther.holdingVerbIsBe),
        grammTypeFromParent(GrammaticalType::UNKNOWN),
        localTextProcContextPtr(pOther.localTextProcContextPtr),
        requestToSet(SemanticRequestType::NOTHING),
        isAtRoot(false)
    {
    }


    ToGenRepContext(ToGenRepContext&& pOther);
    ToGenRepContext& operator=(ToGenRepContext&& pOther) = delete;

    ToGenRepContext(const ToGenRepContext& pContext) = default;
    ToGenRepContext& operator=(const ToGenRepContext& pContext) = delete;

    SemanticRequestType getSynthGraphRequest() const
    {
      if (holdingVerbChunkPtr == nullptr)
        return SemanticRequestType::NOTHING;
      return holdingVerbChunkPtr->requests.firstOrNothing();
    }

    const ChunkLink& chLink;
    Chunk& chunk;
    SemanticRequests holdingSentenceRequests;
    SemanticVerbTense holdingSentenceVerbTense;
    GroundedExpression* holdingGrdExpPtr;
    const Chunk* holdingVerbChunkPtr;
    bool holdingVerbIsBe;
    GrammaticalType grammTypeFromParent;
    std::shared_ptr<TextProcessingContext> localTextProcContextPtr;
    SemanticRequestType requestToSet;
    bool isAtRoot;
  };
  struct ToGenRepGeneral
  {
    ToGenRepGeneral
    (mystd::unique_propagate_const<UniqueSemanticExpression>& pUSemExp,
     const TextProcessingContext& pTextProcContext,
     const SyntacticGraph& pSyntGraph,
     const SemanticTimeGrounding& pSyntGraphTime,
     std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout)
      : uSemExp(pUSemExp),
        textProcContext(pTextProcContext),
        syntGraph(pSyntGraph),
        syntGraphTime(pSyntGraphTime),
        agentWeAreTalkingAbout(std::move(pAgentWeAreTalkingAbout)),
        prevChunks()
    {
    }
    mystd::unique_propagate_const<UniqueSemanticExpression>& uSemExp;
    const TextProcessingContext& textProcContext;
    const SyntacticGraph& syntGraph;
    const SemanticTimeGrounding& syntGraphTime;
    std::unique_ptr<SemanticAgentGrounding> agentWeAreTalkingAbout;
    std::map<const Chunk*, std::shared_ptr<SemanticExpression> > prevChunks;
  };

  mystd::unique_propagate_const<UniqueSemanticExpression> xFillSemExp(
      ToGenRepGeneral& pGeneral,
      ToGenRepContext& pContext) const;

  virtual mystd::unique_propagate_const<UniqueSemanticExpression> _processQuestionWithoutVerb(
        std::list<ChunkLink>::const_iterator& pItChild,
        ToGenRepGeneral& pGeneral,
        const Chunk& pChunk,
        const SyntacticGraph& pSyntGraph) const = 0;

private:
  const AlgorithmSetForALanguage& fConfiguration;
  const SemanticFrameDictionary& fSemFrameDict;
  const LinguisticDictionary& fLingDico;

  void xAddSemExpPtr(mystd::unique_propagate_const<UniqueSemanticExpression>& pUSemExp,
                     UniqueSemanticExpression pSemExp) const;

  void xAddSemExpIfNotEmpty(mystd::unique_propagate_const<UniqueSemanticExpression>& pUSemExp,
                            mystd::unique_propagate_const<UniqueSemanticExpression> pSemExp) const;

  void xReplaceQuestWordByRequest(SemanticExpression& pRootSemExp) const;

  UniqueSemanticExpression xAddSemExpWithMainSentimentalWord
  (const TokensTree& pTokensTree) const;

  void xAddModifiers
  (std::unique_ptr<GroundedExpression>& pGrdExpPtr,
   ToGenRepContext& pContext,
   const Chunk& pChunk,
   bool pLookAtTokenAfterTheHead) const;

  void xAddModifiersOfATokenAfterVerb
  (GroundedExpression& pGrdExp,
   ToGenRepContext& pContext,
   TokIt pItToken,
   const TokIt& pItEndToken) const;

  void xFillPossibleGenders
  (std::set<SemanticGenderType>& pPossibleGenders,
   const InflectedWord& pInflWord,
   SemanticNumberType pNumberOfInflWord) const;

  void xInitGenGroundingsFromToken
  (SemanticGenericGrounding& pGenGroundings,
   TokIt pToken,
   const TokIt& pItEndToken) const;

  void xConvertVerbChunkToRootGrounding
  (SemanticStatementGrounding& pRootGrounding,
   const Chunk& pChunk) const;

  void xAddOwnertTolink(GroundedExpression& pGrdExpParent,
                                  const InflectedWord& pIntroInflWord,
                                  RelativePerson pRelativePerson,
                                  const ToGenRepContext& pContext,
                                  const ToGenRepGeneral& pGeneral) const;

  void xConsiderDeterminerAtTheBeginningOfAGrounding(GroundedExpression& pRootGrdExp,
                                                     const Chunk& pChunk,
                                                     const ToGenRepContext& pContext,
                                                     const ToGenRepGeneral& pGeneral) const;

  TokIt xAddDeterminerToAGrounding(GroundedExpression& pRootGrdExp,
                                   SemanticGrounding& pRootGrounding,
                                   bool& pKnowReferenceForSure,
                                   const TokIt& pItDetToken,
                                   const TokIt& pItEndToken,
                                   const ToGenRepContext& pContext,
                                   const ToGenRepGeneral& pGeneral) const;

  mystd::unique_propagate_const<UniqueSemanticExpression> xConvertToTimeGrounding(ToGenRepContext& pContext,
                                                                    ToGenRepGeneral& pGeneral,
                                                                    const Chunk& pChunk,
                                                                    SemanticReferenceType pReferenceType,
                                                                    const std::map<std::string, char>& pConcepts) const;

  UniqueSemanticExpression xConvertNominalChunkToSemExp(ToGenRepContext& pContext,
                                                        ToGenRepGeneral& pGeneral,
                                                        const ChunkLink& pChunkLink,
                                                        const Chunk& pChunk) const;

  SemanticVerbTense xChunkToVerbTimeTense
  (VerbGoalEnum& pVerbGoal,
   const Chunk& pVerbChunk,
   ChunkLinkType pChunkLinkType) const;

  SemanticVerbTense xChunkToVerbTimeTenseForFrench
  (VerbGoalEnum& pVerbGoal,
   const Chunk& pVerbChunk,
   ChunkLinkType pChunkLinkType) const;

  SemanticVerbTense xChunkToVerbTimeTenseForEnglish
  (VerbGoalEnum& pVerbGoal,
   const Chunk& pVerbChunk,
   ChunkLinkType pChunkLinkType) const;

  static bool xVerbAuxiliaryIndicateFutureForEnglish
  (const Chunk& pVerbChunk);

  std::unique_ptr<GroundedExpression> xInitNewSentence
  (ToGenRepGeneral& pGeneral,
   ToGenRepContext& pContext,
   Chunk& pVerbChunk) const;

  void xFillVerbChunk
  (std::unique_ptr<GroundedExpression>& pGrdExp,
   ToGenRepGeneral& pGeneral,
   ToGenRepContext& pContext,
   Chunk& pVerbChunk) const;

  SemanticRequestType xSubBordinateChkLkToRequest
  (const ChunkLink& pSubBordinateChkLk) const;

  const TextProcessingContext& xGetTextProcContext
  (const ToGenRepContext& pContext,
   const ToGenRepGeneral& pGeneral) const;

  std::unique_ptr<SemanticExpression> xTranslateRelativePersToPerson
  (RelativePerson pRelPers,
   const InflectedWord& pIGram,
   const ToGenRepContext& pContext,
   const ToGenRepGeneral& pGeneral) const;

  UniqueSemanticExpression xConvertInterjectionChunk
  (ToGenRepGeneral& pGeneral,
   ToGenRepContext& pContext) const;

  UniqueSemanticExpression xConvertNominalChunk(ToGenRepGeneral& pGeneral,
                                                ToGenRepContext& pContext) const;

  void xAddSubjectOf(GroundedExpression& pNewGrdExp,
                     ToGenRepGeneral& pGeneral,
                     ToGenRepContext& pContext,
                     ListExpressionType pListExpType) const;

  void xIterateOnChildrenOfNominalChunk(UniqueSemanticExpression& pNewSemExp,
                                        ToGenRepGeneral& pGeneral,
                                        const ToGenRepContext& pContext,
                                        const InflectedWord& pContextHeadIGram) const;

  void _refactorThEPurposeOfPatterns(UniqueSemanticExpression& pUSemExp,
                                     ToGenRepGeneral& pGeneral,
                                     const ToGenRepContext& pContext) const;

  mystd::unique_propagate_const<UniqueSemanticExpression> xConvertSeparatorChunkToSemExp(ToGenRepGeneral& pGeneral,
                                                                           ToGenRepContext& pContext) const;

  GrammaticalType xConvertSeparatorChunkToGrammaticalType(SyntacticGraphToSemantic::ToGenRepContext& pContext) const;

  UniqueSemanticExpression xConvertListChunk(ToGenRepGeneral& pGeneral,
                                             ToGenRepContext& pContext) const;

  void xFillNewSentence(GroundedExpression& pGrdExpSentence,
                        std::map<GrammaticalType, std::unique_ptr<ConditionExpression> >& pChildTypeToCondition,
                        ToGenRepGeneral& pGeneral,
                        const ToGenRepContext& pContext) const;

  void xAfterCreationModificationsOnSentence
  (GroundedExpression& pGrdExpSentence,
   const ToGenRepContext& pContext,
   const ToGenRepGeneral& pGeneral) const;

  void xFillSentenceSubordonates
  (UniqueSemanticExpression& pSemExpSentence,
   std::map<GrammaticalType, std::unique_ptr<ConditionExpression> >& pChildTypeToCondition,
   GrammaticalType pCurrGramChild,
   ToGenRepGeneral& pGeneral,
   const ToGenRepContext& pContext) const;

  void xTryToFillCondition(UniqueSemanticExpression& pSemExpSentence,
                           ToGenRepGeneral& pGeneral,
                           const ToGenRepContext& pContext) const;

  void xFillCondition
  (std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
   GrammaticalType pCurrGramChild,
   ToGenRepGeneral& pGeneral,
   const ChunkLink& pSubClauseChunkLink,
   const ToGenRepContext& pContext) const;

  void xFillSubordonateClause
  (UniqueSemanticExpression& pSemExpSentence,
   ChunkLink const*& pConditionChunkLink,
   ToGenRepGeneral& pGeneral,
   ToGenRepContext& pSubContext,
   const ChunkLink& pSubClauseChunkLink,
   const ToGenRepContext& pContext) const;

  void xAddNewGrammInfo
  (GroundedExpression& pGrdExpSentence,
   ToGenRepGeneral& pGeneral,
   ToGenRepContext& pContext,
   SemanticRequestType pSubRequest = SemanticRequestType::NOTHING,
   ListExpressionType pListExpType = ListExpressionType::UNRELATED,
   std::map<GrammaticalType, UniqueSemanticExpression>* pAdditionalChildren = nullptr) const;

  mystd::unique_propagate_const<UniqueSemanticExpression> xFillLengthStruct(const ToGenRepContext& pContext) const;
  mystd::unique_propagate_const<UniqueSemanticExpression> xFillLocationStruct(const ToGenRepContext& pContext) const;
  mystd::unique_propagate_const<UniqueSemanticExpression> xFillPercentageStruct(const ToGenRepContext& pContext) const;
  mystd::unique_propagate_const<UniqueSemanticExpression> xFillHourTimeStruct(const ToGenRepContext& pContext) const;
  mystd::unique_propagate_const<UniqueSemanticExpression> xFillTimeStruct(const ToGenRepContext& pContext) const;
};


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOSEMANTIC_SYNTACTICGRAPHTOSEMANTIC_HPP
