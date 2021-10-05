#include <onsem/semantictotext/semanticintent.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>

namespace onsem
{
#define ADD_SEMANTIC_TYPEOFINTENT(a, b) {TypeOfIntent::a, b},
static const std::map<TypeOfIntent, std::string> _typeOfIntent_toStr = {
  SEMANTIC_TYPEOFINTENT_TABLE
};
#undef ADD_SEMANTIC_TYPEOFINTENT


struct ONSEMSEMANTICTOTEXT_API SemanticGreetingIntent : SemanticIntent
{
  SemanticGreetingIntent()
   : SemanticIntent(TypeOfIntent::GREETINGS)
  {
  }
};


struct ONSEMSEMANTICTOTEXT_API SemanticFarewellIntent : SemanticIntent
{
  SemanticFarewellIntent(const std::string& pFarewellType)
   : SemanticIntent(TypeOfIntent::FAREWELL)
  {
    _entities.emplace("type", pFarewellType);
  }
};



std::string SemanticIntent::name() const
{
  auto it = _typeOfIntent_toStr.find(_typeOfIntent);
  if (it != _typeOfIntent_toStr.end())
    return it->second;
  assert(false);
  return "";
}


void extractIntents(std::list<SemanticIntent>& pRes,
                    const SemanticExpression& pSemExp)
{
  SemanticEngagementValue engagement = memoryOperation::extractEngagement(pSemExp);
  switch (engagement)
  {
  case SemanticEngagementValue::ENGAGE:
    pRes.emplace_back(SemanticGreetingIntent());
    break;
  case SemanticEngagementValue::DISENGAGE_GOODBYE:
    pRes.emplace_back(SemanticFarewellIntent("goodbye"));
    break;
  case SemanticEngagementValue::DISENGAGE_NEEDTOGO:
    pRes.emplace_back(SemanticFarewellIntent("needToGo"));
    break;
  case SemanticEngagementValue::UNKNWON:
    break;
  }
}



} // End of namespace onsem
