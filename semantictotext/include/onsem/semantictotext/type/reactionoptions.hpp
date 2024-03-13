#ifndef ONSEM_SEMANTICTOTEXT_TYPE_REACTIONOPTIONS_HPP
#define ONSEM_SEMANTICTOTEXT_TYPE_REACTIONOPTIONS_HPP

namespace onsem {

struct ReactionOptions {
    bool canAnswerIDontKnow = true;
    bool canDoAProactivity = true;
    bool canAnswerWithATrigger = true;
    bool canAnswerWithAllTheTriggers = false;
    bool canReactToANoun = false;
    bool canSayOkToAnAffirmation = true;
    bool canAnswerWithExternalEngines = true;
    bool reactWithTextAndResource = false;
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_TYPE_REACTIONOPTIONS_HPP
