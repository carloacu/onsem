#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGINFOSGRAM_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGINFOSGRAM_HPP

#include <list>
#include <set>
#include <vector>
#include <memory>
#include <map>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>
#include "../api.hpp"

namespace onsem {
namespace linguistics {

struct ONSEM_TEXTTOSEMANTIC_API LingWordsGroup {
    LingWordsGroup(std::unique_ptr<SemanticWord> pRootWord)
        : rootWord(std::move(pRootWord))
        , linkedMeanings() {}

    LingWordsGroup(const LingWordsGroup& pOther);
    LingWordsGroup& operator=(const LingWordsGroup& pOther);

    LingWordsGroup(LingWordsGroup&& pOther);
    LingWordsGroup& operator=(LingWordsGroup&& pOther);

    std::unique_ptr<SemanticWord> rootWord;
    std::list<std::pair<std::unique_ptr<SemanticWord>, LinkedMeaningDirection> > linkedMeanings;

private:
    void _set(const LingWordsGroup& pOther);
};

struct ONSEM_TEXTTOSEMANTIC_API CondContextualInfo {
    CondContextualInfo(WordContextualInfos pFirst, WordContextualInfos pSecond)
        : first(pFirst)
        , second(pSecond) {}

    WordContextualInfos first;
    WordContextualInfos second;
};

struct ONSEM_TEXTTOSEMANTIC_API WordAssociatedInfos {
    void mergeWith(const WordAssociatedInfos& pOther);
    void clear();
    bool hasContextualInfo(WordContextualInfos pContextualInfo) const;
    bool isOnlyTransitive() const;

    std::map<std::string, char> concepts{};

    std::set<WordContextualInfos> contextualInfos{};

    mystd::optional<LingWordsGroup> linkedMeanings{};

    std::list<LingWordsGroup> metaMeanings{};
};

/// Grammatical informations of a word.
struct ONSEM_TEXTTOSEMANTIC_API InflectedWord {
    InflectedWord();

    InflectedWord(PartOfSpeech pPartOfSpeech, std::unique_ptr<Inflections> pInflections);

    InflectedWord(InflectedWord&& pOther);
    InflectedWord& operator=(InflectedWord&& pOther);

    InflectedWord(const InflectedWord& pOther);
    InflectedWord& operator=(const InflectedWord& pOther);

    void clear(bool pExceptFlexions = false);
    void moveInflections(std::unique_ptr<Inflections> pInflections);

    static InflectedWord getPuntuationIGram();

    SemanticWord word;

    WordAssociatedInfos infos;

    const Inflections& inflections() const { return *_inflections; }
    Inflections& inflections() { return *_inflections; }

    bool isSameInflectedFormThan(const InflectedWord& pOther) const;

private:
    std::unique_ptr<Inflections> _inflections;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#include "detail/inflectedword.hxx"

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_LINGINFOSGRAM_HPP
