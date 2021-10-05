#include <onsem/semanticdebugger/printer/semexplinestostr.hpp>
#include <onsem/texttosemantic/printer/semlinetoprint.hpp>


namespace onsem
{
std::map<PrintSemExpDiffsOutPutFormat, std::unique_ptr<SemExpLinesToStr>> SemExpLinesToStr::_instance;


void SemExpLinesToStr::_initInstances()
{
  _instance.emplace(PrintSemExpDiffsOutPutFormat::CONSOLE,
                    std::unique_ptr<SemExpLinesToStr>(new SemExpLinesToStr(PrintSemExpDiffsOutPutFormat::CONSOLE)));
  _instance.emplace(PrintSemExpDiffsOutPutFormat::HTML,
                    std::unique_ptr<SemExpLinesToStr>(new SemExpLinesToStr(PrintSemExpDiffsOutPutFormat::HTML)));
}


const SemExpLinesToStr& SemExpLinesToStr::getInstance(PrintSemExpDiffsOutPutFormat pOutputFormat)
{
  auto itInstance = _instance.find(pOutputFormat);
  if (itInstance != _instance.end())
    return *itInstance->second;

  _initInstances();
  itInstance = _instance.find(pOutputFormat);
  assert(itInstance != _instance.end());
  return *itInstance->second;
}



SemExpLinesToStr::SemExpLinesToStr(PrintSemExpDiffsOutPutFormat pOutputFormat)
  : fOutputFormat(pOutputFormat),
    fNewLine(),
    fBeginBold(),
    fEndBold(),
    fBeginStrikethrough(),
    fEndStrikethrough(),
    fBeginGrey(),
    fEndGrey(),
    fSpace()
{
  _setOutputFormat(fOutputFormat);
}


void SemExpLinesToStr::_setOutputFormat(PrintSemExpDiffsOutPutFormat pOutputFormat)
{
  fOutputFormat = pOutputFormat;
  if (fOutputFormat == PrintSemExpDiffsOutPutFormat::HTML)
  {
    fNewLine = "<br>";
    fBeginBold = "<b>";
    fEndBold = "</b>";
    fBeginStrikethrough = "<s>";
    fEndStrikethrough = "</s>";
    fBeginGrey = "<font color=\"grey\">";
    fEndGrey = "</font>";
    fSpace = "&nbsp;";
  }
  else
  {
    fNewLine = "\n";
    fBeginBold = "\033[1m";
    fEndBold = "\033[0m";
    fBeginStrikethrough = "\033[9m";
    fEndStrikethrough = "\033[0m";
    fBeginGrey = "\033[0;36m";
    fEndGrey = "\033[00m";
    fSpace = " ";
  }
}



void SemExpLinesToStr::printAlternativelySemExpAndDiffOfSemExps
(std::string& pSemExpPrettyPrint,
 const std::list<std::list<ALSemLineToPrint> >& pConvOutputs) const
{
  bool weArePrintingASemExp = true;
  const std::list<ALSemLineToPrint>* lastSemExp = nullptr;
  for (std::list<std::list<ALSemLineToPrint> >::const_iterator
       it = pConvOutputs.begin();
       it != pConvOutputs.end(); ++it)
  {
    comparedPrint(pSemExpPrettyPrint, *it,
                  lastSemExp, weArePrintingASemExp);
    pSemExpPrettyPrint += fNewLine + fNewLine;

    if (weArePrintingASemExp)
    {
      lastSemExp = &*it;
    }
    weArePrintingASemExp = !weArePrintingASemExp;
  }

  if (pConvOutputs.size() > 1)
  {
    comparedPrint(pSemExpPrettyPrint, pConvOutputs.back(),
                  nullptr, true);
  }
}



void SemExpLinesToStr::printLines
(std::string& pSemExpPrettyPrint,
 const std::list<ALSemLineToPrint>& pSemExpToPrint) const
{
  for (std::list<ALSemLineToPrint>::const_iterator
       itLine = pSemExpToPrint.begin();
       itLine != pSemExpToPrint.end(); ++itLine)
  {
    if (itLine->inBold)
    {
      pSemExpPrettyPrint += fBeginBold;
    }
    if (itLine->inGrey)
    {
      pSemExpPrettyPrint += fBeginGrey;
    }
    for (std::size_t i = 0; i < itLine->offset; ++i)
    {
      pSemExpPrettyPrint += fSpace;
    }
    _printElts(pSemExpPrettyPrint, itLine->elts);
    if (itLine->inGrey)
    {
      pSemExpPrettyPrint += fEndGrey;
    }
    if (itLine->inBold)
    {
      pSemExpPrettyPrint += fEndBold;
    }
    pSemExpPrettyPrint += fNewLine;
  }
}


void SemExpLinesToStr::comparedPrint
(std::string& pSemExpPrettyPrint,
 const std::list<ALSemLineToPrint>& pSemExpToPrint,
 const std::list<ALSemLineToPrint>* pLastSemExpToPrint,
 bool pWeArePrintingASemExp) const
{
  if (pWeArePrintingASemExp)
  {
    if (pLastSemExpToPrint == nullptr)
    {
      pSemExpPrettyPrint += fBeginBold + "Semantic Expression" + fEndBold + fNewLine;
    }
    else
    {
      pSemExpPrettyPrint += fBeginBold + "Differences with last Semantic Expression" + fEndBold + fNewLine;
    }
  }
  else
  {
    pSemExpPrettyPrint += fBeginBold + "Conversion" + fEndBold + fNewLine;
  }

  if (pWeArePrintingASemExp && pLastSemExpToPrint != nullptr)
  {
    _printDiffComparedToLastSemExp(pSemExpPrettyPrint,
                                   pSemExpToPrint, *pLastSemExpToPrint);
  }
  else
  {
    if (!pWeArePrintingASemExp)
    {
      pSemExpPrettyPrint += fBeginGrey;
    }
    printLines(pSemExpPrettyPrint, pSemExpToPrint);
    if (!pWeArePrintingASemExp)
    {
      pSemExpPrettyPrint += fEndGrey;
    }
  }
}



void SemExpLinesToStr::_printDiffComparedToLastSemExp
(std::string& pSemExpPrettyPrint,
 const std::list<ALSemLineToPrint>& pSemExpToPrint,
 const std::list<ALSemLineToPrint>& pLastSemExpToPrint) const
{
  std::list<ALSemLineToPrint>::const_iterator itLastLine = pLastSemExpToPrint.begin();
  for (std::list<ALSemLineToPrint>::const_iterator itLine = pSemExpToPrint.begin();
       itLine != pSemExpToPrint.end(); )
  {
    if (itLastLine == pLastSemExpToPrint.end())
    {
      for (std::size_t i = 0; i < itLine->offset; ++i)
      {
        pSemExpPrettyPrint += fSpace;
      }
      pSemExpPrettyPrint += fBeginBold;
      _printElts(pSemExpPrettyPrint, itLine->elts);
      pSemExpPrettyPrint += fEndBold + fNewLine;
      ++itLine;
      continue;
    }

    if (_areLinesNearlyTheSame(itLine->elts, itLastLine->elts))
    {
      for (std::size_t i = 0; i < itLine->offset; ++i)
      {
        pSemExpPrettyPrint += fSpace;
      }
      _printDiffComparedToLastLine(pSemExpPrettyPrint,
                                   itLine->elts, itLastLine->elts);
      pSemExpPrettyPrint += fNewLine;
      ++itLine;
      ++itLastLine;
      continue;
    }

    int nbLinesToSkip = _nbLinesWeShouldSkip(*itLine, itLastLine, pLastSemExpToPrint);
    if (nbLinesToSkip == -1) // new line didn't exist before
    {
      for (std::size_t i = 0; i < itLine->offset; ++i)
      {
        pSemExpPrettyPrint += fSpace;
      }
      pSemExpPrettyPrint += fBeginBold;
      _printElts(pSemExpPrettyPrint, itLine->elts);
      pSemExpPrettyPrint += fEndBold + fNewLine;
      ++itLine;
      continue;
    }

    int nbLinesToSkipFromLast = _nbLinesWeShouldSkip(*itLastLine, itLine, pSemExpToPrint);
    if (nbLinesToSkipFromLast == -1) // new line didn't exist before
    {
      for (std::size_t i = 0; i < itLastLine->offset; ++i)
      {
        pSemExpPrettyPrint += fSpace;
      }
      pSemExpPrettyPrint += fBeginBold + fBeginStrikethrough;
      _printElts(pSemExpPrettyPrint, itLastLine->elts);
      pSemExpPrettyPrint += fBeginStrikethrough + fEndBold + fNewLine;
      ++itLastLine;
      continue;
    }

    if (nbLinesToSkip <= nbLinesToSkipFromLast)
    {
      for (std::size_t i = 0; i < itLine->offset; ++i)
      {
        pSemExpPrettyPrint += fSpace;
      }
      pSemExpPrettyPrint += fBeginBold;
      _printElts(pSemExpPrettyPrint, itLine->elts);
      pSemExpPrettyPrint += fEndBold + fNewLine;
      ++itLine;
      continue;
    }

    for (std::size_t i = 0; i < itLastLine->offset; ++i)
    {
      pSemExpPrettyPrint += fSpace;
    }
    pSemExpPrettyPrint += fBeginBold + fBeginStrikethrough;
    _printElts(pSemExpPrettyPrint, itLastLine->elts);
    pSemExpPrettyPrint += fBeginStrikethrough + fEndBold + fNewLine;
    ++itLastLine;
  }

  for (; itLastLine != pLastSemExpToPrint.end(); ++itLastLine)
  {
    for (std::size_t i = 0; i < itLastLine->offset; ++i)
    {
      pSemExpPrettyPrint += fSpace;
    }
    pSemExpPrettyPrint += fBeginBold + fBeginStrikethrough;
    _printElts(pSemExpPrettyPrint, itLastLine->elts);
    pSemExpPrettyPrint += fBeginStrikethrough + fNewLine;
  }
}


void SemExpLinesToStr::_printDiffComparedToLastLine
(std::string& pSemExpPrettyPrint,
 const std::list<std::string>& pEltsToPrint,
 const std::list<std::string>& pLastEltsToPrint) const
{
  auto itLastElt = pLastEltsToPrint.begin();
  for (auto itElt = pEltsToPrint.begin(); itElt != pEltsToPrint.end(); )
  {
    pSemExpPrettyPrint += " ";
    if (itLastElt == pLastEltsToPrint.end())
    {
      pSemExpPrettyPrint += fBeginBold;
      pSemExpPrettyPrint += *itElt;
      pSemExpPrettyPrint += fEndBold;
      ++itElt;
      continue;
    }

    if (*itElt == *itLastElt)
    {
      pSemExpPrettyPrint += *itElt;
      ++itElt;
      ++itLastElt;
      continue;
    }

    int nbEltToSkip = _nbEltsWeShouldSkip(*itElt, itLastElt, pLastEltsToPrint);
    if (nbEltToSkip == -1) // new elt didn't exist before
    {
      pSemExpPrettyPrint += fBeginBold;
      pSemExpPrettyPrint += *itElt;
      pSemExpPrettyPrint += fEndBold;
      ++itElt;
      continue;
    }

    int nbEltToSkipFromLast = _nbEltsWeShouldSkip(*itLastElt, itElt, pEltsToPrint);
    if (nbEltToSkipFromLast == -1) // last elt has been removed
    {
      pSemExpPrettyPrint += fBeginBold + fBeginStrikethrough ;
      pSemExpPrettyPrint += *itLastElt;
      pSemExpPrettyPrint += fBeginStrikethrough + fEndBold;
      ++itLastElt;
      continue;
    }

    if (nbEltToSkip <= nbEltToSkipFromLast)
    {
      pSemExpPrettyPrint += fBeginBold;
      pSemExpPrettyPrint += *itElt;
      pSemExpPrettyPrint += fEndBold;
      ++itElt;
      continue;
    }

    pSemExpPrettyPrint += fBeginBold + fBeginStrikethrough;
    pSemExpPrettyPrint += *itLastElt;
    pSemExpPrettyPrint += fBeginStrikethrough + fEndBold;
    ++itLastElt;
  }

  for (; itLastElt != pLastEltsToPrint.end(); ++itLastElt)
  {
    pSemExpPrettyPrint += " " + fBeginBold + fBeginStrikethrough;
    pSemExpPrettyPrint += *itLastElt;
    pSemExpPrettyPrint += fBeginStrikethrough + fEndBold;
  }
}


bool SemExpLinesToStr::_areLinesNearlyTheSame
(const std::list<std::string>& pElts1,
 const std::list<std::string>& pElts2) const
{
  if (pElts1.empty() && pElts2.empty())
  {
    return true;
  }
  if (pElts1.empty() || pElts2.empty())
  {
    return false;
  }
  return pElts1.front() == pElts2.front();
}



void SemExpLinesToStr::_printElts
(std::string& pSemExpPrettyPrint,
 const std::list<std::string>& pElts) const
{
  bool firstIteration = true;
  for (const auto& currElt : pElts)
  {
    if (firstIteration)
    {
      firstIteration = false;
    }
    else
    {
      pSemExpPrettyPrint += fSpace;
    }
    pSemExpPrettyPrint += currElt;
  }
}



int SemExpLinesToStr::_nbEltsWeShouldSkip
(const std::string& pElt,
 std::list<std::string>::const_iterator pListIt,
 const std::list<std::string>& pList) const
{
  int nbOfSkippedElts = 1;
  for (++pListIt; pListIt != pList.end(); ++pListIt)
  {
    if (*pListIt == pElt)
    {
      return nbOfSkippedElts;
    }
    ++nbOfSkippedElts;
  }
  return -1;
}


int SemExpLinesToStr::_nbLinesWeShouldSkip
(const ALSemLineToPrint& pRefLine,
 std::list<ALSemLineToPrint>::const_iterator pListIt,
 const std::list<ALSemLineToPrint>& pList) const
{
  int nbOfSkippedElts = 1;
  for (++pListIt; pListIt != pList.end(); ++pListIt)
  {
    if (_areLinesNearlyTheSame(pListIt->elts, pRefLine.elts))
    {
      return nbOfSkippedElts;
    }
    ++nbOfSkippedElts;
  }
  return -1;
}



} // End of namespace onsem
