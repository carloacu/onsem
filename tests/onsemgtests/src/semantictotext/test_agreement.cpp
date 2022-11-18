#include "../semanticreasonergtests.hpp"
#include <onsem/semantictotext/tool/semexpagreementdetector.hpp>
#include "../util/util.hpp"

using namespace onsem;


namespace
{
TruenessValue _getAgreement(const std::string& pText,
                           const linguistics::LinguisticDatabase& pLingDb,
                            SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN)
{
  return semExpAgreementDetector::semExpToAgreementValue(*textToSemExp(pText, pLingDb, pLanguage));
}
}


TEST_F(SemanticReasonerGTests, test_agreement)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  // true
  ONSEM_TRUE(_getAgreement("yes", lingDb));
  ONSEM_TRUE(_getAgreement("yep", lingDb));
  ONSEM_TRUE(_getAgreement("Yeah", lingDb));
  ONSEM_TRUE(_getAgreement("Yeah, I will go shopping", lingDb));
  ONSEM_TRUE(_getAgreement("I agree", lingDb));
  ONSEM_TRUE(_getAgreement("I am agree", lingDb));
  ONSEM_TRUE(_getAgreement("oui", lingDb));
  ONSEM_TRUE(_getAgreement("ouep", lingDb));
  ONSEM_TRUE(_getAgreement("Ouai", lingDb));
  ONSEM_TRUE(_getAgreement("ouaip", lingDb));
  ONSEM_TRUE(_getAgreement("ouais", lingDb));
  ONSEM_TRUE(_getAgreement("wouais", lingDb));
  ONSEM_TRUE(_getAgreement("Allez oui", lingDb));
  ONSEM_TRUE(_getAgreement("c'est ça", lingDb));
  ONSEM_TRUE(_getAgreement("C'est ça", lingDb));
  ONSEM_TRUE(_getAgreement("c'est bien ça", lingDb));
  ONSEM_TRUE(_getAgreement("c'est bien  ça", lingDb));
  ONSEM_TRUE(_getAgreement("ok", lingDb));
  ONSEM_TRUE(_getAgreement("of course", lingDb));
  ONSEM_TRUE(_getAgreement("bien sûr", lingDb));
  ONSEM_TRUE(_getAgreement("bien sur", lingDb));
  ONSEM_TRUE(_getAgreement("je suis d'accord", lingDb));

  // false
  ONSEM_FALSE(_getAgreement("no", lingDb));
  ONSEM_FALSE(_getAgreement("non", lingDb));
  ONSEM_FALSE(_getAgreement("nn", lingDb));
  ONSEM_FALSE(_getAgreement("nope", lingDb));
  ONSEM_FALSE(_getAgreement("no of course", lingDb));
  ONSEM_FALSE(_getAgreement("of course not", lingDb));
  ONSEM_FALSE(_getAgreement("non bien sûr", lingDb));
  ONSEM_FALSE(_getAgreement("pas encore", lingDb, SemanticLanguageEnum::FRENCH));
  ONSEM_FALSE(_getAgreement("bien sûr que non", lingDb));

  // unknown
  ONSEM_UNKNOWN(_getAgreement("animal", lingDb));
  ONSEM_UNKNOWN(_getAgreement("hello", lingDb));
  ONSEM_UNKNOWN(_getAgreement("I am happy", lingDb));
}

