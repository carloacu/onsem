#ifndef ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_ANSWERIDONTKNOW_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_ANSWERIDONTKNOW_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>

namespace onsem {
namespace privateImplem {

mystd::unique_propagate_const<UniqueSemanticExpression> answerIDontKnow(const SemanticExpression& pSemExp,
                                                                        bool pForQuestions,
                                                                        bool pForActions);

}    // End of namespace privateImplem
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_ANSWERIDONTKNOW_HPP
