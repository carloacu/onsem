#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_SEMANTICEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_SEMANTICEXPRESSION_HPP

#include <set>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/misc/parameterswithvalue.hpp>
#include <onsem/common/utility/are_equals.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include "../../api.hpp"

namespace onsem
{
struct GroundedExpression;
struct ListExpression;
struct ConditionExpression;
struct ComparisonExpression;
struct InterpretationExpression;
struct SetOfFormsExpression;
struct FeedbackExpression;
struct AnnotatedExpression;
struct MetadataExpression;
struct CommandExpression;
struct FixedSynthesisExpression;
struct UniqueSemanticExpression;
struct GroundedExpressionContainer;


#define SEMANTIC_EXPRESSTION_TYPE_TABLE                        \
  SEMANTIC_EXPRESSION_TYPE(GROUNDED, "grounded")               \
  SEMANTIC_EXPRESSION_TYPE(LIST, "list")                       \
  SEMANTIC_EXPRESSION_TYPE(CONDITION, "condition")             \
  SEMANTIC_EXPRESSION_TYPE(COMPARISON, "comparison")           \
  SEMANTIC_EXPRESSION_TYPE(INTERPRETATION, "interpretation")   \
  SEMANTIC_EXPRESSION_TYPE(FEEDBACK, "feedback")               \
  SEMANTIC_EXPRESSION_TYPE(FIXEDSYNTHESIS, "fixedSynthesis")   \
  SEMANTIC_EXPRESSION_TYPE(ANNOTATED, "annotated")             \
  SEMANTIC_EXPRESSION_TYPE(METADATA, "metadata")               \
  SEMANTIC_EXPRESSION_TYPE(SETOFFORMS, "setOfForms")           \
  SEMANTIC_EXPRESSION_TYPE(COMMAND, "command")

#define SEMANTIC_EXPRESSION_TYPE(a, b) a,
enum class SemanticExpressionType : char
{
  SEMANTIC_EXPRESSTION_TYPE_TABLE
};
#undef SEMANTIC_EXPRESSION_TYPE


#define SEMANTIC_EXPRESSION_TYPE(a, b) b,
static const std::vector<std::string> _semanticExpressionType_toStr = {
  SEMANTIC_EXPRESSTION_TYPE_TABLE
};
#undef SEMANTIC_EXPRESSION_TYPE

#define SEMANTIC_EXPRESSION_TYPE(a, b) {b, SemanticExpressionType::a},
static std::map<std::string, SemanticExpressionType> _semanticExpressionType_fromStr = {
  SEMANTIC_EXPRESSTION_TYPE_TABLE
};
#undef SEMANTIC_EXPRESSION_TYPE


static inline char semanticExpressionType_toChar(SemanticExpressionType pSemExpType)
{
  return static_cast<char>(pSemExpType);
}

static inline SemanticExpressionType semanticExpressionType_fromChar(unsigned char pSemExpType)
{
  return static_cast<SemanticExpressionType>(pSemExpType);
}

static inline std::string semanticExpressionType_toStr
(SemanticExpressionType pSemExpType)
{
  return _semanticExpressionType_toStr[semanticExpressionType_toChar(pSemExpType)];
}

static inline SemanticExpressionType semanticExpressionType_fromStr
(const std::string& pSemExpTypeStr)
{
  auto it = _semanticExpressionType_fromStr.find(pSemExpTypeStr);
  if (it != _semanticExpressionType_fromStr.end())
  {
    return it->second;
  }
  return SemanticExpressionType::GROUNDED;
}





struct ONSEM_TEXTTOSEMANTIC_API SemanticExpression
{
  virtual ~SemanticExpression();
  SemanticExpression(const SemanticExpression&) = delete;
  SemanticExpression& operator=(const SemanticExpression&) = delete;

  bool operator==(const SemanticExpression& pOther) const;
  bool operator!=(const SemanticExpression& pOther) const;
  void assertEqual(const SemanticExpression& pOther) const;

  const SemanticExpressionType type;


  virtual GroundedExpression& getGrdExp();
  virtual const GroundedExpression& getGrdExp() const;
  virtual GroundedExpression* getGrdExpPtr() { return nullptr; }
  virtual const GroundedExpression* getGrdExpPtr() const { return nullptr; }
  GroundedExpression* getGrdExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true);
  const GroundedExpression* getGrdExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true) const;

  ListExpressionType getGrdExpPtrs_SkipWrapperLists(std::list<GroundedExpression*>& pRes,
                                                    bool pFollowInterpretations = true,
                                                    bool pRecurssiveCallsOnEmptyGrounding = false,
                                                    bool pOnlyMainForm = true,
                                                    bool (*pGrdExpFilderPtr)(const GroundedExpression&) = nullptr);
  ListExpressionType getGrdExpPtrs_SkipWrapperLists(std::list<const GroundedExpression*>& pRes,
                                                    bool pFollowInterpretations = true,
                                                    bool pRecurssiveCallsOnEmptyGrounding = false,
                                                    bool pOnlyMainForm = true,
                                                    bool (*pGrdExpFilderPtr)(const GroundedExpression&) = nullptr) const;

  virtual ListExpression& getListExp();
  virtual const ListExpression& getListExp() const;
  virtual ListExpression* getListExpPtr() { return nullptr; }
  virtual const ListExpression* getListExpPtr() const { return nullptr; }
  ListExpression* getListExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true);
  const ListExpression* getListExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true) const;

  virtual ConditionExpression& getCondExp();
  virtual const ConditionExpression& getCondExp() const;
  virtual ConditionExpression* getCondExpPtr() { return nullptr; }
  virtual const ConditionExpression* getCondExpPtr() const { return nullptr; }
  ConditionExpression* getCondExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true);
  const ConditionExpression* getCondExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true) const;

  virtual ComparisonExpression& getCompExp();
  virtual const ComparisonExpression& getCompExp() const;
  virtual ComparisonExpression* getCompExpPtr() { return nullptr; }
  virtual const ComparisonExpression* getCompExpPtr() const { return nullptr; }

  virtual InterpretationExpression& getIntExp();
  virtual const InterpretationExpression& getIntExp() const;
  virtual InterpretationExpression* getIntExpPtr() { return nullptr; }
  virtual const InterpretationExpression* getIntExpPtr() const { return nullptr; }
  InterpretationExpression* getIntExpPtr_SkipWrapperPtrs();
  const InterpretationExpression* getIntExpPtr_SkipWrapperPtrs() const;

  virtual SetOfFormsExpression& getSetOfFormsExp();
  virtual const SetOfFormsExpression& getSetOfFormsExp() const;
  virtual SetOfFormsExpression* getSetOfFormsExpPtr() { return nullptr; }
  virtual const SetOfFormsExpression* getSetOfFormsExpPtr() const { return nullptr; }
  SetOfFormsExpression* getSetOfFormsPtr_SkipWrapperPtrs(bool pFollowInterpretations = true);
  const SetOfFormsExpression* getSetOfFormsPtr_SkipWrapperPtrs(bool pFollowInterpretations = true) const;

  virtual FeedbackExpression& getFdkExp();
  virtual const FeedbackExpression& getFdkExp() const;
  virtual FeedbackExpression* getFdkExpPtr() { return nullptr; }
  virtual const FeedbackExpression* getFdkExpPtr() const { return nullptr; }
  FeedbackExpression* getFdkExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true);
  const FeedbackExpression* getFdkExpPtr_SkipWrapperPtrs(bool pFollowInterpretations = true) const;

  virtual AnnotatedExpression& getAnnExp();
  virtual const AnnotatedExpression& getAnnExp() const;
  virtual AnnotatedExpression* getAnnExpPtr() { return nullptr; }
  virtual const AnnotatedExpression* getAnnExpPtr() const { return nullptr; }

  virtual MetadataExpression& getMetadataExp();
  virtual const MetadataExpression& getMetadataExp() const;
  virtual MetadataExpression* getMetadataExpPtr() { return nullptr; }
  virtual const MetadataExpression* getMetadataExpPtr() const { return nullptr; }
  MetadataExpression* getMetadataPtr_SkipWrapperPtrs();
  const MetadataExpression* getMetadataPtr_SkipWrapperPtrs() const;
  void getMetadataPtr_SkipWrapperAndLists(std::list<const MetadataExpression*>& pRes) const;

  virtual CommandExpression& getCmdExp();
  virtual const CommandExpression& getCmdExp() const;
  virtual CommandExpression* getCmdExpPtr() { return nullptr; }
  virtual const CommandExpression* getCmdExpPtr() const { return nullptr; }

  virtual FixedSynthesisExpression& getFSynthExp();
  virtual const FixedSynthesisExpression& getFSynthExp() const;
  virtual FixedSynthesisExpression* getFSynthExpPtr() { return nullptr; }
  virtual const FixedSynthesisExpression* getFSynthExpPtr() const { return nullptr; }


  bool isEmpty() const;

  std::unique_ptr<SemanticExpression> clone(const IndexToSubNameToParameterValue* pParams = nullptr,
                                            bool pRemoveRecentContextInterpretations = false,
                                            const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;

protected:
  SemanticExpression(SemanticExpressionType pSemExpType);
  static void _assertSemExpOptsEqual(const mystd::unique_propagate_const<UniqueSemanticExpression>& pSemExpOpt1,
                                     const mystd::unique_propagate_const<UniqueSemanticExpression>& pSemExpOpt2);
  static void _assertChildrenEqual(const std::map<GrammaticalType, UniqueSemanticExpression>& pChildren1,
                                   const std::map<GrammaticalType, UniqueSemanticExpression>& pChildren2);
};




struct ONSEM_TEXTTOSEMANTIC_API SemanticExpressionContainer
{
  virtual ~SemanticExpressionContainer() {}

  virtual const SemanticExpression& operator*() const = 0;
  virtual const SemanticExpression* operator->() const = 0;
  virtual const SemanticExpression& getSemExp() const = 0;
};


struct UniqueSemanticExpression : public SemanticExpressionContainer
{
  ONSEM_TEXTTOSEMANTIC_API UniqueSemanticExpression();
  template<typename TSEMEXP>
  UniqueSemanticExpression(std::unique_ptr<TSEMEXP> pSemExp);

  ONSEM_TEXTTOSEMANTIC_API UniqueSemanticExpression(UniqueSemanticExpression&& pOther);
  UniqueSemanticExpression& operator=(UniqueSemanticExpression&& pOther);
  template<typename TSEMEXP>
  UniqueSemanticExpression& operator=(std::unique_ptr<TSEMEXP> pOther);

  ONSEM_TEXTTOSEMANTIC_API UniqueSemanticExpression(const UniqueSemanticExpression&) = delete;
  ONSEM_TEXTTOSEMANTIC_API UniqueSemanticExpression& operator=(const UniqueSemanticExpression&) = delete;

  ONSEM_TEXTTOSEMANTIC_API bool operator==(const UniqueSemanticExpression& pOther) const;

  ONSEM_TEXTTOSEMANTIC_API void swap(UniqueSemanticExpression& pOther);
  ONSEM_TEXTTOSEMANTIC_API const SemanticExpression& operator*() const override;
  ONSEM_TEXTTOSEMANTIC_API SemanticExpression& operator*();
  ONSEM_TEXTTOSEMANTIC_API SemanticExpression* operator->();
  ONSEM_TEXTTOSEMANTIC_API const SemanticExpression* operator->() const override;
  ONSEM_TEXTTOSEMANTIC_API SemanticExpression& getSemExp();
  ONSEM_TEXTTOSEMANTIC_API const SemanticExpression& getSemExp() const override;
  ONSEM_TEXTTOSEMANTIC_API void clear();
  ONSEM_TEXTTOSEMANTIC_API UniqueSemanticExpression extractContent();
  ONSEM_TEXTTOSEMANTIC_API static UniqueSemanticExpression& wrapInContainer(UniqueSemanticExpression& pUSemExp) { return pUSemExp; }
  ONSEM_TEXTTOSEMANTIC_API static const UniqueSemanticExpression& wrapInContainer(const UniqueSemanticExpression& pUSemExp) { return pUSemExp; }

  ONSEM_TEXTTOSEMANTIC_API std::shared_ptr<SemanticExpression> getSharedPtr()
  { return _semanticExpression; }

private:
  std::shared_ptr<SemanticExpression> _semanticExpression;
};


struct ONSEM_TEXTTOSEMANTIC_API ReferenceOfSemanticExpressionContainer : public SemanticExpressionContainer
{
  ReferenceOfSemanticExpressionContainer(const SemanticExpression& pSemExp)
    : SemanticExpressionContainer(),
      _semExp(pSemExp)
  {
  }

  static ReferenceOfSemanticExpressionContainer wrapInContainer(const UniqueSemanticExpression& pUSemExp)
  { return ReferenceOfSemanticExpressionContainer(*pUSemExp); }
  const SemanticExpression& operator*() const override { return _semExp; }
  const SemanticExpression* operator->() const override { return &_semExp; }
  const SemanticExpression& getSemExp() const override { return _semExp; }

private:
  const SemanticExpression& _semExp;
};


ONSEM_TEXTTOSEMANTIC_API
std::unique_ptr<GroundedExpressionContainer> makeGrdExpContainer(
    UniqueSemanticExpression& pUSemExp);

ONSEM_TEXTTOSEMANTIC_API
std::unique_ptr<GroundedExpressionContainer> makeGrdExpContainer(
    const ReferenceOfSemanticExpressionContainer& pSemExpRef);


} // End of namespace onsem

#include "details/semanticexpression.hxx"

#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_SEMANTICEXPRESSION_HPP
