#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_LINGUISTICANALYSISFINISHDEBUGSTEPENUM_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_LINGUISTICANALYSISFINISHDEBUGSTEPENUM_HPP

#include <string>
#include <map>

namespace onsem
{
namespace linguistics
{
static const std::string tokenizerDefaultEndingStep = "finish";

#define LINGUISTICFINISHDEBUGSTEP_TABLE                                                       \
  LINGUISTICFINISHDEBUGSTEP(VERBALCHUNKER, "verbal chunker")                                  \
  LINGUISTICFINISHDEBUGSTEP(NOMINALCHUNKER, "nominal chunker")                                \
  LINGUISTICFINISHDEBUGSTEP(VERBALTONOMINALCHUNKSLINKER, "verbal_to_nominal chunks_linker")   \
  LINGUISTICFINISHDEBUGSTEP(ENTITYRECOGNIZER, "entity recognizer")                            \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_1, "sub. extractor 1")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_2, "sub. extractor 2")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_3, "sub. extractor 3")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_4, "sub. extractor 4")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_5, "sub. extractor 5")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_6, "sub. extractor 6")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_7, "sub. extractor 7")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_8, "sub. extractor 8")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_9, "sub. extractor 9")                       \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_10, "sub. extractor 10")                     \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR_11, "sub. extractor 11")                     \
  LINGUISTICFINISHDEBUGSTEP(SUBORDONATEEXTRACTOR, "sub. extractor")                           \
  LINGUISTICFINISHDEBUGSTEP(RESOLVEINCOMPLETELISTS, "resolve incomplete lists")               \
  LINGUISTICFINISHDEBUGSTEP(ERRORDETECTOR, "error detector")                                  \
  LINGUISTICFINISHDEBUGSTEP(LINKER, "linker")                                                 \
  LINGUISTICFINISHDEBUGSTEP(INTERJECTIONS_ADDER, "interjections adder")                       \
  LINGUISTICFINISHDEBUGSTEP(TEACH_PARSING, "teach parsing")                                   \
  LINGUISTICFINISHDEBUGSTEP(FINISH, "finish")


#define LINGUISTICFINISHDEBUGSTEP(a, b) a,
enum class LinguisticAnalysisFinishDebugStepEnum
{
  LINGUISTICFINISHDEBUGSTEP_TABLE
};
#undef LINGUISTICFINISHDEBUGSTEP



#define LINGUISTICFINISHDEBUGSTEP(a, b) {LinguisticAnalysisFinishDebugStepEnum::a, b},
static const std::map<LinguisticAnalysisFinishDebugStepEnum, std::string> linguisticAnalysisFinishDebugStepEnum_toStr = {
  LINGUISTICFINISHDEBUGSTEP_TABLE
};
#undef LINGUISTICFINISHDEBUGSTEP


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_LINGUISTICANALYSISFINISHDEBUGSTEPENUM_HPP
