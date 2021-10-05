#ifndef ALSEMEXPDATABASES_SEMANTICEXPRESSION_HXX
#define ALSEMEXPDATABASES_SEMANTICEXPRESSION_HXX

#include "../semanticexpression.hpp"
#include <assert.h>

namespace onsem
{


template<typename TSEMEXP>
UniqueSemanticExpression::UniqueSemanticExpression
(std::unique_ptr<TSEMEXP> pSemExp)
  : _semanticExpression(std::move(pSemExp))
{
  assert(_semanticExpression);
}


inline UniqueSemanticExpression::UniqueSemanticExpression
(UniqueSemanticExpression&& pOther)
  : _semanticExpression(std::move(pOther._semanticExpression))
{
  assert(_semanticExpression);
}

inline UniqueSemanticExpression& UniqueSemanticExpression::operator=
(UniqueSemanticExpression&& pOther)
{
  _semanticExpression = std::move(pOther._semanticExpression);
  assert(_semanticExpression);
  return *this;
}


template<typename TSEMEXP>
UniqueSemanticExpression& UniqueSemanticExpression::operator=
(std::unique_ptr<TSEMEXP> pOther)
{
  _semanticExpression = std::move(pOther);
  assert(_semanticExpression);
  return *this;
}


inline bool UniqueSemanticExpression::operator==(const UniqueSemanticExpression& pOther) const
{
  if (!_semanticExpression && !pOther._semanticExpression)
    return true;
  if (!_semanticExpression || !pOther._semanticExpression)
    return false;
  return *_semanticExpression == *pOther._semanticExpression;
}

inline void UniqueSemanticExpression::swap(UniqueSemanticExpression& pOther)
{
  _semanticExpression.swap(pOther._semanticExpression);
  assert(_semanticExpression);
}

inline const SemanticExpression& UniqueSemanticExpression::operator*() const
{
  return *_semanticExpression;
}

inline SemanticExpression& UniqueSemanticExpression::operator*()
{
  return *_semanticExpression;
}

inline SemanticExpression* UniqueSemanticExpression::operator->()
{
  return &*_semanticExpression;
}

inline const SemanticExpression* UniqueSemanticExpression::operator->() const
{
  return &*_semanticExpression;
}

inline SemanticExpression& UniqueSemanticExpression::getSemExp()
{
  return *_semanticExpression;
}

inline const SemanticExpression& UniqueSemanticExpression::getSemExp() const
{
  return *_semanticExpression;
}

inline UniqueSemanticExpression UniqueSemanticExpression::extractContent()
{
  UniqueSemanticExpression res;
  _semanticExpression.swap(res._semanticExpression);
  return res;
}


} // End of namespace onsem


#endif // !ALSEMEXPDATABASES_SEMANTICEXPRESSION_HXX
