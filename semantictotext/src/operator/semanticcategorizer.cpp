#include "semanticcategorizer.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>

namespace onsem {
namespace privateImplem {

SemanticExpressionCategory _categorizeRec(const SemanticExpression& pSemExp, const std::string& pAuthorUserId) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            const GroundedExpression& grdExp = pSemExp.getGrdExp();
            return categorizeGrdExp(grdExp, pAuthorUserId);
        }
        case SemanticExpressionType::CONDITION: {
            const ConditionExpression& conditionExp = pSemExp.getCondExp();
            auto condtionThenCategory = _categorizeRec(*conditionExp.thenExp, pAuthorUserId);
            if (condtionThenCategory == SemanticExpressionCategory::COMMAND)
                return SemanticExpressionCategory::CONDITIONTOCOMMAND;
            return SemanticExpressionCategory::CONDITION;
        }
        case SemanticExpressionType::COMPARISON: {
            const ComparisonExpression& comparisonExp = pSemExp.getCompExp();
            if (comparisonExp.request == SemanticRequestType::ACTION)
                return SemanticExpressionCategory::COMMAND;
            if (comparisonExp.request != SemanticRequestType::NOTHING)
                return SemanticExpressionCategory::QUESTION;
            return SemanticExpressionCategory::AFFIRMATION;
        }
        case SemanticExpressionType::LIST: {
            const ListExpression& listExp = pSemExp.getListExp();
            if (!listExp.elts.empty())
                return _categorizeRec(*listExp.elts.front(), pAuthorUserId);
            assert(false);
            return SemanticExpressionCategory::AFFIRMATION;
        }
        case SemanticExpressionType::SETOFFORMS: {
            const SetOfFormsExpression& setOfFormsExp = pSemExp.getSetOfFormsExp();
            const GroundedExpression* originalGrdExpForm = SemExpGetter::getOriginalGrdExpForm(setOfFormsExp);
            if (originalGrdExpForm != nullptr)
                return categorizeGrdExp(*originalGrdExpForm, pAuthorUserId);
            assert(false);
            return SemanticExpressionCategory::AFFIRMATION;
        }
        case SemanticExpressionType::INTERPRETATION: {
            return _categorizeRec(*pSemExp.getIntExp().interpretedExp, pAuthorUserId);
        }
        case SemanticExpressionType::FEEDBACK: {
            return _categorizeRec(*pSemExp.getFdkExp().concernedExp, pAuthorUserId);
        }
        case SemanticExpressionType::ANNOTATED: {
            return _categorizeRec(*pSemExp.getAnnExp().semExp, pAuthorUserId);
        }
        case SemanticExpressionType::METADATA: {
            const MetadataExpression& metadataExp = pSemExp.getMetadataExp();
            const SemanticAgentGrounding* authorPtr = metadataExp.getAuthorPtr();
            if (authorPtr != nullptr)
                return _categorizeRec(*metadataExp.semExp, authorPtr->userId);
            return _categorizeRec(*metadataExp.semExp, pAuthorUserId);
        }
        case SemanticExpressionType::COMMAND: {
            return SemanticExpressionCategory::NOMINALGROUP;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS: {
            return _categorizeRec(pSemExp.getFSynthExp().getSemExp(), pAuthorUserId);
        }
    }

    assert(false);
    return SemanticExpressionCategory::AFFIRMATION;
}

SemanticExpressionCategory categorizeGrdExp(const GroundedExpression& pGrdExp, const std::string& pAuthorUserId) {
    if (pGrdExp->type == SemanticGroundingType::STATEMENT) {
        const SemanticStatementGrounding& statementGrd = pGrdExp->getStatementGrounding();
        SemanticRequestType requestType = statementGrd.requests.firstOrNothing();
        if (requestType == SemanticRequestType::ACTION
            || SemExpGetter::getGrdExpToDo(pGrdExp, statementGrd, pAuthorUserId) != nullptr)
            return SemanticExpressionCategory::COMMAND;
        if (requestType != SemanticRequestType::NOTHING)
            return SemanticExpressionCategory::QUESTION;
        if (SemExpGetter::isAnActionDefinition(pGrdExp))
            return SemanticExpressionCategory::ACTIONDEFINITION;
        if (SemExpGetter::isAnExtractExternalTeaching(pGrdExp, pAuthorUserId))
            return SemanticExpressionCategory::EXTERNALTEACHING;
        return SemanticExpressionCategory::AFFIRMATION;
    }
    return SemanticExpressionCategory::NOMINALGROUP;
}

SemanticExpressionCategory categorize(const SemanticExpression& pSemExp) {
    return _categorizeRec(pSemExp, "");
}

}    // End of namespace privateImplem
}    // End of namespace onsem
