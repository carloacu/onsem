#ifndef SEMANTICTEXTGROUNDING_H
#define SEMANTICTEXTGROUNDING_H

#include <list>
#include "semanticgrouding.hpp"
#include "../../api.hpp"
#include <onsem/common/enum/semanticlanguagetype.hpp>

namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticTextGrounding : public SemanticGrounding
{
  SemanticTextGrounding(const std::string& pText,
                        SemanticLanguageEnum pForLanguage = SemanticLanguageEnum::UNKNOWN,
                        bool pHasQuotationMark = false);


  const SemanticTextGrounding& getTextGrounding() const override { return *this; }
  SemanticTextGrounding& getTextGrounding() override { return *this; }
  const SemanticTextGrounding* getTextGroundingPtr() const override { return this; }
  SemanticTextGrounding* getTextGroundingPtr() override { return this; }

  bool operator==(const SemanticTextGrounding& pOther) const;
  bool isEqual(const SemanticTextGrounding& pOther) const;

  static bool isAQuotedText(const std::string& pText);
  static std::string getTheUnquotedText(const std::string& pText);

  std::string text;
  SemanticLanguageEnum forLanguage;
  bool hasQuotationMark;
};


} // End of namespace onsem

#endif // SEMANTICTEXTGROUNDING_H
