#ifndef ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICHOWYOUKNOWTHATANSWERGETTER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICHOWYOUKNOWTHATANSWERGETTER_HPP

#include <memory>

namespace onsem {
struct LeafSemAnswer;

namespace howYouThatAnswer {

void process(std::unique_ptr<LeafSemAnswer>& pLeafAnswer);

}    // End of namespace howYouThatAnswer
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICHOWYOUKNOWTHATANSWERGETTER_HPP
