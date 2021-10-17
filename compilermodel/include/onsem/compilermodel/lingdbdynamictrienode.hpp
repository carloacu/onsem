#ifndef ONSEM_COMPILERMODEL_LINGDBDYNAMICTRIENODE_HPP
#define ONSEM_COMPILERMODEL_LINGDBDYNAMICTRIENODE_HPP

#include <vector>
#include <string>
#include <set>
#include <list>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/compilermodel/lingdbmultimeaningnode.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class LingdbMeaning;
class LingdbWordForms;
class CompositePoolAllocator;
template<typename T>
struct ForwardPtrList;


/// A node of the dynamic trie that store the words.
class LingdbDynamicTrieNode
{
public:

  // Modifiers
  // ---------

  /**
   * @brief Add a wordform to this node
   * (this node has to be the end of a word of course).
   * @param pLingDatabase The main object of the database.
   * @param pLemme Lemme of this wordform.
   * @param pGram Grammatical type of this wordform.
   * @param pOnHead Put the new wf at the front or at the back of the list.
   * @return The new WordForm created and added to the database.
   */
  LingdbWordForms* addWordForm
  (LinguisticIntermediaryDatabase& pLingDatabase,
   const std::string& pLemma,
   PartOfSpeech pPartOfSpeech,
   bool pAtFront = true);

  LingdbWordForms* addWordFormFromMeaning
  (LinguisticIntermediaryDatabase& pLingDatabase,
   LingdbMeaning& pMeaning,
   bool pAtFront = true);

  /**
   * @brief Put at the top of the worform list a specific one.
   * @param pGram The grammatical type of the wordform.
   */
  void putAWordFormAtTheTopOfTheList
  (PartOfSpeech pGram,
   const std::string& pLemme);



  // Getters
  // -------

  /**
   * @brief Get the number of brothers that have an higher letter
   * (alphabeticaly speaking).
   * @return The number of brothers that have an higher letter.
   */
  unsigned char nbSupBrother() const;

  /**
   * @brief Get the letter hold by this node.
   * @return The letter hold by this node.
   */
  char getLetter() const;

  /**
   * @brief Get the word that finish at this node.
   * @return The word that finish at this node.
   */
  std::string getWord(bool pNaturalLangPrint = false) const;

  /**
   * @brief Know if the "word" is in reality an expression.
   * @return True if the "word" is an expression, False otherwise.
   */
  bool isExpr() const;

  /**
   * @brief Know if the "word" can be a separator (space, comma, ...).
   * @return True if the "word" is can be a separator, False otherwise.
   */
  bool canBeASeparator() const;

  /**
   * @brief Get the number of children of this node.
   * @return The number of children of this node.
   */
  unsigned char getNbChildren() const;

  /**
   * @brief Get the WordForm associated with a grammatical type and
   * hold by this node.
   * @param gram The grammatical type.
   * @return The WordForm associated with the grammatical type and
   * hold by this node.
   */
  LingdbWordForms* getWordForm
  (const std::string& pLemme,
   PartOfSpeech pPartOfSpeech) const;

  LingdbWordForms* getWordFormFromMeaning
  (LingdbMeaning& pMeaning) const;


  /**
   * @brief Get all the WordForms hold by this node.
   * @return All the WordForms hold by this node.
   */
  const ForwardPtrList<LingdbWordForms>* getWordForms
  () const;

  const ForwardPtrList<LingdbMeaning>* getMeaningsAtThisLemme
  () const;

  ForwardPtrList<LingdbDynamicTrieNode>* getMultiMeaningsNodes
  () const;

  LingdbMultiMeaningsNode* getDatasIfItsAMultiMeaningNode
  () const;



  /**
   * @brief Get the number of meanings hold by this node.
   * @return The number of meanings hold by this node.
   */
  unsigned char nbMeaningsAtThisLemme
  () const;

  unsigned char nbWordForms() const;

  void getWordFormsAndMeanings
  (std::list<std::pair<LingdbWordForms*, LingdbMeaning*> >& pWordFromsOrMeanings) const;

  /**
   * @brief Get the first child of this node.
   * @return The first child of this node.
   */
  LingdbDynamicTrieNode* getFirstChild
  () const;

  /**
   * @brief Get the next brother of this node.
   * @return The next brother of this node.
   */
  LingdbDynamicTrieNode* getNextBrother
  () const;

  LingdbDynamicTrieNode* getFather
  () const;


  /**
   * @brief Get the next node that represent the end of a word.
   * @return The next node that represent the end of a word.
   */
  LingdbDynamicTrieNode* getNextWordNode
  () const;



  /**
   * @brief Advance in the trie by going to the child that have
   * the current letter of the word.
   * @param word The word.
   * @param pUntilEndOfWord If we have go until the end of the word.
   * @return The resulting node of the trie.
   */
  LingdbDynamicTrieNode* advanceInTrie
  (const std::string& word,
   bool pUntilEndOfWord) const;

  /**
   * @brief Advance until the end of the word if the final node is marked
   * has a node that hold the end of a word.
   * @param word The word.
   * @return The resulting node of the trie.
   */
  LingdbDynamicTrieNode* advanceInTrieIfEndOfAWord
  (const std::string& word) const;





  /**
   * @brief Get a meaning that have a specific grammatical type and
   * that have a lemme at this node.
   * @param gram The grammatical type.
   * @return The meaning or nullptr if haven't found the meaning.
   */
  LingdbMeaning* getMeaning(PartOfSpeech pPartOfSpeech) const;




private:
  /// If this node correspond to the end of a word.
  bool fEndOfWord; 
  /// The letter hold by this node.
  char fLetter;
  LingdbMultiMeaningsNode* fDatasIfItsAMultiMeaningNode;
  /// All the meanings that have a lemme at this node.
  ForwardPtrList<LingdbMeaning>* fMeaningsAtThisLemme;
  /// The WordForms hold by this node.
  ForwardPtrList<LingdbWordForms>* fWordForms;
  /// The father of this node in the trie.
  LingdbDynamicTrieNode* fFather;
  /// The first child of this node in the trie.
  LingdbDynamicTrieNode* fFirstChild;
  /// The next brother of this node in the trie.
  LingdbDynamicTrieNode* fNextBrother;
  ForwardPtrList<LingdbDynamicTrieNode>* fMultiMeaningsNodes;


private:
  friend class LinguisticIntermediaryDatabase;
  friend class LingdbMeaning;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /**
   * @brief Constructor.
   * @param pLetter The letter hold by the node.
   */
  LingdbDynamicTrieNode(char pLetter);

  /**
   * @brief Initialize the node.
   * @param pLetter The letter hold by the node.
   */
  void xInit(char pLetter);

  /**
   * @brief Remove all the WordForms hold by this node.
   * @param pAlloc The allocator of the database.
   */
  void xRemoveAllWordForms
  (CompositePoolAllocator& pAlloc);

  /**
   * @brief Remove a WordFrom.
   * @param pAlloc The allocator of the database.
   * @param pGram The grammatical type of the WordFrom.
   */
  void xRemoveWordForm
  (CompositePoolAllocator& pAlloc,
   PartOfSpeech pPartOfSpeech);


  /**
   * @brief Get the nearest node that is the end of a word.
   * @return The nearest node that is the end of a word.
   */
  LingdbDynamicTrieNode* xGetWordNode() const;

  /**
   * @brief Get the child that have a specific letter.
   * @param pLetter The letter.
   * @return The child that have the letter or nullptr if no child have this
   * letter.
   */
  LingdbDynamicTrieNode* xGetChild
  (char pLetter) const;

  /**
   * @brief Get the brother that have a specific letter.
   * @param pLetter The letter.
   * @return The brother that have a specific letter or nullptr if no brother
   * have this letter.
   */
  LingdbDynamicTrieNode* xGetBrother
  (char pLetter) const;

  /**
   * @brief Remove the meaning that is associated with the word that end
   * at this node.
   * @param pFPAlloc The allocator.
   * @param pGram The grammatical type of the meaning.
   */
  void xRemoveMeaning
  (CompositePoolAllocator& pFPAlloc,
   PartOfSpeech pPartOfSpeech);

  /**
   * @brief Insert a word that begins at this node.
   * @param pFPAlloc The allocator.
   * @param pWord The word to insert.
   * @param pOffset The begin offset of the word.
   * @return The node at the end of the new word inserted.
   */
  LingdbDynamicTrieNode* xInsertWord
  (CompositePoolAllocator& pFPAlloc, const std::string& pWord,
   std::size_t pOffset);

  LingdbWordForms* xAddWordFormFromMeaning
  (CompositePoolAllocator& pAlloc,
   LingdbMeaning& pMeaning,
   bool pAtFront);

  /**
   * @brief Add a meaning that have this word as meaning.
   * @param pFPAlloc The allocator.
   * @param pGram The grammatical type of the meaning.
   * @return The new meaning added.
   */
  LingdbMeaning* xAddMeaning
  (CompositePoolAllocator& pFPAlloc,
   PartOfSpeech pPartOfSpeech);

  /**
   * @brief Deallocate, if possible, the word that ends at this node.
   * @param pFPAlloc The allocator.
   */
  void xRemoveWordIfNoLongerAWord
  (CompositePoolAllocator& pFPAlloc);

  /**
   * @brief Recursively deallocate, if possible, the node.
   * @param pFPAlloc The allocator.
   */
  void xTryToDeallocateNodes
  (CompositePoolAllocator& pFPAlloc);


  LingdbDynamicTrieNode* xAddMultiMeaningsNode
  (CompositePoolAllocator& pAlloc,
   LingdbMeaning* pRootMeaning,
   std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings);

};


} // End of namespace onsem

#include "details/lingdbdynamictrienode.hxx"

#endif // ONSEM_COMPILERMODEL_LINGDBDYNAMICTRIENODE_HPP
