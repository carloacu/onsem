#include "tabletfallback.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>

namespace onsem {

TabletFallback::TabletFallback()
    : ExternalFallback() {}

void TabletFallback::addFallback(UniqueSemanticExpression& pSemExp,
                                 const std::string& pUserId,
                                 const GroundedExpression&) const {
    if (pUserId == SemanticAgentGrounding::getUserNotIdentified()) {
        return;
    }

    auto rootGrdExp = std::make_unique<GroundedExpression>([]() {
        // verb
        auto statementGrd = std::make_unique<SemanticStatementGrounding>();
        statementGrd->verbTense = SemanticVerbTense::PUNCTUALPRESENT;
        statementGrd->verbGoal = VerbGoalEnum::ABILITY;
        statementGrd->concepts.emplace("verb_action_use", 4);
        return statementGrd;
    }());

    rootGrdExp->children.emplace(
        GrammaticalType::SUBJECT,
        std::make_unique<GroundedExpression>(std::make_unique<SemanticAgentGrounding>(pUserId)));

    rootGrdExp->children.emplace(GrammaticalType::OBJECT, []() {
        auto tabletGrdExp = std::make_unique<GroundedExpression>([]() {
            auto tablettGrd = std::make_unique<SemanticGenericGrounding>();
            tablettGrd->concepts.emplace("tablet", 4);
            return tablettGrd;
        }());

        tabletGrdExp->children.emplace(
            GrammaticalType::OWNER, std::make_unique<GroundedExpression>(SemanticAgentGrounding::getRobotAgentPtr()));
        return tabletGrdExp;
    }());

    rootGrdExp->children.emplace(GrammaticalType::PURPOSE, ([]() {
                                     auto findStatement = std::make_unique<GroundedExpression>([]() {
                                         auto statementGrd = std::make_unique<SemanticStatementGrounding>();
                                         statementGrd->concepts.emplace("verb_find", 4);
                                         return statementGrd;
                                     }());

                                     findStatement->children.emplace(
                                         GrammaticalType::OBJECT, std::make_unique<GroundedExpression>([]() {
                                             auto statementGrd = std::make_unique<SemanticGenericGrounding>();
                                             statementGrd->referenceType = SemanticReferenceType::DEFINITE;
                                             statementGrd->entityType = SemanticEntityType::THING;
                                             statementGrd->concepts.emplace("answer", 4);
                                             return statementGrd;
                                         }()));
                                     return findStatement;
                                 }()));

    SemExpModifier::addNewSemExp(pSemExp, std::move(rootGrdExp));
}

}    // end namespace onsem
