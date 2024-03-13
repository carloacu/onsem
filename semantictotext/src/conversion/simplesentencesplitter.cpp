#include "simplesentencesplitter.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>

namespace onsem {
namespace splitter {

void _splitGrdExp(UniqueSemanticExpression& pRootSemExp,
                  GroundedExpression& pRootGrdExp,
                  GrammaticalType pChildType,
                  ListExpressionType pListType,
                  std::list<UniqueSemanticExpression>& pChildrenToPutInSepSentences) {
    if (pChildrenToPutInSepSentences.empty()) {
        assert(false);
        return;
    }
    auto newListExp = std::make_unique<ListExpression>(pListType);
    auto endIt = pChildrenToPutInSepSentences.end();
    auto beginIt = pChildrenToPutInSepSentences.begin();
    auto itElt = endIt;
    --itElt;
    while (true) {
        // if we are on the last elt we don't need to copy grdExp
        if (itElt == beginIt) {
            pRootGrdExp.children.emplace(pChildType, std::move(*itElt));
            newListExp->elts.emplace_front(std::move(pRootSemExp));
            break;
        }

        auto nextIt = itElt;
        --nextIt;
        auto newGrdExp = SemExpGetter::getASimplifiedVersionFromGrdExp(pRootGrdExp);
        newGrdExp->children.emplace(pChildType, std::move(*itElt));
        newListExp->elts.emplace_front(std::move(newGrdExp));
        itElt = nextIt;
    }
    pRootSemExp = std::move(newListExp);
}

void splitInVerySimpleSentences(UniqueSemanticExpression& pSemExp, bool pDoWeSplitQuestions) {
    bool needToReloop = false;
    switch (pSemExp->type) {
        case SemanticExpressionType::GROUNDED: {
            GroundedExpression& grdExp = pSemExp->getGrdExp();
            if (grdExp->type == SemanticGroundingType::STATEMENT) {
                if (SemExpGetter::isAnActionDefinition(grdExp) || SemExpGetter::isATeachingElement(grdExp)
                    || (!pDoWeSplitQuestions
                        && SemExpGetter::getMainRequestTypeFromGrdExp(grdExp) != SemanticRequestType::NOTHING))
                    return;
                for (const auto& currChild : grdExp.children)
                    if (SemExpGetter::isACoreference(*currChild.second, CoreferenceDirectionEnum::BEFORE))
                        return;

                for (auto itChild = grdExp.children.begin(); itChild != grdExp.children.end(); ++itChild) {
                    GrammaticalType childType = itChild->first;
                    ListExpression* subListExp = itChild->second->getListExpPtr();
                    if (subListExp != nullptr
                        && (subListExp->listType == ListExpressionType::AND
                            || subListExp->listType == ListExpressionType::UNRELATED)
                        && childType != GrammaticalType::SPECIFIER && childType != GrammaticalType::SIMILARITY
                        && childType != GrammaticalType::WITH) {
                        std::list<UniqueSemanticExpression> childrenToPutInSepSentences;
                        childrenToPutInSepSentences.splice(childrenToPutInSepSentences.begin(), subListExp->elts);
                        ListExpressionType listType = subListExp->listType;
                        grdExp.children.erase(itChild);
                        _splitGrdExp(pSemExp, grdExp, childType, listType, childrenToPutInSepSentences);
                        needToReloop = true;
                        break;
                    }
                }
            }
            break;
        }
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp->getListExp();
            for (auto& currElt : listExp.elts)
                splitInVerySimpleSentences(currElt, pDoWeSplitQuestions);
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            splitInVerySimpleSentences(pSemExp->getIntExp().interpretedExp, pDoWeSplitQuestions);
            break;
        }
        case SemanticExpressionType::FEEDBACK: {
            splitInVerySimpleSentences(pSemExp->getFdkExp().concernedExp, pDoWeSplitQuestions);
            break;
        }
        case SemanticExpressionType::ANNOTATED: {
            splitInVerySimpleSentences(pSemExp->getAnnExp().semExp, pDoWeSplitQuestions);
            break;
        }
        case SemanticExpressionType::METADATA: {
            splitInVerySimpleSentences(pSemExp->getMetadataExp().semExp, pDoWeSplitQuestions);
            break;
        }
        case SemanticExpressionType::CONDITION: {
            ConditionExpression& condExp = pSemExp->getCondExp();
            splitInVerySimpleSentences(condExp.conditionExp, pDoWeSplitQuestions);
            splitInVerySimpleSentences(condExp.thenExp, pDoWeSplitQuestions);
            if (condExp.elseExp) {
                splitInVerySimpleSentences(*condExp.elseExp, pDoWeSplitQuestions);
            }
            break;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::SETOFFORMS: break;
    }

    if (needToReloop) {
        splitInVerySimpleSentences(pSemExp, pDoWeSplitQuestions);
    }
}

}    // End of namespace splitter
}    // End of namespace onsem
