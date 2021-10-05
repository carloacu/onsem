#ifndef ONSEM_TEXTTOSEMANTIC_DETAIL_METAWORDTREEDB_HPP
#define ONSEM_TEXTTOSEMANTIC_DETAIL_METAWORDTREEDB_HPP

#include <list>
#include <map>
#include "virtualsembinarydatabase.hpp"
#include "../../../api.hpp"

namespace onsem
{


enum class SearchForLongestWordMode
{
  DISABLED,
  ENBABLED_BUTWITHSEPARATORAFTER,
  ENABLED
};


class ONSEM_TEXTTOSEMANTIC_API MetaWordTreeDb : public VirtualSemBinaryDatabase
{
public:
  MetaWordTreeDb();


protected:
  /// The patricia trie.
  signed char* fPtrPatriciaTrie;

  virtual bool xIsLoaded() const;

  static bool isASeparator(const std::string& pString,
                                std::size_t pPos);

  /**
   * @brief Search the longest word that match the begin of a string.
   * @param pLongestWord Length of the longest word found.
   * @param pString String that we look for.
   * @param pBeginOfString Begin of "pString" that we consider.
   * @return The end node of the word "pString" begining at "pBeginOfString" end terminating at
   * the end of "pString". nullptr if the word doesn't exist.
   */
  const signed char* xSearchInPatriciaTrie(
      std::size_t& pLongestWord,
      const std::string& pString,
      const std::size_t pBeginOfString, const std::size_t pSizeOfString,
      bool pOnlyWordWithWordFroms,
      SearchForLongestWordMode pLongWordMode) const;


  void xGetWordsThatBeginWith
  (std::list<const signed char*>& pResWords,
   const std::string& pBeginOfWords) const;


  void xGetWord
  (std::string& pWord,
   int pWordNode) const;



  // Basic getters
  // =============

  // Node getters
  // ------------

  /**
   * @brief Get the number of letters coded in the node.
   * @param pNode The node.
   * @return The number of letters coded in the node.
   */
  unsigned char xNbLetters(const signed char* pNode) const;

  /**
   * @brief Get the nth letter coded in the node.
   * @param pNode The node.
   * @return The nth letter coded in the node.
   */
  signed char xGetLetter(const signed char* pNode, unsigned char i) const;

  /**
   * @brief Get the number of children in the node.
   * @param pNode The node.
   * @return The number of children in the node.
   */
  unsigned char xNbChildren(const signed char* pNode) const;

  /**
   * @brief Get the node that correspond to the end of the word in the patricia trie.
   * @param pWord The word that we look for.
   * @return The node that correspond to the end of the word in the patricia trie.
   */
  const signed char* xGetNode(
      const std::string& pWord,
      std::size_t pBeginPos,
      std::size_t pSizeOfWord,
      bool pOnlyWordWithWordFroms) const;

  virtual bool xIfEndOfAWord
  (const signed char* pNode,
   bool pOnlyWordWithWordFroms) const;

  /**
   * @brief Get the number of meanings in the node.
   * @param pNode The node.
   * @return The number of meanings in the node.
   */
  unsigned char xNbMeanings(const signed char* pNode) const;


  const int* xGetFather
  (const signed char* pNode) const;

  const signed char* xAlignedDecToPtr
  (signed char* pBeginMem,
   int pAlignedDec) const;


  signed char xGetCharAfterAlignedDec(int pAlignedDec) const;


  /**
   * @brief Get the first child of the node.
   * @param pNode The node.
   * @return A pointer to the First child of the node.
   */
  const int* xGetFirstChild(const signed char* pNode) const;

  const int* xGetBeginOfEndingStruct(const signed char* pNode) const;



private:
  MetaWordTreeDb
  (const MetaWordTreeDb& pOther);

  MetaWordTreeDb& operator=
  (const MetaWordTreeDb& pOther);

  void xGetWordsFromANodeOfTheTree
  (std::list<const signed char*>& pResWords,
   const signed char* pCurrNode,
   bool pCanAddCurrNode) const;
};


} // End of namespace onsem

#include "metawordtreedb.hxx"

#endif // ONSEM_TEXTTOSEMANTIC_DETAIL_METAWORDTREEDB_HPP
