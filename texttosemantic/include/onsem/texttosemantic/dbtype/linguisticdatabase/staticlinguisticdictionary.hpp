#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICLINGUISTICDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICLINGUISTICDICTIONARY_HPP

#include <list>
#include <vector>
#include <set>
#include <map>
#include <istream>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/metawordtreedb.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>
#include <onsem/common/enum/wordtoverbrelation.hpp>
#include "../../api.hpp"


namespace onsem
{
class StaticConceptSet;
namespace linguistics
{
struct InflectedWord;
struct LingWordsGroup;
struct WordAssociatedInfos;


/**
 * @brief This class store: (for a specific language)
 * -> all the words
 * -> all the meanings that can take each word
 * -> a grammatical type for each meaning
 * -> an eventual tag for a meaning
 */
class ONSEM_TEXTTOSEMANTIC_API StaticLinguisticDictionary : public MetaWordTreeDb
{
public:
  StaticLinguisticDictionary(std::istream& pDictIStream,
                             const StaticConceptSet& pStaticConceptSet,
                             SemanticLanguageEnum pLangEnum);

  ~StaticLinguisticDictionary();

  /**
   * @brief Get the version of the database
   * @param pFilename Filename of the database.
   * @return The version of the database.
   */
  static int getVersion(const std::string& pFilename);


  void getGramPossibilitiesAndPutUnknownIfNothingFound
  (std::list<InflectedWord>& pInfosGram,
   const std::string& pWord,
   std::size_t pBeginPos,
   std::size_t pSizeOfWord) const;

  void getGramPossibilities
  (std::list<InflectedWord>& pInfosGram,
   const std::string& pWord,
   std::size_t pBeginPos,
   std::size_t pSizeOfWord) const;

  std::size_t getConfidenceOfWordInThatLanguage(PartOfSpeech& pMainPartOfSpeech,
                                                const std::string& pWord,
                                                std::size_t pBeginPos,
                                                std::size_t pSizeOfWord) const;

  SemanticLanguageEnum getLanguageType() const;

  void getLemma(std::string& pLemmaStr,
                const std::string& pWord) const;

  std::string getLemma(const StaticLinguisticMeaning& pMeaning,
                       bool pWithLinkMeanings) const;

  void getWord(SemanticWord& pWord,
               const StaticLinguisticMeaning& pMeaning) const;

  void getSemanticWord(SemanticWord& pWord,
                       int pMeaningId) const;

  StaticLinguisticMeaning getRootMeaning(const StaticLinguisticMeaning& pMeaning) const;

  void getInfoGram(InflectedWord& pIGram,
                   const StaticLinguisticMeaning& pMeaning) const;

  void getConcepts(std::map<std::string, char>& pConcepts,
                   const StaticLinguisticMeaning& pMeaning) const;

  /**
   * @brief Check if there has to have a separator between words in a sentence.
   * @return True if there has to have a separator between words in a sentence.
   */
  bool haveSeparatorBetweenWords() const;



  /**
   * @brief Length of the longest word that match the begin of a string.
   * @param pStr String that we want to compare.
   * @param pBeginStr Begin in "pStr" that we consider.
   * @return Length of the longest word that match the begin of "pStr".
   */
  std::size_t getLengthOfLongestWord
  (const std::string& pStr, std::size_t pBeginStr) const;


  StaticLinguisticMeaning getLingMeaning
  (const std::string& pWord,
   PartOfSpeech pGram,
   bool pWordIsALemma) const;

  bool hasContextualInfo
  (WordContextualInfos pContextualInfo,
   const StaticLinguisticMeaning& pMeaning) const;


  bool isAGatheringMeaning(const StaticLinguisticMeaning& pMeaning) const;


  void putRootMeaning(InflectedWord& pInfosGram) const;


  bool getConjugaisionsId(int& pConjId,
                          const StaticLinguisticMeaning& pMeaning) const;

  void getWord(std::string& pWord,
               int pWordNode) const;




  struct QuestionWords
  {
    QuestionWords
    (WordToVerbRelation pCanBeBeforeVerb,
     WordToVerbRelation pCanBeAfterVerb,
     bool pCanBeAlone,
     SemanticRequestType pRequest,
     bool pFollowedByRequestedWord)
      : canBeBeforeVerb(pCanBeBeforeVerb),
        canBeAfterVerb(pCanBeAfterVerb),
        canBeAlone(pCanBeAlone),
        request(pRequest),
        followedByRequestedWord(pFollowedByRequestedWord)
    {
    }

    WordToVerbRelation canBeBeforeVerb;
    WordToVerbRelation canBeAfterVerb;
    bool canBeAlone;
    SemanticRequestType request;
    bool followedByRequestedWord;
  };

  const QuestionWords* wordToQuestionWord(const SemanticWord& pWord,
                                          bool pAfterVerb,
                                          bool pIsCloseToTheVerb) const;
  const StaticLinguisticDictionary::QuestionWords* inflWordsToQuestionWord(const std::list<InflectedWord>& pInflWords,
                                                                           bool pAfterVerb,
                                                                           bool pIsCloseToTheVerb) const;
  SemanticRequestType wordToSubordinateRequest(const SemanticWord& pWord) const;
  SemanticRequestType aloneWordToRequest(const SemanticWord& pWord) const;
  SemanticRequestType semWordToRequest(const SemanticWord& pWord) const;

protected:
  virtual bool xIfEndOfAWord
  (const signed char* pNode,
   bool pOnlyWordWithWordFroms) const;


private:
  /// Type to store the header of the database in memory
  union DatabaseHeader
  {
    int intValues[5];
    char charValues[20];
  };
  const StaticConceptSet& fConceptsDatabaseSingleton;
  const SemanticLanguageEnum fLangEnum;
  /// The meanings.
  signed char* fPtrMeaning;
  /// Some Flexions.
  signed char* fPtrSomeFlexions;
  /// Indicate if there has to have a separator between words in a sentence.
  bool fIfSeparatorBetweenWords;
  unsigned char* fMemRules;
  std::map<SemanticWord, std::vector<QuestionWords> > fQuestWordsBefore;
  std::map<SemanticWord, std::vector<QuestionWords> > fQuestWordsAfter;
  std::map<SemanticWord, std::vector<QuestionWords> > fQuestWordsThatCanBeAlone;
  std::map<SemanticWord, std::vector<QuestionWords> > fQuestWordsSubordinate;

  static std::map<SemanticLanguageEnum, std::unique_ptr<StaticLinguisticDictionary>> _instances;


  const StaticLinguisticDictionary::QuestionWords* _aloneWordToQuestionWord(const SemanticWord& pWord) const;

  /// Load a binary file in memory.
  void xLoad(std::istream& pDictIStream);

  /// Deallocate all.
  void xUnload();

  void xInitQuestionWords(unsigned char const** pChar);

  void xGetSemanticWord(SemanticWord& pWord,
                        int32_t pMeaningId) const;

  void xFillIGram(InflectedWord& pIGram,
                  int32_t pMeaningId) const;

  void xFillWordAssInfos(WordAssociatedInfos& pInfos,
                         int pMeaningId) const;

  void xTryToFillMeaningGroup
  (mystd::optional<LingWordsGroup>& pLinkedMeanings,
   int pMeaningId) const;

  void xGetMetaMeanings
  (std::list<LingWordsGroup>& pMetaMeanings,
   const signed char* pMeaningPtr) const;

  /**
   * @brief Constructor by copy.
   * This function is private because, we don't want to allow copies.
   * @param pDb Other database.
   */
  StaticLinguisticDictionary(const StaticLinguisticDictionary& pDb);


  /**
   * @brief Copy of an other object.
   * This function is private because, we don't want to allow copies.
   * @param pDb Other database.
   * @return The resulting database.
   */
  StaticLinguisticDictionary& operator=
  (const StaticLinguisticDictionary& pDB);


  std::size_t xGetConfOfNode(PartOfSpeech& pMainPartOfSpeech,
                             const signed char* pNode) const;

  void xGetGramPossFromNode
  (std::list<InflectedWord>& pInfosGram,
   const signed char* pNode,
   bool pBeginsWithUpperCase) const;


  std::string xGetLemma(int32_t pMeaningId,
                        bool pWithLinkMeanings) const;


  void xGetWordContextualInfos(std::set<WordContextualInfos>& pContextualInfos,
                               const signed char* pMeaningPtr) const;

  bool xWordHasContextualInfos(WordContextualInfos pContextualInfos,
                               const signed char* pMeaningPtr) const;

  void xGetConcepts(std::map<std::string, char>& pConcepts,
                    const signed char* pMeaningPtr) const;


  const StaticLinguisticDictionary::QuestionWords* _wordToQuestionWord
  (const SemanticWord& pWord,
   const std::map<SemanticWord, std::vector<StaticLinguisticDictionary::QuestionWords>>& pQuestWords,
   bool pIsCloseToTheVerb) const;

  // Basic getters
  // =============


  void xFillMeaningGroup
  (LingWordsGroup& pMeaningGroup,
   int pRootMeaningId) const;


  /**
   * @brief Get the list of all the possible meanings
   * of the word that terminated at this node. (if exist)
   * @param pNode The node.
   * @return A pointer to the begin of the meaning list.
   * nullptr if the node doesn't correspond to the end of a word.
   */
  const int32_t* xGetMeaningList(const signed char* pNode) const;




  bool xIsAWordFrom
  (const int* pMeaningFromNode) const;

  int32_t xMeaningFromNodeToMeaningId(const int32_t* pMeaningFromNode) const;

  const signed char* xGetRawCharPtrNbFlexions
  (const int* pMeaningFromNode) const;

  bool xAreTheFlexionsWritenInTheNode
  (const int* pMeaningFromNode) const;

  const signed char* xGetNbFlexionsPtr(const int* pMeaningFromNode) const;

  const int* xGetNextMeaningFromNode
  (const int* pMeaningFromNode) const;




  // Meaning getters
  // ---------------

  int xGetConjugaisonId(const signed char* pMeaning) const;

  PartOfSpeech xGetPartOfSpeech
  (const signed char* pMeaning) const;

  bool xIsAGatheringMeaning(const signed char* pMeaning) const;

  int xGetLemmeNodeId(const signed char* pMeaning) const;

  char xGetNbGatheringMeanings(const signed char* pMeaning) const;

  char xNbLinkedMeanings(const signed char* pMeaning) const;

  char xNbConcepts(const signed char* pMeaning) const;

  char xNbContextInfos(const signed char* pMeaning) const;


  const signed char* xGetFirstContextInfo(const signed char* pMeaning) const;
  //const signed char* xGetContextInfo(const signed char* pMeaning, char pIndex) const;

  const int* xGetFirstConcept(const signed char* pMeaning) const;

  const int* xGetFirstGatheringMeaning(const signed char* pMeaning) const;

  const int* xGetFirstLinkedMeaning(const signed char* pMeaning) const;



  const signed char* xNextContextInfo(const signed char* pContextInfo) const;

  const int* xGetNextConcept(const int* pConcept) const;

  const int* xGetNextGatheringMeaning(const int* pGatheringMeaning) const;

  const int* xGetNextLinkedMeaning(const int* pLinkedMeaning) const;


  char xGetRelationToConcept(int pConceptLink) const;
};


} // End of namespace linguistics
} // End of namespace onsem

#include "detail/staticlinguisticdictionary.hxx"


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICLINGUISTICDICTIONARY_HPP
