#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_STATICTRANSLATIONDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_STATICTRANSLATIONDICTIONARY_HPP

#include <set>
#include <onsem/common/keytostreams.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/metawordtreedb.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include "../../api.hpp"

namespace onsem
{
struct SemanticLanguageGrounding;


class ONSEM_TEXTTOSEMANTIC_API StaticTranslationDictionary : public MetaWordTreeDb
{
public:
  StaticTranslationDictionary(linguistics::LinguisticDatabaseStreams& pIStreams,
                              SemanticLanguageEnum pInLangEnum,
                              SemanticLanguageEnum pOutLangEnum);
  ~StaticTranslationDictionary();

  int getTranslation(const std::string& pLemma,
                     const StaticLinguisticMeaning& pInMeaning) const;


private:
  union DatabaseHeader
  {
    int intValues[2];
    char charValues[8];
  };
  struct TranslationId
  {
    TranslationId
    (SemanticLanguageEnum pInLangEnum,
     SemanticLanguageEnum pOutLangEnum)
      : inLangEnum(pInLangEnum),
        outLangEnum(pOutLangEnum)
    {
    }

    bool operator<(const TranslationId& pOther) const
    {
      if (inLangEnum != pOther.inLangEnum)
      {
        return inLangEnum < pOther.inLangEnum;
      }
      return outLangEnum < pOther.outLangEnum;
    }

    SemanticLanguageEnum inLangEnum;
    SemanticLanguageEnum outLangEnum;
  };
  TranslationId fTransId;


  /// Load a binary file in memory.
  void xLoad(linguistics::LinguisticDatabaseStreams& pIStreams);

  /// Deallocate all.
  void xUnload();



  // Basic getters
  // =============

  const int* xGetTranslationList(const signed char* pNode) const;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SRC_TYPE_LINGUISTICDATABASE_STATICTRANSLATIONDICTIONARY_HPP
