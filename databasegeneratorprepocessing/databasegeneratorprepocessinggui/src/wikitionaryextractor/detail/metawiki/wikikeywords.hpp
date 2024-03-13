#ifndef METAWIKIKEYWORDS_H
#define METAWIKIKEYWORDS_H

#include <set>
#include <string>
#include <onsem/common/enum/partofspeech.hpp>

namespace onsem {

class WikiKeyWords {
public:
    virtual ~WikiKeyWords() {}

    virtual void getGramEnum(std::set<PartOfSpeech>& pRes,
                             const std::string& pGramStr,
                             const std::string& pLine) const = 0;
};

}    // End of namespace onsem

#endif    // METAWIKIKEYWORDS_H
