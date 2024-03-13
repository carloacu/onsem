#ifndef HC_DEBUGGER_MAINWINDOW_H
#define HC_DEBUGGER_MAINWINDOW_H

#include <QMainWindow>
#include <list>
#include <vector>
#include <string>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>

namespace Ui {
class DbGeneratorPreprocessingWindow;
}
namespace onsem {
class LingdbTree;
namespace linguistics {
struct LinguisticDatabaseStreams;
}
}

class DbGeneratorPreprocessingWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit DbGeneratorPreprocessingWindow(const onsem::LingdbTree& pLingDbTree,
                                            const std::string& pShareDbFolder,
                                            const std::string& pInputResourcesFolder,
                                            onsem::linguistics::LinguisticDatabaseStreams& pIStreams,
                                            QWidget* parent = 0);
    ~DbGeneratorPreprocessingWindow();

private:
    Ui::DbGeneratorPreprocessingWindow* _ui;
    const onsem::LingdbTree& _lingDbTree;
    const std::string _shareDbFolder;
    const std::string _inputResourcesFolder;
    onsem::linguistics::LinguisticDatabase _lingDb;

    /**
     * @brief Refresh the display when we change the current word.
     * @param pWord The new word.
     */
    void refreshWord(const std::string& pWord);

    void refreshAMeaning(std::list<std::string>& pConceptsStr, int pIdMeaning);

    struct WordsToTagsToDisplay {
        WordsToTagsToDisplay()
            : conjugaisons()
            , wordSuggestions()
            , meaningsStr() {}
        std::string conjugaisons;
        std::string wordSuggestions;
        std::vector<std::string> meaningsStr;
    };

private Q_SLOTS:
    void on_pushButton_dbpedia_xml_to_txt_clicked();

    void on_lineEdit_WordsToTags_Word1_Word_textChanged(const QString& arg1);

    /**
     * @brief Select a meaning in the meaning list event.
     * @param currentRow Row of selected meaning.
     */
    void on_listWidget_WordsToTags_Word1_Meanings_currentRowChanged(int currentRow);

    void on_pushButton_words_open_clicked();

    void on_pushButton_words_mergeWith_clicked();

    void on_pushButton_words_saveAs_clicked();

    void on_pushButton_MemoryClearer_Refresh_clicked();

    void on_pushButton_process_auxiliariesextractor_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_addFrenchIntransitiveVerbs_clicked();

    void on_pushButton_11_clicked();

private:
    struct WordsToTagsCurrentState {
        WordsToTagsCurrentState()
            : wordFroms(nullptr)
            , meaning(nullptr) {}
        const onsem::ForwardPtrList<onsem::LingdbWordForms>* wordFroms;
        onsem::LingdbMeaning* meaning;
    };
    /// The current infos to display for the panel "WordsToTags".
    WordsToTagsToDisplay fWordsToTagsToDisplay;
    /// The current state to handle the panel "WordsToTags".
    WordsToTagsCurrentState fWordsToTagsCurrentState;

    /**
     * @brief Refresh the suggestion list of similar words.
     * @param node Node of the end of the current word.
     */
    void _refreshSuggestions(onsem::LingdbDynamicTrieNode* node);

    void _conjAMeaning(std::string& pConj, onsem::LingdbMeaning* pMeaning) const;

    void _getFilenameToOpen(std::string& pFilename);
    void _getFilenameToMerge(std::string& pFilename);
    void _getFilenameToSave(std::string& pFilename);

private:
    /// Dynamic Trie that store all the database.
    onsem::LinguisticIntermediaryDatabase fWords;
    /// Filename of the current database loaded
    std::string _fileLoaded;
    const std::string fTmpFolder;
};

#endif    // HC_DEBUGGER_MAINWINDOW_H
