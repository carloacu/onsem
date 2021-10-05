#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_RESOURCEGROUNDINGEXTRACTOR_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_RESOURCEGROUNDINGEXTRACTOR_HPP

#include <vector>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include "../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API PairOfLabelAndBeginOfResource
{
  PairOfLabelAndBeginOfResource(const std::string& pLabel);

  std::string label;
  std::string begOfResourceStr;
  std::size_t begOfResourceStrSize;
};


struct ONSEM_TEXTTOSEMANTIC_API ResourceGroundingExtractor
{
  ResourceGroundingExtractor(const std::string& pLabelStr);
  ResourceGroundingExtractor(const std::vector<std::string>& pLabelsStrs);

  const std::string& extractBeginOfAResource(const std::string& pStr) const;
  bool isBeginOfAResource(const std::string& pStr) const;

  static std::unique_ptr<SemanticResourceGrounding> makeResourceGroundingFromStr(
      const std::string& pStr,
      const std::string& pLabelStr);

private:
  std::vector<PairOfLabelAndBeginOfResource> _labels;
  static const std::string _emptyString;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPES_RESOURCEGROUNDINGEXTRACTOR_HPP
