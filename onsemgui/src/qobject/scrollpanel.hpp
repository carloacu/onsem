#ifndef SCROLLPANEL_H
#define SCROLLPANEL_H

#include <QWidget>
#include <QScrollBar>
#include <QFrame>

namespace onsem
{

class ScrollPanel : public QObject
{
  Q_OBJECT

public:
  ScrollPanel() = default;
  ~ScrollPanel();

  ScrollPanel(const ScrollPanel&) = delete;
  ScrollPanel& operator=(const ScrollPanel&) = delete;

  void setParentWidget
  (QWidget* pParent);

  void setGeometry
  (int pX, int pY, int pW, int pH);

  void setContentHeight
  (int pContentHeight);

  QWidget* getContentWidget() const;


private Q_SLOTS:

  void onVerticalScrollBarValueChanged
  (int pValue);


private:
  QScrollBar* fScrollBar = nullptr;
  QWidget* fViewWidget = nullptr;
  QWidget* fContentWidget = nullptr;
  int fContentHeight = 0;

  void xInitScrollBar();
};

} // End of namespace onsem

#endif // SCROLLPANEL_H
