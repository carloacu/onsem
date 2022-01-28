#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <iostream>
#include <fstream>
#include <onsem/common/binary/binaryloader.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/inflections.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>


namespace onsem
{
namespace linguistics
{

StaticLinguisticDictionary::StaticLinguisticDictionary(std::istream& pDictIStream,
                                                       const StaticConceptSet& pStaticConceptSet,
                                                       SemanticLanguageEnum pLangEnum)
  : MetaWordTreeDb(),
    fConceptsDatabaseSingleton(pStaticConceptSet),
    fLangEnum(pLangEnum),
    fPtrMeaning(nullptr),
    fPtrSomeFlexions(nullptr),
    fIfSeparatorBetweenWords(true),
    fMemRules(nullptr),
    fQuestWordsBefore(),
    fQuestWordsAfter(),
    fQuestWordsThatCanBeAlone(),
    fQuestWordsSubordinate()
{
  xLoad(pDictIStream);
}

StaticLinguisticDictionary::~StaticLinguisticDictionary()
{
  xUnload();
}



int StaticLinguisticDictionary::getVersion
(const std::string& pFilename)
{
  std::ifstream binDatabaseFile(pFilename, std::ifstream::binary);
  if (!binDatabaseFile.is_open())
  {
    return -1;
  }
  DatabaseHeader header;
  binDatabaseFile.read(header.charValues, sizeof(DatabaseHeader));
  if (header.intValues[0] == fFormalism)
  {
    return header.intValues[1];
  }
  return -1;
}


void StaticLinguisticDictionary::xUnload()
{
  binaryloader::deallocMemZone(&fPtrMeaning);
  binaryloader::deallocMemZone(&fPtrSomeFlexions);
  binaryloader::deallocMemZone(&fPtrPatriciaTrie);
  binaryloader::deallocMemZoneU(&fMemRules);
  fIfSeparatorBetweenWords = true;
  fTotalSize = 0;
  fErrorMessage = "NOT_LOADED";
}


void StaticLinguisticDictionary::xLoad(std::istream& pDictIStream)
{
  assert(!xIsLoaded());

  /*
  if (!binDatabaseFile.is_open())
  {
    throw std::runtime_error("NOT_FOUND (" + pLingAnalFilename.string() + ")");
  }
  */

  // Read the 3 first int of the file, which correspond to:
  // -> Version of the database
  // -> Offset of the begin of the trie
  // -> Size between the begin of the trie and the end of the file
  DatabaseHeader header;
  pDictIStream.read(header.charValues, sizeof(DatabaseHeader));
  if (header.intValues[0] != fFormalism)
  {
    //binDatabaseFile.close();
    fErrorMessage = "BAD_FORMALISM";
    return;
  }
  std::size_t meaningsSize = static_cast<std::size_t>(header.intValues[2]);
  std::size_t someFlexionsSize = static_cast<std::size_t>(header.intValues[3]);
  std::size_t patriciaTrieSize = static_cast<std::size_t>(header.intValues[4]);

  fTotalSize = meaningsSize + someFlexionsSize + patriciaTrieSize;

  // Read if the language has to have a separator between words
  pDictIStream.read(reinterpret_cast<char*>(&fIfSeparatorBetweenWords),
                       sizeof(bool));

  // Read all the database and put it in RAM
  if (!binaryloader::allocMemZone(&fPtrMeaning, pDictIStream, meaningsSize) ||
      !binaryloader::allocMemZone(&fPtrSomeFlexions, pDictIStream, someFlexionsSize) ||
      !binaryloader::allocMemZone(&fPtrPatriciaTrie, pDictIStream, patriciaTrieSize))
  {
    //binDatabaseFile.close();
    xUnload();
    fErrorMessage = "BAD_ALLOC";
    return;
  }

  // read the rules memory
  int rulesSizeRead;
  pDictIStream.read(reinterpret_cast<char*>(&rulesSizeRead), sizeof(rulesSizeRead));
  std::size_t rulesSize = rulesSizeRead;
  if (!binaryloader::allocMemZoneU(&fMemRules, pDictIStream, rulesSize))
  {
    //binDatabaseFile.close();
    xUnload();
    fErrorMessage = "BAD_ALLOC";
    return;
  }

  unsigned char const* listSeparatorsPtr = fMemRules;
  xInitQuestionWords(&listSeparatorsPtr);

  // Close database file
  //binDatabaseFile.close();
  fErrorMessage = "";
}


const StaticLinguisticDictionary::QuestionWords* StaticLinguisticDictionary::_wordToQuestionWord
(const SemanticWord& pWord,
 const std::map<SemanticWord, std::vector<StaticLinguisticDictionary::QuestionWords>>& pQuestWords,
 bool pIsCloseToTheVerb) const
{
  auto itQW = pQuestWords.find(pWord);
  if (itQW != pQuestWords.end())
  {
    for (std::size_t i = 0; i < itQW->second.size(); ++i)
    {
      const StaticLinguisticDictionary::QuestionWords& res = itQW->second[i];
      if (!pIsCloseToTheVerb)
      {
        if (&pQuestWords == &fQuestWordsAfter)
        {
          if (res.canBeAfterVerb == WordToVerbRelation::ONLY_NEXT_TO_THE_VERB)
            return nullptr;
        }
        else if (&pQuestWords == &fQuestWordsBefore)
        {
          if (res.canBeBeforeVerb == WordToVerbRelation::ONLY_NEXT_TO_THE_VERB)
            return nullptr;
        }
      }
      return &res;
    }
  }
  return nullptr;
}


const StaticLinguisticDictionary::QuestionWords* StaticLinguisticDictionary::wordToQuestionWord
(const SemanticWord& pWord,
 bool pAfterVerb,
 bool pIsCloseToTheVerb) const
{
  if (pAfterVerb)
    return _wordToQuestionWord(pWord, fQuestWordsAfter, pIsCloseToTheVerb);
  return _wordToQuestionWord(pWord, fQuestWordsBefore, pIsCloseToTheVerb);
}

const StaticLinguisticDictionary::QuestionWords* StaticLinguisticDictionary::inflWordsToQuestionWord
(const std::list<InflectedWord>& pInflWords,
 bool pAfterVerb,
 bool pIsCloseToTheVerb) const
{
  const StaticLinguisticDictionary::QuestionWords* res = nullptr;
  for (const auto& currInflWord : pInflWords)
  {
    res = wordToQuestionWord(currInflWord.word, pAfterVerb, pIsCloseToTheVerb);
    if (res != nullptr)
      return res;
  }
  return res;
}

const StaticLinguisticDictionary::QuestionWords* StaticLinguisticDictionary::_aloneWordToQuestionWord
(const SemanticWord& pWord) const
{
  auto itQW = fQuestWordsThatCanBeAlone.find(pWord);
  if (itQW != fQuestWordsThatCanBeAlone.end())
    for (std::size_t i = 0; i < itQW->second.size(); ++i)
      return &itQW->second[i];
  return nullptr;
}

SemanticRequestType StaticLinguisticDictionary::wordToSubordinateRequest
(const SemanticWord& pWord) const
{
  auto qWord = _wordToQuestionWord(pWord, fQuestWordsSubordinate, true /* don't care cause it's not read here */);
  if (qWord != nullptr)
    return qWord->request;
  return SemanticRequestType::NOTHING;
}

SemanticRequestType StaticLinguisticDictionary::aloneWordToRequest
(const SemanticWord& pWord) const
{
  auto qWord = _aloneWordToQuestionWord(pWord);
  if (qWord != nullptr)
    return qWord->request;
  return SemanticRequestType::NOTHING;
}

SemanticRequestType StaticLinguisticDictionary::semWordToRequest
(const SemanticWord& pWord) const
{
  const QuestionWords* qWord = wordToQuestionWord(pWord, false, true);
  if (qWord != nullptr)
    return qWord->request;
  return SemanticRequestType::NOTHING;
}



void StaticLinguisticDictionary::xInitQuestionWords(unsigned char const** pChar)
{
  fQuestWordsBefore.clear();
  fQuestWordsAfter.clear();
  fQuestWordsThatCanBeAlone.clear();
  fQuestWordsSubordinate.clear();

  if (*pChar != nullptr)
  {
    unsigned char nbOFElts = *((*pChar)++);
    for (unsigned int i = 0; i < nbOFElts; ++i)
    {
      int meaning = (*(reinterpret_cast<int const*>(*pChar)));
      meaning *= 4;
      *pChar += sizeof(int);
      WordToVerbRelation canBeBeforeVerb = wordToVerbRelations_fromChar(*((*pChar)++));
      WordToVerbRelation canBeAfterVerb = wordToVerbRelations_fromChar(*((*pChar)++));
      bool canBeAlone = static_cast<bool>(*((*pChar)++));
      SemanticRequestType request = SemanticRequestType(*((*pChar)++));
      bool followedByRequestedWord = static_cast<bool>(*((*pChar)++));
      if (meaning != LinguisticMeaning_noMeaningId)
      {
        SemanticWord questionSemWord;
        xGetSemanticWord(questionSemWord, meaning);
        if (canBeAlone)
          fQuestWordsThatCanBeAlone[questionSemWord].
              emplace_back(canBeBeforeVerb, canBeAfterVerb, canBeAlone, request, followedByRequestedWord);
        if (canBeBeforeVerb == WordToVerbRelation::NO && canBeAfterVerb == WordToVerbRelation::NO)
        {
          fQuestWordsSubordinate[std::move(questionSemWord)].
              emplace_back(canBeBeforeVerb, canBeAfterVerb, canBeAlone, request, followedByRequestedWord);
        }
        else
        {
          if (canBeBeforeVerb != WordToVerbRelation::NO)
            fQuestWordsBefore[questionSemWord].
                emplace_back(canBeBeforeVerb, canBeAfterVerb, canBeAlone, request, followedByRequestedWord);
          if (canBeAfterVerb != WordToVerbRelation::NO)
            fQuestWordsAfter[std::move(questionSemWord)].
                emplace_back(canBeBeforeVerb, canBeAfterVerb, canBeAlone, request, followedByRequestedWord);
        }
      }
    }
  }
}


void StaticLinguisticDictionary::getGramPossibilitiesAndPutUnknownIfNothingFound
(std::list<InflectedWord>& pInfosGram,
 const std::string& pWord,
 std::size_t pBeginPos,
 std::size_t pSizeOfWord) const
{
  getGramPossibilities(pInfosGram, pWord, pBeginPos, pSizeOfWord);
  if (pInfosGram.empty())
  {
    pInfosGram.emplace_back();
  }
}


void StaticLinguisticDictionary::getGramPossibilities
(std::list<InflectedWord>& pInfosGram,
 const std::string& pWord,
 std::size_t pBeginPos,
 std::size_t pSizeOfWord) const
{
  assert(xIsLoaded());
  // get a ptr to the end of the word
  const auto* node = xGetNode(pWord, pBeginPos, pSizeOfWord, true);
  if (node != nullptr)
  {
    xGetGramPossFromNode(pInfosGram, node);
  }
}


std::size_t StaticLinguisticDictionary::getConfidenceOfWordInThatLanguage
(PartOfSpeech& pMainPartOfSpeech,
 const std::string& pWord,
 std::size_t pBeginPos,
 std::size_t pSizeOfWord) const
{
  assert(xIsLoaded());
  // get a ptr to the end of the word
  const auto* node = xGetNode(pWord, pBeginPos, pSizeOfWord, true);
  if (node != nullptr)
  {
    return xGetConfOfNode(pMainPartOfSpeech, node);
  }
  return 0;
}


std::size_t StaticLinguisticDictionary::xGetConfOfNode
(PartOfSpeech& pMainPartOfSpeech,
 const signed char* pNode) const
{
  // iterate over all meanings
  unsigned char nbMeanings = xNbMeanings(pNode);
  const int* mns = xGetMeaningList(pNode);
  std::size_t res = static_cast<std::size_t>(nbMeanings);
  {
    if (!xIsAWordFrom(mns))
    {
      return res; // because wordFroms are written before
    }
    int meaningId = xMeaningFromNodeToMeaningId(mns);
    const auto* meaningPtr = fPtrMeaning + meaningId;
    pMainPartOfSpeech = xGetPartOfSpeech(meaningPtr);
  }
  return res;
}


template<typename T>
void _initOneNomInflection(T& pNomInfl,
                           unsigned char pInflection)
{
  if ((pInflection & 0x03) == 0x01)
  {
    pNomInfl.gender = SemanticGenderType::MASCULINE;
  }
  else if ((pInflection & 0x03) == 0x02)
  {
    pNomInfl.gender = SemanticGenderType::FEMININE;
  }
  else if ((pInflection & 0x03) == 0x03)
  {
    pNomInfl.gender = SemanticGenderType::NEUTRAL;
  }
  if ((pInflection & 0x0C) == 0x04)
  {
    pNomInfl.number = SemanticNumberType::SINGULAR;
  }
  else if ((pInflection & 0x0C) == 0x08)
  {
    pNomInfl.number = SemanticNumberType::PLURAL;
  }
}



void _initNomInflections(std::list<NominalInflection>& pNomInfls,
                         const signed char* pFls,
                         unsigned char pNbFlexions)
{
  for (unsigned char i = 0; i < pNbFlexions; ++i, ++pFls)
  {
    pNomInfls.emplace_back();
    NominalInflection& nomInfl = pNomInfls.back();
    unsigned char currInflection = *pFls;
    _initOneNomInflection(nomInfl, currInflection);
  }
}

void _initAdjInflections(std::list<AdjectivalInflection>& pAdjInfls,
                         const signed char* pFls,
                         unsigned char pNbFlexions)
{
  for (unsigned char i = 0; i < pNbFlexions; ++i, ++pFls)
  {
    pAdjInfls.emplace_back();
    AdjectivalInflection& adjInfl = pAdjInfls.back();
    unsigned char currInflection = *pFls;

    _initOneNomInflection(adjInfl, currInflection);
    if ((currInflection & 0x30) == 0x10)
    {
      adjInfl.comparisonType = ComparisonType::COMPARATIVE;
    }
    else if ((currInflection & 0x30) == 0x20)
    {
      adjInfl.comparisonType = ComparisonType::SUPERLATIVE;
    }
  }
}

void _initRelPerson(RelativePerson& pRelPerson,
                    char pPerson,
                    char pNumber)
{
  switch (pPerson)
  {
  case '1':
  {
    pRelPerson = pNumber == 'p' ?
          RelativePerson::FIRST_PLUR : RelativePerson::FIRST_SING;
    break;
  }
  case '2':
  {
    pRelPerson = pNumber == 'p' ?
          RelativePerson::SECOND_PLUR : RelativePerson::SECOND_SING;
    break;
  }
  case '3':
  {
    pRelPerson = pNumber == 'p' ?
          RelativePerson::THIRD_PLUR : RelativePerson::THIRD_SING;
    break;
  }
  case '0':
  {
    if (pNumber != 'u')
      pRelPerson = pNumber == 'p' ?
            RelativePerson::THIRD_PLUR : RelativePerson::THIRD_SING;
    break;
  }
  }
}

void _initPronInflections(std::list<PronominalInflection>& pPronInfls,
                          const signed char* pFls,
                          unsigned char pNbFlexions)
{
  for (unsigned char i = 0; i < pNbFlexions; ++i, ++pFls)
  {
    pPronInfls.emplace_back();
    PronominalInflection& pronInfl = pPronInfls.back();
    unsigned char currInflection = *pFls;

    if ((currInflection & 0x03) == 0x01)
    {
      pronInfl.personWithoutNumber = RelativePersonWithoutNumber::FIRST;
    }
    else if ((currInflection & 0x03) == 0x02)
    {
      pronInfl.personWithoutNumber = RelativePersonWithoutNumber::SECOND;
    }
    else if ((currInflection & 0x03) == 0x03)
    {
      pronInfl.personWithoutNumber = RelativePersonWithoutNumber::THIRD;
    }

    if ((currInflection & 0x0C) == 0x04)
    {
      pronInfl.gender = SemanticGenderType::MASCULINE;
    }
    else if ((currInflection & 0x0C) == 0x08)
    {
      pronInfl.gender = SemanticGenderType::FEMININE;
    }
    else if ((currInflection & 0x0C) == 0x0C)
    {
      pronInfl.gender = SemanticGenderType::NEUTRAL;
    }

    if ((currInflection & 0x30) == 0x10)
    {
      pronInfl.number = SemanticNumberType::SINGULAR;
    }
    else if ((currInflection & 0x30) == 0x20)
    {
      pronInfl.number = SemanticNumberType::PLURAL;
    }
  }
}


void _initVerbInflections(std::list<VerbalInflection>& pVerbInfls,
                          const signed char* pFls,
                          unsigned char pNbFlexions)
{
  for (unsigned char i = 0; i < pNbFlexions; ++i, ++pFls)
  {
    pVerbInfls.emplace_back();
    VerbalInflection& verbInfl = pVerbInfls.back();
    unsigned char currInflection = *pFls;

    bool mascFemNeutre = false;
    bool pers = true;
    bool singPlur = true;
    if ((currInflection & 0x0F) == 0x00)
    {
      verbInfl.tense = LinguisticVerbTense::PRESENT_INDICATIVE;
    }
    else if ((currInflection & 0x0F) == 0x01)
    {
      verbInfl.tense = LinguisticVerbTense::IMPERFECT_INDICATIVE;
    }
    else if ((currInflection & 0x0F) == 0x02)
    {
      verbInfl.tense = LinguisticVerbTense::PRESENT_SUBJONCTIVE;
    }
    else if ((currInflection & 0x0F) == 0x03)
    {
      verbInfl.tense = LinguisticVerbTense::IMPERFECT_SUBJONCTIVE;
    }
    else if ((currInflection & 0x0F) == 0x04)
    {
      verbInfl.tense = LinguisticVerbTense::PRESENT_IMPERATIVE;
    }
    else if ((currInflection & 0x0F) == 0x05)
    {
      verbInfl.tense = LinguisticVerbTense::PRESENT_CONDITIONAL;
    }
    else if ((currInflection & 0x0F) == 0x06)
    {
      verbInfl.tense = LinguisticVerbTense::SIMPLE_PAST_INDICATIVE;
    }
    else if ((currInflection & 0x0F) == 0x07)
    {
      verbInfl.tense = LinguisticVerbTense::INFINITIVE;
      pers = false;
      singPlur = false;
    }
    else if ((currInflection & 0x0F) == 0x08)
    {
      verbInfl.tense = LinguisticVerbTense::PRESENT_PARTICIPLE;
      pers = false;
      singPlur = false;
    }
    else if ((currInflection & 0x0F) == 0x09)
    {
      verbInfl.tense = LinguisticVerbTense::PAST_PARTICIPLE;
      mascFemNeutre = true;
      pers = false;
    }
    else // 0x0A
    {
      verbInfl.tense = LinguisticVerbTense::FUTURE_INDICATIVE;
    }

    char person = '0';
    if (pers)
    {
      if ((currInflection & 0x30) == 0x10)
      {
        person = '1';
      }
      else if ((currInflection & 0x30) == 0x20)
      {
        person = '2';
      }
      else if ((currInflection & 0x30) == 0x30)
      {
        person = '3';
      }
    }

    if (mascFemNeutre)
    {
      if ((currInflection & 0x30) == 0x10)
      {
        verbInfl.gender = SemanticGenderType::MASCULINE;
      }
      else if ((currInflection & 0x30) == 0x20)
      {
        verbInfl.gender = SemanticGenderType::FEMININE;
      }
      else if ((currInflection & 0x30) == 0x30)
      {
        verbInfl.gender = SemanticGenderType::NEUTRAL;
      }
    }

    char number = 'u';
    if (singPlur)
    {
      if ((currInflection & 0xC0) == 0x40)
      {
        number = 's';
      }
      else if ((currInflection & 0xC0) == 0x80)
      {
        number = 'p';
      }
    }
    _initRelPerson(verbInfl.person, person, number);
  }
}


std::unique_ptr<Inflections> _getInflections(PartOfSpeech pPartOfSpeech,
                                             const signed char* pFls,
                                             unsigned char pNbFlexions)
{
  if (pNbFlexions == 0)
    return mystd::make_unique<EmptyInflections>();
  switch (inflectionType_fromPartOfSpeech(pPartOfSpeech))
  {
  case InflectionType::ADJECTIVAL:
  {
    auto inflections = mystd::make_unique<AdjectivalInflections>();
    AdjectivalInflections& adjInfls = inflections->getAdjectivalI();
    _initAdjInflections(adjInfls.inflections, pFls, pNbFlexions);
    return std::move(inflections);
  }
  case InflectionType::NOMINAL:
  {
    auto inflections = mystd::make_unique<NominalInflections>();
    NominalInflections& nomInfls = inflections->getNominalI();
    _initNomInflections(nomInfls.inflections, pFls, pNbFlexions);
    return std::move(inflections);
  }
  case InflectionType::VERBAL:
  {
    auto inflections = mystd::make_unique<VerbalInflections>();
    VerbalInflections& verbInfls = inflections->getVerbalI();
    _initVerbInflections(verbInfls.inflections, pFls, pNbFlexions);
    return std::move(inflections);
  }
  case InflectionType::PRONOMINAL:
  {
    auto inflections = mystd::make_unique<PronominalInflections>();
    PronominalInflections& pronInfls = inflections->getPronominalI();
    _initPronInflections(pronInfls.inflections, pFls, pNbFlexions);
    return std::move(inflections);
  }
  case InflectionType::EMPTY:
    return mystd::make_unique<EmptyInflections>();
  }
  return mystd::make_unique<EmptyInflections>();
}



void StaticLinguisticDictionary::xGetGramPossFromNode
(std::list<InflectedWord>& pInfosGram,
 const signed char* pNode) const
{
  // iterate over all meanings
  unsigned char nbMeanings = xNbMeanings(pNode);
  const int32_t* mns = xGetMeaningList(pNode);
  for (unsigned char i = 0; i < nbMeanings; ++i)
  {
    if (!xIsAWordFrom(mns))
    {
      return; // because wordFroms are written before
    }
    int32_t meaningId = xMeaningFromNodeToMeaningId(mns);
    pInfosGram.emplace_back();
    InflectedWord& infosGramBack = pInfosGram.back();
    xFillIGram(infosGramBack, meaningId);

    const auto* nbFlexionsPtr = xGetNbFlexionsPtr(mns);
    unsigned char nbFlexions = *nbFlexionsPtr;
    const auto* fls = nbFlexionsPtr + 1;
    infosGramBack.moveInflections(_getInflections(infosGramBack.word.partOfSpeech,
                                                   fls, nbFlexions));
    mns = xGetNextMeaningFromNode(mns);
  }
}



void StaticLinguisticDictionary::xFillIGram
(InflectedWord& pIGram,
 int32_t pMeaningId) const
{
  xGetSemanticWord(pIGram.word, pMeaningId);
  xFillWordAssInfos(pIGram.infos, pMeaningId);
}

void StaticLinguisticDictionary::xFillWordAssInfos
(WordAssociatedInfos& pInfos,
 int pMeaningId) const
{
  const signed char* meaningPtr = fPtrMeaning + pMeaningId;
  xGetConcepts(pInfos.concepts, meaningPtr);
  xGetWordContextualInfos(pInfos.contextualInfos, meaningPtr);
  xTryToFillMeaningGroup(pInfos.linkedMeanings, pMeaningId);
  xGetMetaMeanings(pInfos.metaMeanings, meaningPtr);
}


std::size_t StaticLinguisticDictionary::getLengthOfLongestWord
(const std::string& pStr, std::size_t pBeginStr) const
{
  assert(xIsLoaded());
  std::size_t pos;
  xSearchInPatriciaTrie(pos, pStr, pBeginStr, pStr.size() - pBeginStr, true,
                        fIfSeparatorBetweenWords ? SearchForLongestWordMode::ENBABLED_BUTWITHSEPARATORAFTER : SearchForLongestWordMode::ENABLED);
  return pos;
}


StaticLinguisticMeaning StaticLinguisticDictionary::getLingMeaning
(const std::string& pWord,
 PartOfSpeech pGram,
 bool pWordIsALemma) const
{
  assert(xIsLoaded());
  // search a simple meaning
  {
    const signed char* node = xGetNode(pWord, 0, pWord.size(), false);
    if (node != nullptr)
    {
      unsigned char nbMeanings = xNbMeanings(node);
      const int* mns = xGetMeaningList(node);
      for (unsigned char i = 0; i < nbMeanings; ++i)
      {
        int meaningId = xMeaningFromNodeToMeaningId(mns);
        const signed char* meaningPtr = fPtrMeaning + meaningId;
        if (PartOfSpeech(xGetPartOfSpeech(meaningPtr)) == pGram &&
            (!pWordIsALemma || xGetLemma(meaningId, false) == pWord))
        {
          return {fLangEnum, meaningId};
        }
        mns = xGetNextMeaningFromNode(mns);
      }
    }
  }

  // search a gathering meaning
  if (pWordIsALemma)
  {
    std::size_t sepPos = pWord.find_first_of('~');
    if (sepPos != std::string::npos)
    {
      const std::string rootLemmaStr = pWord.substr(0, sepPos);
      const signed char* rootWordNode = xGetNode(rootLemmaStr, 0, rootLemmaStr.size(), false);
      if (rootWordNode == nullptr)
      {
        return StaticLinguisticMeaning();
      }

      unsigned char nbMeanings = xNbMeanings(rootWordNode);
      const int* mns = xGetMeaningList(rootWordNode);
      for (unsigned char i = 0; i < nbMeanings; ++i)
      {
        int meaningId = xMeaningFromNodeToMeaningId(mns);
        const signed char* meaningPtr = fPtrMeaning + meaningId;
        char nbOfGatheringMeanings = xGetNbGatheringMeanings(meaningPtr);
        if (nbOfGatheringMeanings > 0)
        {
          const int* gatheringMeaning = xGetFirstGatheringMeaning(meaningPtr);
          for (char j = 0; j < nbOfGatheringMeanings; ++j)
          {
            int gatherMeaningId = xMeaningFromNodeToMeaningId(gatheringMeaning);
            if (pWord == xGetLemma(gatherMeaningId, true))
            {
              return {fLangEnum, gatherMeaningId};
            }
            gatheringMeaning = xGetNextGatheringMeaning(gatheringMeaning);
          }
        }
        mns = xGetNextMeaningFromNode(mns);
      }
    }
  }
  return StaticLinguisticMeaning();
}


void StaticLinguisticDictionary::getLemma
(std::string& pLemmaStr,
 const std::string& pWord) const
{
  assert(xIsLoaded());
  // search a simple meaning
  auto* node = xGetNode(pWord, 0, pWord.size(), false);
  if (node != nullptr &&
      xNbMeanings(node) > 0)
  {
     xGetWord(pLemmaStr,
              xGetLemmeNodeId(fPtrMeaning +
                              xMeaningFromNodeToMeaningId(xGetMeaningList(node))));
  }
}


std::string StaticLinguisticDictionary::getLemma
(const StaticLinguisticMeaning& pMeaning,
 bool pWithLinkMeanings) const
{
  assert(xIsLoaded());
  assert(pMeaning.language == fLangEnum);
  return xGetLemma(pMeaning.meaningId, pWithLinkMeanings);
}


void StaticLinguisticDictionary::getWord
(SemanticWord& pWord,
 const StaticLinguisticMeaning& pMeaning) const
{
  assert(xIsLoaded());
  assert(pMeaning.language == fLangEnum);
  xGetSemanticWord(pWord, pMeaning.meaningId);
}


void StaticLinguisticDictionary::getSemanticWord
(SemanticWord& pWord,
 int pMeaningId) const
{
  assert(xIsLoaded());
  xGetSemanticWord(pWord, pMeaningId);
}


void StaticLinguisticDictionary::xGetSemanticWord
(SemanticWord& pWord,
 int32_t pMeaningId) const
{
  pWord.language = fLangEnum;
  if (pMeaningId != LinguisticMeaning_noMeaningId)
  {
    pWord.lemma = xGetLemma(pMeaningId, true);
    auto* meaningPtr = fPtrMeaning + pMeaningId;
    pWord.partOfSpeech = xGetPartOfSpeech(meaningPtr);
  }
}


void StaticLinguisticDictionary::xGetMetaMeanings
(std::list<LingWordsGroup>& pMetaMeanings,
 const signed char* pMeaningPtr) const
{
  char nbGatheringMeanings = xGetNbGatheringMeanings(pMeaningPtr);
  if (nbGatheringMeanings > 0)
  {
    const int* currGatheringMeaning = xGetFirstGatheringMeaning(pMeaningPtr);
    for (unsigned char i = 0; i < nbGatheringMeanings; ++i)
    {
      int rootMeaningId = binaryloader::alignedDecToInt(*currGatheringMeaning);
      auto rootWord = mystd::make_unique<SemanticWord>();
      xGetSemanticWord(*rootWord, rootMeaningId);
      LingWordsGroup meaningGroup(std::move(rootWord));
      xFillMeaningGroup(meaningGroup, rootMeaningId);
      pMetaMeanings.emplace_back(meaningGroup);
      currGatheringMeaning = xGetNextGatheringMeaning(currGatheringMeaning);
    }
  }
}


void StaticLinguisticDictionary::xFillMeaningGroup
(LingWordsGroup& pMeaningGroup,
 int pRootMeaningId) const
{
  auto* gatheringMeaningPtr = fPtrMeaning + pRootMeaningId;
  char nbLinkedMeanings = xNbLinkedMeanings(gatheringMeaningPtr);
  const int* currLinkedMeaning = xGetFirstLinkedMeaning(gatheringMeaningPtr);
  for (unsigned char i = 0; i < nbLinkedMeanings; ++i)
  {
    auto rootWord = mystd::make_unique<SemanticWord>();
    xGetSemanticWord(*rootWord, binaryloader::alignedDecToInt(*currLinkedMeaning));
    pMeaningGroup.linkedMeanings.emplace_back
        (std::move(rootWord),
         LinkedMeaningDirection(xGetCharAfterAlignedDec(*currLinkedMeaning)));
    currLinkedMeaning = xGetNextLinkedMeaning(currLinkedMeaning);
  }
}


void StaticLinguisticDictionary::xTryToFillMeaningGroup
(mystd::optional<LingWordsGroup>& pLinkedMeanings,
 int pMeaningId) const
{
  auto* meaningPtr = fPtrMeaning + pMeaningId;
  if (xIsAGatheringMeaning(meaningPtr))
  {
    auto rootWord = mystd::make_unique<SemanticWord>();
    xGetSemanticWord(*rootWord, pMeaningId);
    pLinkedMeanings = LingWordsGroup(std::move(rootWord));
    xFillMeaningGroup(*pLinkedMeanings, pMeaningId);
  }
}



StaticLinguisticMeaning StaticLinguisticDictionary::getRootMeaning
(const StaticLinguisticMeaning& pMeaning) const
{
  assert(xIsLoaded());
  assert(pMeaning.language == fLangEnum);
  if (pMeaning.meaningId == LinguisticMeaning_noMeaningId)
    return StaticLinguisticMeaning();

  auto* meaningPtr = fPtrMeaning + pMeaning.meaningId;
  if (xIsAGatheringMeaning(meaningPtr))
  {
    auto* lemmaNodePtr = fPtrPatriciaTrie + xGetLemmeNodeId(meaningPtr);
    unsigned char nbMeanings = xNbMeanings(lemmaNodePtr);
    if (nbMeanings > 0)
    {
      const int* mns = xGetMeaningList(lemmaNodePtr);
      for (unsigned char i = 0; i < nbMeanings; ++i)
      {
        int rootMeaningId = xMeaningFromNodeToMeaningId(mns);
        auto* rootMeaningPtr = fPtrMeaning + rootMeaningId;

        char nbGatheringMeanings = xGetNbGatheringMeanings(rootMeaningPtr);
        if (nbGatheringMeanings > 0)
        {
          const int* currGatheringMeaning = xGetFirstGatheringMeaning(rootMeaningPtr);
          for (unsigned char j = 0; j < nbMeanings; ++j)
          {
            if (binaryloader::alignedDecToInt(*currGatheringMeaning) == pMeaning.meaningId)
            {
              return StaticLinguisticMeaning{fLangEnum, rootMeaningId};
            }
            currGatheringMeaning = xGetNextGatheringMeaning(currGatheringMeaning);
          }
        }
        mns = xGetNextMeaningFromNode(mns);
      }
    }
  }
  return pMeaning;
}


void StaticLinguisticDictionary::getInfoGram
(InflectedWord& pIGram,
 const StaticLinguisticMeaning& pMeaning) const
{
  assert(xIsLoaded());
  if (pMeaning.meaningId == LinguisticMeaning_noMeaningId)
  {
    pIGram.clear();
    return;
  }
  assert(pMeaning.language == fLangEnum);

  pIGram.clear(true);
  xFillIGram(pIGram, pMeaning.meaningId);
}


void StaticLinguisticDictionary::getConcepts
(std::map<std::string, char>& pConcepts,
 const StaticLinguisticMeaning& pMeaning) const
{
  assert(xIsLoaded());
  if (pMeaning.meaningId == LinguisticMeaning_noMeaningId)
    return;
  assert(pMeaning.language == fLangEnum);
  auto* meaningPtr = fPtrMeaning + pMeaning.meaningId;
  xGetConcepts(pConcepts, meaningPtr);
}



bool StaticLinguisticDictionary::isAGatheringMeaning
(const StaticLinguisticMeaning& pMeaning) const
{
  assert(xIsLoaded());
  assert(pMeaning.language == fLangEnum);
  if (pMeaning.meaningId == LinguisticMeaning_noMeaningId)
    return false;
  return xIsAGatheringMeaning(fPtrMeaning + pMeaning.meaningId);
}


bool StaticLinguisticDictionary::hasContextualInfo
(WordContextualInfos pContextualInfo,
 const StaticLinguisticMeaning& pMeaning) const
{
  assert(xIsLoaded());
  if (pMeaning.meaningId == LinguisticMeaning_noMeaningId)
    return false;
  bool languageIsOk = pMeaning.language == fLangEnum;
  assert(languageIsOk);
  if (!languageIsOk)
    return false;

  auto* meaningPtr = fPtrMeaning + pMeaning.meaningId;
  return xWordHasContextualInfos(pContextualInfo, meaningPtr);
}


void StaticLinguisticDictionary::xGetWordContextualInfos
(std::set<WordContextualInfos>& pContextualInfos,
 const signed char* pMeaningPtr) const
{
  char nbContextInfos = xNbContextInfos(pMeaningPtr);
  auto* contInfoPtr = xGetFirstContextInfo(pMeaningPtr);
  for (char j = 0; j < nbContextInfos; ++j)
  {
    pContextualInfos.insert(WordContextualInfos(*contInfoPtr));
    contInfoPtr = xNextContextInfo(contInfoPtr);
  }
}

bool StaticLinguisticDictionary::xWordHasContextualInfos
(WordContextualInfos pContextualInfos,
 const signed char* pMeaningPtr) const
{
  char nbContextInfos = xNbContextInfos(pMeaningPtr);
  auto* contInfoPtr = xGetFirstContextInfo(pMeaningPtr);
  for (char j = 0; j < nbContextInfos; ++j)
  {
    if (pContextualInfos == WordContextualInfos(*contInfoPtr))
      return true;
    contInfoPtr = xNextContextInfo(contInfoPtr);
  }
  return false;
}

void StaticLinguisticDictionary::xGetConcepts
(std::map<std::string, char>& pConcepts,
 const signed char* pMeaningPtr) const
{
  const int* currConcept = xGetFirstConcept(pMeaningPtr);
  char nbConcepts = xNbConcepts(pMeaningPtr);
  for (char i = 0; i < nbConcepts; ++i)
  {
    pConcepts.emplace(fConceptsDatabaseSingleton.conceptName(binaryloader::alignedDecToInt(*currConcept)),
                      xGetRelationToConcept(*currConcept));
    currConcept = xGetNextConcept(currConcept);
  }
}


void StaticLinguisticDictionary::putRootMeaning(InflectedWord& pInfosGram) const
{
  StaticLinguisticMeaning lingMeaning = getLingMeaning(pInfosGram.word.lemma,
                                                 pInfosGram.word.partOfSpeech, true);
  StaticLinguisticMeaning rootLingMeaning = getRootMeaning(lingMeaning);
  if (rootLingMeaning.meaningId != LinguisticMeaning_noMeaningId)
  {
    pInfosGram.clear(true);
    xFillIGram(pInfosGram, rootLingMeaning.meaningId);
  }
}


bool StaticLinguisticDictionary::getConjugaisionsId
(int& pConjId,
 const StaticLinguisticMeaning& pMeaning) const
{
  assert(xIsLoaded());
  assert(pMeaning.language == fLangEnum);
  if (pMeaning.meaningId == LinguisticMeaning_noMeaningId)
    return false;
  pConjId = xGetConjugaisonId(fPtrMeaning + pMeaning.meaningId);
  return true;
}


void StaticLinguisticDictionary::getWord
(std::string& pWord,
 int pWordNode) const
{
  assert(xIsLoaded());
  xGetWord(pWord, pWordNode);
}



std::string StaticLinguisticDictionary::xGetLemma
(int32_t pMeaningId,
 bool pWithLinkMeanings) const
{
  if (pMeaningId == LinguisticMeaning_noMeaningId)
    return "";

  auto* meaningPtr = fPtrMeaning + pMeaningId;
  int lemmaNodeId = xGetLemmeNodeId(meaningPtr);
  std::string res;
  xGetWord(res, lemmaNodeId);
  if (pWithLinkMeanings && xIsAGatheringMeaning(meaningPtr))
  {
    char nbLinkedMeanings = xNbLinkedMeanings(meaningPtr);
    const int* currLinkedMeaning = xGetFirstLinkedMeaning(meaningPtr);
    for (unsigned char i = 0; i < nbLinkedMeanings; ++i)
    {
      std::string subLemme;
      xGetWord(subLemme, xGetLemmeNodeId(fPtrMeaning + binaryloader::alignedDecToInt(*currLinkedMeaning)));
      res += "~" + subLemme;
      currLinkedMeaning = xGetNextLinkedMeaning(currLinkedMeaning);
    }
  }
  return res;
}

} // End of namespace linguistics
} // End of namespace onsem
