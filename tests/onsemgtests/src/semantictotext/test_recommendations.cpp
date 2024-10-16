#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"
#include <onsem/semantictotext/recommendations.hpp>
#include <onsem/tester/sentencesloader.hpp>

using namespace onsem;

namespace {

void _groundingsCoef(SemanticGroundingsToRecommendationCoef& pGrdContainer,
                     const std::string& pText,
                     int pCoef,
                     const linguistics::LinguisticDatabase& pLingDb,
                     SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN) {
    addGroundingCoef(pGrdContainer, textToSemExp(pText, pLingDb, pLanguage), pCoef, pLingDb);
}

void _addRecommendationPossibility(SemanticRecommendationsContainer& pContainer,
                                   const std::vector<std::string>& pTexts,
                                   const linguistics::LinguisticDatabase& pLingDb) {
    for (const auto& currText : pTexts)
        addARecommendation(pContainer, textToSemExp(currText, pLingDb), currText, pLingDb);
}

std::string _getRecommendations(const std::string& pText,
                                const SemanticRecommendationsContainer& pContainer,
                                const linguistics::LinguisticDatabase& pLingDb,
                                SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN,
                                std::size_t pMaxNbOfRecommendationsToAdd = 10,
                                const std::set<std::string>& pForbiddenRecommendations = {}) {
    std::map<int, std::set<std::string>> recommendations;
    auto semExp = textToContextualSemExp(pText, pLingDb, pLanguage);
    getRecommendations(
        recommendations, pMaxNbOfRecommendationsToAdd, *semExp, pContainer, pLingDb, pForbiddenRecommendations);
    std::string res;
    std::size_t nbOfIterations = 0;
    for (auto itRecommendations = recommendations.rbegin(); itRecommendations != recommendations.rend();
         ++itRecommendations) {
        for (const auto& currRec : itRecommendations->second) {
            if (nbOfIterations > 5)
                break;
            if (nbOfIterations > 0)
                res += " | ";
            res += currRec;
            ++nbOfIterations;
        }
        if (nbOfIterations > 5)
            break;
    }
    return res;
}

}

TEST_F(SemanticReasonerGTests, test_recommendations) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticRecommendationsContainer recommendationContainer;
    const std::vector<std::string> recommendations{"Où sont les toilettes ?",
                                                   "Combien coûte le téléphone Xperia S5 ?",
                                                   "Combien coûte le téléphone samsoung D4 ?",
                                                   "Où est l'iphone ?",
                                                   "Parle plus fort",
                                                   "Est-ce que N5 est un robot de Short Circuit ?",
                                                   "Qui est N7 ?",
                                                   "Comment être grand ?",
                                                   "Qu'est-ce que l'ascension ?",
                                                   "C'est un homme",
                                                   "Qu'est-ce que la Toussaint ?"};
    _addRecommendationPossibility(recommendationContainer, recommendations, lingDb);

    EXPECT_EQ("Où est l'iphone ?", _getRecommendations("Je veux un iphone", recommendationContainer, lingDb));
    EXPECT_EQ("Combien coûte le téléphone Xperia S5 ?",
              _getRecommendations("Je veux un Xperia S5", recommendationContainer, lingDb));
    EXPECT_EQ("Combien coûte le téléphone Xperia S5 ?",
              _getRecommendations("Je veux un téléphone Xperia S5", recommendationContainer, lingDb));
    EXPECT_EQ("Parle plus fort", _getRecommendations("Plus fort", recommendationContainer, lingDb));
    EXPECT_EQ("Est-ce que N5 est un robot de Short Circuit ?",
              _getRecommendations("N5", recommendationContainer, lingDb));
    EXPECT_EQ("Est-ce que N5 est un robot de Short Circuit ? | Combien coûte le téléphone Xperia S5 ?",
              _getRecommendations("Combien coûte N5 ?", recommendationContainer, lingDb));
    EXPECT_EQ(
        "Qui est N7 ? | C'est un homme | Comment être grand ?",
        _getRecommendations("N7 est un grand homme", recommendationContainer, lingDb, SemanticLanguageEnum::FRENCH));
    // Test the size limit of recommendations
    EXPECT_EQ(
        "Qui est N7 ? | C'est un homme",
        _getRecommendations("N7 est un grand homme", recommendationContainer, lingDb, SemanticLanguageEnum::FRENCH, 2));
    // Test to forbid some recommendations
    EXPECT_EQ("C'est un homme",
              _getRecommendations("N7 est un grand homme",
                                  recommendationContainer,
                                  lingDb,
                                  SemanticLanguageEnum::FRENCH,
                                  10,
                                  {"Comment être grand ?", "Qui est N7 ?"}));
    EXPECT_EQ("Qu'est-ce que l'ascension ?", _getRecommendations("Ascension", recommendationContainer, lingDb));
    EXPECT_EQ("Qu'est-ce que l'ascension ?",
              _getRecommendations("Qu'est-ce que l'Ascension ?", recommendationContainer, lingDb));
    EXPECT_EQ("Qu'est-ce que la Toussaint ?",
              _getRecommendations("Qu'est-ce que la toussaint ?", recommendationContainer, lingDb));
}

TEST_F(SemanticReasonerGTests, test_recommendations_faq_carrefour) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticRecommendationsContainer recommendationContainer;

    SentencesLoader carrefourFaqLoader;
    carrefourFaqLoader.loadFile(_corpusInputFolder + "/" + frenchStr + "/french_carrefour_faq.txt");
    const std::vector<std::string> recommendations = carrefourFaqLoader.getSentences();
    _addRecommendationPossibility(recommendationContainer, recommendations, lingDb);
    _groundingsCoef(recommendationContainer.goundingsToCoef, "nous", 1, lingDb);
    _groundingsCoef(recommendationContainer.goundingsToCoef, "on", 1, lingDb, SemanticLanguageEnum::FRENCH);

    EXPECT_EQ("Comment payer en ligne ? | "
              "Peut-on payer par Carte PASS ? | "
              "Peut-on payer par chèque ? | "
              "Comment payer avec la Carte Pass ? | "
              "Peut-on payer par chèque à la livraison ?",
              _getRecommendations("Comment peut-on payer ?", recommendationContainer, lingDb));
}
