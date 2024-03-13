#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKS_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKS_HPP

#include <string>
#include <map>
#include <memory>
#include "../../api.hpp"
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexpsaver.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinksid.hpp>

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}
struct SentenceWithLinks;
struct GroundedExpression;
struct GroundedExpWithLinksPrivate;
struct SemanticMemoryGrdExp;
struct SemanticExpressionContainer;

class ONSEMSEMANTICTOTEXT_API MemGrdExpPtrOffsets {
public:
    MemGrdExpPtrOffsets(unsigned char* pBeginPtr);
    void addMemGrdExp(const SemanticMemoryGrdExp& pMemGrdExp, unsigned char* pPtr);
    uint32_t getOffset(const SemanticMemoryGrdExp& pMemGrdExp) const;

private:
    unsigned char* _beginPtr;
    std::map<const SemanticMemoryGrdExp*, uint32_t> _memGrdExpPtrs;
};

/// This class contains a grounded expression with the links, that merged in a memory will allow to do a fast matching.
struct ONSEMSEMANTICTOTEXT_API GroundedExpWithLinks {
    GroundedExpWithLinks(SentenceWithLinks& pContextAxiom,
                         const GroundedExpression& pGrdExp,
                         bool pInRecommendationMode,
                         const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                         bool pIsATrigger,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pIsAConditionToSatisfy,
                         bool pIsEnabled = true,
                         intSemId pId = 0);
    ~GroundedExpWithLinks();

    bool isATrigger() const { return _isATrigger; }
    void setEnabled(bool pEnabled);
    bool isEnabled() const { return _isEnabled; }
    bool isANoun() const { return _isANoun; }
    bool isAConditionToSatisfy() const { return _isAConditionToSatisfy; }
    bool isOtherSentenceMoreRevelant(const GroundedExpWithLinks& pOther) const;
    std::string getName(const std::string& pUserId) const;
    bool hasEquivalentUserIds(const std::string& pUserId) const;
    const std::map<GrammaticalType, const SemanticExpression*>& getAnnotations() const { return _annotations; }
    const SemanticExpression* getSemExpForGrammaticalType(GrammaticalType pGrammType) const;
    void writeInBinary(binarymasks::Ptr& pPtr,
                       MemGrdExpPtrOffsets& pMemGrdExpPtrs,
                       const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const;

    SentenceWithLinks& getContextAxiom() { return _contextAxiom; }
    const SentenceWithLinks& getContextAxiom() const { return _contextAxiom; }
    const GroundedExpression& getGrdExpRef() const { return grdExp; }

    /// An id to sort and allways have the same orders of answers (to not sort by pointer values)
    const intSemId id;

    /// The sentence root grdExp
    const GroundedExpression& grdExp;
    const bool inRecommendationMode;

private:
    SentenceWithLinks& _contextAxiom;
    std::map<GrammaticalType, const SemanticExpression*> _annotations;
    bool _isATrigger;
    bool _isEnabled;
    bool _isANoun;
    bool _isAConditionToSatisfy;
    friend struct GroundedExpWithLinksPrivate;
    mystd::unique_propagate_const<GroundedExpWithLinksPrivate> _impl;
};

struct ONSEMSEMANTICTOTEXT_API GroundedExpWithLinksWithParameters {
    GroundedExpWithLinksWithParameters(const GroundedExpWithLinks& pLinks)
        : links(pLinks)
        , parametersLabelsToValue() {}

    std::map<std::string, std::vector<UniqueSemanticExpression>> cloneParameters() const;

    const GroundedExpWithLinks& links;
    std::map<std::string, std::vector<UniqueSemanticExpression>> parametersLabelsToValue;
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKS_HPP
