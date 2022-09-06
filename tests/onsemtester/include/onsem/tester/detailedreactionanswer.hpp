#ifndef ONSEM_TESTER_DETAILEDREACTIONANSWER_HPP
#define ONSEM_TESTER_DETAILEDREACTIONANSWER_HPP

#include <list>
#include <string>
#include <onsem/common/enum/contextualannotation.hpp>
#include "api.hpp"

namespace onsem
{

struct ONSEMTESTER_API DetailedReactionAnswer
{
  std::string answer{};
  ContextualAnnotation reactionType{ContextualAnnotation::ANSWERNOTFOUND};
  std::list<std::string> references{};

  std::string toStr() const;
};


} // End of namespace onsem

#endif // ONSEM_TESTER_DETAILEDREACTIONANSWER_HPP
