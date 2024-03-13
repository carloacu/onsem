#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICANNOTATIONS_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICANNOTATIONS_HPP

#include <map>
#include <functional>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>

namespace onsem {

struct SemanticAnnotations {
    virtual const SemanticExpression* grammaticalTypeToSemExpPtr(GrammaticalType pGrammaticalType) const = 0;
    virtual void iterateOverAllValues(
        const std::function<void(GrammaticalType, const SemanticExpression&)>& pOnElement) const = 0;
};

struct SemanticAnnotationsPtrs : public SemanticAnnotations {
    SemanticAnnotationsPtrs(const std::map<GrammaticalType, const SemanticExpression*>* pAnnotations = nullptr)
        : SemanticAnnotations()
        , _annotations(pAnnotations) {}

    const SemanticExpression* grammaticalTypeToSemExpPtr(GrammaticalType pGrammaticalType) const override {
        auto it = _annotations->find(pGrammaticalType);
        if (it != _annotations->end())
            return it->second;
        return nullptr;
    }

    void iterateOverAllValues(
        const std::function<void(GrammaticalType, const SemanticExpression&)>& pOnElement) const override {
        for (const auto& currElt : *_annotations)
            pOnElement(currElt.first, *currElt.second);
    }

private:
    const std::map<GrammaticalType, const SemanticExpression*>* _annotations;
};

struct SemanticAnnotationsInstances : public SemanticAnnotations {
    SemanticAnnotationsInstances()
        : SemanticAnnotations()
        , _annotations() {}

    void addAnnotation(GrammaticalType pGrammaticalType, UniqueSemanticExpression pSemExp) {
        _annotations.emplace(pGrammaticalType, std::move(pSemExp));
    }

    const SemanticExpression* grammaticalTypeToSemExpPtr(GrammaticalType pGrammaticalType) const override {
        auto it = _annotations.find(pGrammaticalType);
        if (it != _annotations.end())
            return &*it->second;
        return nullptr;
    }

    void iterateOverAllValues(
        const std::function<void(GrammaticalType, const SemanticExpression&)>& pOnElement) const override {
        for (const auto& currElt : _annotations)
            pOnElement(currElt.first, *currElt.second);
    }

private:
    std::map<GrammaticalType, UniqueSemanticExpression> _annotations;
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICANNOTATIONS_HPP
