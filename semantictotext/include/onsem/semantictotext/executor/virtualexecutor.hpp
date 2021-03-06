#ifndef ONSEM_SEMANTICTOTEXT_EXECUTOR_VIRTUALEXECUTOR_HPP
#define ONSEM_SEMANTICTOTEXT_EXECUTOR_VIRTUALEXECUTOR_HPP

#include <atomic>
#include <mutex>
#include <memory>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/executor/executorcontext.hpp>
#include <onsem/semantictotext/executor/executorlogger.hpp>
#include <onsem/semantictotext/utility/futurevoid.hpp>
#include "../api.hpp"


namespace onsem
{
struct SynthesizerResult;



/// Mother class execute a semantic expression.
struct ONSEMSEMANTICTOTEXT_API VirtualExecutor
{
  /**
   * @brief VirtualExecutor
   * @param pHowTheTextWillBeExposed Specified how the text will be exposed (voice, written text, ...).
   * @param pLogOnSynchronousExecutionCasePtr Optional logger to use only for synchronous executors.
   */
  VirtualExecutor(SemanticSourceEnum pHowTheTextWillBeExposed,
                  VirtualExecutorLogger* pLogOnSynchronousExecutionCasePtr = nullptr);

  VirtualExecutor(const VirtualExecutor&) = delete;
  VirtualExecutor& operator=(const VirtualExecutor&) = delete;

  virtual ~VirtualExecutor() {}

  /**
   * @brief runSemExp Execute a semantic expression.
   * /!\ Only one run is allowed per VirtualExecutor object.
   * @param pUSemExp The semantic expression to execute.
   * @param pExecutorContext Context of the execution.
   * @return A SharedExecutorResult finished when the execution of the semantic expression is completed.
   */
  FutureVoid runSemExp(UniqueSemanticExpression pUSemExp,
                       std::shared_ptr<ExecutorContext>& pExecutorContext);

  /// Stop the execution. If it is called before runSemExp(...), the execution will not start.
  void stop();

  bool isStopped() const;

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
   * @param pResource The command to execute (encoded in a string).
   * @param pStopRequest The object to notify about the stop of the execution.
   * @return A SharedExecutorResult finished when the execution of the command is completed.
   */
  virtual FutureVoid _exposeResource(const SemanticResource& pResource,
                                     const FutureVoid& pStopRequest);

  /**
   * @brief _exposeText Defines how to expose a text.
   * @param pText The text to expose.
   * @param pLanguage The language of the text.
   * @param pStopRequest The object to notify about the stop of the execution.
   * @return A SharedExecutorResult finished when the text has been completed.
   */
  virtual FutureVoid _exposeText(const std::string& pText,
                                 SemanticLanguageEnum pLanguage,
                                 const FutureVoid& pStopRequest);


  /**
   * @brief _handleDurationAnnotations If the expression specify a time to wait it waits this specified time otherwise it does nothing.
   * @param pIsHandled Result set to true when we wait.
   * @param pAnnExp The expression.
   * @param pExecutorContext The context of the execution.
   * @return A SharedExecutorResult finished when we finished to wait.
   */
  virtual FutureVoid _handleDurationAnnotations(bool& pIsHandled,
                                                const AnnotatedExpression& pAnnExp,
                                                std::shared_ptr<ExecutorContext> pExecutorContext,
                                                const FutureVoid& pStopRequest);

  /**
   * @brief _returnError Handle the error reports. (not implemented yet)
   * @param pErrorMrg The error message.
   * @return A SharedExecutorResult already finished.
   */
  FutureVoid _reportAnError(const std::string& pErrorMrg);

  /**
   * @brief _doExecutionUntil Execute the expression in loop until a stop is asked in the context object.
   * @param pAnnExp The expression to execute.
   * @param pExecutorContext The context that contains the stopper.
   * @param pLimitOfRecursions Maximum number of looping before to stop.
   * @return A SharedExecutorResult finished when the execution of the expression in loop is completed.
   */
  FutureVoid _doExecutionUntil(AnnotatedExpression& pAnnExp,
                               std::shared_ptr<ExecutorContext> pExecutorContext,
                               const FutureVoid& pStopRequest,
                               std::shared_ptr<int> pLimitOfRecursions);



private:
  const SemanticSourceEnum _typeOfExecutor;
  VirtualExecutorLogger* _syncLoggerPtr;
  std::shared_ptr<PromiseVoid> _stopper;

  void _addLogAutoScheduling(const std::string& pLog)
  { if (_syncLoggerPtr != nullptr) _syncLoggerPtr->onMetaInformation(pLog); }
  void _addLogAutoSchedulingBeginOfScope()
  { if (_syncLoggerPtr != nullptr) _syncLoggerPtr->onMetaInformation_BeginOfScope(); }
  void _addLogAutoSchedulingEndOfScope()
  { if (_syncLoggerPtr != nullptr) _syncLoggerPtr->onMetaInformation_EndOfScope(); }
  void _addLogAutoResource(const SemanticResource& pResource)
  { if (_syncLoggerPtr != nullptr) _syncLoggerPtr->onAutoResource(pResource); }
  void _addLogAutoSaidText(const std::string& pLog)
  { if (_syncLoggerPtr != nullptr) _syncLoggerPtr->onAutoSaidText(pLog); }

  void _usageOfMemoryAndLingDb(std::function<void(SemanticMemory&, const linguistics::LinguisticDatabase&)> pFunction);
  void _assertPunctually(const SemanticExpression& pSemExp);
  void _teachInformation(UniqueSemanticExpression pUSemExp);
  void _assertPermanently(UniqueSemanticExpression pUSemExp);
  void _convertToText(std::list<std::unique_ptr<SynthesizerResult>>& pRes,
                      const UniqueSemanticExpression& pUSemExp,
                      const TextProcessingContext& pTextProcContext);
  FutureVoid _waitUntil(const SemanticExpression& pSemExp,
                        const FutureVoid& pStopRequest);

  FutureVoid _runSemExp(UniqueSemanticExpression& pUSemExp,
                        std::shared_ptr<ExecutorContext> pExecutorContext,
                        const FutureVoid& pStopRequest);

  FutureVoid _handleAndList(ListExpression& pListExp,
                            std::shared_ptr<ExecutorContext> pExecutorContext,
                            const FutureVoid& pStopRequest);
  FutureVoid _handleThenList(ListExpression& pListExp,
                             std::shared_ptr<ExecutorContext> pExecutorContext,
                             const FutureVoid& pStopRequest);
  FutureVoid _handleThenReversedList(ListExpression& pListExp,
                                     std::shared_ptr<ExecutorContext> pExecutorContext,
                                     const FutureVoid& pStopRequest);
  FutureVoid _runConditionExp(ConditionExpression& pCondExp,
                              std::shared_ptr<ExecutorContext> pExecutorContext,
                              const FutureVoid& pStopRequest);

  FutureVoid _runSemExpNTimes(UniqueSemanticExpression& pSemExp,
                              std::shared_ptr<ExecutorContext> pExecutorContext,
                              const FutureVoid& pStopRequest,
                              int pNbOfTimes);

  FutureVoid _runGrdExp(const UniqueSemanticExpression& pUSemExp,
                        std::shared_ptr<ExecutorContext> pExecutorContext,
                        const FutureVoid& pStopRequest);

  FutureVoid _sayAndAddDescriptionTree(const UniqueSemanticExpression& pUSemExp,
                                       std::shared_ptr<ExecutorContext> pExecutorContext,
                                       const FutureVoid& pStopRequest,
                                       SemanticSourceEnum pFrom,
                                       ContextualAnnotation pContextualAnnotation);

  FutureVoid _sayWithAnnotations(const UniqueSemanticExpression& pUSemExp,
                                 std::shared_ptr<ExecutorContext> pExecutorContext,
                                 const FutureVoid& pStopRequest,
                                 SemanticSourceEnum pFrom,
                                 ContextualAnnotation pContextualAnnotation);


};

} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_EXECUTOR_VIRTUALEXECUTOR_HPP
