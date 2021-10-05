#include <onsem/guiutility/lineedithistoricwrapper.hpp>


namespace onsem
{

LineEditHistoricalWrapper::LineEditHistoricalWrapper()
  : _lineEdit(nullptr),
    _isActivated(false),
    _historical(),
    _currPos(_historical.end()),
    _currCompletionStr()
{
}


LineEditHistoricalWrapper::LineEditHistoricalWrapper(QLineEdit* pLineEdit,
                                                     QWidget* pHoldingWidget)
  : _lineEdit(pLineEdit),
    _isActivated(false),
    _historical(),
    _currPos(_historical.end()),
    _currCompletionStr()
{
  pLineEdit->installEventFilter(pHoldingWidget);
}


LineEditHistoricalWrapper::LineEditHistoricalWrapper
(const LineEditHistoricalWrapper& pOther)
  : _lineEdit(pOther._lineEdit),
    _isActivated(pOther._isActivated),
    _historical(pOther._historical),
    _currPos(_historical.end()),
    _currCompletionStr(pOther._currCompletionStr)
{
}


LineEditHistoricalWrapper& LineEditHistoricalWrapper::operator=
(const LineEditHistoricalWrapper& pOther)
{
  _lineEdit = pOther._lineEdit;
  _isActivated = pOther._isActivated;
  _historical = pOther._historical;
  _currPos = _historical.end();
  _currCompletionStr = pOther._currCompletionStr;
  return *this;
}


void LineEditHistoricalWrapper::getHistorical
(std::vector<std::string>& pHistorical)
{
  std::copy(_historical.begin(), _historical.end(), std::back_inserter(pHistorical));
  _currPos = _historical.end();
}


void LineEditHistoricalWrapper::setHistorical
(const std::vector<std::string>& pHistorical)
{
  std::copy(pHistorical.begin(), pHistorical.end(), std::back_inserter(_historical));
  _currPos = _historical.end();
  _currCompletionStr = "";
  _lineEdit->clear();
}


void LineEditHistoricalWrapper::addNewText
(const std::string& pText,
 bool pCanClearOldTexts)
{
  if (pCanClearOldTexts)
  {
    auto itHist = _historical.end();
    while (itHist != _historical.begin())
    {
      --itHist;
      if (*itHist == pText)
      {
        _historical.erase(itHist);
        pCanClearOldTexts = false;
        break;
      }
    }
  }

  _historical.push_back(pText);
  if (pCanClearOldTexts &&
      _historical.size() > 100)
  {
    _historical.pop_front();
  }
}


void LineEditHistoricalWrapper::goToEndOfHistorical()
{
  _currPos = _historical.end();
  _currCompletionStr = "";
  _lineEdit->clear();
}


void LineEditHistoricalWrapper::activate()
{
  _isActivated = true;
}


void LineEditHistoricalWrapper::desactivate()
{
  _isActivated = false;
}


void LineEditHistoricalWrapper::displayPrevText()
{
  if (_isActivated && !_historical.empty())
  {
    std::string beginText;
    _refreshPos(beginText);
    std::size_t beginText_size = beginText.size();
    while (_currPos != _historical.begin())
    {
      --_currPos;
      if (_currPos->compare(0, beginText_size, beginText) == 0)
      {
        int cursorPosition = _lineEdit->cursorPosition();
        _lineEdit->setText(QString::fromUtf8(_currPos->c_str()));
        _lineEdit->setCursorPosition(cursorPosition);
        return;
      }
    }
  }
}


void LineEditHistoricalWrapper::displayNextText()
{
  if (_isActivated && !_historical.empty())
  {
    std::string beginText;
    _refreshPos(beginText);
    std::size_t beginText_size = beginText.size();
    while (_currPos != _historical.end())
    {
      ++_currPos;
      if (_currPos == _historical.end())
      {
        if (beginText.empty())
        {
          _lineEdit->clear();
        }
        return;
      }
      else if (_currPos->compare(0, beginText_size, beginText) == 0)
      {
        int cursorPosition = _lineEdit->cursorPosition();
        _lineEdit->setText(QString::fromUtf8(_currPos->c_str()));
        _lineEdit->setCursorPosition(cursorPosition);
        return;
      }
    }

    if (_currPos == _historical.end() &&
        beginText.empty())
    {
      _lineEdit->clear();
    }
  }
}


void LineEditHistoricalWrapper::concat()
{
  if (!_isActivated || _historical.empty())
  {
    return;
  }
  std::string beginText;
  _refreshPos(beginText);
  std::size_t beginText_size = beginText.size();
  std::list<std::list<std::string>::iterator> possibilities;
  while (_currPos != _historical.begin())
  {
    if (_currPos != _historical.end() &&
        _currPos->compare(0, beginText_size, beginText) == 0)
    {
      possibilities.push_back(_currPos);
    }
    --_currPos;
  }
  if (_currPos != _historical.end() &&
      _currPos->compare(0, beginText_size, beginText) == 0)
  {
    possibilities.push_back(_currPos);
  }

  if (!possibilities.empty())
  {
    _currPos = *possibilities.begin();
    std::string ref = *_currPos;
    std::size_t minSize = beginText_size;
    std::size_t currSize = ref.size();
    std::list<std::list<std::string>::iterator>::iterator
        itPossib = ++possibilities.begin();
    while (currSize > minSize &&
           itPossib != possibilities.end())
    {
      currSize = _updateMinAndMaxSize(currSize, minSize, **itPossib, ref);
      ++itPossib;
    }

    std::string newTextToPrint = ref.substr(0, currSize);
    std::string lineEditCurrText = _lineEdit->text().toUtf8().constData();
    if (newTextToPrint.size() > lineEditCurrText.size() ||
        lineEditCurrText.compare(0, newTextToPrint.size(), newTextToPrint) != 0)
    {
      _lineEdit->setText(QString::fromUtf8(newTextToPrint.c_str()));
    }
    _lineEdit->setCursorPosition(currSize);
  }
}


void LineEditHistoricalWrapper::_refreshPos(std::string& pBeginText)
{
  pBeginText = std::string(_lineEdit->text().toUtf8().constData()).substr(0, _lineEdit->cursorPosition());
  if (_currCompletionStr != pBeginText)
  {
    _currPos = _historical.end();
    _currCompletionStr = pBeginText;
  }
}


std::size_t LineEditHistoricalWrapper::_updateMinAndMaxSize(
    std::size_t pCurrSize,
    std::size_t pMinSize,
    const std::string& pNewText,
    const std::string& pRefText)
{
  std::size_t i = pMinSize;
  while (i < pNewText.size() && i < pCurrSize)
  {
    if (pNewText[i] != pRefText[i])
    {
      break;
    }
    ++i;
  }
  return i;
}


} // End of namespace onsem
