#ifndef ONSEM_SEMANTICTOTEXT_OUTPUTTER_VIRTUALOUTPUTTER_HPP
#define ONSEM_SEMANTICTOTEXT_OUTPUTTER_VIRTUALOUTPUTTER_HPP

#include <atomic>
#include <memory>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/outputter/outputtercontext.hpp>
#include "../api.hpp"

namespace onsem {
struct SynthesizerResult;

/// Parent class output a semantic expression.
struct ONSEMSEMANTICTOTEXT_API VirtualOutputter {
    /**
     * @brief Constrcut a VirtualOutputter.
     * @param pSemanticMemory Semantic memory.
     * @param pLingDb Linguistic database.
     * @param pHowTheTextWillBeExposed Specified how the text will be exposed (voice, written text, ...).
     */
    VirtualOutputter(SemanticMemory& pSemanticMemory,
                     const linguistics::LinguisticDatabase& pLingDb,
                     SemanticSourceEnum pHowTheTextWillBeExposed);

    VirtualOutputter(const VirtualOutputter&) = delete;
    VirtualOutputter& operator=(const VirtualOutputter&) = delete;

    virtual ~VirtualOutputter() {}

    enum class Link { AND, THEN, THEN_REVERSED, IN_BACKGROUND };

    /**
     * @brief Process a semantic expression.
     * /!\ Only one run is allowed per VirtualOutputter object.
     * @param pSemExp The semantic expression to process.
     * @param pOutputterContext Context of the outputter.
     */
    void processSemExp(const SemanticExpression& pSemExp, const OutputterContext& pOutputterContext);

protected:
    const linguistics::LinguisticDatabase& _lingDb;

    /**
     * @brief _exposeResource Defines how to expose a resource.
     * @param pResource The command to expose (encoded in a string).
     * @param pInutSemExpPtr Semantic expression of the input.
     */
    virtual void _exposeResource(const SemanticResource& pResource,
                                 const std::map<std::string, std::vector<std::string>>& pParameters) {}

    /**
     * @brief _exposeText Defines how to expose a text.
     * @param pText The text to expose.
     * @param pLanguage The language of the text.
     */
    virtual void _exposeText(const std::string& pText, SemanticLanguageEnum pLanguage) {}

    virtual void _beginOfScope(Link pLink) {}
    virtual void _endOfScope() {}
    virtual void _resourceNbOfTimes(int pNumberOfTimes) {}
    virtual void _insideScopeNbOfTimes(int pNumberOfTimes) {}

    virtual void _assertPunctually(const SemanticExpression& pSemExp) {}
    virtual void _teachInformation(UniqueSemanticExpression pUSemExp) {}
    virtual void _assertPermanently(UniqueSemanticExpression pUSemExp) {}

    /**
     * @brief _handleDurationAnnotations If the expression specify a time to wait it waits this specified time otherwise
     * it does nothing.
     * @param pIsHandled Result set to true when we wait.
     * @param pAnnExp The expression.
     * @param pOutputterContext Context of the outputter.
     */
    virtual void _handleDurationAnnotations(bool& pIsHandled,
                                            const AnnotatedExpression& pAnnExp,
                                            const OutputterContext& pOutputterContext);

    /**
     * @brief _returnError Handle the error reports. (not implemented yet)
     * @param pErrorMrg The error message.
     */
    void _reportAnError(const std::string& pErrorMrg);

    /**
     * @brief Process the expression in loop until a stop is asked in the context object.
     * @param pAnnExp The expression to process.
     * @param pOutputterContext Context of the outputter.
     * @param pLimitOfRecursions Maximum number of looping before to stop.
     */
    void _doUntil(const AnnotatedExpression& pAnnExp,
                  std::shared_ptr<OutputterContext> pOutputterContext,
                  std::shared_ptr<int> pLimitOfRecursions);

private:
    SemanticMemory& _semanticMemory;
    const SemanticSourceEnum _typeOfOutputter;

    void _convertToText(std::list<std::unique_ptr<SynthesizerResult>>& pRes,
                        const SemanticExpression& pSemExp,
                        const TextProcessingContext& pTextProcContext);

    void _handleList(const ListExpression& pListExp, Link pLink, const OutputterContext& pOutputterContext);
    void _handleThenReversedList(const ListExpression& pListExp, const OutputterContext& pOutputterContext);
    void _runConditionExp(const ConditionExpression& pCondExp, const OutputterContext& pOutputterContext);

    void _processGrdExp(const SemanticExpression& pSemExp, const OutputterContext& pOutputterContext);

    void _sayAndAddDescriptionTree(const SemanticExpression& pSemExp,
                                   const OutputterContext& pOutputterContext,
                                   SemanticSourceEnum pFrom,
                                   ContextualAnnotation pContextualAnnotation);

    void _sayWithAnnotations(const SemanticExpression& pSemExp,
                             const OutputterContext& pOutputterContext,
                             SemanticSourceEnum pFrom,
                             ContextualAnnotation pContextualAnnotation);

    void _processResource(const SemanticResource& pResource, const SemanticExpression* pInputSemExpPtr);
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_OUTPUTTER_VIRTUALOUTPUTTER_HPP
