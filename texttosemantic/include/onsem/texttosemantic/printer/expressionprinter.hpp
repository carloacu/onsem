#ifndef ONSEM_TEXTTOSEMANTIC_PRINTER_EXPRESSIONPRINTER_HPP
#define ONSEM_TEXTTOSEMANTIC_PRINTER_EXPRESSIONPRINTER_HPP

#include <list>
#include "../api.hpp"
#include "semlinetoprint.hpp"

namespace onsem
{
struct UniqueSemanticExpression;
struct SemanticExpression;
namespace printer
{


ONSEM_TEXTTOSEMANTIC_API
void oneWordPrint(std::string& pRes,
                  const UniqueSemanticExpression& pSemanticExp,
                  const std::string& pCurrentUserId);

ONSEM_TEXTTOSEMANTIC_API
void prettyPrintSemExp(std::list<ALSemLineToPrint>& pLines,
                       const SemanticExpression& pSemExp);




} // End of namespace printer
} // End of namespace onsem



#endif // ONSEM_TEXTTOSEMANTIC_PRINTER_EXPRESSIONPRINTER_HPP
