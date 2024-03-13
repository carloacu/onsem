#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_SYNTHANALENDINGSTEPFORDEBUG_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_SYNTHANALENDINGSTEPFORDEBUG_HPP

#include <string>
#include <onsem/texttosemantic/type/enum/linguisticanalysisfinishdebugstepenum.hpp>
#include "../../api.hpp"

namespace onsem {
namespace linguistics {

struct ONSEM_TEXTTOSEMANTIC_API SynthAnalEndingStepForDebug {
    std::string tokenizerEndingStep = tokenizerDefaultEndingStep;
    std::size_t nbOfDebugRoundsForTokens = 1;
    LinguisticAnalysisFinishDebugStepEnum endingStep = LinguisticAnalysisFinishDebugStepEnum::FINISH;
    std::size_t nbOfDebugRoundsForSynthAnalysis = 1;

    bool doWeShouldStopForDebug(LinguisticAnalysisFinishDebugStepEnum pEndingStep) const {
        return endingStep == pEndingStep && nbOfDebugRoundsForSynthAnalysis <= 1;
    }
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_SYNTHANALENDINGSTEPFORDEBUG_HPP
