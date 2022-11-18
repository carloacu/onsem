#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_DETAIL_STATICLINGUISTICDICTIONARY_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_DETAIL_STATICLINGUISTICDICTIONARY_HXX

#include "../staticlinguisticdictionary.hpp"
#include <onsem/common/binary/binaryloader.hpp>

namespace onsem
{
namespace linguistics
{


/**
 * Specification of the database file format
 * =========================================
 *
 *
 * At the begin of the file:
 * -------------------------
 *                  int    idFormalism              To be sure that we don't load a database with an old formalism.
 *                  int    versionOfTheDatabase     The version of the database.
 *                  int    offsetOfTheFirstNode
 *                  int    sizeOfThePatriciaTrie    "Patricia Trie" = "All the nodes".
 *                  bool   hasSeparator             Indicate if there has to have a separator between words in a sentence
 *                                                  (ex: " ", "'", ...).
 *
 *
 *
 * <possible stuffing bytes>
 *
 *
 * For each meaning:
 * -----------------
 *                  <3b>   conjStruct               Offset to the conjugaison structure.
 *                  char   gram                     Grammatical type associated with this meaning.
 *                  <3b>   lemmeNode                Offset to the lemme node.
 *                  char   nbGathMeanings           The number of gather meanings. (ex: "dire", verb => one gatherMeaning is "dire~se")
 *                  char   nbLinkMeanings           The number of linked meanings. ("dire~se", verb => linkedMeaning is "se", pron_compl)
 *                  char   nbConcepts               The number of concepts associated to this meaning. (ex: "fermer", verb => verb_action_close)
 *                  char   nbContextInfos           The number of context infos for ths meaning. (ex: "ne", adverb => negation_info)
 * nbContextInfos*: char   contextInfo              A contextInfo associated to this meaning.
 *                  <possible stuffing bytes>
 * nbConcepts*:     <3b>   conceptOffset            An offset to a concept.
 *            |     char   conceptConfidence        Confidence between the concept and this meaning.
 * nbGathMeanings*: <3b>   gatheringMeaning         The meaning that gather the root and the linked meanings.
 *                | char   0
 * nbLinkMeanings*: <3b>   linkedMeaning             A linked meaning.
 *                | char   direction                 Direction of the linked meaning from the root meaning.
 *
 *
 * <possible stuffing bytes>
 *
 *
 * For each node:
 * --------------
 *                  <3b>   parentNode               Offset to the parent node.
 *                  char   nbLetters                The number of letters in the node.
 * nbLetters*:      char   letter                   A letter of the node.
 *                  char   nbMeanings               The number of meanings.
 *                  char   nbChildren               The number of children nodes.
 *                  <possible stuffing bytes>
 * nbChildren*:     <3b>   child                    Offset to a child node.
 *          |       char   firstLetter              The first letter of the child node.
 * nbMeanings*:     <3b>   meaning                  A meaning offset of this node.
 *            |     char   isAWordForm              1 = True, 0 = False
 *            |     isAWordForm: char   nbFlexions  The number of flexions previously written for this meaning. (if his first byte = 0, the flexions are written in this node, if his first byte = 1, flexions are written before this node)
 *            |     isAWordForm: nbFlexions*:    char   flexion  A flexion for the meaning.
 *            |     <possible stuffing bytes>
 */




// We don't want to allow copies that's why this function is private.
inline StaticLinguisticDictionary& StaticLinguisticDictionary::operator=
(const StaticLinguisticDictionary&)
{
  return *this;
}


inline SemanticLanguageEnum StaticLinguisticDictionary::getLanguageType() const
{
  return fLangEnum;
}



inline bool StaticLinguisticDictionary::haveSeparatorBetweenWords
() const
{
  return fIfSeparatorBetweenWords;
}



inline const int32_t* StaticLinguisticDictionary::xGetMeaningList
(const signed char* pNode) const
{
  return xGetBeginOfEndingStruct(pNode);
}


inline bool StaticLinguisticDictionary::xIfEndOfAWord
(const signed char* pNode,
 bool pOnlyWordWithWordFroms) const
{
  return xNbMeanings(pNode) > 0 &&
      (!pOnlyWordWithWordFroms ||
       xIsAWordFrom(xGetMeaningList(pNode)));
}




inline bool StaticLinguisticDictionary::xIsAWordFrom
(const int* pMeaningFromNode) const
{
  return xGetCharAfterAlignedDec(*pMeaningFromNode) == 1;
}

inline int32_t StaticLinguisticDictionary::xMeaningFromNodeToMeaningId
(const int32_t* pMeaningFromNode) const
{
  return binaryloader::alignedDecToInt(*pMeaningFromNode);
}

inline const signed char* StaticLinguisticDictionary::xGetRawCharPtrNbFlexions(const int* pMeaningFromNode) const
{
  return reinterpret_cast<const signed char*>(pMeaningFromNode) + 4;
}

inline bool StaticLinguisticDictionary::xAreTheFlexionsWritenInTheNode
(const int* pMeaningFromNode) const
{
  return ((*xGetRawCharPtrNbFlexions(pMeaningFromNode)) & 0x80) == 0;
}

inline const signed char* StaticLinguisticDictionary::xGetNbFlexionsPtr
(const int* pMeaningFromNode) const
{
  const auto* rawNbFlexions = xGetRawCharPtrNbFlexions(pMeaningFromNode);
  if (xAreTheFlexionsWritenInTheNode(pMeaningFromNode))
  {
    return rawNbFlexions;
  }
  return fPtrSomeFlexions + (static_cast<std::size_t>(0x7F & (*rawNbFlexions)) * 4);
}

inline const int* StaticLinguisticDictionary::xGetNextMeaningFromNode
(const int* pMeaningFromNode) const
{
  if (!xIsAWordFrom(pMeaningFromNode))
  {
    return pMeaningFromNode + 1;
  }
  if (xAreTheFlexionsWritenInTheNode(pMeaningFromNode))
  {
    return reinterpret_cast<const int*>
        (binaryloader::alignMemory(reinterpret_cast<const signed char*>(pMeaningFromNode + 1) + 1 +
                                   *xGetNbFlexionsPtr(pMeaningFromNode)));
  }
  return reinterpret_cast<const int*>
      (binaryloader::alignMemory(reinterpret_cast<const signed char*>(pMeaningFromNode + 1) + 1));
}






inline int StaticLinguisticDictionary::xGetConjugaisonId
(const signed char* pMeaning) const
{
  return binaryloader::alignedDecToInt(*reinterpret_cast<const int*>(pMeaning));
}

inline PartOfSpeech StaticLinguisticDictionary::xGetPartOfSpeech
(const signed char* pMeaning) const
{
  return PartOfSpeech(pMeaning[3]);
}

inline int StaticLinguisticDictionary::xGetLemmeNodeId
(const signed char* pMeaning) const
{
  return binaryloader::alignedDecToInt
      (*reinterpret_cast<const int*>(pMeaning + sizeof(int)));
}

inline bool StaticLinguisticDictionary::xIsAGatheringMeaning
(const signed char* pMeaning) const
{
  return xNbLinkedMeanings(pMeaning) > 0;
}

inline char StaticLinguisticDictionary::xGetNbGatheringMeanings
(const signed char* pMeaning) const
{
  return (pMeaning + sizeof(int))[3];
}

inline char StaticLinguisticDictionary::xNbLinkedMeanings
(const signed char* pMeaning) const
{
  return *(pMeaning + sizeof(int) * 2);
}

inline char StaticLinguisticDictionary::xNbConcepts
(const signed char* pMeaning) const
{
  return (pMeaning + sizeof(int) * 2)[1];
}

inline char StaticLinguisticDictionary::xNbContextInfos
(const signed char* pMeaning) const
{
  return (pMeaning + sizeof(int) * 2)[2];
}

inline const signed char* StaticLinguisticDictionary::xGetFirstContextInfo
(const signed char* pMeaning) const
{
  return pMeaning + (sizeof(int) * 2) + 3;
}

/*
inline const signed char* StaticLinguisticDictionary::xGetContextInfo
(const signed char* pMeaning,
 char pIndex) const
{
  return xGetFirstContextInfo(pMeaning) + pIndex;
}
*/

inline const int* StaticLinguisticDictionary::xGetFirstConcept
(const signed char* pMeaning) const
{
  return reinterpret_cast<const int*>
      (binaryloader::alignMemory(xGetFirstContextInfo(pMeaning) + xNbContextInfos(pMeaning)));
}

inline const int* StaticLinguisticDictionary::xGetFirstGatheringMeaning
(const signed char* pMeaning) const
{
  return xGetFirstConcept(pMeaning) + xNbConcepts(pMeaning);
}

inline const int* StaticLinguisticDictionary::xGetFirstLinkedMeaning
(const signed char* pMeaning) const
{
  return xGetFirstGatheringMeaning(pMeaning) + xGetNbGatheringMeanings(pMeaning);
}



inline const signed char* StaticLinguisticDictionary::xNextContextInfo
(const signed char* pContextInfo) const
{
  return ++pContextInfo;
}

inline const int* StaticLinguisticDictionary::xGetNextConcept
(const int* pConcept) const
{
  return ++pConcept;
}

inline const int* StaticLinguisticDictionary::xGetNextGatheringMeaning
(const int* pGatheringMeaning) const
{
  return ++pGatheringMeaning;
}

inline const int* StaticLinguisticDictionary::xGetNextLinkedMeaning
(const int* pLinkedMeaning) const
{
  return ++pLinkedMeaning;
}



inline char StaticLinguisticDictionary::xGetRelationToConcept
(int pConceptLink) const
{
  return xGetCharAfterAlignedDec(pConceptLink);
}



inline const StaticLinguisticDictionary::StaticWord& StaticLinguisticDictionary::getBeAux() const
{
  return _beAux;
}

inline const StaticLinguisticDictionary::StaticWord& StaticLinguisticDictionary::getHaveAux() const
{
  return _haveAux;
}

inline const StaticLinguisticDictionary::StaticWord& StaticLinguisticDictionary::getBeVerb() const
{
  return _beVerb;
}

inline const StaticLinguisticDictionary::StaticWord& StaticLinguisticDictionary::getSayVerb() const
{
  return _sayVerb;
}

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_DETAIL_STATICLINGUISTICDICTIONARY_HXX
