#ifndef ONSEM_SEMANTCDEBUGGER_PRINTER_SEMEXPLINESTOSTR_HPP
#define ONSEM_SEMANTCDEBUGGER_PRINTER_SEMEXPLINESTOSTR_HPP

#include <string>
#include <list>
#include <map>
#include "../api.hpp"
#include <onsem/texttosemantic/printer/semlinetoprint.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>


namespace onsem
{
enum class PrintSemExpDiffsOutPutFormat
{
  HTML,
  CONSOLE
};


class ONSEMSEMANTICDEBUGGER_API SemExpLinesToStr
{
public:
  static const SemExpLinesToStr& getInstance(PrintSemExpDiffsOutPutFormat pOutputFormat);

  void printAlternativelySemExpAndDiffOfSemExps
  (std::string& pSemExpPrettyPrint,
   const std::list<std::list<SemLineToPrint> >& pConvOutputs) const;

  void printLines
  (std::string& pSemExpPrettyPrint,
   const std::list<SemLineToPrint>& pSemExpToPrint) const;

  void comparedPrint
  (std::string& pSemExpPrettyPrint,
   const std::list<SemLineToPrint>& pSemExpToPrint,
   const std::list<SemLineToPrint>* pLastSemExpToPrint,
   bool pWeArePrintingASemExp) const;

private:
  PrintSemExpDiffsOutPutFormat fOutputFormat;
  std::string fNewLine;
  std::string fBeginBold;
  std::string fEndBold;
  std::string fBeginStrikethrough;
  std::string fEndStrikethrough;
  std::string fBeginGrey;
  std::string fEndGrey;
  std::string fSpace;

  static std::map<PrintSemExpDiffsOutPutFormat, std::unique_ptr<SemExpLinesToStr>> _instance;

  SemExpLinesToStr(PrintSemExpDiffsOutPutFormat pOutputFormat);

  static void _initInstances();

  void _setOutputFormat
  (PrintSemExpDiffsOutPutFormat pOutputFormat);

  void _printDiffComparedToLastSemExp
  (std::string& pSemExpPrettyPrint,
   const std::list<SemLineToPrint>& pSemExpToPrint,
   const std::list<SemLineToPrint>& pLastSemExpToPrint) const;

  void _printDiffComparedToLastLine
  (std::string& pSemExpPrettyPrint,
   const std::list<std::string>& pEltsToPrint,
   const std::list<std::string>& pLastEltsToPrint) const;

  bool _areLinesNearlyTheSame
  (const std::list<std::string>& pElts1,
   const std::list<std::string>& pElts2) const;

  void _printElts
  (std::string& pSemExpPrettyPrint,
   const std::list<std::string>& pElts) const;

  int _nbEltsWeShouldSkip
  (const std::string& pElt,
   std::list<std::string>::const_iterator pListIt,
   const std::list<std::string>& pList) const;

  int _nbLinesWeShouldSkip
  (const SemLineToPrint& pRefLine,
   std::list<SemLineToPrint>::const_iterator pListIt,
   const std::list<SemLineToPrint>& pList) const;
};


} // End of namespace onsem

#endif // ONSEM_SEMANTCDEBUGGER_PRINTER_SEMEXPLINESTOSTR_HPP
