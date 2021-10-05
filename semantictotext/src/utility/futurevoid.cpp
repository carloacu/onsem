#include <onsem/semantictotext/utility/futurevoid.hpp>
#include <assert.h>


namespace onsem
{

Continuation::Continuation(std::function<FutureVoid()> function)
  : function(function),
    result(std::make_shared<PromiseVoid>())
{
}


PromiseVoid::~PromiseVoid()
{
  if (!_isFinished.load())
    setFinished();
}


FutureVoid PromiseVoid::then(std::function<FutureVoid()> pFunction)
{
  std::unique_lock<std::mutex> lock(_mutex);

  if (_isFinished.load())
    return pFunction();

  // Otherwise register continuations to call them when this is finished.
  _continuations.emplace_back(pFunction);
  const Continuation& latestContinuation = _continuations.back();
  return FutureVoid(latestContinuation.result);
}

void PromiseVoid::setFinished()
{
  std::unique_lock<std::mutex> lock(_mutex);

  if (_isFinished.load())
    return;
  _isFinished = true;

  for (Continuation& continuation : _continuations)
  {
    const auto& result = continuation.result;
    continuation.function().thenVoid([result]
    {
      assert(result);
      result->setFinished();
    });
  }
  _continuations.clear();
}


FutureVoid::FutureVoid()
 :_sharedPromiseVoid(std::make_shared<PromiseVoid>())
{
  _sharedPromiseVoid->setFinished();
}

FutureVoid::FutureVoid(const std::shared_ptr<PromiseVoid>& pSharedPromiseVoid)
  :_sharedPromiseVoid(pSharedPromiseVoid)
{
}


FutureVoid::FutureVoid(const std::list<FutureVoid>& pFuturesToWait)
  :_sharedPromiseVoid(std::make_shared<PromiseVoid>())
{
  std::weak_ptr<PromiseVoid> weakPromise = _sharedPromiseVoid;

  auto nofFinishedFutures = std::make_shared<std::atomic<size_t>>(0u);
  const auto nofFuturesToWait = pFuturesToWait.size();
  for (const auto& subFuture: pFuturesToWait)
  {
    subFuture.then([weakPromise, nofFinishedFutures, nofFuturesToWait]
    {
      if (auto promise = weakPromise.lock())
      {
        if (++(*nofFinishedFutures) == nofFuturesToWait)
          promise->setFinished();
        assert(nofFinishedFutures->load() <= nofFuturesToWait);
      }
      return FutureVoid();
    });
  }
}

FutureVoid FutureVoid::then(std::function<FutureVoid()> pThenFuction) const
{
  auto& promise = _sharedPromiseVoid;
  return _sharedPromiseVoid->then([pThenFuction, promise]{ return pThenFuction(); });
}

FutureVoid FutureVoid::thenVoid(std::function<void()> pThenFuction) const
{
  auto promise = _sharedPromiseVoid;
  return promise->then([pThenFuction, promise] // keep alive the promise
  {
    pThenFuction();
    return FutureVoid();
  });
}


} // End of namespace onsem
