#ifndef ONSEM_SEMANTICTOTEXT_TYPE_NATURALLANGUAGEEXPRESSION_HPP
#define ONSEM_SEMANTICTOTEXT_TYPE_NATURALLANGUAGEEXPRESSION_HPP

#include <string>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>

namespace onsem
{

enum NaturalLanguageTypeOfText
{
  NOUN,
  EXPRESSION,
  VERB,
  AGENT,
  QUOTE
};


struct NaturalLanguageText
{
  NaturalLanguageText()
    : text(""),
      type(NaturalLanguageTypeOfText::EXPRESSION),
      language(SemanticLanguageEnum::UNKNOWN)
  {
  }
  NaturalLanguageText(const std::string& pText,
                      NaturalLanguageTypeOfText pType,
                      SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN)
    : text(pText),
      type(pType),
      language(pLanguage)
  {
  }

  std::string text;
  NaturalLanguageTypeOfText type;
  SemanticLanguageEnum language;
};

struct NaturalLanguageExpression
{
  NaturalLanguageText word{};
  SemanticVerbTense verbTense = SemanticVerbTense::PRESENT;
  VerbGoalEnum verbGoal{VerbGoalEnum::NOTIFICATION};
  bool polarity = true; // false: sure of the contrary, true: sure of it
  std::map<GrammaticalType, NaturalLanguageExpression> children{};
  SemanticQuantity quantity{};
  SemanticReferenceType reference = SemanticReferenceType::UNDEFINED;
};

} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_TYPE_NATURALLANGUAGEEXPRESSION_HPP
