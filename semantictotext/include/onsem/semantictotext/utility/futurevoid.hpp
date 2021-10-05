#ifndef ONSEM_SEMANTICTOTEXT_UTILITY_FUTUREVOID_HPP
#define ONSEM_SEMANTICTOTEXT_UTILITY_FUTUREVOID_HPP

#include <atomic>
#include <functional>
#include <mutex>
#include <list>
#include <memory>
#include "../api.hpp"


namespace onsem
{
class FutureVoid;
class PromiseVoid;

/// Class to store information about a callback registration.
struct Continuation
{
  /**
   * @brief Continuation Constructor.
   * @param pThenFuction Callback to call when the promise will enter in the finished state.
   */
  Continuation(std::function<FutureVoid()> function);

  /// Callback to call when the promise will enter in the finished state.
  std::function<FutureVoid()> function;

  /// Keeping the result of the function.
  std::shared_ptr<PromiseVoid> result;
};


/// Base class of promises without value.
class ONSEMSEMANTICTOTEXT_API PromiseVoid
{
public:
  /// Destructor. Finishes the promise if not already finished.
  virtual ~PromiseVoid();

  /**
   * @brief then Execute a function when the promise is in the finished state.
   * @param pThenFuction The function to execute when the promise is in the finished state.
   * @return The result of the pThenFuction.
   */
  FutureVoid then(std::function<FutureVoid()> pFunction);

  /// True, if the promise is in the finished state, false otherwise.
  bool isFinished() const { return _isFinished.load(); }

  /// Function that children classes have to call when they finish.
  void setFinished();

private:
  mutable std::mutex _mutex;
  std::atomic<bool> _isFinished{false};
  std::list<Continuation> _continuations;
};


/// Future without value.
class ONSEMSEMANTICTOTEXT_API FutureVoid
{
public:
  /// Future already in the finished state.
  FutureVoid();
  /**
   * @brief FutureVoid Future that will forward the state of the given promise.
   * @param pSharedPromiseVoid Promise to track.
   */
  FutureVoid(const std::shared_ptr<PromiseVoid>& pSharedPromiseVoid);

  /**
   * @brief FutureVoid Future that be finished when all the sub future will be finished.
   * @param pFuturesToWait Sub futures to track.
   */
  FutureVoid(const std::list<FutureVoid>& pFuturesToWait);

  /**
   * @brief then Execute a function when the future is in the finished state.
   * @param pThenFuction The function to execute when the future is in the finished state.
   * @return The result of the pThenFuction.
   */
  FutureVoid then(std::function<FutureVoid()> pThenFuction) const;

  /**
   * @brief then Execute a function that doesn't return anything when the future is in the finished state.
   * @param pThenFuction The function to execute when the future is in the finished state.
   * @return The result of the pThenFuction.
   */
  FutureVoid thenVoid(std::function<void()> pThenFuction) const;

  /// True, if the promise is in the finished state, false otherwise.
  bool isFinished() const { return _sharedPromiseVoid->isFinished(); }

private:
  std::shared_ptr<PromiseVoid> _sharedPromiseVoid;
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_UTILITY_FUTUREVOID_HPP
