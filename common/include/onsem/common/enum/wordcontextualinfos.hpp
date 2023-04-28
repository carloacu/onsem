#ifndef ONSEM_COMMON_TYPE_ENUM_WORDCONTEXTUALINFOS_HPP
#define ONSEM_COMMON_TYPE_ENUM_WORDCONTEXTUALINFOS_HPP

#include <string>
#include <map>


namespace onsem
{


#define WORDCONTEXTUALINFOS_TYPE_TABLE                                                                          \
  ADD_WORDCONTEXTUALINFOS_TYPE(ANY, "any")                                                                      \
  ADD_WORDCONTEXTUALINFOS_TYPE(BEGINSWITHUPPERCHARACTER, "beginsWithUpperCharacter")                            \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANBEBEFORENOUN, "canBeBeforeNoun")                                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANBEBEFOREVERB, "canBeBeforeVerb")                                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANNOTBEBEFORENOUN, "cannotBeBeforeNoun")                                        \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANNOTBEAFTERVERB, "cannotBeAfterVerb")                                          \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANNOTBEPASSIVE, "cannotBePassive")                                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANNOTBECONTINUOUS, "cannotBeContinuous")                                        \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANNOTBEASUBJECT, "cannotBeASubject")                                            \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANNOTBEACOREFRENCE, "cannotBeACoreference")                                     \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANNOTLINKSUBJECT, "cannotLinkSubject")                                          \
  ADD_WORDCONTEXTUALINFOS_TYPE(CONDITION, "condition")                                                          \
  ADD_WORDCONTEXTUALINFOS_TYPE(THEN, "then")                                                                    \
  ADD_WORDCONTEXTUALINFOS_TYPE(ELSE, "else")                                                                    \
  ADD_WORDCONTEXTUALINFOS_TYPE(BEISTHEAUXILIARY, "beIsTheAuxiliary")                                            \
  ADD_WORDCONTEXTUALINFOS_TYPE(HAVEISTHEAUXILIARY, "haveIsTheAuxiliary")                                        \
  ADD_WORDCONTEXTUALINFOS_TYPE(INTRANSITIVEVERB, "intransitiveVerb")                                            \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANBEFOLLOWEDBYINDIRECTOBJECT, "canBeFollowedByIndirectObject")                  \
  ADD_WORDCONTEXTUALINFOS_TYPE(CANBEFOLLOWEDBYLOCATION, "canBeFollowedByLocation")                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(SENTENCECANBEGINWITH, "sentenceCanBeginWith")                                    \
  ADD_WORDCONTEXTUALINFOS_TYPE(SENTENCECANNOTBEGINWITH, "sentenceCannotBeginWith")                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(SENTENCEENDSWITH, "sentenceEndsWith")                                            \
  ADD_WORDCONTEXTUALINFOS_TYPE(SENTENCECANNOTENDWITH, "sentenceCannotEndWith")                                  \
  ADD_WORDCONTEXTUALINFOS_TYPE(NEGATION, "negation")                                                            \
  ADD_WORDCONTEXTUALINFOS_TYPE(UNCOUNTABLE, "uncountable")                                                      \
  ADD_WORDCONTEXTUALINFOS_TYPE(PASSIVE, "passive")                                                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(POSSESSIVE, "possessive")                                                        \
  ADD_WORDCONTEXTUALINFOS_TYPE(TRANSITIVEVERB, "transitiveVerb")                                                \
  ADD_WORDCONTEXTUALINFOS_TYPE(REFTOAPERSON, "refToAPerson")                                                    \
  ADD_WORDCONTEXTUALINFOS_TYPE(REFTOASENTENCE, "refToASentence")                                                \
  ADD_WORDCONTEXTUALINFOS_TYPE(FR_VERBFOLLOWEDBYWORDDE, "fr_verbFollowedByWordDe")                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(FR_ASPIREDH, "fr_aspiredH")                                                      \
  ADD_WORDCONTEXTUALINFOS_TYPE(EN_TIMEWORD, "en_timeWord")                                                      \
  ADD_WORDCONTEXTUALINFOS_TYPE(EN_TIMEWORDPAST, "en_timeWordPast")                                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(EN_TIMEWORDTO, "en_timeWordTo")                                                  \
  ADD_WORDCONTEXTUALINFOS_TYPE(EN_TIMEWORDHALF, "en_timeWordHalf")                                              \
  ADD_WORDCONTEXTUALINFOS_TYPE(EN_TIMEWORDQUARTER, "en_timeWordQuarter")                                        \
  ADD_WORDCONTEXTUALINFOS_TYPE(ENDOFENUM, "end of enum")


#define ADD_WORDCONTEXTUALINFOS_TYPE(a, b) a,
enum class WordContextualInfos : char
{
  WORDCONTEXTUALINFOS_TYPE_TABLE
};
#undef ADD_WORDCONTEXTUALINFOS_TYPE


#define ADD_WORDCONTEXTUALINFOS_TYPE(a, b) {b, WordContextualInfos::a},
static const std::map<std::string, WordContextualInfos> _wordContextualInfos_fromStr = {
  WORDCONTEXTUALINFOS_TYPE_TABLE
};
#undef ADD_WORDCONTEXTUALINFOS_TYPE

#define ADD_WORDCONTEXTUALINFOS_TYPE(a, b) {WordContextualInfos::a, b},
static const std::map<WordContextualInfos, std::string> _wordContextualInfos_toStr = {
  WORDCONTEXTUALINFOS_TYPE_TABLE
};
#undef ADD_WORDCONTEXTUALINFOS_TYPE



static inline std::string wordContextualInfos_toStr
(WordContextualInfos pWordContextualInfos)
{
  return _wordContextualInfos_toStr.find(pWordContextualInfos)->second;
}


inline static WordContextualInfos wordContextualInfos_fromStr
(const std::string& pStr)
{
  auto it = _wordContextualInfos_fromStr.find(pStr);
  if (it != _wordContextualInfos_fromStr.end())
  {
    return it->second;
  }
  return WordContextualInfos::ENDOFENUM;
}




} // End of namespace onsem


#endif // ONSEM_COMMON_TYPE_ENUM_WORDCONTEXTUALINFOS_HPP
