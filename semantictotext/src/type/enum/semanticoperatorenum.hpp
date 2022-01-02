#ifndef ONSEM_SEMANTICTOTEXT_TYPES8ENUM_SEMANTICOPERATORENUM_HPP
#define ONSEM_SEMANTICTOTEXT_TYPES8ENUM_SEMANTICOPERATORENUM_HPP


namespace onsem
{

enum class SemanticOperatorEnum
{
  ANSWER,
  CHECK,
  FEEDBACK,
  FIND,
  GET,
  RESOLVECOMMAND, // convert to a command with the starting condition
  EXECUTEBEHAVIOR, // convert to a command without the starting condition
  EXECUTEFROMTRIGGER, // replace a text to his linked command
  HOWYOUKNOW,
  INFORM,
  REACT,
  REACTFROMTRIGGER,
  SHOW,
  TEACHBEHAVIOR,
  TEACHCONDITION,
  TEACHINFORMATION,
  UNINFORM
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_TYPES8ENUM_SEMANTICOPERATORENUM_HPP
