#ifndef ONSEM_LINGDBEDITOR_LINGUISTICINTERMADIARYDATABASE_HPP
#define ONSEM_LINGDBEDITOR_LINGUISTICINTERMADIARYDATABASE_HPP

#include <string>
#include <vector>
#include <list>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/lingdbeditor/databaserules/allingdbquestionwords.hpp>
#include <onsem/lingdbeditor/allingdbmultimeaningnode.hpp>
#include <onsem/lingdbeditor/meaningandconfidence.hpp>


namespace onsem
{
class ALLingdbDynamicTrieNode;
template<typename T>
struct ForwardPtrList;
class ALLingdbLinkToAConcept;
class ALLingdbConcept;
class ALLingdbAnimationsTag;



/// Class that hold a linguistic database.
class LinguisticIntermediaryDatabase
{

public:
  /// Constructor.
  LinguisticIntermediaryDatabase();

  ~LinguisticIntermediaryDatabase();


  LinguisticIntermediaryDatabase(const LinguisticIntermediaryDatabase&) = delete;
  LinguisticIntermediaryDatabase& operator=(const LinguisticIntermediaryDatabase&) = delete;


  // Savers/Loaders
  // --------------

  /**
   * @brief Serialize all the database in a (.bdb32/.bdb64) file.
   * @param pFilename Filename of file where we will store the serialised database.
   */
  void save(const std::string& pFilename);

  /**
   * @brief Load the database hold by a (.bdb32/.bdb64) file in the memory.
   * It removes all the content previously hold by this database.
   * @param pFilename Filename of the file that store the database.
   * @param pCoef Coefficient that represent how much memory space
   * we reserve relatively to the size of the database store by the file.
   * @return If the loading has succeeded.
   */
  void load(const std::string& pFilename, float pCoef = 1.2f);




  // Modifiers
  // ---------

  /**
   * @brief Add a word to the database.
   * ex: addWord("les", "le", PartOfSpeech::DETERMINER, {"mp", "fp"});
   * @param pWord Word that we want to add.
   * @param pLemme Lemme of the meaning associated to the new word.
   * @param pGram Grammatical type of the meaning associated to the new word.
   * @param pFlexions Flexion informations for the new pair word - meaning that we create.
   */
  void addWord
  (const std::string& pWord,
   const std::string& pLemma,
   PartOfSpeech pPartOfSpeech,
   const std::vector<std::string>& pFlexions,
   char pFrequency);

  void addWordWithSpecificMeaning
  (const std::string& pWord,
   ALLingdbMeaning& pMeaning,
   const std::vector<std::string>& pFlexions,
   char pFrequency);

  void addMultiMeaningsWord
  (ALLingdbMeaning* pRootMeaning,
   std::list<std::pair<ALLingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings,
   PartOfSpeech pPartOfSpeech);


  ALLingdbAnimationsTag* addATag
  (const std::string& pTag);


  void newQWords
  (ALLingdbQuestionWords* pQWords);


  void getConceptToMeanings
  (std::map<const ALLingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings) const;


  /**
   * @brief Change the boolean that indicate if the words
   * have to be followed by a separator or not.
   * @param pSeparatorNeeded The new boolean that indicate
   * if the words have to be followed by a separator or not.
   */
  void setSeparatorNeeded
  (bool pSeparatorNeeded);



  // Deleters
  // --------

  /// Remove all the content of the database and reinitialize it to his default value.
  void clear();

  /**
   * @brief Remove a word of the database.
   * @param word The word that we want to remove.
   */
  void removeWord(const std::string& word);

  /**
   * @brief Remove a wordForm of the database.
   * @param word The word.
   * @param pGram The grammatical type.
   */
  void removeWordForm
  (const std::string& word,
   PartOfSpeech pPartOfSpeech);



  /// Remove all the tags of the database.
  void removeAllTags();

  ALLingdbDynamicTrieNode* findComposedWordFromString
  (const std::string& pStr) const;


  // Getters
  // -------

  /**
   * @brief Know if a word exist in the database.
   * @param word The word.
   * @return True if we have found the word.
   */
  bool doesWordExist(const std::string& word) const;

  /**
   * @brief Get the final node of a word, if the word exist.
   * (doesn't return the final node of a sub-word).
   * @param word The word.
   * @return A pointer to the final node of the word, nullptr
   * if the node is not found.
   */
  ALLingdbDynamicTrieNode* getPointerToEndOfWord
  (const std::string& word) const;

  /**
   * @brief Get the final node of the begin of a word.
   * (we can also have the final node of an entire word)
   * @param word The begin of a word.
   * @return A pointer to the final node of the word.
   */
  ALLingdbDynamicTrieNode* getPointerInTrie
  (const std::string& word) const;

  /**
   * @brief Get the root of the dynamic trie that store the words.
   * @return The root of the dynamic trie that store the words.
   */
  ALLingdbDynamicTrieNode* getRoot
  () const;


  /**
   * @brief Get the boolean that indicate if the words
   * have to be followed by a separator or not.
   * @return Return True if the words have to be followed by a separator.
   */
  bool isSeparatorNeeded
  () const;

  /**
   * @brief Get the version of the database.
   * @return The version of the database.
   */
  unsigned int getVersion
  () const;

  /**
   * @brief Set the version of the database.
   * @param pVersion The new version of the database.
   */
  void setVersion
  (unsigned int pVersion);

  const ALLingdbString* getLanguage
  () const;

  void setLanguage
  (const std::string& pLanguage);

  /**
   * @brief Get a meaning from the database.
   * @param pLemme The lemme of the meaning.
   * @param pPartOfSpeech The grammatical type of the meaning.
   * @return The meaning found or nullptr if the meaning not exist in the database.
   */
  ALLingdbMeaning* getMeaning
  (const std::string& pLemma,
   PartOfSpeech pPartOfSpeech) const;


  ALLingdbConcept* getConcept
  (const std::string& pConceptName) const;

  ALLingdbConcept* addConcept(bool& pNewCptHasBeenInserted,
                              const std::string& pConceptName,
                              bool pAutoFill);

  ALLingdbQuestionWords* getQuestionWords() const;

  /**
   * @brief Pretty print the memory.
   * @param[out] pOs The input and output stream.
   */
  void prettyPrintMemory
  (std::ostream& pOs) const;

  /**
   * @brief Get the allocator used to store the database.
   * @return The allocator used to store the database.
   */
  const ALCompositePoolAllocator& getFPAlloc() const;




private:
  struct ALLingdbInfos
  {
    /// Version of the database.
    unsigned int version;
    ALLingdbString* language;
    /// If the words have to be followed by a separator.
    bool separatorNeeded;
    ALLingdbQuestionWords* questionWords;

    void init()
    {
      version = 0;
      language = nullptr;
      separatorNeeded = true;
      questionWords = nullptr;
    }

    static void getPointers
    (std::vector<const void*>& pRes, void* pVar)
    {
      pRes.emplace_back(&reinterpret_cast<ALLingdbInfos*>
                     (pVar)->language);
      pRes.emplace_back(&reinterpret_cast<ALLingdbInfos*>
                     (pVar)->questionWords);
    }
  };
  struct WordToAdd
  {
    WordToAdd
    (const std::string& pWord,
     const std::string& pLemma,
     PartOfSpeech pPartOfSpeech,
     const std::vector<std::string>& pFlexions,
     char pFrequency)
      : word(pWord),
        lemma(pLemma),
        partOfSpeech(pPartOfSpeech),
        flexions(pFlexions),
        frequency(pFrequency)
    {
    }

    std::string word;
    std::string lemma;
    PartOfSpeech partOfSpeech;
    const std::vector<std::string> flexions;
    char frequency;
  };


  /// The allocator and serializer object.
  ALCompositePoolAllocator fAlloc;
  /// The root of the dynamic trie that store the words.
  ALLingdbDynamicTrieNode* fRoot;
  ALLingdbInfos* fInfos;
  /// To allow to make reference to a composed word (ex: next~to before it exist)
  std::map<std::string, std::list<WordToAdd> > fMainWordToSimpleWordToAdd;
  std::map<std::string, ALLingdbConcept*> fConceptNameToPtr;

private:
  friend class ALLingdbDynamicTrieNode;
  friend class ALLingdbWordForms;
  friend class ALLingdbMeaning;
  friend class ALLingdbModifier;
  friend class ALBinaryDatabaseDicoSaver;
  friend class ALXmlDatabaseLoader;

  /// Initilyze an empty database.
  void xInitDatabase();


  bool xAddNewSubWord
  (const ForwardPtrList<ALLingdbDynamicTrieNode>*& pMultiMeaningNodes,
   std::list<ALLingdbDynamicTrieNode*>& pSubWords,
   const std::string& pSubWord) const;

  void xGetMainLemma
  (std::string& pMainLemma,
   const std::string& pComposedWordsLemma) const;

  /**
   * @brief Get the allocator used to store the database.
   * @return The allocator used to store the database.
   */
  ALCompositePoolAllocator& xGetFPAlloc();
};


} // End of namespace onsem

#include "details/linguisticintermediarydatabase.hxx"

#endif // ONSEM_LINGDBEDITOR_LINGUISTICINTERMADIARYDATABASE_HPP
