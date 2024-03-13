#include "utility.hpp"

namespace onsem {
namespace utility {
namespace {
void _removeAllFeedbacks(CompositeSemAnswer& pCompositeSemAnswer) {
    for (auto itDetAnsw = pCompositeSemAnswer.semAnswers.begin(); itDetAnsw != pCompositeSemAnswer.semAnswers.end();) {
        LeafSemAnswer* leafPtr = (*itDetAnsw)->getLeafPtr();
        if (leafPtr != nullptr) {
            if (leafPtr->type == ContextualAnnotation::FEEDBACK)
                itDetAnsw = pCompositeSemAnswer.semAnswers.erase(itDetAnsw);
            else
                ++itDetAnsw;
            continue;
        }
        CompositeSemAnswer* compPtr = (*itDetAnsw)->getCompositePtr();
        if (compPtr != nullptr)
            _removeAllFeedbacks(*compPtr);
        ++itDetAnsw;
    }
}
}

void keepOnlyLastFeedback(CompositeSemAnswer& pCompositeSemAnswer) {
    std::list<std::list<std::unique_ptr<SemAnswer>>::iterator> feedbackAnswers;
    for (auto itDetAnsw = pCompositeSemAnswer.semAnswers.begin(); itDetAnsw != pCompositeSemAnswer.semAnswers.end();) {
        LeafSemAnswer* leafPtr = (*itDetAnsw)->getLeafPtr();
        if (leafPtr != nullptr) {
            LeafSemAnswer& leafAnswer = *leafPtr;
            if (leafAnswer.type == ContextualAnnotation::FEEDBACK) {
                if (pCompositeSemAnswer.listType != ListExpressionType::UNRELATED
                    && itDetAnsw != --pCompositeSemAnswer.semAnswers.end()) {
                    itDetAnsw = pCompositeSemAnswer.semAnswers.erase(itDetAnsw);
                    continue;
                }
                feedbackAnswers.emplace_back(itDetAnsw);
            }
        } else {
            CompositeSemAnswer* compPtr = (*itDetAnsw)->getCompositePtr();
            if (compPtr != nullptr) {
                if (itDetAnsw != --pCompositeSemAnswer.semAnswers.end())
                    _removeAllFeedbacks(*compPtr);
                else
                    keepOnlyLastFeedback(*compPtr);
            }
        }
        ++itDetAnsw;
    }

    while (feedbackAnswers.size() > 1) {
        pCompositeSemAnswer.semAnswers.erase(feedbackAnswers.front());
        feedbackAnswers.pop_front();
    }
}

}    // End of namespace utility
}    // End of namespace onsem
