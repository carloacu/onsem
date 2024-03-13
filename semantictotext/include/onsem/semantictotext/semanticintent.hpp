#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICINTENT_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICINTENT_HPP

#include <string>
#include <map>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include "api.hpp"

namespace onsem {
struct SemanticMemoryBlock;

#define SEMANTIC_TYPEOFINTENT_TABLE                   \
    ADD_SEMANTIC_TYPEOFINTENT(GREETINGS, "Greetings") \
    ADD_SEMANTIC_TYPEOFINTENT(FAREWELL, "Farewell")

#define ADD_SEMANTIC_TYPEOFINTENT(a, b) a,
enum class TypeOfIntent { SEMANTIC_TYPEOFINTENT_TABLE };
#undef ADD_SEMANTIC_TYPEOFINTENT

struct ONSEMSEMANTICTOTEXT_API SemanticIntent {
    std::string name() const;
    inline const std::map<std::string, std::string>& entities() const;

protected:
    TypeOfIntent _typeOfIntent;
    std::map<std::string, std::string> _entities;

    SemanticIntent(const TypeOfIntent& pTypeOfIntent)
        : _typeOfIntent(pTypeOfIntent)
        , _entities() {}
};

const std::map<std::string, std::string>& SemanticIntent::entities() const {
    return _entities;
}

ONSEMSEMANTICTOTEXT_API
void extractIntents(std::list<SemanticIntent>& pRes, const SemanticExpression& pSemExp);

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SEMANTICINTENT_HPP
