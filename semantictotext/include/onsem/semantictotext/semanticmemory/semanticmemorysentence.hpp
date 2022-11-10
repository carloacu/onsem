#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORYSENTENCE_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORYSENTENCE_HPP

#include <string>
#include <map>
#include <memory>
#include "../api.hpp"
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexpsaver.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentenceid.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticContextAxiom;
struct GroundedExpression;
struct SemanticMemorySentencePrivate;
struct SemanticExpression;
struct SemanticMemoryGrdExp;
struct SemanticExpressionContainer;


class ONSEMSEMANTICTOTEXT_API MemGrdExpPtrOffsets
{
public:
  MemGrdExpPtrOffsets(unsigned char* pBeginPtr);
  void addMemGrdExp(const SemanticMemoryGrdExp& pMemGrdExp,
                    unsigned char* pPtr);
  uint32_t getOffset(const SemanticMemoryGrdExp& pMemGrdExp) const;

private:
  unsigned char* _beginPtr;
  std::map<const SemanticMemoryGrdExp*, uint32_t> _memGrdExpPtrs;
};



/**
 * @brief This class contains a grounded expression with the links, that merged in a memory will allow to do a fast matching.
 */
struct ONSEMSEMANTICTOTEXT_API SemanticMemorySentence
{
  SemanticMemorySentence(SemanticContextAxiom& pContextAxiom,
                         const GroundedExpression& pGrdExp,
                         bool pGatherAllTheLinks,
                         const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pIsAConditionToSatisfy,
                         bool pIsEnabled = true,
                         intSemId pId = 0);
  ~SemanticMemorySentence();

  void setEnabled(bool pEnabled);
  bool isEnabled() const { return _isEnabled; }
  bool isANoun() const { return _isANoun; }
  bool isAConditionToSatisfy() const { return _isAConditionToSatisfy; }
  bool isOtherSentenceMoreRevelant(const SemanticMemorySentence& pOther) const;
  std::string getName(const std::string& pUserId) const;
  bool hasEquivalentUserIds(const std::string& pUserId) const;
  const std::map<GrammaticalType, const SemanticExpression*>& getAnnotations() const { return _annotations; }
  const SemanticExpression* getSemExpForGrammaticalType(GrammaticalType pGrammType) const;
  void writeInBinary(binarymasks::Ptr& pPtr,
                     MemGrdExpPtrOffsets& pMemGrdExpPtrs,
                     const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const;

  SemanticContextAxiom& getContextAxiom() { return _contextAxiom; }
  const SemanticContextAxiom& getContextAxiom() const { return _contextAxiom; }
  const GroundedExpression& getGrdExpRef() const { return grdExp; }

  /// An id to sort and allways have the same orders of answers (to not sort by pointer values)
  const intSemId id;

  /// The sentence root grdExp
  const GroundedExpression& grdExp;
  const bool gatherAllTheLinks;


private:
  SemanticContextAxiom& _contextAxiom;
  std::map<GrammaticalType, const SemanticExpression*> _annotations;
  bool _isEnabled;
  bool _isANoun;
  bool _isAConditionToSatisfy;
  friend struct SemanticMemorySentencePrivate;
  mystd::unique_propagate_const<SemanticMemorySentencePrivate> _impl;
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORYSENTENCE_HPP
