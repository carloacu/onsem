#ifndef ONSEMGUIUTILITY_LINEEDITHISTORICWRAPPER_HPP
#define ONSEMGUIUTILITY_LINEEDITHISTORICWRAPPER_HPP

#include <list>
#include <string>
#include <QLineEdit>

namespace onsem
{

class LineEditHistoricalWrapper
{
public:
  LineEditHistoricalWrapper();

  LineEditHistoricalWrapper(QLineEdit* pLineEdit,
                            QWidget* pHoldingWidget);

  LineEditHistoricalWrapper(const LineEditHistoricalWrapper&);
  LineEditHistoricalWrapper& operator=(const LineEditHistoricalWrapper&);

  void activate();

  void desactivate();

  void getHistorical(std::vector<std::string>& pHistorical);

  void setHistorical
  (const std::vector<std::string>& pHistorical);

  void addNewText
  (const std::string& pText,
   bool pCanClearOldTexts);

  void goToEndOfHistorical();

  void displayPrevText();

  void displayNextText();

  void concat();



private:
  QLineEdit* _lineEdit;
  bool _isActivated;
  std::list<std::string> _historical;
  std::list<std::string>::iterator _currPos;
  std::string _currCompletionStr;

  void _refreshPos(std::string& pBeginText);

  static std::size_t _updateMinAndMaxSize(std::size_t pCurrSize,
                                          std::size_t pMinSize,
                                          const std::string& pNewText,
                                          const std::string& pRefText);
};

} // End of namespace onsem

#endif // ONSEMGUIUTILITY_LINEEDITHISTORICWRAPPER_HPP
