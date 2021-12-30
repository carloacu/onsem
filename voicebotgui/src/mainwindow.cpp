#include "mainwindow.h"
#include <iostream>
#include <sstream>
#include <QString>
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QFileDialog>
#include "ui_mainwindow.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/executor/executorlogger.hpp>
#include <onsem/semantictotext/executor/executorcontext.hpp>
#include <onsem/semantictotext/executor/textexecutor.hpp>
#include <onsem/semantictotext/serialization.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticbehaviordefinition.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semanticdebugger/dotsaver.hpp>
#include <onsem/semanticdebugger/syntacticgraphresult.hpp>
#include <onsem/semanticdebugger/diagnosisprinter.hpp>
#include <onsem/tester/loadchatbot.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include <onsem/tester/syntacticanalysisxmlsaver.hpp>


namespace {
const QString _anyLangLabel = "Any Language";
const QString _currentResultsStr = "current results";
const QString _referenceResultsStr = "reference results";
const int _bottomBoxHeight = 70;
const int _leftTokens = 10;
const int _leftGramPoss = 170;
const int _leftFinalGramPoss = 390;
const int _leftFinalConcepts = 610;
const int _leftContextualInfos = 830;
static const QString microStr = "micro";
static const QString stopMicroStr = "stop";
const std::string _tmpFolder = ".";
}

using namespace onsem;

MainWindow::MainWindow(const boost::filesystem::path& pCorpusEquivalencesFolder,
                       const boost::filesystem::path& pCorpusResultsFolder,
                       const boost::filesystem::path& pInputScenariosFolder,
                       const boost::filesystem::path& pOutputScenariosFolder,
                       const boost::filesystem::path& pCorpusFolder,
                       linguistics::LinguisticDatabaseStreams& pIStreams,
                       QWidget *parent) :
  QMainWindow(parent),
  _ui(new Ui::MainWindow),
  _sizeWindow(0, 0),
  _corpusEquivalencesFolder(pCorpusEquivalencesFolder),
  _corpusResultsFolder(pCorpusResultsFolder),
  _inputScenariosFolder(pInputScenariosFolder),
  _outputScenariosFolder(pOutputScenariosFolder),
  _corpusFolder(pCorpusFolder),
  _listenToANewTokenizerStep(false),
  _lingDb(pIStreams),
  _currentLanguage(SemanticLanguageEnum::UNKNOWN),
  _currReformulationInSameLanguage(),
  fLangToTokenizerSteps(),
  _newOrOldVersion(true),
  _semMemoryPtr(mystd::make_unique<SemanticMemory>()),
  _semMemoryBinaryPtr(mystd::make_unique<SemanticMemory>()),
  _infActionAddedConnection(),
  _chatbotDomain(),
  _chatbotProblem(),
  _currentActionParameters(),
  _effectAfterCurrentInput(),
  _scenarioContainer(),
  _inLabel("in:"),
  _outFontColor("grey"),
  _inFontColor("black"),
  _isSpeaking(false),
  _nbOfSecondToWaitAfterTtsSpeech(0),
  _asrIsWaiting(true),
  _shouldWaitForNewSpeech(false),
  fTokensPanel(),
  fTokens(),
  fGramPossiblities(),
  fFinalGramPossiblities(),
  fFinalConcepts(),
  fContextualInfos(),
  fTaggedTokens(),
  fTokenTagsPossibilities(),
  fDispDotImage(false),
  fSentenceLoader(),
  _lineEditHistorical()
{
  _ui->setupUi(this);
  resize(1267, 750);
  _clearLoadedScenarios();

  _ui->pushButton_micro->setText(microStr);
  _ui->pushButton_history_microForChat->setText(microStr);
  _ui->textBrowser_genRep->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_genRep->setAttribute(Qt::WA_TranslucentBackground, true);

  _ui->textBrowser_AATester_Logs_Sentiments->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_AATester_Logs_Sentiments->setAttribute(Qt::WA_TranslucentBackground, true);

  _ui->textBrowser_AATester_Completeness_Value->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_AATester_Completeness_Value->setAttribute(Qt::WA_TranslucentBackground, true);

  _ui->textBrowser_AATester_Logs_Compare->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_AATester_Logs_Compare->setAttribute(Qt::WA_TranslucentBackground, true);

  _ui->textBrowser_AATester_Logs_Reformulation->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_AATester_Logs_Reformulation->setAttribute(Qt::WA_TranslucentBackground, true);
  _ui->pushButton_addEquivalence_AATester_Logs_Reformulation->setVisible(false);

  _ui->textBrowser_chat_history->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_chat_history->setAttribute(Qt::WA_TranslucentBackground, true);

  _ui->textBrowser_PrintMemory->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_PrintMemory->setAttribute(Qt::WA_TranslucentBackground, true);
  _ui->textBrowser_PrintMemory->setLineWrapMode(QTextEdit::NoWrap);

  _ui->tabWidget_AATester_Logs->setAttribute(Qt::WA_TranslucentBackground, true);

  _lineEditHistorical.emplace(_ui->lineEdit_history_newText,
                              LineEditHistoricalWrapper(_ui->lineEdit_history_newText, this));

  // Fit objects to the windows every seconds
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(onRefresh()));
  timer->start(100);

  fTokensPanel.setParentWidget(_ui->tab_AATester_Logs_Words);

  _ui->comboBox_AATester_languageSelection->clear();
  _ui->comboBox_AATester_languageSelection->addItem(_anyLangLabel);
  std::list<SemanticLanguageEnum> languageTypes;
  getAllLanguageTypes(languageTypes);
  for (std::list<SemanticLanguageEnum>::const_iterator
       it = languageTypes.begin(); it != languageTypes.end(); ++it)
    _ui->comboBox_AATester_languageSelection->addItem(QString::fromUtf8
                                                      (semanticLanguageEnum_toLegacyStr(*it).c_str()));
  _ui->lineEdit_AATester_InputSentence->setEnabled(true);

  // fill convertion output comboBox
  for (std::size_t i = 0; i < CONV_OUTPUT_TABLE_ENDFORNOCOMPILWARNING; ++i)
  {
    _ui->comboBox_AATester_convertionToShow->addItem
        (QString::fromUtf8(ConvertionOutputEnum_toStr[i].c_str()));
  }

  // fill tokenizer steps
  auto fillDebugSteps = [this](std::list<std::string>& pSteps,
      SemanticLanguageEnum pLanguageType)
  {
    const auto& specLingDb = _lingDb.langToSpec[pLanguageType];
    for (const auto& currContextFilter : specLingDb.getContextFilters())
      pSteps.emplace_back(currContextFilter->getName());
  };
  fillDebugSteps(fLangToTokenizerSteps[SemanticLanguageEnum::ENGLISH], SemanticLanguageEnum::ENGLISH);
  fillDebugSteps(fLangToTokenizerSteps[SemanticLanguageEnum::FRENCH], SemanticLanguageEnum::FRENCH);
  _updateCurrentLanguage(_currentLanguage);

  // fill ending steps
  _ui->comboBox_AATester_endingStep->clear();
  for (const auto& currDebugingStep : linguistics::linguisticAnalysisFinishDebugStepEnum_toStr)
    _ui->comboBox_AATester_endingStep->addItem(QString::fromUtf8(currDebugingStep.second.c_str()));
  _ui->comboBox_AATester_endingStep->setCurrentIndex(_ui->comboBox_AATester_endingStep->count() - 1);
  xSetSwitchVersionNewOrOld(true);

  SemanticTimeGrounding::setAnHardCodedTimeElts(true, true);
}

MainWindow::~MainWindow()
{
  _infActionAddedConnection.disconnect();
  delete _ui;
  _ui = nullptr;
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
  if (_ui == nullptr)
    return QObject::eventFilter(obj, event);
  auto itHistorical = _lineEditHistorical.find(obj);
  if (itHistorical != _lineEditHistorical.end())
  {
    if (event->type() == QEvent::FocusIn)
      itHistorical->second.activate();
    else if (event->type() == QEvent::FocusOut)
      itHistorical->second.desactivate();
  }
  return QObject::eventFilter(obj, event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
  case Qt::Key_Up:
  {
    for (auto& currHistorical : _lineEditHistorical)
      currHistorical.second.displayPrevText();
    break;
  }
  case Qt::Key_Down:
  {
    for (auto& currHistorical : _lineEditHistorical)
      currHistorical.second.displayNextText();
    break;
  }
  case Qt::Key_F1:
  {
    for (auto& currHistorical : _lineEditHistorical)
      currHistorical.second.concat();
    break;
  }
  }
}

void MainWindow::onRefresh()
{
  if (_sizeWindow.first != width() || _sizeWindow.second != height())
  {
    _sizeWindow.first = width();
    _sizeWindow.second = height();

    onRescale();
  }

  if (_isSpeaking)
  {
    std::ifstream ttsFinishedFile;
    ttsFinishedFile.open("tts_finished.txt");
    std::string line;
    _isSpeaking = getline(ttsFinishedFile, line) && line == "n";
    if (!_isSpeaking)
    {
      _nbOfSecondToWaitAfterTtsSpeech = 20;
      _asrIsWaiting = false;
      _shouldWaitForNewSpeech = true;
    }
  }
  else if (_nbOfSecondToWaitAfterTtsSpeech > 0)
  {
    --_nbOfSecondToWaitAfterTtsSpeech;
  }

  bool microEnabled = _ui->pushButton_micro->text() == stopMicroStr;
  bool microForChatEnabled = _ui->pushButton_history_microForChat->text() == stopMicroStr;
  if (microEnabled || microForChatEnabled)
  {
    bool textEnd = false;
    auto asrText = _getAsrText(textEnd);
    if (!asrText.empty())
    {
      auto asrTextQStr = QString::fromUtf8(asrText.c_str());
      if (microEnabled && _ui->lineEdit_AATester_InputSentence->text() != asrTextQStr)
        _ui->lineEdit_AATester_InputSentence->setText(asrTextQStr);
      if (microForChatEnabled && (textEnd || _ui->lineEdit_history_newText->text() != asrTextQStr))
      {
        _ui->lineEdit_history_newText->setText(asrTextQStr);
        if (textEnd)
          on_lineEdit_history_newText_returnPressed();
      }
    }
  }
}


void MainWindow::onRescaleTokens()
{
  const int offsetXY = 10;

  // position of tokenizer labels
  _ui->label_AATester_Logs_Words->setGeometry(_leftTokens + offsetXY, 10 + offsetXY, 150, 25);
  _ui->label_AATester_Logs_InitGrams->setGeometry(_leftGramPoss + offsetXY, 10 + offsetXY, 170, 25);
  _ui->label_AATester_Logs_FinalGrams->setGeometry(_leftFinalGramPoss + offsetXY, 10 + offsetXY, 170, 25);
  _ui->label_AATester_Logs_Concepts->setGeometry(_leftFinalConcepts + offsetXY, 10 + offsetXY, 170, 25);
  _ui->label_AATester_Logs_ContInfos->setGeometry(_leftContextualInfos + offsetXY, 10 + offsetXY, 170, 25);

  fTokensPanel.setGeometry(offsetXY, 40 + offsetXY, _ui->tab_AATester_Logs_Words->width() - 20,
                           _ui->tab_AATester_Logs_Words->height() - (40 + offsetXY + 10));
}


void MainWindow::xSetSwitchVersionNewOrOld(bool pNewOrOld)
{
  if (pNewOrOld)
    _ui->pushButton_AATester_SwitchVersion->setText(_currentResultsStr);
  else
    _ui->pushButton_AATester_SwitchVersion->setText(_referenceResultsStr);
  _newOrOldVersion = pNewOrOld;
}


std::string MainWindow::_getSelectedLanguageStr() const
{
  QString langStr = _ui->comboBox_AATester_languageSelection->currentText();
  if (langStr == _anyLangLabel)
    return "common";
  return langStr.toUtf8().constData();
}


SemanticLanguageEnum MainWindow::_getSelectedLanguageType()
{
  return semanticLanguageTypeGroundingEnumFromStr(_getSelectedLanguageStr());
}


void MainWindow::_updateCurrentLanguage(SemanticLanguageEnum pNewLanguage)
{
  if (pNewLanguage != _currentLanguage)
  {
    _ui->comboBox_tokenizer_endingStep->clear();
    auto itStepsForLanguage =
        fLangToTokenizerSteps.find(pNewLanguage);
    if (itStepsForLanguage != fLangToTokenizerSteps.end())
    {
      for (const auto& currStep : itStepsForLanguage->second)
      {
        _ui->comboBox_tokenizer_endingStep->addItem
            (QString::fromUtf8(currStep.c_str()));
      }
    }
    if (_ui->comboBox_tokenizer_endingStep->count() > 0)
    {
      _ui->comboBox_tokenizer_endingStep->addItem("finish");
      _ui->comboBox_tokenizer_endingStep->setCurrentIndex
          (_ui->comboBox_tokenizer_endingStep->count() - 1);
      _ui->comboBox_tokenizer_endingStep->setEnabled(true);
    }
    else
    {
      _ui->comboBox_tokenizer_endingStep->setEnabled(false);
    }
    _currentLanguage = pNewLanguage;
  }
}


void MainWindow::xPrintDotImage(const std::string& pDotContent) const
{
  const auto synDotFile = _tmpFolder + "/synt.dot";
  const auto synPngFile = _tmpFolder + "/synt.png";
  linguistics::DotSaver::save(synDotFile, synPngFile, pDotContent);
}


void MainWindow::on_lineEdit_AATester_InputSentence_textChanged(const QString &arg1)
{
  if (!_ui->lineEdit_AATester_InputSentence->isEnabled())
    return;

  _listenToANewTokenizerStep = false;
  SemanticLanguageEnum languageType = _getSelectedLanguageType();
  const std::string sentence = arg1.toUtf8().constData();
  if (languageType == SemanticLanguageEnum::UNKNOWN)
    languageType = linguistics::getLanguage(sentence, _lingDb);
  _updateCurrentLanguage(languageType);
  SyntacticAnalysisResultToDisplay autoAnnotToDisplay;

  SemanticAnalysisDebugOptions debugOptions;
  debugOptions.outputFormat = PrintSemExpDiffsOutPutFormat::HTML;
  debugOptions.convOutput = ConvertionOutputEnum(_ui->comboBox_AATester_convertionToShow->currentIndex());
  {
    debugOptions.endingStep.tokenizerEndingStep =
        _ui->comboBox_tokenizer_endingStep->currentText().toUtf8().constData();
    try
    {
      debugOptions.endingStep.nbOfDebugRoundsForTokens = boost::lexical_cast<std::size_t>
          (_ui->lineEdit_AATester_tokenizer_nbOfSteps->text().toUtf8().constData());
    }
    catch (...)
    {
      debugOptions.endingStep.nbOfDebugRoundsForTokens = 1;
    }

    debugOptions.endingStep.endingStep =
        linguistics::LinguisticAnalysisFinishDebugStepEnum(_ui->comboBox_AATester_endingStep->currentIndex());
    try
    {
      debugOptions.endingStep.nbOfDebugRoundsForSynthAnalysis = boost::lexical_cast<std::size_t>
          (_ui->lineEdit_AATester_synthGraph_nbOfSteps->text().toUtf8().constData());
    }
    catch (...)
    {
      debugOptions.endingStep.nbOfDebugRoundsForSynthAnalysis = 1;
    }
  }
  std::map<std::string, std::string> equivalences;
  _readEquivalences(equivalences, _getEquivalencesFilename());
  SemanticDebug::debugTextAnalyze(autoAnnotToDisplay, sentence,
                                  _spellingMistakeTypesPossibleForDebugOnTextComparisons,
                                  debugOptions, languageType, _lingDb, &equivalences);
  xDisplayResult(autoAnnotToDisplay);
  _listenToANewTokenizerStep = true;
}


void MainWindow::on_comboBox_tokenizer_endingStep_currentIndexChanged(int)
{
  if (_listenToANewTokenizerStep)
    on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
}


void MainWindow::_clearPrintTokens()
{
  for (int i = 0; i < fTokens.size(); ++i)
  {
    delete fTokens[i];
    delete fGramPossiblities[i];
    delete fFinalGramPossiblities[i];
    delete fFinalConcepts[i];
    delete fContextualInfos[i];
  }
  fTokens.clear();
  fGramPossiblities.clear();
  fFinalGramPossiblities.clear();
  fFinalConcepts.clear();
  fContextualInfos.clear();
}


void MainWindow::_clearPrintTags()
{
  for (int i = 0; i < fTaggedTokens.size(); ++i)
  {
    delete fTaggedTokens[i];
    delete fTokenTagsPossibilities[i];
  }
  fTaggedTokens.clear();
  fTokenTagsPossibilities.clear();
}


void MainWindow::xDisplayResult
(const SyntacticAnalysisResultToDisplay& pAutoAnnotToDisplay)
{
  xPrintDotImage(pAutoAnnotToDisplay.highLevelResults.syntGraphStr);
  _clearPrintTokens();
  _clearPrintTags();
  int idToken = 0;
  auto initGramsIt = pAutoAnnotToDisplay.highLevelResults.initialGramPossibilities.begin();
  auto finalGramsIt = pAutoAnnotToDisplay.finalGramPossibilities.begin();
  auto finalConceptsIt = pAutoAnnotToDisplay.finalConcepts.begin();
  auto contextualInfosIt = pAutoAnnotToDisplay.contextualInfos.begin();
  for (auto tokensIt = pAutoAnnotToDisplay.tokens.begin();
       tokensIt != pAutoAnnotToDisplay.tokens.end(); ++tokensIt, ++initGramsIt, ++finalGramsIt, ++finalConceptsIt, ++contextualInfosIt)
  {
    fTokens << new QLabel(fTokensPanel.getContentWidget());
    fGramPossiblities << new QComboBox(fTokensPanel.getContentWidget());
    fFinalGramPossiblities << new QComboBox(fTokensPanel.getContentWidget());
    fFinalConcepts << new QComboBox(fTokensPanel.getContentWidget());
    fContextualInfos << new QComboBox(fTokensPanel.getContentWidget());
    fTokens[idToken]->setGeometry(static_cast<int>(_leftTokens + tokensIt->first * 20),
                                  static_cast<int>(10 + idToken * 30), 150, 25);
    fGramPossiblities[idToken]->setGeometry(static_cast<int>(_leftGramPoss + tokensIt->first * 20),
                                            static_cast<int>(10 + idToken * 30), 200, 25);
    fFinalGramPossiblities[idToken]->setGeometry(static_cast<int>(_leftFinalGramPoss + tokensIt->first * 20),
                                                 static_cast<int>(10 + idToken * 30), 200, 25);
    fFinalConcepts[idToken]->setGeometry(static_cast<int>(_leftFinalConcepts + tokensIt->first * 20),
                                         static_cast<int>(10 + idToken * 30), 200, 25);
    fContextualInfos[idToken]->setGeometry(static_cast<int>(_leftContextualInfos + tokensIt->first * 20),
                                           static_cast<int>(10 + idToken * 30), 200, 25);
    fTokens[idToken]->setText(QString::fromUtf8(tokensIt->second.c_str()));
    for (const auto& currGram : *initGramsIt)
      fGramPossiblities[idToken]->addItem(QString::fromUtf8(currGram.c_str()));
    if (initGramsIt->size() == 1)
      fGramPossiblities[idToken]->setEnabled(false);
    for (const auto& currGram : *finalGramsIt)
      fFinalGramPossiblities[idToken]->addItem(QString::fromUtf8(currGram.c_str()));
    if (finalGramsIt->size() == 1)
      fFinalGramPossiblities[idToken]->setEnabled(false);
    for (const auto& currCpt : *finalConceptsIt)
      fFinalConcepts[idToken]->addItem(QString::fromUtf8(currCpt.c_str()));
    if (finalConceptsIt->size() == 1)
      fFinalConcepts[idToken]->setEnabled(false);
    for (const auto& currContInfo : *contextualInfosIt)
      fContextualInfos[idToken]->addItem(QString::fromUtf8(currContInfo.c_str()));
    if (contextualInfosIt->size() == 1)
      fContextualInfos[idToken]->setEnabled(false);


    fTokens[idToken]->show();
    fGramPossiblities[idToken]->show();
    fFinalGramPossiblities[idToken]->show();
    if (!finalConceptsIt->empty())
    {
      fFinalConcepts[idToken]->setVisible(true);
      fFinalConcepts[idToken]->show();
    }
    else
    {
      fFinalConcepts[idToken]->setVisible(false);
    }
    if (!contextualInfosIt->empty())
    {
      fContextualInfos[idToken]->setVisible(true);
      fContextualInfos[idToken]->show();
    }
    else
    {
      fContextualInfos[idToken]->setVisible(false);
    }
    ++idToken;
  }

  fTokensPanel.setContentHeight(10 + idToken * 30);

  idToken = 0;
  std::list<std::list<std::string> >::const_iterator
      tagsIt = pAutoAnnotToDisplay.taggedTokensTagsPossibilities.begin();
  for (std::list<std::string>::const_iterator tokensIt = pAutoAnnotToDisplay.taggedTokens.begin();
       tokensIt != pAutoAnnotToDisplay.taggedTokens.end();
       ++tokensIt, ++tagsIt)
  {
    fTaggedTokens << new QLabel(_ui->tabWidget_AATester_Logs);
    fTokenTagsPossibilities << new QComboBox(_ui->tabWidget_AATester_Logs);
    fTaggedTokens[idToken]->setGeometry(10, 45 + idToken * 30, 150, 25);
    fTokenTagsPossibilities[idToken]->setGeometry(170, 45 + idToken * 30, 110, 25);
    fTaggedTokens[idToken]->setText(QString::fromUtf8(tokensIt->c_str()));
    fTokenTagsPossibilities[idToken]->addItem(QString::fromUtf8(tagsIt->begin()->c_str()));
    fTokenTagsPossibilities[idToken]->setEnabled(false);
    fTaggedTokens[idToken]->show();
    fTokenTagsPossibilities[idToken]->show();
    ++idToken;
  }

  _ui->label_AATester_Logs_Performances->setText
      (QString::fromUtf8(pAutoAnnotToDisplay.performances.c_str()));

  _ui->textBrowser_genRep->setHtml
      (QString::fromUtf8(pAutoAnnotToDisplay.highLevelResults.semExpStr.c_str()));

  _ui->textBrowser_AATester_Logs_Sentiments->setText
      (QString::fromUtf8(pAutoAnnotToDisplay.highLevelResults.sentimentsInfos.c_str()));

  _ui->textBrowser_AATester_Completeness_Value->setText
      (pAutoAnnotToDisplay.highLevelResults.completeness ? "The text is complete" : "The text needs to be completed");

  std::string reformulationsStr = pAutoAnnotToDisplay.highLevelResults.reformulations;
  if (!pAutoAnnotToDisplay.isReformulationOk)
    reformulationsStr += "\n\n\n\n\nReformulation is maybe bad";
  _ui->textBrowser_AATester_Logs_Reformulation->setText(QString::fromUtf8(reformulationsStr.c_str()));
  _ui->pushButton_addEquivalence_AATester_Logs_Reformulation->setVisible
      (!pAutoAnnotToDisplay.isReformulationOk);
  _currReformulationInSameLanguage = pAutoAnnotToDisplay.highLevelResults.reformulationInputLanguage;

  fDispDotImage = true;
  const std::string syntPngFile = _tmpFolder + "/synt.png";
  _showImageInACanvas(syntPngFile,
                      *_ui->tab_AATester_Logs_SyntacticGraph,
                      *_ui->comboBox_AATester_syntGraph);
}


void MainWindow::onRescaleSynthGraph()
{
  _ui->widget_synthGraphEndingStep->setGeometry
      (_ui->tabWidget_AATester_Logs->width() - _ui->widget_synthGraphEndingStep->width() - 10,
       _ui->widget_synthGraphEndingStep->y(),
       _ui->widget_synthGraphEndingStep->width(),
       _ui->widget_synthGraphEndingStep->height());

  if (fDispDotImage)
  {
    const auto synthFilename = _tmpFolder + "/synt.png";
    _showImageInACanvas(synthFilename,
                        *_ui->tab_AATester_Logs_SyntacticGraph,
                        *_ui->comboBox_AATester_syntGraph);
  }
}


void MainWindow::_showImageInACanvas
(const std::string& pImagePath,
 const QWidget& pHoldingWidget,
 QLabel& pLabelWeToDisplayTheImage)
{
  QPixmap img;
  img.load(QString::fromUtf8(pImagePath.c_str()));
  if (img.isNull())
  {
    return;
  }

  float coefImg = static_cast<float>(img.width()) / static_cast<float>(img.height());
  float coefPanel = static_cast<float>(pHoldingWidget.width()) /
      static_cast<float>(pHoldingWidget.height());
  int x, y, w, h;
  y = 0;
  // if we will fit the image with the width
  if (coefImg > coefPanel)
  {
    x = 0;
    w = pHoldingWidget.width();
    h = static_cast<int>(static_cast<float>(w) / coefImg);
  }
  else // if we will fit the image with the height
  {
    h = pHoldingWidget.height();
    w = static_cast<int>(static_cast<float>(h) * coefImg);
    x = (pHoldingWidget.width() - w) / 2;
  }
  pLabelWeToDisplayTheImage.setPixmap(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  pLabelWeToDisplayTheImage.setGeometry(x, y, w, h);
}


void MainWindow::on_pushButton_AATester_PrevSentence_clicked()
{
  std::string sentence;
  fSentenceLoader.getPrevSentence(sentence);
  _ui->lineEdit_AATester_InputSentence->setText
      (QString::fromUtf8(sentence.c_str()));
  if (!_newOrOldVersion)
    xDisplayOldResult();
}


void MainWindow::on_pushButton_AATester_NextSentence_clicked()
{
  std::string sentence;
  fSentenceLoader.getNextSentence(sentence);
  _ui->lineEdit_AATester_InputSentence->setText
      (QString::fromUtf8(sentence.c_str()));
  if (!_newOrOldVersion)
    xDisplayOldResult();
}

void MainWindow::xDisplayOldResult()
{
  SyntacticAnalysisResultToDisplay autoAnnotToDisplay;
  const auto& resToDisp =
      *fSentenceLoader.getOldResults().oldResultThatDiffers[fSentenceLoader.getCurrIndex()];
  SemanticDebug::semAnalResultToStructToDisplay(autoAnnotToDisplay, resToDisp.semAnal);
  autoAnnotToDisplay.highLevelResults = resToDisp.semAnalHighLevelResults;
  xDisplayResult(autoAnnotToDisplay);
}


void MainWindow::on_pushButton_AATester_Logs_Compare_OldXml_clicked()
{
  const std::string selLanguageStr = _getSelectedLanguageStr();
  SentencesLoader sentencesXml;
  auto copusFolder = _corpusFolder.string() + "/input";
  sentencesXml.loadFolder(copusFolder + "/" + selLanguageStr);
  syntacticAnalysisXmlSaver::save(_corpusResultsFolder.string() + "/" +
                                  selLanguageStr + "_syntacticanalysis_results.xml",
                                  _getSelectedLanguageType(), sentencesXml.getSentences(), _lingDb);
}


void MainWindow::on_pushButton_AATester_Logs_Compare_NewXml_clicked()
{
  auto diffResults = std::make_shared<syntacticAnalysisXmlLoader::DeserializedTextResults>();
  diffResults->whatNeedToChecked.tokens =
      _ui->checkBox_regressiontests_tokens->isChecked();
  diffResults->whatNeedToChecked.tagsGram =
      _ui->checkBox_regressiontests_gramtags->isChecked();
  diffResults->whatNeedToChecked.tokConcepts =
      _ui->checkBox_regressiontests_concepts->isChecked();
  diffResults->whatNeedToChecked.syntaticGraph =
      _ui->checkBox_regressiontests_syntgraph->isChecked();
  diffResults->whatNeedToChecked.syntaticGraph =
      _ui->checkBox_regressiontests_syntgraph->isChecked();
  diffResults->whatNeedToChecked.sentimentsInfos =
      _ui->checkBox_regressiontests_sentimentsinfos->isChecked();
  diffResults->whatNeedToChecked.completeness =
      _ui->checkBox_regressiontests_completeness->isChecked();
  diffResults->whatNeedToChecked.semExps =
      _ui->checkBox_regressiontests_semexps->isChecked();
  diffResults->whatNeedToChecked.reformulations =
      _ui->checkBox_regressiontests_reformulations->isChecked();
  diffResults->whatNeedToChecked.input_reformulation =
      _ui->checkBox_regressiontests_inputReformulation->isChecked();

  std::map<std::string, std::string> equivalences;
  if (diffResults->whatNeedToChecked.input_reformulation)
    _readEquivalences(equivalences, _getEquivalencesFilename());
  std::string performances;
  syntacticAnalysisXmlSaver::compareResults(diffResults, _getSelectedLanguageStr(), _lingDb, _corpusResultsFolder.string(),
                                            &equivalences, &performances);
  _ui->label_AATester_Logs_Performances->setText(QString::fromUtf8(performances.c_str()));

  _ui->textBrowser_AATester_Logs_Compare->setText(QString::fromUtf8(diffResults->bilan.c_str()));
  if (!diffResults->diffsInputSentences.empty())
  {
    xSetSwitchVersionNewOrOld(true);
    _ui->pushButton_AATester_SwitchVersion->setEnabled(true);

    fSentenceLoader.loadSentencesWithOldResults(diffResults);
    _ui->pushButton_AATester_PrevSentence->setEnabled(true);
    _ui->pushButton_AATester_NextSentence->setEnabled(true);
  }
}

void MainWindow::onRescale()
{
  onRescaleLinguisticAnalyzerPanel();
  onRescaleChatPanel();
}

void MainWindow::onRescaleLinguisticAnalyzerPanel()
{
  _ui->comboBox_AATester_languageSelection->setGeometry(_ui->tab_AATester->width() - _ui->comboBox_AATester_languageSelection->width() - 10,
                                                        _ui->comboBox_AATester_languageSelection->y(),
                                                        _ui->comboBox_AATester_languageSelection->width(),
                                                        _ui->comboBox_AATester_languageSelection->height());

  _ui->label_AATester_title->setGeometry(0, 10, _ui->tab_AATester->width(), 27);
  int textEditX = _ui->tab_AATester->width() / 4;
  int textEditW = _ui->tab_AATester->width() / 2;
  _ui->lineEdit_AATester_InputSentence->setGeometry(textEditX, _ui->lineEdit_AATester_InputSentence->y(),
                                                    textEditW, _ui->lineEdit_AATester_InputSentence->height());
  _ui->pushButton_micro->setGeometry(textEditX + textEditW + 10, _ui->pushButton_micro->y(),
                                     _ui->pushButton_micro->width(), _ui->pushButton_micro->height());

  _ui->tabWidget_AATester_Logs->setGeometry(10, _ui->tabWidget_AATester_Logs->y(), _ui->tab_AATester->width() - 20,
                                            _ui->tab_AATester->height() - _ui->tabWidget_AATester_Logs->y() - 10);

  _ui->comboBox_AATester_convertionToShow->setGeometry(_ui->tabWidget_AATester_Logs->width() - _ui->comboBox_AATester_convertionToShow->width() - 40,
                                                       _ui->comboBox_AATester_convertionToShow->y(),
                                                       _ui->comboBox_AATester_convertionToShow->width(),
                                                       _ui->comboBox_AATester_convertionToShow->height());

  onRescaleTokens();
  onRescaleSynthGraph();
  onRescaleSentiments();
  onRescaleGenRep();
}

void MainWindow::onRescaleChatPanel()
{
  int lineEditHeight = 40;
  int grouBoxSidesWidth = 130;
  int asrFrameY = _ui->tab_Chat->height() - _bottomBoxHeight - 10;

  _ui->label_chat_title->setGeometry(10, 10, _ui->tab_Chat->width(), _ui->label_chat_title->height());
  {
    int mainFrameY = 90;
    int asrFrameY = _ui->tab_Chat->height() - _bottomBoxHeight - 10;

    _ui->textBrowser_chat_history->setGeometry(10, mainFrameY,
                                               _ui->tab_Chat->width() - 20, asrFrameY - mainFrameY - 20);
  }


  // speech frame
  _ui->frame_history_asr->setGeometry(10, asrFrameY, _ui->tab_Chat->width() - 20, _bottomBoxHeight);
  _ui->lineEdit_history_newText->setGeometry(20 + grouBoxSidesWidth, 15,
                                             _ui->frame_history_asr->width() - 40 - grouBoxSidesWidth * 2, lineEditHeight);
  _ui->widgetRight_history->setGeometry(_ui->frame_history_asr->width() - 10 - grouBoxSidesWidth, 0,
                                        grouBoxSidesWidth, _bottomBoxHeight);
}


void MainWindow::onRescaleChatDiagnosisPanel()
{
  _ui->textBrowser_PrintMemory->setGeometry(10, 100, _ui->tab_chatdiagnosis->width() - 20,
                                            _ui->tab_chatdiagnosis->height() - 110);
}


void MainWindow::onRescaleGenRep()
{
  _ui->textBrowser_genRep->setGeometry(10, 10, _ui->tab_AATester_Logs_GenRep->width() - 20,
                                       _ui->tab_AATester_Logs_GenRep->height() - 20);
}


void MainWindow::onRescaleSentiments()
{
  _ui->textBrowser_AATester_Logs_Sentiments->setGeometry(10, 10, _ui->tab_AATester_Logs_Sentiments->width() - 20,
                                                         _ui->tab_AATester_Logs_Sentiments->height() - 20);
}



void MainWindow::on_tabWidget_AATester_Logs_currentChanged(int index)
{
  switch (index)
  {
  case 0:
    onRescaleTokens();
    break;
  case 1:
    onRescaleSynthGraph();
    break;

  case 2:
    onRescaleGenRep();
    break;
  case 4:
    onRescaleSentiments();
    break;
  }
}

void MainWindow::on_pushButton_AATester_SwitchVersion_clicked()
{
  xSetSwitchVersionNewOrOld(!_newOrOldVersion);
  _ui->lineEdit_AATester_InputSentence->setEnabled(_newOrOldVersion);
  if (_newOrOldVersion)
  {
    on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
  }
  else
  {
    std::string currText;
    fSentenceLoader.getCurrSentence(currText);
    _ui->lineEdit_AATester_InputSentence->setText
        (QString::fromUtf8(currText.c_str()));
    xDisplayOldResult();
  }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
  switch (index)
  {
  case 0:
    onRescaleLinguisticAnalyzerPanel();
    break;
  case 1:
    onRescaleChatPanel();
    break;
  case 2:
    onRescaleChatDiagnosisPanel();
    break;
  }
}

void MainWindow::on_comboBox_AATester_endingStep_currentIndexChanged(int)
{
  on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
}

void MainWindow::on_comboBox_AATester_languageSelection_currentIndexChanged(int index)
{
  on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
  _ui->textBrowser_AATester_Logs_Compare->setText("");
  _updateCurrentLanguage(_getSelectedLanguageType());
}

void MainWindow::on_pushButton_MemoryClearer_RefreshPkg_clicked()
{
  _ui->textBrowser_MemoryClearer_DisplayPkg->setText
      (QString::fromUtf8
       (diagnosisPrinter::diagnosis({"loadedDatabases"}, SemanticMemory(), _lingDb).c_str()));
}


void MainWindow::on_texts_load_triggered()
{
  xSetSwitchVersionNewOrOld(true);
  _ui->pushButton_AATester_SwitchVersion->setEnabled(false);

  SemanticLanguageEnum langType = _getSelectedLanguageType();
  const std::string textCorpusFolder = _corpusFolder.string() + "/input";
  if (langType == SemanticLanguageEnum::UNKNOWN)
    _loadSentences(true, textCorpusFolder);
  else
    _loadSentences(true, textCorpusFolder + "/" + semanticLanguageEnum_toLegacyStr(langType));

  _ui->pushButton_AATester_PrevSentence->setEnabled(true);
  _ui->pushButton_AATester_NextSentence->setEnabled(true);
}


void MainWindow::_loadSentences
(bool pTxtFirstChoice,
 const std::string& pTextCorpusPath)
{
  std::string filter = pTxtFirstChoice ?
        "Text file (*.txt);;Xml file (*.xml)" :
        "Xml file (*.xml);;Text file (*.txt)";
  QString fichier = QFileDialog::getOpenFileName
      (this, "Load a sentences file", QString::fromUtf8(pTextCorpusPath.c_str()), filter.c_str());
  std::string filename = fichier.toUtf8().constData();
  if (!filename.empty())
    fSentenceLoader.loadFile(filename);
}

void MainWindow::on_comboBox_AATester_convertionToShow_currentIndexChanged(int)
{
  on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
}


void MainWindow::on_lineEdit_AATester_synthGraph_nbOfSteps_textChanged(const QString&)
{
  on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
}


void MainWindow::on_actionExport_to_ldic_triggered()
{
  const QString extensionLdic = ".ldic";
  const QString firstStr = "Dictionaries (*" + extensionLdic + ")";
  QString selectedFilter;
  QString fileToWrite = QFileDialog::getSaveFileName
      (this, "Export the dictionaries", QString(),
       firstStr, &selectedFilter);
  std::string fileToWriteStr = std::string(fileToWrite.toUtf8().constData());
  if (fileToWriteStr.empty())
    return;
  fileToWriteStr += extensionLdic.toUtf8().constData();
  boost::property_tree::ptree propTree;
  serialization::saveLingDatabase(propTree, _lingDb);
  serialization::propertyTreeToZipedFile(propTree, fileToWriteStr, ".ldic");
}


void MainWindow::on_actionImport_from_ldic_triggered()
{
  const QString extensionLdic = ".ldic";
  const QString firstStr = "Dictionaries (*" + extensionLdic + ")";

  std::string filename = QFileDialog::getOpenFileName
      (this, "Import dictionaries", QString(), firstStr).toUtf8().constData();
  if (filename.empty())
    return;
  boost::property_tree::ptree propTree;
  serialization::propertyTreeFromZippedFile(propTree, filename);
  serialization::loadLingDatabase(propTree, _lingDb);
}

void MainWindow::on_lineEdit_history_newText_returnPressed()
{
  _onNewTextSubmitted(_ui->lineEdit_history_newText->text().toUtf8().constData());
  _ui->lineEdit_history_newText->clear();
}





std::string MainWindow::_operator_react(
    ContextualAnnotation& pContextualAnnotation,
    std::list<std::string>& pReferences,
    const std::string& pText,
    SemanticLanguageEnum& pTextLanguage)
{
  auto& semMemory = *_semMemoryPtr;
  if (pTextLanguage == SemanticLanguageEnum::UNKNOWN)
    pTextLanguage = linguistics::getLanguage(pText, _lingDb);
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  pTextLanguage);
  inContext.spellingMistakeTypesPossible.insert(SpellingMistakeType::CONJUGATION);
  auto semExp =
      converter::textToContextualSemExp(pText, inContext,
                                        SemanticSourceEnum::ASR, _lingDb);

  auto urlizeInput = mystd::urlizeText(pText);
  const ChatbotSemExpParam* paramSelectedPtr = nullptr;
  for (const auto& currParam : _currentActionParameters)
  {
    if (urlizeInput == currParam.urlizeInput ||
        SemExpComparator::getSemExpsImbrications(*semExp, *currParam.semExp, semMemory.memBloc, _lingDb, nullptr) != ImbricationType::DIFFERS)
    {
      paramSelectedPtr = &currParam;
      break;
    }
  }
  memoryOperation::mergeWithContext(semExp, semMemory, _lingDb);
  if (paramSelectedPtr == nullptr)
  {
    for (const auto& currParam : _currentActionParameters)
    {
      if (SemExpComparator::getSemExpsImbrications(*semExp, *currParam.semExpMergedWithContext, semMemory.memBloc, _lingDb, nullptr) != ImbricationType::DIFFERS)
      {
        paramSelectedPtr = &currParam;
        break;
      }
    }
  }

  if (pTextLanguage == SemanticLanguageEnum::UNKNOWN)
    pTextLanguage = semMemory.defaultLanguage;
  mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
  if (_effectAfterCurrentInput && _chatbotProblem)
    _chatbotProblem->problem.modifyFacts(*_effectAfterCurrentInput);
  if (paramSelectedPtr != nullptr && _chatbotProblem)
  {
    if (!paramSelectedPtr->goalsToAdd.empty())
      _chatbotProblem->problem.addGoals(paramSelectedPtr->goalsToAdd);
    _chatbotProblem->problem.modifyFacts(paramSelectedPtr->effect);
  }
  else
  {
    memoryOperation::react(reaction, semMemory, std::move(semExp), _lingDb, nullptr);
  }
  if (!reaction)
    return "";
  pContextualAnnotation = SemExpGetter::extractContextualAnnotation(**reaction);
  SemExpGetter::extractReferences(pReferences, **reaction);

  TextProcessingContext outContext(SemanticAgentGrounding::me,
                                   SemanticAgentGrounding::currentUser,
                                   pTextLanguage);
  auto execContext = std::make_shared<ExecutorContext>(outContext);
  std::string res;
  ExecutorLoggerWithoutMetaInformation logger(res);
  TextExecutor textExec(semMemory, _lingDb, logger);
  textExec.runSemExp(std::move(*reaction), execContext);
  return res;
}



void MainWindow::_onNewTextSubmitted(const std::string& pText)
{
  _ui->textBrowser_chat_history->setTextColor(_inFontColor);
  _ui->textBrowser_chat_history->append(QString::fromUtf8(_inLabel.c_str()) + " \"" +
                                        QString::fromUtf8(pText.c_str()) + "\"");

  LineEditHistoricalWrapper& hWrapper = _lineEditHistorical[_ui->lineEdit_history_newText];
  hWrapper.addNewText(pText, true);
  hWrapper.goToEndOfHistorical();

  auto textLanguage = SemanticLanguageEnum::UNKNOWN;
  auto contextualAnnotation = ContextualAnnotation::ANSWER;
  std::list<TextWithLanguage> textsToSay;
  std::list<std::string> references;
  auto text =
      _operator_react(contextualAnnotation, references, pText, textLanguage);
  bool actinHasBeenPrinted = false;
  if (!text.empty())
  {
    _printChatRobotMessage(text);
    textsToSay.emplace_back(text, textLanguage);

    for (const auto& currRef : references)
    {
      auto beginOfActionIdSize = beginOfActionId.size();
      if (currRef.compare(0, beginOfActionIdSize, beginOfActionId) == 0)
      {
        auto actionId = currRef.substr(beginOfActionIdSize, currRef.size() - beginOfActionIdSize);
        if (!actionId.empty())
        {
          auto itAction = _chatbotDomain->actions.find(actionId);
          if (itAction != _chatbotDomain->actions.end())
          {
            auto& action = itAction->second;
            _currentActionParameters.clear();
            _effectAfterCurrentInput.reset();
            std::map<std::string, std::string> parameters;
            _printParametersAndNotifyPlanner(action, actionId, parameters);
            actinHasBeenPrinted = true;
            break;
          }
        }
      }
    }
  }

  if (!actinHasBeenPrinted)
  {
    if (contextualAnnotation != ContextualAnnotation::QUESTION)
    {
      std::set<std::string> actionIdsToSkip;
      _proactivityFromPlanner(textsToSay, actionIdsToSkip);
    }
    else
    {
      _currentActionParameters.clear();
      _effectAfterCurrentInput.reset();
    }
  }

  if (_ui->checkBox_enable_tts->isChecked() && !textsToSay.empty())
    _sayText(textsToSay);
}


void MainWindow::_proactivityFromPlanner(std::list<TextWithLanguage>& pTextsToSay,
                                         std::set<std::string>& pActionIdsToSkip)
{
  std::string textToSay;
  _currentActionParameters.clear();
  _effectAfterCurrentInput.reset();
  if (_chatbotDomain && _chatbotProblem)
  {
    std::map<std::string, std::string> parameters;
    auto actionId = cp::lookForAnActionToDo(parameters, _chatbotProblem->problem, *_chatbotDomain->compiledDomain, &_chatbotProblem->problem.historical);
    if (!actionId.empty() && pActionIdsToSkip.count(actionId) == 0)
    {
      auto itAction = _chatbotDomain->actions.find(actionId);
      if (itAction != _chatbotDomain->actions.end())
      {
        auto& action = itAction->second;
        std::string text = action.text;
        cp::replaceVariables(text, _chatbotProblem->problem);
        _printChatRobotMessage(text);
        pTextsToSay.emplace_back(text, action.language);

        // notify memory of the text said
        {
          TextProcessingContext outContext(SemanticAgentGrounding::me,
                                           SemanticAgentGrounding::currentUser,
                                           action.language);
          auto semExp =
              converter::textToContextualSemExp(text, outContext,
                                                SemanticSourceEnum::ASR, _lingDb);
          memoryOperation::mergeWithContext(semExp, *_semMemoryPtr, _lingDb);
          memoryOperation::inform(std::move(semExp), *_semMemoryPtr, _lingDb);
        }
        _printParametersAndNotifyPlanner(action, actionId, parameters);
        if (action.parameters.empty() && !action.inputPtr)
        {
          pActionIdsToSkip.insert(actionId);
          _proactivityFromPlanner(pTextsToSay, pActionIdsToSkip);
        }
      }
    }
  }
}


void MainWindow::_printParametersAndNotifyPlanner(const ChatbotAction& pAction,
                                                  const std::string& pActionId,
                                                  const std::map<std::string, std::string>& pParameters)
{
  std::string paramLines;
  for (const auto& currParameter : pAction.parameters)
  {
    if (!paramLines.empty())
      paramLines += "   |";
    paramLines += "   \"" + currParameter.text + "\"";
    TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                    SemanticAgentGrounding::me,
                                    pAction.language);
    auto semExp =
        converter::textToContextualSemExp(currParameter.text, inContext,
                                          SemanticSourceEnum::UNKNOWN, _lingDb);
    _currentActionParameters.emplace_back();
    auto& actionParam = _currentActionParameters.back();
    actionParam.urlizeInput = mystd::urlizeText(currParameter.text);
    actionParam.semExp = semExp->clone();
    memoryOperation::mergeWithContext(semExp, *_semMemoryPtr, _lingDb);
    actionParam.semExpMergedWithContext = std::move(semExp);
    actionParam.effect = currParameter.effect;
    actionParam.goalsToAdd = currParameter.goalsToAdd;
  }
  if (pAction.inputPtr)
    _effectAfterCurrentInput = mystd::make_unique<cp::SetOfFacts>(pAction.inputPtr->effect);
  if (!paramLines.empty())
  {
    _ui->textBrowser_chat_history->setTextColor(_outFontColor);
    _ui->textBrowser_chat_history->append(QString::fromUtf8(paramLines.c_str()));
  }

  _chatbotProblem->problem.notifyActionDone(pActionId, pParameters, pAction.effect, &pAction.goalsToAdd);
}


void MainWindow::_printChatRobotMessage(const std::string& pText)
{
  _ui->textBrowser_chat_history->setTextColor(_outFontColor);
  _ui->textBrowser_chat_history->append("out: \"" + QString::fromUtf8(pText.c_str()) + "\"");
}


void MainWindow::_sayText(std::list<TextWithLanguage>& pTextsToSay)
{
  system("echo 'n' > tts_finished.txt");
  std::string comandLine;
  std::size_t i = 0;
  for (auto& currTextToSay : pTextsToSay)
  {
    boost::replace_all(currTextToSay.text, "\"", "\\\"");
    const std::string languageCode = currTextToSay.language == SemanticLanguageEnum::FRENCH ? "fr" : "en";
    std::stringstream ssOutSoundFilename;
    ssOutSoundFilename << "out_tts_" << i << ".mp3";
    std::string outSoundFilename = ssOutSoundFilename.str();
    comandLine += "gtts-cli \"" + currTextToSay.text + "\" --output " + outSoundFilename + " -l " + languageCode + " && ";
    comandLine += "play " + outSoundFilename + " && rm " + outSoundFilename + "  && ";
    ++i;
  }
  comandLine += "echo 'y' > tts_finished.txt &";
  system(comandLine.c_str());
  _isSpeaking = true;
}



void MainWindow::on_actionNew_triggered()
{
  _clearLoadedScenarios();
}

void MainWindow::on_actionQuit_triggered()
{
  this->close();
}


void MainWindow::on_actionSave_triggered()
{
  // Save the current scenario to a file
  const std::string scenarioContent =
      _ui->textBrowser_chat_history->document()->toPlainText().toUtf8().constData();
  if (scenarioContent.empty())
  {
    QMessageBox::warning(this, this->tr("Nothing to save"),
                         this->tr("The current scenario is empty."),
                         QMessageBox::Ok);
    return;
  }

  std::string pathToFile;
  if (_scenarioContainer.isEmpty())
  {
    bool okOfEnterScenarioName;
    QString fileName = QInputDialog::getText(this, this->tr("Enter Scenario name"),
                                             this->tr("Scenario name:"),
                                             QLineEdit::Normal, "",
                                             &okOfEnterScenarioName);
    if (!okOfEnterScenarioName || fileName.isEmpty())
    {
      return;
    }

    pathToFile = _inputScenariosFolder.string() + "/" + std::string(fileName.toUtf8().constData()) + ".txt";
    if (ScenarioContainer::doesFileExist(pathToFile))
    {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, "Scenario name already exist",
                                    "Do you want to overwrite it ?",
                                    QMessageBox::Yes | QMessageBox::No);
      if (reply != QMessageBox::Yes)
        return;
    }
  }
  else
  {
    _scenarioContainer.getCurrScenarioFilename(pathToFile);
    pathToFile = _inputScenariosFolder.string() + "/" + pathToFile;
  }

  std::list<std::string> inputLabels{_inLabel};
  ScenarioContainer::writeScenarioToFile(pathToFile, scenarioContent, inputLabels);
}

void MainWindow::on_scenarios_load_triggered()
{
  _scenarioContainer.load(_inputScenariosFolder.string());
  _loadCurrScenario();
}


void MainWindow::_loadCurrScenario()
{
  _ui->pushButton_Chat_PrevScenario->setEnabled(true);
  _ui->pushButton_Chat_NextScenario->setEnabled(true);

  std::string scenarioFilename;
  _scenarioContainer.getCurrScenarioFilename(scenarioFilename);

  std::list<std::string> linesToDisplay;
  _scenarioContainer.getOldContent(linesToDisplay);
  _switchToReferenceButtonSetEnabled(!linesToDisplay.empty());

  if (_ui->pushButton_Chat_SwitchToReference->text() == _referenceResultsStr)
  {
    this->setWindowTitle("reference - " + QString::fromUtf8(scenarioFilename.c_str()));
  }
  else
  {
    this->setWindowTitle(QString::fromUtf8(scenarioFilename.c_str()));
    linesToDisplay.clear();
    getResultOfAScenario(linesToDisplay,
                         (_inputScenariosFolder / scenarioFilename).string(), *_semMemoryPtr, _lingDb);
  }
  _ui->textBrowser_chat_history->clear();
  _appendLogs(linesToDisplay);
}


void MainWindow::_switchToReferenceButtonSetEnabled(bool pEnabled)
{
  bool allowToEnterANewText = true;
  if (pEnabled)
  {
    _ui->pushButton_Chat_SwitchToReference->setEnabled(true);
    allowToEnterANewText = _ui->pushButton_Chat_SwitchToReference->text() == _currentResultsStr;
  }
  else
  {
    _ui->pushButton_Chat_SwitchToReference->setEnabled(false);
    _ui->pushButton_Chat_SwitchToReference->setText(_currentResultsStr);
    allowToEnterANewText = true;
  }
  _ui->lineEdit_history_newText->setEnabled(allowToEnterANewText);
  _ui->pushButton_history_microForChat->setEnabled(allowToEnterANewText);
}


void MainWindow::_appendLogs(const std::list<std::string>& pLogs)
{
  for (const auto& currLog : pLogs)
  {
    _ui->textBrowser_chat_history->setTextColor
        (currLog.compare(0, _inLabel.size(), _inLabel) == 0 ?
           _inFontColor : _outFontColor);
    _ui->textBrowser_chat_history->append(QString::fromUtf8(currLog.c_str()));
  }
}


void MainWindow::_clearLoadedScenarios()
{
  this->setWindowTitle("Semantic reasoner viewer");
  auto& semMemory = *_semMemoryPtr;
  semMemory.clear();
  _semMemoryBinaryPtr->clear();
  // clear the planner
  {
    _chatbotDomain.reset();
    _chatbotProblem.reset();
    _currentActionParameters.clear();
    _effectAfterCurrentInput.reset();
  }
  _lingDb.reset();
  memoryOperation::defaultKnowledge(semMemory, _lingDb);

  _infActionAddedConnection.disconnect();
  _infActionAddedConnection =
      semMemory.memBloc.infActionAdded.connectUnsafe([&](intSemId, const SemanticMemorySentence* pMemorySentencePtr)
  {
    if (_chatbotProblem && pMemorySentencePtr != nullptr)
    {
      auto textProcToRobot = TextProcessingContext::getTextProcessingContextToRobot(SemanticLanguageEnum::FRENCH);
      auto textProcFromRobot = TextProcessingContext::getTextProcessingContextFromRobot(SemanticLanguageEnum::FRENCH);
      auto behaviorDef = SemanticMemoryBlock::extractActionFromMemorySentence(*pMemorySentencePtr);
      UniqueSemanticExpression formulation1;
      UniqueSemanticExpression formulation2;
      converter::getInfinitiveToTwoDifferentPossibleWayToAskForIt(formulation1, formulation2, std::move(behaviorDef.label));
      std::map<std::string, std::string> varToValue;
      converter::semExpToText(varToValue["comportement_appris"], std::move(formulation1),
                              textProcToRobot, false, semMemory, _lingDb, nullptr);
      converter::semExpToText(varToValue["comportement_appris_2"], std::move(formulation2),
                              textProcToRobot, false, semMemory, _lingDb, nullptr);
      converter::semExpToText(varToValue["comportement_appris_resultat"], converter::getFutureIndicativeFromInfinitive(std::move(behaviorDef.composition)),
                              textProcFromRobot, false, semMemory, _lingDb, nullptr);
      _chatbotProblem->problem.addVariablesToValue(varToValue);
      _chatbotProblem->problem.addFact(cp::Fact("robot_learnt_a_behavior"));
    }
  });

  _scenarioContainer.clear();
  _ui->textBrowser_chat_history->clear();
  _ui->pushButton_Chat_PrevScenario->setEnabled(false);
  _ui->pushButton_Chat_NextScenario->setEnabled(false);
  _switchToReferenceButtonSetEnabled(false);
}

void MainWindow::on_pushButton_Chat_PrevScenario_clicked()
{
  _scenarioContainer.getMoveToPrevScenario();
  _loadCurrScenario();
}

void MainWindow::on_pushButton_Chat_NextScenario_clicked()
{
  _scenarioContainer.getMoveToNextScenario();
  _loadCurrScenario();
}


void MainWindow::on_pushButton_Chat_SwitchToReference_clicked()
{
  if (_ui->pushButton_Chat_SwitchToReference->text() == _currentResultsStr)
    _ui->pushButton_Chat_SwitchToReference->setText(_referenceResultsStr);
  else
    _ui->pushButton_Chat_SwitchToReference->setText(_currentResultsStr);
  _loadCurrScenario();
}

void MainWindow::on_pushButton_History_compareResults_clicked()
{
  std::string bilan;
  _ui->textBrowser_Chat_RegressionTests->clear();
  if (_scenarioContainer.compareScenariosToReferenceResults(bilan, _inputScenariosFolder.string(),
                                                            _outputScenariosFolder.string(),
                                                            &getResultOfAScenario, _lingDb))
  {
    _ui->pushButton_Chat_PrevScenario->setEnabled(true);
    _ui->pushButton_Chat_NextScenario->setEnabled(true);
    _loadCurrScenario();
  }
  else
  {
    _clearLoadedScenarios();
  }
  _ui->textBrowser_Chat_RegressionTests->append(QString::fromUtf8(bilan.c_str()));
}

void MainWindow::on_pushButton_History_updateResults_clicked()
{
  _scenarioContainer.updateScenariosResults(_inputScenariosFolder.string(), _outputScenariosFolder.string(),
                                            &getResultOfAScenario, _lingDb);
}

void MainWindow::on_pushButton_clicked()
{
  _ui->textBrowser_PrintMemory->setText
      (QString::fromUtf8
       (diagnosisPrinter::diagnosis({"memory", "memoryInformations"}, *_semMemoryPtr, _lingDb).c_str()));
}

void MainWindow::on_pushButton_addEquivalence_AATester_Logs_Reformulation_clicked()
{
  std::string inStr = _ui->lineEdit_AATester_InputSentence->text().toUtf8().constData();
  auto equivalencesFilename = _getEquivalencesFilename();

  std::map<std::string, std::string> equivalences;
  equivalences.emplace(inStr, _currReformulationInSameLanguage);
  _readEquivalences(equivalences, equivalencesFilename);
  _writeEquivalences(equivalences, equivalencesFilename);
  on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
}


boost::filesystem::path MainWindow::_getEquivalencesFilename()
{
  std::string languageStr = semanticLanguageEnum_toLegacyStr(_currentLanguage);
  return _corpusEquivalencesFolder /
      boost::filesystem::path(languageStr + "_equivalences.xml");
}

void MainWindow::_readEquivalences(std::map<std::string, std::string>& pEquivalences,
                                   const boost::filesystem::path& pPath)
{
  try
  {
    boost::property_tree::ptree tree;
    boost::property_tree::read_xml(pPath.string(), tree);
    for (const auto& currSubTree : tree.get_child("equivalences"))
    {
      const boost::property_tree::ptree& attrs = currSubTree.second.get_child("<xmlattr>");
      pEquivalences.emplace(attrs.get<std::string>("in"),
                            attrs.get<std::string>("out"));
    }
  }
  catch (...) {}
}

void MainWindow::_writeEquivalences(const std::map<std::string, std::string>& pEquivalences,
                                    const boost::filesystem::path& pPath)
{
  boost::property_tree::ptree tree;
  boost::property_tree::ptree& resultsTree = tree.add_child("equivalences", {});

  for (const auto& currEqu : pEquivalences)
  {
    boost::property_tree::ptree& textTree = resultsTree.add_child("text", {});
    textTree.put("<xmlattr>.in", currEqu.first);
    textTree.put("<xmlattr>.out", currEqu.second);
  }
  boost::property_tree::write_xml(pPath.string(), tree);
}


void MainWindow::on_lineEdit_AATester_tokenizer_nbOfSteps_textChanged(const QString &arg1)
{
  on_lineEdit_AATester_InputSentence_textChanged(_ui->lineEdit_AATester_InputSentence->text());
}

void MainWindow::on_actionLoad_chat_content_triggered()
{
  const QString extension = ".txt";
  const QString firstStr = "Text corpus (*" + extension + ")";

  std::string filenameStr = QFileDialog::getOpenFileName
      (this, "Import text corpus", QString(), firstStr).toUtf8().constData();
  if (filenameStr.empty())
    return;
  std::size_t nbOfInforms = 0;
  loadOneFileInSemanticMemory(nbOfInforms, filenameStr, *_semMemoryPtr, _lingDb, true, &_tmpFolder);

  if (nbOfInforms > 0)
  {
    std::stringstream message;
    message << "<" << nbOfInforms << " texts informed>\n";
    _ui->textBrowser_chat_history->setTextColor(_outFontColor);
    _ui->textBrowser_chat_history->append(QString::fromUtf8(message.str().c_str()));
  }
}


void MainWindow::on_actionSave_memory_triggered()
{
  const QString extensionSmem = ".smem";
  const QString firstStr = "Memories (*" + extensionSmem + ")";
  QString selectedFilter;
  QString fileToWrite = QFileDialog::getSaveFileName
      (this, "Serialize the memory", QString(),
       firstStr, &selectedFilter);
  std::string fileToWriteStr = std::string(fileToWrite.toUtf8().constData());
  if (fileToWriteStr.empty())
    return;
  fileToWriteStr += extensionSmem.toUtf8().constData();

  boost::property_tree::ptree memoryTree;
  serialization::saveSemMemory(memoryTree, *_semMemoryPtr);
  serialization::propertyTreeToZipedFile(memoryTree, fileToWriteStr, ".smem");
}

void MainWindow::on_actionLoad_memory_triggered()
{
  const QString extensionSmem = ".smem";
  const QString firstStr = "Memories (*" + extensionSmem + ")";

  std::string filename = QFileDialog::getOpenFileName
      (this, "Deserialize a memory", QString(), firstStr).toUtf8().constData();
  if (filename.empty())
    return;

  boost::property_tree::ptree memoryTree;
  serialization::propertyTreeFromZippedFile(memoryTree, filename);
  _clearLoadedScenarios();
  serialization::loadSemMemory(memoryTree, *_semMemoryPtr, _lingDb);
}


void MainWindow::on_actionload_a_smb_triggered()
{
  const QString extensionSmem = ".smb";
  const QString firstStr = "Memories (*" + extensionSmem + ")";

  std::string filenameStr = QFileDialog::getOpenFileName
      (this, "Load a binary memory", QString(), firstStr).toUtf8().constData();
  if (filenameStr.empty())
    return;

  _clearLoadedScenarios();
  _semMemoryBinaryPtr->memBloc.loadBinaryFile(filenameStr);
  _semMemoryPtr->memBloc.subBlockPtr = &_semMemoryBinaryPtr->memBloc;
}

void MainWindow::on_pushButton_micro_clicked()
{
  if (_ui->pushButton_micro->text() == microStr)
  {
    _ui->pushButton_micro->setText(stopMicroStr);
  }
  else
  {
    bool textEnd = false;
    auto asrText = _getAsrText(textEnd);
    auto asrTextQString = QString::fromUtf8(asrText.c_str());
    if (!asrText.empty() && _ui->lineEdit_AATester_InputSentence->text() != asrTextQString)
      _ui->lineEdit_AATester_InputSentence->setText(asrTextQString);
    _ui->pushButton_micro->setText(microStr);
  }
}


void MainWindow::on_pushButton_history_microForChat_clicked()
{
  if (_ui->pushButton_history_microForChat->text() == microStr)
  {
    _ui->pushButton_history_microForChat->setText(stopMicroStr);
  }
  else
  {
    bool textEnd = false;
    auto asrText = _getAsrText(textEnd);
    auto asrTextQString = QString::fromUtf8(asrText.c_str());
    if (!asrText.empty() && (textEnd || _ui->lineEdit_history_newText->text() != asrTextQString))
    {
      _ui->lineEdit_history_newText->setText(QString::fromUtf8(asrText.c_str()));
      if (textEnd)
        on_lineEdit_history_newText_returnPressed();
    }
    _ui->pushButton_history_microForChat->setText(microStr);
  }
}


std::string MainWindow::_getAsrText(bool& pTextEnd)
{
  if (_isSpeaking || _nbOfSecondToWaitAfterTtsSpeech > 0)
    return "";
  static std::string lastAsrText = "";
  std::ifstream fin;
  fin.open("out_asr.txt");
  static const std::string partialBeginOfStr = "  \"partial\" : \"";
  static const auto partialBeginOfStr_size = partialBeginOfStr.size();
  static const std::string textBeginOfStr = "  \"text\" : \"";
  static const auto textBeginOfStr_size = textBeginOfStr.size();
  std::string res;
  std::string line;
  bool firstLine = true;
  while (getline(fin, line))
  {
    if (firstLine)
    {
      if (line == "o")
        _asrIsWaiting = true;
      else if (_asrIsWaiting)
      {
        _asrIsWaiting = false;
        _shouldWaitForNewSpeech = false;
      }
      firstLine = false;
      continue;
    }
    if (line.compare(0, partialBeginOfStr_size, partialBeginOfStr) == 0 &&
        line.size() > partialBeginOfStr_size - 1)
    {
      lastAsrText = "";
      res = line.substr(partialBeginOfStr_size, line.size() - partialBeginOfStr_size - 1);
      if (!res.empty() && !_shouldWaitForNewSpeech)
      {
        fin.close();
        pTextEnd = false;
        return res;
      }
    }
    else if (line.compare(0, textBeginOfStr_size, textBeginOfStr) == 0 &&
             line.size() > textBeginOfStr_size - 1)
    {
      res = line.substr(textBeginOfStr_size, line.size() - textBeginOfStr_size - 1);
      if (res != lastAsrText)
      {
        lastAsrText = res;
        if (!_shouldWaitForNewSpeech)
        {
          fin.close();
          pTextEnd = true;
          return res;
        }
      }
    }
    break;
  }
  fin.close();
  pTextEnd = false;
  return "";
}


void MainWindow::on_actionAdd_domain_triggered()
{
  const QString extension = ".json";
  const QString firstStr = "Chatbot domain (*" + extension + ")";

  std::string filenameStr = QFileDialog::getOpenFileName
      (this, "Import chatbot domain", QString(), firstStr).toUtf8().constData();
  if (filenameStr.empty())
    return;
  std::ifstream file(filenameStr.c_str(), std::ifstream::in);
  if (!_chatbotDomain)
    _chatbotDomain = mystd::make_unique<ChatbotDomain>();
  loadChatbotDomain(*_chatbotDomain, file);
  addChatbotDomaintoASemanticMemory(*_semMemoryPtr, *_chatbotDomain, _lingDb);
  if (!_chatbotProblem)
    _chatbotProblem = mystd::make_unique<ChatbotProblem>();
  _proactivelyAskThePlanner();
}



void MainWindow::on_actionSet_problem_triggered()
{
  const QString extension = ".json";
  const QString firstStr = "Chatbot problem (*" + extension + ")";

  std::string filenameStr = QFileDialog::getOpenFileName
      (this, "Import chatbot problem", QString(), firstStr).toUtf8().constData();
  if (filenameStr.empty())
    return;
  std::ifstream file(filenameStr.c_str(), std::ifstream::in);
  _chatbotProblem = mystd::make_unique<ChatbotProblem>();
  loadChatbotProblem(*_chatbotProblem, file);
  _proactivelyAskThePlanner();
}


void MainWindow::_proactivelyAskThePlanner()
{
  std::list<TextWithLanguage> textsToSay;
  std::set<std::string> actionIdsToSkip;
  _proactivityFromPlanner(textsToSay, actionIdsToSkip);
  if (_ui->checkBox_enable_tts->isChecked() && !textsToSay.empty())
    _sayText(textsToSay);
}
