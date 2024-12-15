#include "synthesizertypes.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>

namespace onsem {

SynthesizerCurrentContext::SynthesizerCurrentContext() = default;

SynthesizerCurrentContext::SynthesizerCurrentContext(const SynthesizerCurrentContext& pOther)
    : grammaticalTypeFromParent(pOther.grammaticalTypeFromParent)
    , contextType(pOther.contextType)
    , verbTense(pOther.verbTense)
    , verbGoal(pOther.verbGoal)
    , requests(pOther.requests)
    , wordContext(pOther.wordContext)
    , compPolarity(pOther.compPolarity)
    , verbContextOpt(pOther.verbContextOpt)
    , currSentence(pOther.currSentence)
    , parentSubject(pOther.parentSubject)
    , rootSubject(pOther.rootSubject)
    , rootStatement(pOther.rootStatement)
    , isParentARelativeGrounding(pOther.isParentARelativeGrounding)
    , isAtRoot(false)
    , isPositive(pOther.isPositive)
    , isPassive(pOther.isPassive)
    , hasASubject(pOther.hasASubject)
    , ownerWrittenBefore(pOther.ownerWrittenBefore)
    , isPartOfANameAssignement(pOther.isPartOfANameAssignement)
    , isASubordinateWithoutPreposition(pOther.isASubordinateWithoutPreposition) {}

SynthesizerConfiguration::SynthesizerConfiguration(const SemanticMemoryBlock& pMemBlock,
                                                   bool pOneLinePerSentence,
                                                   const std::string& pCurrentUserId,
                                                   const TextProcessingContext& pTextProcContext,
                                                   const linguistics::LinguisticDatabase& pLingDb)
    : memBlock(pMemBlock)
    , oneLinePerSentence(pOneLinePerSentence)
    , lingDb(pLingDb)
    , semExp(nullptr)
    , authorId(_mergeUserIdWithContext(pTextProcContext.author.userId, pCurrentUserId))
    , receiverId(_mergeUserIdWithContext(pTextProcContext.receiver.userId, pCurrentUserId))
    , textProcessingContext(pTextProcContext)
    , _nbOfRemainingPossibleRecursiveCalls(5) {}

std::string SynthesizerConfiguration::_mergeUserIdWithContext(const std::string& pUserId,
                                                              const std::string& pCurrentUserId) {
    std::string res = pUserId == SemanticAgentGrounding::getCurrentUser() ? pCurrentUserId : pUserId;
    if (res == SemanticAgentGrounding::getUserNotIdentified())
        return SemanticAgentGrounding::getCurrentUser();
    return res;
}

bool SynthesizerConfiguration::canDoAnotherRecurssiveCall() {
    return --_nbOfRemainingPossibleRecursiveCalls >= 0;
}

PartOfSpeech OutSemExp::getFirstPartOfSpeech() const {
    if (out.empty())
        return partOfSpeech;
    return out.front().word.partOfSpeech;
}

PartOfSpeech OutSemExp::getLastPartOfSpeech() const {
    if (out.empty())
        return partOfSpeech;
    return out.back().word.partOfSpeech;
}

bool OutSemExp::containsOnly(PartOfSpeech pPartOfSpeech) const {
    for (const auto& currElt : out)
        if (currElt.word.partOfSpeech != pPartOfSpeech)
            return false;
    return true;
}

std::size_t OutSemExp::getPriority() {
    if (!out.empty()) {
        std::size_t res = out.size();
        auto& firstOutWord = out.front();
        auto pos = firstOutWord.word.partOfSpeech;
        res += firstOutWord.priorityOffset;
        if (pos == PartOfSpeech::PREPOSITION)
            res += 2;
        else if (pos == PartOfSpeech::ADVERB)
            res += 1000;
        return res;
    }
    return 0;
}

void OutSemExp::moveContent(OutSemExp& pOut) {
    partOfSpeech = pOut.partOfSpeech;
    gender = pOut.gender;
    relativePerson = pOut.relativePerson;
    out.splice(out.end(), pOut.out);
}

}    // End of namespace onsem
