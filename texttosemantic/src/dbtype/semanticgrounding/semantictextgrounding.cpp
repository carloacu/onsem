#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>


namespace onsem
{

SemanticTextGrounding::SemanticTextGrounding
(const std::string& pText,
 SemanticLanguageEnum pForLanguage,
 bool pHasQuotationMark)
  : SemanticGrounding(SemanticGroundingType::TEXT),
    text(pText),
    forLanguage(pForLanguage),
    hasQuotationMark(pHasQuotationMark)
{
}


bool SemanticTextGrounding::operator==(const SemanticTextGrounding& pOther) const
{
  return this->isEqual(pOther);
}

bool SemanticTextGrounding::isEqual(const SemanticTextGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      text == pOther.text &&
      forLanguage == pOther.forLanguage &&
      hasQuotationMark == pOther.hasQuotationMark;
}


bool SemanticTextGrounding::isAQuotedText(const std::string& pText)
{
  std::size_t sizeOfTheText = pText.size();
  if (sizeOfTheText < 2)
    return false;
  return pText[0] == '"' && pText[sizeOfTheText - 1] == '"';
}

std::string SemanticTextGrounding::getTheUnquotedText(const std::string& pText)
{
  std::size_t sizeOfTheText = pText.size();
  if (sizeOfTheText < 2)
    return pText;
  return pText.substr(1, sizeOfTheText - 2);
}


} // End of namespace onsem
