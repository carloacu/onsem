#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERFROMDATASTOREDINSIDETHEQUESTION_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERFROMDATASTOREDINSIDETHEQUESTION_HPP

#include <map>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include "../../type/semanticdetailledanswer.hpp"

namespace onsem
{
namespace answerFromDataStoredInsideTheQuestion
{


void getAnswers(std::map<QuestionAskedInformation, AllAnswerElts> &pAllAnswers,
                const GroundedExpression& pGrdExp,
                SemanticRequestType pRootRequest,
                const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace answerFromDataStoredInsideTheQuestion
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERFROMDATASTOREDINSIDETHEQUESTION_HPP
