#ifndef ONSEM_SEMANTICTOTEXT_GREET_HPP
#define ONSEM_SEMANTICTOTEXT_GREET_HPP

#include <functional>
#include <string>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/semantictotext/api.hpp>

namespace onsem
{
struct SemanticMemory;
namespace linguistics
{
struct LinguisticDatabase;
} // linguistics

/// A structure to carry functions that may be called
/// in certain cases of the greetings.
struct GreetCallbacks
{
  GreetCallbacks(){} // TODO: understand why not defining this causes undefined reference
  std::function<void()> onHumanIntroduced;
  std::function<void(const std::string&)> onHumanHasGivenName;
  std::function<void()> onHumanLeaving;
  std::function<void()> onHumanAskingData;
  std::function<void(const std::string&)> lookAtOtherPerson;
};


/// Responds to a greeting with another greeting.
/// If the input is not a greeting,
/// no response will be produced.
ONSEMSEMANTICTOTEXT_API
mystd::unique_propagate_const<UniqueSemanticExpression> greetInResponseTo(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const GreetCallbacks& pCallbacks = {},
    std::unique_ptr<SemanticExpression> pInterlocutor = {});


/// Produce a greeting expression.
// TODO: support greeting a specific user ID.
ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression greet(
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const GreetCallbacks pCallbacks = {},
    std::unique_ptr<GroundedExpression> pInterlocutor = {});


/// Produce a bye expression.
// TODO: support a specific user ID.
ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression bye(
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const GreetCallbacks pCallbacks = {},
    std::unique_ptr<GroundedExpression> pInterlocutor = {});

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_GREET_HPP


