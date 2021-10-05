#include <onsem/semantictotext/sentiment/sentimentanalyzer.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include "../interpretation/completewithcontext.hpp"


namespace onsem
{

SentimentAnalyzer::SentimentAnalyzer(const linguistics::LinguisticDatabase& pLingDb)
  : _semMem(),
    _lingDb(pLingDb),
    _prevSemExp()
{
}

void SentimentAnalyzer::extract
(std::list<std::unique_ptr<SentimentContext>>& pSentContexts,
 UniqueSemanticExpression pSemExp,
 const SemanticAgentGrounding& pAuthor)
{
  const SemanticExpression* semExpAut1 = nullptr;
  const SemanticExpression* semExpAut2 = nullptr;
  bool sameAuthor = false;
  if (semExpAut1 != nullptr && semExpAut2 != nullptr)
  {
    semExpAut1 = SemExpGetter::extractAuthorSemExp(*semExpAut1);
    semExpAut2 = SemExpGetter::extractAuthorSemExp(*semExpAut2);
    sameAuthor = SemExpComparator::semExpsAreEqualFromMemBlock(*semExpAut1, *semExpAut2, _semMem.memBloc, _lingDb, nullptr);
  }

  completeWithContext(pSemExp, GrammaticalType::UNKNOWN, *_prevSemExp, sameAuthor, semExpAut1, _semMem.memBloc, _lingDb);
  sentimentDetector::semExpToSentimentInfos(pSentContexts, *pSemExp, pAuthor, _lingDb.conceptSet);
  _prevSemExp = std::move(pSemExp);
}


void SentimentAnalyzer::inform(UniqueSemanticExpression pSemExp)
{
  _prevSemExp = std::move(pSemExp);
}


void SentimentAnalyzer::clear()
{
  _prevSemExp.clear();
}


} // End of namespace onsem
