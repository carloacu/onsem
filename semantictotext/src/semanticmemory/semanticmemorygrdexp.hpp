#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYGRDEXP_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYGRDEXP_HPP


namespace onsem
{
struct GroundedExpWithLinks;
struct GroundedExpression;


struct SemanticMemoryGrdExp
{
public:
  SemanticMemoryGrdExp(GroundedExpWithLinks& pMemSentence,
                       const GroundedExpression& pGrdExp)
    : grdExp(pGrdExp),
      _memSentence(pMemSentence)
  {
  }

  GroundedExpWithLinks& getMemSentence() { return _memSentence; }
  const GroundedExpWithLinks& getMemSentence() const { return _memSentence; }

  /// One linked grdExp (can be the object, the subject, the statement, ...)
  const GroundedExpression& grdExp;

private:
  /// Parent memory sentence
  GroundedExpWithLinks& _memSentence;
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYGRDEXP_HPP
