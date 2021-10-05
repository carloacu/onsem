#ifndef ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERRESULTTYPES_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERRESULTTYPES_HPP

#include <string>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>

namespace onsem
{

enum class SynthesizerResultEnum
{
  TEXT,
  TASK
};


struct SynthesizerResult
{
  SynthesizerResult(SynthesizerResultEnum pType)
    : type(pType)
  {
  }

  virtual ~SynthesizerResult() {}
  SynthesizerResultEnum type;
};


struct SynthesizerText : public SynthesizerResult
{
  SynthesizerText(const std::string& pText)
    : SynthesizerResult(SynthesizerResultEnum::TEXT),
      text(pText)
  {
  }

  std::string text;
};


struct SynthesizerTask : public SynthesizerResult
{
  SynthesizerTask(const SemanticResource& pResource)
    : SynthesizerResult(SynthesizerResultEnum::TASK),
      resource(pResource)
  {
  }

  SemanticResource resource;
};


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_LINGUISTICSYNTHESIZER_SYNTHESIZERRESULTTYPES_HPP
