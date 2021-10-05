#ifndef SEMANTICANALYZER_ALSEMANTICHOWYOUKNOWTHATANSWERGETTER_H
#define SEMANTICANALYZER_ALSEMANTICHOWYOUKNOWTHATANSWERGETTER_H

#include <memory>

namespace onsem
{
struct LeafSemAnswer;

namespace howYouThatAnswer
{

void process(std::unique_ptr<LeafSemAnswer>& pLeafAnswer);


} // End of namespace howYouThatAnswer
} // End of namespace onsem


#endif // SEMANTICANALYZER_ALSEMANTICHOWYOUKNOWTHATANSWERGETTER_H
