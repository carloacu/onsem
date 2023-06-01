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
#include <onsem/semantictotext/outputter/outputterlogger.hpp>
#include "../api.hpp"


namespace onsem
{
struct SynthesizerResult;



/// Mother class output a semantic expression.
struct ONSEMSEMANTICTOTEXT_API VirtualOutputter
{
  /**
   * @brief Constrcut a VirtualOutputter.
   * @param pHowTheTextWillBeExposed Specified how the text will be exposed (voice, written text, ...).
   * @param pLoggerPtr Optional logger.
   */
  VirtualOutputter(SemanticSourceEnum pHowTheTextWillBeExposed,
                   VirtualOutputterLogger* pLoggerPtr = nullptr);

  VirtualOutputter(const VirtualOutputter&) = delete;
  VirtualOutputter& operator=(const VirtualOutputter&) = delete;

  virtual ~VirtualOutputter() {}

  enum class Link
  {
    AND,
    THEN,
    THEN_REVERSED,
    IN_BACKGROUND
  };

  static std::string linkToStr(Link pLink);

  /**
   * @brief Process a semantic expression.
   * /!\ Only one run is allowed per VirtualOutputter object.
   * @param pSemExp The semantic expression to process.
   * @param pOutputterContext Context of the outputter.
   */
  void processSemExp(const SemanticExpression& pSemExp,
                     const OutputterContext& pOutputterContext);


protected:
  /**
   * @brief _usageOfMemory Wrapper to protect any function call that can modify the memory.
   * /!\ This function has to be synchronous!
   * @param pFunction Function to call in your wrapper.
   */
  virtual void _usageOfMemory(std::function<void(SemanticMemory&)> pFunction) = 0;

  /**
   * @brief _usageOfMemblock Wrapper to protect any function call that use the meomry without modifing it.
   * /!\ This function has to be synchronous!
   * @param pFunction Function to call in your wrapper.
   */
  virtual void _usageOfMemblock(std::function<void(const SemanticMemoryBlock&, const std::string&)> pFunction) = 0;

  /**
   * @brief _usageOfLingDb Wrapper to protect any function call that can modify the linguistic database.
   * /!\ This function has to be synchronous!
   * /!\ _usageOfLingDb will always be already protected by the method _usageOfMemory!
   * @param pFunction Function to call in your wrapper.
   */
  virtual void _usageOfLingDb(std::function<void(const linguistics::LinguisticDatabase&)> pFunction) = 0;

  /**
   * @brief _synchronicityWrapper Define how a callback subscribed to an extenral event will called.
   * @param pFunction The callback subscribed to an extenral event.
   */
  virtual void _synchronicityWrapper(std::function<void()> pFunction) = 0;

  /**
   * @brief _exposeResource Defines how to expose a resource.
   * @param pResource The command to expose (encoded in a string).
   * @param pInutSemExpPtr Semantic expression of the input.
   */
  virtual void _exposeResource(const SemanticResource& pResource,
                               const SemanticExpression* pInutSemExpPtr);

  /**
   * @brief _exposeText Defines how to expose a text.
   * @param pText The text to expose.
   * @param pLanguage The language of the text.
   */
  virtual void _exposeText(const std::string& pText,
                           SemanticLanguageEnum pLanguage);

  virtual void _beginOfScope();
  virtual void _insideScopeLink(Link pLink);
  virtual void _insideScopeRepetition(int pNumberOfRepetitions);
  virtual void _endOfScope();

  /**
   * @brief _handleDurationAnnotations If the expression specify a time to wait it waits this specified time otherwise it does nothing.
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

  void _addLogAutoResource(const SemanticResource& pResource,
                           const std::map<std::string, std::vector<std::string>>& pParameters)
  { if (_loggerPtr != nullptr) _loggerPtr->onAutoResource(pResource, pParameters); }

private:
  const SemanticSourceEnum _typeOfOutputter;
  VirtualOutputterLogger* _loggerPtr;

  void _addLogAutoScheduling(const std::string& pLog)
  { if (_loggerPtr != nullptr) _loggerPtr->onMetaInformation(pLog); }
  void _addLogAutoSchedulingBeginOfScope()
  { if (_loggerPtr != nullptr) _loggerPtr->onMetaInformation_BeginOfScope(); }
  void _addLogAutoSchedulingEndOfScope()
  { if (_loggerPtr != nullptr) _loggerPtr->onMetaInformation_EndOfScope(); }
  void _addLogAutoSaidText(const std::string& pLog)
  { if (_loggerPtr != nullptr) _loggerPtr->onAutoSaidText(pLog); }

  void _usageOfMemoryAndLingDb(std::function<void(SemanticMemory&, const linguistics::LinguisticDatabase&)> pFunction);
  void _assertPunctually(const SemanticExpression& pSemExp);
  void _teachInformation(UniqueSemanticExpression pUSemExp);
  void _assertPermanently(UniqueSemanticExpression pUSemExp);
  void _convertToText(std::list<std::unique_ptr<SynthesizerResult>>& pRes,
                      const SemanticExpression& pSemExp,
                      const TextProcessingContext& pTextProcContext);

  void _handleList(const ListExpression& pListExp,
                   Link pLink,
                   const OutputterContext &pOutputterContext);
  void _handleThenReversedList(const ListExpression &pListExp,
                               const OutputterContext &pOutputterContext);
  void _runConditionExp(const ConditionExpression &pCondExp,
                        const OutputterContext &pOutputterContext);

  void _runGrdExp(const SemanticExpression& pSemExp,
                  const OutputterContext &pOutputterContext);

  void _sayAndAddDescriptionTree(const SemanticExpression& pSemExp,
                                 const OutputterContext &pOutputterContext,
                                 SemanticSourceEnum pFrom,
                                 ContextualAnnotation pContextualAnnotation);

  void _sayWithAnnotations(const SemanticExpression& pSemExp,
                           const OutputterContext &pOutputterContext,
                           SemanticSourceEnum pFrom,
                           ContextualAnnotation pContextualAnnotation);


};

} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_OUTPUTTER_VIRTUALOUTPUTTER_HPP
