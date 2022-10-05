#include "dbGeneratorpreprocessingwindow.hpp"
#include <sstream>
#include <QFileDialog>
#include <QString>
#include "ui_dbGeneratorpreprocessingwindow.h"
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/lingdbflexions.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbmeaningtowords.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/compilermodel/loaders/anydatabaseloader.hpp>
#include <onsem/compilermodel/loaders/deladatabaseloader.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/dictionaryextractor/auxiliariesextractor.hpp>
#include "frgrammarbookextractor/frgrammarbookextractor.hpp"
#include "textextractor/textextractor.hpp"
#include "wikitionaryextractor/wikitionaryextractor.hpp"
#include "webtranslator/webtranslator.hpp"
#include "traductiongenerator/traductiongenerator.hpp"
#include "firstnamesextractor/firstnamesextractor.hpp"

using namespace onsem;

/// Number of similar words proposed when we enter a wrong word
const unsigned int _nbSuggestions = 10;

// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)


DbGeneratorPreprocessingWindow::DbGeneratorPreprocessingWindow(
    const LingdbTree& pLingDbTree,
    const std::string& pShareDbFolder,
    const std::string& pInputResourcesFolder,
    linguistics::LinguisticDatabaseStreams& pIStreams,
    QWidget *parent) :
  QMainWindow(parent),
  _ui(new Ui::DbGeneratorPreprocessingWindow),
  _lingDbTree(pLingDbTree),
  _shareDbFolder(pShareDbFolder),
  _inputResourcesFolder(pInputResourcesFolder),
  _lingDb(pIStreams),
  fWordsToTagsToDisplay(),
  fWordsToTagsCurrentState(),
  fWords(),
  _fileLoaded(),
  fTmpFolder("tmpFolder")
{
  _ui->setupUi(this);

  _ui->textBrowser_MemoryClearer_Display->viewport()->setAutoFillBackground(false);
  _ui->textBrowser_MemoryClearer_Display->setAttribute(Qt::WA_TranslucentBackground, true);
  boost::filesystem::create_directory(fTmpFolder);
}

DbGeneratorPreprocessingWindow::~DbGeneratorPreprocessingWindow()
{
  boost::filesystem::remove_all(fTmpFolder);
  delete _ui;
}



void _conjAVerbTense
(std::stringstream& pSs,
 const std::vector<WordLinkForConj>& pTense)
{
  if (pTense[0].node != nullptr)
  {
    pSs << "1s " << pTense[0].node->getWord() << std::endl;
  }
  if (pTense[1].node != nullptr)
  {
    pSs << "2s " << pTense[1].node->getWord() << std::endl;
  }
  if (pTense[2].node != nullptr)
  {
    pSs << "3s " << pTense[2].node->getWord() << std::endl;
  }
  if (pTense[3].node != nullptr)
  {
    pSs << "1p " << pTense[3].node->getWord() << std::endl;
  }
  if (pTense[4].node != nullptr)
  {
    pSs << "2p " << pTense[4].node->getWord() << std::endl;
  }
  if (pTense[5].node != nullptr)
  {
    pSs << "3p " << pTense[5].node->getWord() << std::endl;
  }
}


void _conjImperativeVerbTense
(std::stringstream& pSs,
 const std::vector<WordLinkForConj>& pTense)
{
  if (pTense[0].node != nullptr)
  {
    pSs << "2s " << pTense[0].node->getWord() << std::endl;
  }
  if (pTense[1].node != nullptr)
  {
    pSs << "1p " << pTense[1].node->getWord() << std::endl;
  }
  if (pTense[2].node != nullptr)
  {
    pSs << "2p " << pTense[2].node->getWord() << std::endl;
  }
}



void DbGeneratorPreprocessingWindow::_conjAMeaning
(std::string& pConj,
 LingdbMeaning* pMeaning) const
{
  LingdbMeaningToWords meaningToWords;
  std::map<const LingdbMeaning*, VerbConjugaison> verbConjugaison;
  std::map<const LingdbMeaning*, NounAdjConjugaison> nounConjugaison;
  meaningToWords.findWordsConjugaisons(verbConjugaison, nounConjugaison, fWords);
  auto itConj = verbConjugaison.find(pMeaning);
  if (itConj != verbConjugaison.end())
  {
    std::stringstream ss;
    if (itConj->second.infinitive.node != nullptr)
    {
      ss << "infinitive: " << itConj->second.infinitive.node->getWord() << std::endl;
      ss << std::endl;
    }
    ss << "present:" << std::endl;
    _conjAVerbTense(ss, itConj->second.present);
    ss << std::endl;
    ss << "imperfect:" << std::endl;
    _conjAVerbTense(ss, itConj->second.imperfect);
    ss << std::endl;
    ss << "future:" << std::endl;
    _conjAVerbTense(ss, itConj->second.future);
    ss << std::endl;
    ss << "imperative:" << std::endl;
    _conjImperativeVerbTense(ss, itConj->second.imperative);
    ss << std::endl;
    ss << "pastParticiple:" << std::endl;
    if (itConj->second.pastParticiple.masculineSingular.node != nullptr)
    {
      ss << "ms: " << itConj->second.pastParticiple.masculineSingular.node->getWord() << std::endl;
    }
    if (itConj->second.pastParticiple.masculinePlural.node != nullptr)
    {
      ss << "mp: " << itConj->second.pastParticiple.masculinePlural.node->getWord() << std::endl;
    }
    if (itConj->second.pastParticiple.feminineSingular.node != nullptr)
    {
      ss << "fs: " << itConj->second.pastParticiple.feminineSingular.node->getWord() << std::endl;
    }
    if (itConj->second.pastParticiple.femininePlural.node != nullptr)
    {
      ss << "fp: " << itConj->second.pastParticiple.femininePlural.node->getWord() << std::endl;
    }
    ss << std::endl;
    ss << "conditional:" << std::endl;
    _conjAVerbTense(ss, itConj->second.conditional);
    pConj = ss.str();
  }
  else
  {
    auto itNounConj = nounConjugaison.find(pMeaning);
    if (itNounConj != nounConjugaison.end())
    {
      std::stringstream ss;
      if (itNounConj->second.masculineSingular.node != nullptr)
      {
        ss << "ms: " << itNounConj->second.masculineSingular.node->getWord() << std::endl;
      }
      if (itNounConj->second.masculinePlural.node != nullptr)
      {
        ss << "mp: " << itNounConj->second.masculinePlural.node->getWord() << std::endl;
      }
      if (itNounConj->second.feminineSingular.node != nullptr)
      {
        ss << "fs: " << itNounConj->second.feminineSingular.node->getWord() << std::endl;
      }
      if (itNounConj->second.femininePlural.node != nullptr)
      {
        ss << "fp: " << itNounConj->second.femininePlural.node->getWord() << std::endl;
      }
      if (itNounConj->second.neutralSingular.node != nullptr)
      {
        ss << "ns: " << itNounConj->second.neutralSingular.node->getWord() << std::endl;
      }
      if (itNounConj->second.neutralPlural.node != nullptr)
      {
        ss << "np: " << itNounConj->second.neutralPlural.node->getWord() << std::endl;
      }
      pConj = ss.str();
    }
  }
}




void DbGeneratorPreprocessingWindow::refreshWord(const std::string& pWord)
{
  fWordsToTagsCurrentState.wordFroms = nullptr;
  fWordsToTagsCurrentState.meaning = nullptr;
  fWordsToTagsToDisplay.conjugaisons = "";
  fWordsToTagsToDisplay.wordSuggestions = "";
  fWordsToTagsToDisplay.meaningsStr.clear();
  if (pWord.empty())
  {
    return;
  }
  LingdbDynamicTrieNode* node = fWords.getPointerToEndOfWord(pWord);
  if (node != nullptr)
  {
    fWordsToTagsCurrentState.wordFroms = node->getWordForms();
    const ForwardPtrList<LingdbWordForms>* wf = fWordsToTagsCurrentState.wordFroms;
    while (wf != nullptr)
    {
      std::stringstream ss;
      PartOfSpeech currPartOfSpeech = wf->elt->getMeaning()->getPartOfSpeech();
      ss << wf->elt->getMeaning()->getLemma()->getWord()
         << " : " << partOfSpeech_toStr(currPartOfSpeech);
      if (wf->elt->getFlexions() != nullptr)
      {
        ss << " (";
        wf->elt->getFlexions()->writeInStream(ss, currPartOfSpeech);
        ss << ")";
      }
      fWordsToTagsToDisplay.meaningsStr.emplace_back(ss.str());

      if (fWordsToTagsToDisplay.conjugaisons == "")
      {
        _conjAMeaning(fWordsToTagsToDisplay.conjugaisons, wf->elt->getMeaning());
      }

      wf = wf->next;
    }
  }
  else
  {
    std::string subWord = pWord;
    node = fWords.getPointerInTrie(subWord);
    while (node == nullptr && subWord.size() > 0)
    {
      subWord = pWord.substr(0, subWord.size() - 1);
      node = fWords.getPointerInTrie(subWord);
    }
    _refreshSuggestions(node);
  }
}



void DbGeneratorPreprocessingWindow::refreshAMeaning
(std::list<std::string>& pConceptsStr,
 int pIdMeaning)
{
  fWordsToTagsCurrentState.meaning = nullptr;
  if (pIdMeaning >= 0)
  {
    const ForwardPtrList<LingdbWordForms>* wf = fWordsToTagsCurrentState.wordFroms;
    while (wf != nullptr)
    {
      if (pIdMeaning == 0)
      {
        fWordsToTagsCurrentState.meaning = wf->elt->getMeaning();
        break;
      }
      --pIdMeaning;
      wf = wf->next;
    }
  }
  if (fWordsToTagsCurrentState.meaning != nullptr)
  {
    fWordsToTagsCurrentState.meaning->getLinkToConceptsStr(pConceptsStr);
  }
}



void DbGeneratorPreprocessingWindow::_refreshSuggestions
(LingdbDynamicTrieNode* node)
{
  unsigned int nbOcc = _nbSuggestions;
  std::stringstream ss;
  node = node->getNextWordNode();
  while (node != nullptr && nbOcc > 0)
  {
    ss << node->getWord() << "\n";
    node = node->getNextWordNode();
    --nbOcc;
  }
  fWordsToTagsToDisplay.wordSuggestions = ss.str();
}



void DbGeneratorPreprocessingWindow::on_lineEdit_WordsToTags_Word1_Word_textChanged(const QString &arg1)
{
  refreshWord(arg1.toUtf8().constData());
  const DbGeneratorPreprocessingWindow::WordsToTagsToDisplay& wf = fWordsToTagsToDisplay;
  _ui->listWidget_WordsToTags_Word1_Meanings->clear();
  for (std::size_t i = 0; i < wf.meaningsStr.size(); ++i)
    _ui->listWidget_WordsToTags_Word1_Meanings->addItem
        (QString::fromUtf8(wf.meaningsStr[i].c_str()));
  _ui->listWidget_WordsToTags_Word1_Meanings->setCurrentRow(0);
  _ui->label_WordsToTags_Word1_Word_Suggestions->setText
      (QString::fromUtf8(wf.wordSuggestions.c_str()));

  _ui->textBrowser_WordsToTags_Conj->setText
      (QString::fromUtf8(wf.conjugaisons.c_str()));
}

void DbGeneratorPreprocessingWindow::on_listWidget_WordsToTags_Word1_Meanings_currentRowChanged(int currentRow)
{
  std::list<std::string> conceptsStr;
  refreshAMeaning(conceptsStr, currentRow);
  _ui->listWidget_WordsToTags_Word1_Concepts->clear();
  for (std::list<std::string>::const_iterator it = conceptsStr.begin();
       it != conceptsStr.end(); ++it)
  {
    _ui->listWidget_WordsToTags_Word1_Concepts->addItem
        (QString::fromUtf8(it->c_str()));
  }
}


std::string _getFilterDynDatabase()
{
  static const bool in32Bits = sizeof(std::size_t) == 4;
  if (in32Bits)
    return "Dynamic 32 bits Database (*.ddb32)";
  return "Dynamic 64 bits Database (*.ddb64)";
}

std::string _getFilterRlaDatabase()
{
  return "Reload All (*.rla)";
}

std::string _getFilterXmlDatabase()
{
  return "Xml Database (*.xml)";
}

std::string _getFilterDelaDatabase()
{
  return "Dela Database (*.dela)";
}

std::string _getFilterCptsDatabase()
{
  return "Concepts database (*.cpts)";
}

std::string _getFilterGfsDatabase()
{
  return "Gfs Database (*.gfs)";
}

std::string _getFilterBinaryDatabase()
{
  return "Binary Database (*.bdb)";
}


void DbGeneratorPreprocessingWindow::_getFilenameToOpen(std::string& pFilename)
{
  std::stringstream ss;
  ss << _getFilterDynDatabase() << ";;"
     << _getFilterRlaDatabase() << ";;"
     << _getFilterXmlDatabase() << ";;"
     << _getFilterDelaDatabase() << ";;"
     << _getFilterCptsDatabase();
  pFilename = QFileDialog::getOpenFileName
      (this, "Open a database", QString(),
       QString::fromUtf8(ss.str().c_str())).toUtf8().constData();
}

void DbGeneratorPreprocessingWindow::on_pushButton_words_open_clicked()
{
  std::string filename = "";
  _getFilenameToOpen(filename);
  if (filename.empty())
  {
    return;
  }
  _fileLoaded = filename;
  this->setWindowTitle(QFileInfo(QString::fromUtf8(_fileLoaded.c_str())).fileName());

  AnyDatabaseLoader anyloader;
  anyloader.open(_fileLoaded, _lingDbTree.getInputResourcesDir(),
                 fWords, _lingDbTree);

  // Unlock databases handle options
  _ui->pushButton_words_saveAs->setEnabled(true);
  _ui->pushButton_words_mergeWith->setEnabled(true);
}


void DbGeneratorPreprocessingWindow::_getFilenameToMerge(std::string& pFilename)
{
  std::stringstream ss;
  ss << _getFilterXmlDatabase() << ";;"
     << _getFilterRlaDatabase() << ";;"
     << _getFilterDelaDatabase() << ";;"
     << _getFilterCptsDatabase() << ";;"
     << _getFilterGfsDatabase();
  pFilename = QFileDialog::getOpenFileName
      (this, "Merge with another database", QString(),
       QString::fromUtf8(ss.str().c_str())).toUtf8().constData();
}


void DbGeneratorPreprocessingWindow::on_pushButton_words_mergeWith_clicked()
{
  std::string filename = "";
  _getFilenameToMerge(filename);
  if (filename.empty())
    return;
  AnyDatabaseLoader anyloader;
  anyloader.mergeWith(filename, _lingDbTree.getInputResourcesDir(), fWords, _lingDbTree);
}


void DbGeneratorPreprocessingWindow::_getFilenameToSave(std::string& pFilename)
{
  std::stringstream ss;
  ss << _getFilterDynDatabase() << ";;"
     << _getFilterBinaryDatabase();
  QString selectedFilter;
  QString fichier = QFileDialog::getSaveFileName
      (this, "Save the database", QString(),
       QString::fromUtf8(ss.str().c_str()), &selectedFilter);

  pFilename = fichier.toUtf8().constData();
  std::string selectedFilterStr = selectedFilter.toUtf8().constData();
  QFileInfo fileInfo(fichier);
  std::string suffixStr = fileInfo.suffix().toUtf8().constData();

  if (selectedFilterStr == _getFilterDynDatabase())
  {
    if (suffixStr != _lingDbTree.getExtDynDatabase())
      pFilename += "." + _lingDbTree.getExtDynDatabase();
  }
  else if (selectedFilterStr == _getFilterBinaryDatabase())
  {
    if (suffixStr != _lingDbTree.getExtBinaryDatabase())
      pFilename += "." + _lingDbTree.getExtBinaryDatabase();
  }
}


void DbGeneratorPreprocessingWindow::on_pushButton_words_saveAs_clicked()
{
  std::string filename = "";
  _getFilenameToSave(filename);
  if (filename.empty())
    return;
  _fileLoaded = filename;
  this->setWindowTitle(QFileInfo(QString::fromUtf8(_fileLoaded.c_str())).fileName());

  QFileInfo fileInfo(_fileLoaded.c_str());
  std::string suffixStr = fileInfo.suffix().toUtf8().constData();
  if (suffixStr == _lingDbTree.getExtDynDatabase())
    fWords.save(_fileLoaded);
}

void DbGeneratorPreprocessingWindow::on_pushButton_MemoryClearer_Refresh_clicked()
{
  std::stringstream ss;
  fWords.prettyPrintMemory(ss);
  _ui->textBrowser_MemoryClearer_Display->setText
      (QString::fromUtf8(ss.str().c_str()));
}

void DbGeneratorPreprocessingWindow::on_pushButton_process_auxiliariesextractor_clicked()
{
  const std::string inPath =
      _ui->lineEdit_auxilitiesextractorInputPath->text().toUtf8().constData();
  onsem::AuxiliariesExtractor auxExt(_lingDb);
  auxExt.processFile(inPath);
}

void DbGeneratorPreprocessingWindow::on_pushButton_clicked()
{
  onsem::frgrammarbookextractor::run(_lingDbTree, _shareDbFolder, _lingDb);
}

void DbGeneratorPreprocessingWindow::on_pushButton_2_clicked()
{
  std::set<std::string> frLemmaToKeep;
  DelaDatabaseLoader delaLoader;
  delaLoader.simplifyDelaFile
      (_inputResourcesFolder + "/french/readonly/french_dela.dela",
       _inputResourcesFolder + "/french/readonly/french_simplified_dela.dela",
       frLemmaToKeep, true, true /* TODO: to check */);

  std::set<std::string> enLemmaToKeep{
    "acorn squash",
    "banana squash",
    "bell pepper",
    "bitter melon",
    "black bean",
    "black-eyed pea",
    "broad bean",
    "Brussel's sprout",
    "cayenne pepper",
    "chili pepper",
    "green bean",
    "green onion",
    "kidney bean",
    "lima bean",
    "mangel-wurzel",
    "mung bean",
    "navy bean",
    "pinto bean",
    "root vegetable",
    "runner bean",
    "split pea",
    "spring onion",
    "sweet corn",
    "sweet potato",
    "water chestnut"};
  delaLoader.simplifyDelaFile
      (_inputResourcesFolder + "/english/readonly/english_dela.dela",
       _inputResourcesFolder + "/english/readonly/english_simplified_dela.dela",
       enLemmaToKeep, false, false  /* TODO: to check */);
}

void DbGeneratorPreprocessingWindow::on_pushButton_3_clicked()
{
  onsem::textextractor::run(_lingDbTree, fTmpFolder, _lingDb, _shareDbFolder, _inputResourcesFolder);
}

void DbGeneratorPreprocessingWindow::on_pushButton_4_clicked()
{
  const std::string myDataMiningPath =
      _ui->lineEdit_MyDataMiningPath->text().toUtf8().constData();
  onsem::wikitionaryExtractor::runTraductions(_lingDbTree, myDataMiningPath, fTmpFolder);
}

void DbGeneratorPreprocessingWindow::on_pushButton_5_clicked()
{
  onsem::webTranslator::run(_lingDbTree, fTmpFolder, _shareDbFolder, _inputResourcesFolder);
}

void DbGeneratorPreprocessingWindow::on_pushButton_6_clicked()
{
  onsem::traductiongenerator::run(_lingDbTree, fTmpFolder, _inputResourcesFolder);
}

void DbGeneratorPreprocessingWindow::on_pushButton_7_clicked()
{
  const std::string myDataMiningPath =
      _ui->lineEdit_MyDataMiningPath->text().toUtf8().constData();
  onsem::wikitionaryExtractor::addComposedWords(_lingDbTree, myDataMiningPath, _inputResourcesFolder);
}

void DbGeneratorPreprocessingWindow::on_pushButton_8_clicked()
{
  const std::string myDataMiningPath =
      _ui->lineEdit_MyDataMiningPath->text().toUtf8().constData();
  onsem::firstnamesExtractor::run(myDataMiningPath, _inputResourcesFolder);
}

void DbGeneratorPreprocessingWindow::on_pushButton_9_clicked()
{
  const std::string myDataMiningPath =
      _ui->lineEdit_MyDataMiningPath->text().toUtf8().constData();
  onsem::wikitionaryExtractor::addTransitiveVerbs(_lingDbTree, myDataMiningPath, _inputResourcesFolder);
}

void DbGeneratorPreprocessingWindow::on_pushButton_10_clicked()
{
  const std::string myDataMiningPath =
      _ui->lineEdit_MyDataMiningPath->text().toUtf8().constData();
  onsem::wikitionaryExtractor::addConcepts(_lingDbTree, myDataMiningPath, _inputResourcesFolder);
}

struct OutLine
{
  std::string referenceStr;
  std::string valueStr;
  std::string valueLanguage;
  std::string revelancePrettyPrint;
};

struct WordKindness
{
  WordKindness(std::size_t pKindness,
               std::size_t pWickedness)
    : kindness(pKindness),
      wickedness(pWickedness)
  {
  }
  std::size_t kindness{1};
  std::size_t wickedness{1};
};


std::size_t _calculateRevelance(std::size_t& pKindness,
                                std::size_t& pWickedness,
                                std::size_t pWikiPageLength,
                                const std::string& pText,
                                const std::map<SemanticLanguageEnum, std::map<std::string, WordKindness>>& pLanguageToLemmaWithKindness,
                                SemanticLanguageEnum pLanguage,
                                const linguistics::LinguisticDatabase& pLingDb)
{
  auto itWords = pLanguageToLemmaWithKindness.find(pLanguage);
  if (itWords != pLanguageToLemmaWithKindness.end())
  {
    linguistics::TokensTree tokensTree;
    linguistics::AlgorithmSetForALanguage langConfig(pLingDb, pLanguage);
    linguistics::tokenizeText(tokensTree, langConfig, pText, {});
    auto& words = itWords->second;
    for (const auto& currToken : tokensTree.tokens)
    {
      auto it = words.find(currToken.str);
      if (it != words.end())
      {
        pKindness += it->second.kindness;
        pWickedness += it->second.wickedness;
      }
    }
  }
  return (pWikiPageLength * pKindness) / pWickedness;
}


void _loadXmlDbPediaFile(std::map<std::size_t, std::list<OutLine>>& pTextsSorted,
                         const std::string& pFile,
                         const std::map<SemanticLanguageEnum, std::map<std::string, WordKindness>>& pLanguageToLemmaWithKindness,
                         const linguistics::LinguisticDatabase& pLingDb)
{
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(pFile, tree);
  auto sparclTreeOpt = tree.get_child_optional("sparql");
  if (sparclTreeOpt)
  {
    auto resultsTreeOpt = sparclTreeOpt->get_child_optional("results");
    if (resultsTreeOpt)
    {
      std::size_t i = 0;
      for (const auto& currResult : *resultsTreeOpt)
      {
        if (++i % 10000 == 0)
          std::cout << i << std::endl;
        if (currResult.first == "<xmlattr>")
          continue;
        std::string referenceStr;
        std::string valueFrStr;
        std::string valueEnStr;
        std::size_t wikiPageLength = 0;
        std::size_t kindness = 1;
        std::size_t wickedness = 1;
        for (const auto& currBinding : currResult.second)
        {
          if (currBinding.first == "<xmlattr>")
            continue;
          auto literalOpt = currBinding.second.get_optional<std::string>("literal");
          if (literalOpt)
          {
            if (referenceStr.empty())
              referenceStr = *literalOpt;
            else if (valueFrStr.empty())
              valueFrStr = *literalOpt;
            else if (valueEnStr.empty())
              valueEnStr = *literalOpt;
            else
            {
              try
              {
                wikiPageLength = boost::lexical_cast<std::size_t>(*literalOpt);
              }
              catch (...) {}
            }
          }
        }
        if (!referenceStr.empty() && (!valueFrStr.empty() || !valueEnStr.empty()))
        {
          OutLine outLine;
          outLine.referenceStr = referenceStr;
          SemanticLanguageEnum language = SemanticLanguageEnum::FRENCH;
          if (!valueFrStr.empty())
          {
            outLine.valueStr = valueFrStr;
            outLine.valueLanguage = "fr";
          }
          else
          {
            outLine.valueStr = valueEnStr;
            outLine.valueLanguage = "en";
            language = SemanticLanguageEnum::ENGLISH;
          }
          std::size_t revelance = _calculateRevelance(kindness, wickedness, wikiPageLength, outLine.valueStr,
                                                      pLanguageToLemmaWithKindness, language, pLingDb);
          std::stringstream revelancePrettyPrintSs;
          revelancePrettyPrintSs << revelance << "=" << wikiPageLength << "*" << kindness << "/" << wickedness;
          outLine.revelancePrettyPrint = revelancePrettyPrintSs.str();
          pTextsSorted[revelance].emplace_back(std::move(outLine));
        }
      }
    }
  }
}


void _addWordWithKindness(std::map<std::string, WordKindness>& pWords,
                          const std::string& pLemma,
                          std::size_t pKindness,
                          std::size_t pWickedness)
{
  pWords.emplace(pLemma, WordKindness(pKindness, pWickedness));
  pWords.emplace(getFirstLetterInUpperCase(pLemma), WordKindness(pKindness, pWickedness));
}

void DbGeneratorPreprocessingWindow::on_pushButton_dbpedia_xml_to_txt_clicked()
{
  const boost::filesystem::path fromFile =
      _ui->lineEdit_dbpedia_from_file->text().toUtf8().constData();

  std::map<SemanticLanguageEnum, std::map<std::string, WordKindness>> languageToLemmaWithKindness;
  auto& frWord = languageToLemmaWithKindness[SemanticLanguageEnum::FRENCH];
  _addWordWithKindness(frWord, "anti-slave", 0, 5);
  _addWordWithKindness(frWord, "antisémite", 0, 5);
  _addWordWithKindness(frWord, "antisémitisme", 0, 5);
  _addWordWithKindness(frWord, "antisioniste", 0, 5);
  _addWordWithKindness(frWord, "condamnation", 0, 5);
  _addWordWithKindness(frWord, "crime", 0, 5);
  _addWordWithKindness(frWord, "dictature", 0, 5);
  _addWordWithKindness(frWord, "guerre", 0, 5);
  _addWordWithKindness(frWord, "haine", 0, 5);
  _addWordWithKindness(frWord, "nazi", 0, 5);
  _addWordWithKindness(frWord, "nazisme", 0, 5);
  _addWordWithKindness(frWord, "pédophile", 0, 5);
  _addWordWithKindness(frWord, "racisme", 0, 5);
  _addWordWithKindness(frWord, "raciste", 0, 5);
  _addWordWithKindness(frWord, "shoah", 0, 5);
  _addWordWithKindness(frWord, "tueur", 0, 5);
  _addWordWithKindness(frWord, "viol", 0, 5);
  _addWordWithKindness(frWord, "violeur", 0, 5);
  _addWordWithKindness(frWord, "amour", 1, 0);
  _addWordWithKindness(frWord, "charité", 1, 0);
  _addWordWithKindness(frWord, "commémoré", 1, 0);
  _addWordWithKindness(frWord, "paix", 1, 0);

  std::map<std::size_t, std::list<OutLine>> textsSorted;
  if (fromFile.extension().string() == ".txt")
  {
    std::ifstream propFile(fromFile.string().c_str(), std::ifstream::in);
    std::string line;
    while (getline(propFile, line))
    {
      if (line.empty())
        continue;
      if (line[0] == '>') // if it's a new property
      {
        const std::string subFilename = line.substr(1, line.size() - 1);
        boost::filesystem::path subFile(fromFile.parent_path());
        subFile = subFile / subFilename;
        _loadXmlDbPediaFile(textsSorted, subFile.string(), languageToLemmaWithKindness, _lingDb);
      }
    }
    propFile.close();
  }
  else
  {
    _loadXmlDbPediaFile(textsSorted, fromFile.string(), languageToLemmaWithKindness, _lingDb);
  }

  const std::string toPath =
      _ui->lineEdit_dbpedia_to_path->text().toUtf8().constData();
  std::ofstream outfile(toPath);
  for (auto it = textsSorted.rbegin(); it != textsSorted.rend(); ++it)
    for (const auto& currElt : it->second)
      outfile << "#" << currElt.referenceStr << "#" << currElt.valueLanguage << "#"
              << currElt.revelancePrettyPrint << "#" << currElt.valueStr << "\n";
  outfile.close();
}


