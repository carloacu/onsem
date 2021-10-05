#include "semanticmemoryblockviewer.hpp"
#include "semanticmemoryblockprivate.hpp"

namespace onsem
{

SemanticMemoryBlockPrivate* SemanticMemoryBlockViewer::getViewPrivatePtr()
{
  return _viewPtr != nullptr ? &*_viewPtr->_impl : nullptr;
}


const SemanticMemoryBlockPrivate* SemanticMemoryBlockViewer::getViewPrivatePtr() const
{
  return _viewPtr != nullptr ? &*_viewPtr->_impl : nullptr;
}


const SemanticMemoryBlockPrivate& SemanticMemoryBlockViewer::getConstViewPrivate() const
{
  return *constView._impl;
}


bool SemanticMemoryBlockViewer::areSameUser(const std::string& pUserId1,
                                            const std::string& pUserId2,
                                            RelatedContextAxiom& pRelContextAxiom)
{
  if (_viewPtr != nullptr)
    return _viewPtr->areSameUser(pUserId1, pUserId2, pRelContextAxiom);
  return constView.areSameUserConst(pUserId1, pUserId2, &pRelContextAxiom);
}


} // End of namespace onsem

