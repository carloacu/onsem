#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>


namespace onsem
{
namespace
{

void _copyMapStrToUSemExp(
    std::map<std::string, std::vector<UniqueSemanticExpression>>& pOutPut,
    const std::map<std::string, std::vector<UniqueSemanticExpression>>& pInput)
{
  for (const auto& currElt : pInput)
  {
    auto& newElt = pOutPut[currElt.first];
    for (const auto& currSemExp : currElt.second)
      newElt.push_back(currSemExp->clone());
  }
}

}


SemanticResource::SemanticResource(const SemanticResource& pOther)
  : label(pOther.label),
    language(pOther.language),
    value(pOther.value),
    parameterLabelsToQuestions(),
    parametersLabelsToValue()
{
  _copyMapStrToUSemExp(parameterLabelsToQuestions, pOther.parameterLabelsToQuestions);
  _copyMapStrToUSemExp(parametersLabelsToValue, pOther.parametersLabelsToValue);
}


std::string SemanticResource::toStr() const
{
  if (language != SemanticLanguageEnum::UNKNOWN)
    return label + "=#" + semanticLanguageEnum_toStr(language) + "#" + value;
  return label + "=" + value;
}


} // End of namespace onsem
