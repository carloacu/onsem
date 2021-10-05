#include "scrollpanel.hpp"
#include <iostream>
#include <algorithm>
#include <assert.h>

namespace onsem
{

ScrollPanel::~ScrollPanel()
{
  if (fScrollBar)
  {
    delete fScrollBar;
    fScrollBar = nullptr;
  }
  if (fContentWidget)
  {
    delete fContentWidget;
    fContentWidget = nullptr;
  }
  if (fViewWidget)
  {
    delete fViewWidget;
    fViewWidget = nullptr;
  }
}



void ScrollPanel::setParentWidget
(QWidget* pParent)
{
  assert(fScrollBar == nullptr);

  fScrollBar = new QScrollBar(pParent);
  fViewWidget = new QWidget(pParent);
  fContentWidget = new QWidget(fViewWidget);

  fScrollBar->setValue(0);
  fScrollBar->setSingleStep(10);
  fScrollBar->show();
  fViewWidget->show();
  fContentWidget->show();

  connect(fScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onVerticalScrollBarValueChanged(int)));
}

void ScrollPanel::setGeometry
(int pX, int pY, int pW, int pH)
{
  fScrollBar->setGeometry(pX + pW - 16, pY, 16, pH);
  fViewWidget->setGeometry(pX, pY, pW - 26, pH);
  xInitScrollBar();
}


void ScrollPanel::setContentHeight
(int pContentHeight)
{
  fContentHeight = pContentHeight;
  xInitScrollBar();
}


void ScrollPanel::xInitScrollBar()
{
  fContentWidget->setGeometry(0, -fScrollBar->value(), fViewWidget->width(), fContentHeight);
  int max = std::max(fContentWidget->height(), fViewWidget->height()) - fViewWidget->height();
  if (fScrollBar->value() > max)
  {
    fScrollBar->setValue(max);
  }
  fScrollBar->setMaximum(max);
  fScrollBar->setPageStep(fViewWidget->height());
}

QWidget* ScrollPanel::getContentWidget
() const
{
  return fContentWidget;
}

void ScrollPanel::onVerticalScrollBarValueChanged
(int pValue)
{
  fContentWidget->setGeometry(fContentWidget->x(),
                              -pValue,
                              fContentWidget->width(),
                              fContentWidget->height());
}


} // End of namespace onsem
