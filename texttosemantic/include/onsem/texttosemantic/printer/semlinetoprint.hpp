#ifndef ONSEM_TEXTTOSEMANTIC_PRINTER_SEMLINETOPRINT_HPP
#define ONSEM_TEXTTOSEMANTIC_PRINTER_SEMLINETOPRINT_HPP

#include <string>
#include <sstream>
#include <list>
#include "../api.hpp"
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>


namespace onsem
{
struct SemanticExpression;


static const std::size_t semLineToPrint_subLabelOffsets = 3;


struct ONSEM_TEXTTOSEMANTIC_API ALSemLineToPrint
{
  ALSemLineToPrint
  (const std::string& pLine)
    : offset(0),
      elts(1, pLine),
      inGrey(false),
      inBold(false),
      semExp(nullptr)
  {
  }

  ALSemLineToPrint
  (std::size_t pOffset,
   const std::string& pLine)
    : offset(pOffset),
      elts(1, pLine),
      inGrey(false),
      inBold(false),
      semExp(nullptr)
  {
  }

  ALSemLineToPrint
  (std::size_t pOffset,
   std::list<std::string>& pElts,
   const SemanticExpression* pSemExp = nullptr)
    : offset(pOffset),
      elts(),
      inGrey(false),
      inBold(false),
      semExp(pSemExp)
  {
    elts.splice(elts.begin(), pElts);
  }

  ALSemLineToPrint()
    : offset(0),
      elts(),
      inGrey(false),
      inBold(false),
      semExp(nullptr)
  {
  }

  ALSemLineToPrint(const ALSemLineToPrint&) = delete;
  ALSemLineToPrint& operator=(const ALSemLineToPrint&) = delete;

  void removeOneOffset()
  {
    if (offset >= semLineToPrint_subLabelOffsets)
    {
      offset -= semLineToPrint_subLabelOffsets;
    }
  }

  std::size_t offset;
  std::list<std::string> elts;
  bool inGrey;
  bool inBold;
  const SemanticExpression* semExp;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_PRINTER_SEMLINETOPRINT_HPP
