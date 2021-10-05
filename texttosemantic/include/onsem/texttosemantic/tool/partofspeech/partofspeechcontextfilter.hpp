#ifndef LINGUISTICANALYZER_TAGGER_RULES_SYNTACTICANALYZERMETATAGGERRULE_HPP
#define LINGUISTICANALYZER_TAGGER_RULES_SYNTACTICANALYZERMETATAGGERRULE_HPP

#include <vector>
#include <string>
#include <memory>
#include "../../api.hpp"

namespace onsem
{
namespace linguistics
{
struct Token;


class ONSEM_TEXTTOSEMANTIC_API PartOfSpeechContextFilter
{
public:
  explicit PartOfSpeechContextFilter(const std::string& pName)
    : _name(pName)
  {
  }

  virtual ~PartOfSpeechContextFilter() {}

  virtual bool process(std::vector<Token>& pTokens) const = 0;

  std::string getName() const { return _name; }

private:
  const std::string _name;
};

} // End of namespace linguistics
} // End of namespace onsem


#endif // LINGUISTICANALYZER_TAGGER_RULES_SYNTACTICANALYZERMETATAGGERRULE_HPP
