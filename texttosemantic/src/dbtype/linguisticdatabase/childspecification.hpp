#ifndef ONSEM_TEXTTOSEMANTIC_SRC_DBTYPE_LINGUISTICDATABASE_CHILDSPECIFICATION_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_DBTYPE_LINGUISTICDATABASE_CHILDSPECIFICATION_HPP

#include <list>
#include <vector>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/common/enum/linguisticcondition.hpp>

namespace onsem
{
namespace linguistics
{


enum class LinguisticConditionTreeOperandEnum
{
  AND,
  NOT,
  OR
};

enum class LinguisticConditionTreeType
{
  OPERAND,
  VALUE
};

struct LinguisticConditionTreeOperand;
struct LinguisticConditionTreeValue;

struct LinguisticConditionTree
{
  LinguisticConditionTreeType type;

  virtual const LinguisticConditionTreeOperand& getOperand() const;
  virtual const LinguisticConditionTreeValue& getValue() const;
  virtual LinguisticConditionTreeOperand& getOperand();

protected:
  LinguisticConditionTree(LinguisticConditionTreeType pType)
    : type(pType)
  {
  }
};

struct LinguisticConditionTreeOperand : public LinguisticConditionTree
{
  LinguisticConditionTreeOperand(LinguisticConditionTreeOperandEnum pOperand)
   : LinguisticConditionTree(LinguisticConditionTreeType::OPERAND),
     operand(pOperand),
     children()
  {
  }
  const LinguisticConditionTreeOperand& getOperand() const override { return *this; }
  LinguisticConditionTreeOperand& getOperand() override { return *this; }
  LinguisticConditionTreeOperandEnum operand;
  std::list<std::shared_ptr<LinguisticConditionTree>> children;
};

struct LinguisticConditionTreeValue : public LinguisticConditionTree
{
  LinguisticConditionTreeValue(LinguisticCondition pCondition)
   : LinguisticConditionTree(LinguisticConditionTreeType::VALUE),
     condition(pCondition),
     parameters()
  {
  }
  const LinguisticConditionTreeValue& getValue() const override { return *this; }
  LinguisticCondition condition;
  std::vector<std::string> parameters;
};


struct ChildSpecification
{
  int templatePos{0};
  ChunkLinkType chunkLinkType{ChunkLinkType::SIMPLE};
  mystd::optional<SemanticWord> introWord{};
  std::shared_ptr<LinguisticConditionTree> conditionTree{};
  std::list<std::string> verbConceptsToRemove{};
  std::list<std::string> conceptsToAdd{};
};


const boost::property_tree::ptree& fillChildSpecs(ChildSpecification& pChildSpec,
                                                  const boost::property_tree::ptree& pChildTree,
                                                  SemanticLanguageEnum pLanguage);

void fillChildSpecsFromId(ChildSpecification& pChildSpec,
                          const boost::property_tree::ptree& pChildTree,
                          SemanticLanguageEnum pLanguage,
                          int& pLastTemplatePos);

void fillChildSpecsFromBookmark(ChildSpecification& pChildSpec,
                                const boost::property_tree::ptree& pChildTree,
                                SemanticLanguageEnum pLanguage,
                                const std::map<std::string, int>& pBookmarkToTemplatePos);


inline const LinguisticConditionTreeOperand& LinguisticConditionTree::getOperand() const
{
  assert(false);
  return *dynamic_cast<const LinguisticConditionTreeOperand*>(this);
}

inline const LinguisticConditionTreeValue& LinguisticConditionTree::getValue() const
{
  assert(false);
  return *dynamic_cast<const LinguisticConditionTreeValue*>(this);
}

inline LinguisticConditionTreeOperand& LinguisticConditionTree::getOperand()
{
  assert(false);
  return *dynamic_cast<LinguisticConditionTreeOperand*>(this);
}

} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SRC_DBTYPE_LINGUISTICDATABASE_CHILDSPECIFICATION_HPP
