#ifndef ONSEM_COMPILERMODEL_LINGDBMODIFIER_HPP
#define ONSEM_COMPILERMODEL_LINGDBMODIFIER_HPP

#include <string>
#include <onsem/common/enum/partofspeech.hpp>


namespace onsem
{
class LinguisticIntermediaryDatabase;
template <typename T>
struct ForwardPtrList;
class LingdbWordForms;
class LingdbMeaning;

/// Class that allow to make some modifications of the database.
class LingdbModifier
{
public:
  /**
   * @brief Find "*que" words and add "*qu" ones.
   * @param pLingDatabase The linguistic database.
   * @return The number of words added.
   */
  std::size_t findQueAddQu
  (LinguisticIntermediaryDatabase& pLingDatabase) const;

  /**
   * @brief Duplicate an existing meaning to a new one
   * with another grammatical type.
   * @param pLingDatabase The linguistic database.
   * @param pLemme The lemme common to the 2 meanings.
   * @param pRefGram The grammatical type of the existing meaning.
   * @param pNewGram The grammatical type of the new meaning.
   */
  void associateANewGramForAMeaning
  (LinguisticIntermediaryDatabase& pLingDatabase,
   const std::string& pLemma,
   PartOfSpeech pRefPartOfSpeech,
   PartOfSpeech pNewPartOfSpeech) const;

  std::size_t addToAtBeginOfVerbsForEnglish
  (LinguisticIntermediaryDatabase& pLingDatabase) const;

  /**
   * @brief Delete all the expressions.
   * @param pLingDatabase The linguistic database.
   * @return The number of words deleted.
   */
  std::size_t delExprs
  (LinguisticIntermediaryDatabase& pLingDatabase) const;


  /**
   * @brief Delete all the words of the database.
   * @param pLingDatabase The linguistic database.
   * @return The number of words deleted.
   */
  std::size_t delAllWords
  (LinguisticIntermediaryDatabase& pLingDatabase) const;

private:
  /**
   * @brief Know if a word finish with "que"
   * @param pWord The word.
   * @return True if the word finish with "que", False otherwise.
   */
  bool xEndWithQue
  (const std::string& pWord) const;

  bool xEndWithQuApostrophe
  (const std::string& pWord) const;

  /**
   * @brief Get the wordform of a list that has a specific meaning.
   * @param pWf The list of wordforms.
   * @param pMeaning The meaning searched.
   * @return A pointer to the found wordform, nullptr if nothing has been found.
   */
  LingdbWordForms* xGetWordFormThatAsASpecificMeaning
  (const ForwardPtrList<LingdbWordForms>* pWf,
   LingdbMeaning* pMeaning) const;

};


} // End of namespace onsem

#endif // ONSEM_COMPILERMODEL_LINGDBMODIFIER_HPP
