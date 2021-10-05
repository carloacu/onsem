#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYGRDEXP_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYGRDEXP_HPP


namespace onsem
{
struct SemanticMemorySentence;
struct GroundedExpression;


struct SemanticMemoryGrdExp
{
public:
  SemanticMemoryGrdExp(SemanticMemorySentence& pMemSentence,
                       const GroundedExpression& pGrdExp)
    : grdExp(pGrdExp),
      _memSentence(pMemSentence)
  {
  }

  SemanticMemorySentence& getMemSentence() { return _memSentence; }
  const SemanticMemorySentence& getMemSentence() const { return _memSentence; }

  /// One linked grdExp (can be the object, the subject, the statement, ...)
  const GroundedExpression& grdExp;

private:
  /// Parent memory sentence
  SemanticMemorySentence& _memSentence;
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYGRDEXP_HPP
