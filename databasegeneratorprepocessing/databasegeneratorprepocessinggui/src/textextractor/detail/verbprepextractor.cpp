#include "verbprepextractor.hpp"
#include <fstream>
#include <onsem/texttosemantic/type/chunklink.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>


namespace onsem
{

VerbPrepExtractor::VerbPrepExtractor()
  : fAQuiPronWord(),
    fAQuiConjSWord(),
    fVerbsToFreq()
{
  xInit();
}


VerbPrepExtractor::VerbPrepExtractor(const std::vector<VerbPrepExtractor>& pOtherVerbPrepExtractors)
  : fAQuiPronWord(),
    fAQuiConjSWord(),
    fVerbsToFreq()
{
  xInit();

  for (const auto& cuuVerbPrepExtractor : pOtherVerbPrepExtractors)
    for (const auto& currVerbsToFreq : cuuVerbPrepExtractor.fVerbsToFreq)
      fVerbsToFreq[currVerbsToFreq.first] += currVerbsToFreq.second;
}

void VerbPrepExtractor::processText
(const linguistics::SyntacticGraph& pSyntGraph,
 const std::string& pText)
{
  xProcessChunkLinkList(pSyntGraph.firstChildren, pText);
}


void VerbPrepExtractor::writeResults
(const std::string& pResultFilename) const
{
  std::cout << "Write results life: " << pResultFilename << std::endl;
  std::ofstream resultFile(pResultFilename.c_str());
  for (std::map<std::string, int>::const_iterator
       it = fVerbsToFreq.begin(); it != fVerbsToFreq.end(); ++it)
  {
    resultFile << "#" << it->first << "\t#"
               << it->second << "_" << std::endl;
  }
  resultFile.close();
}

const linguistics::Chunk* _getChildrenOfAChunk(const linguistics::Chunk& pVerbChunk,
                                               linguistics::ChunkLinkType pChunkLinkType)
{
  for (const auto& currChild : pVerbChunk.children)
    if (currChild.type == pChunkLinkType)
      return &*currChild.chunk;
  return nullptr;
}

const linguistics::ChunkLink* _getChildrenOfAChunkLink(const linguistics::Chunk& pVerbChunk,
                                                       linguistics::ChunkLinkType pChunkLinkType)
{
  for (const auto& currChild : pVerbChunk.children)
    if (currChild.type == pChunkLinkType)
      return &currChild;
  return nullptr;
}


const linguistics::ChunkLink* _getSubjectOfChunkLink(const linguistics::Chunk& pVerbChunk)
{
  for (const auto& currChild : pVerbChunk.children)
  {
    if (currChild.type == linguistics::ChunkLinkType::SUBJECT)
      return &currChild;
    if (currChild.type == linguistics::ChunkLinkType::AUXILIARY)
      return _getSubjectOfChunkLink(*currChild.chunk);
  }
  return nullptr;
}

void VerbPrepExtractor::xInit()
{
  fAQuiPronWord.language = SemanticLanguageEnum::FRENCH;
  fAQuiPronWord.lemma = "à qui";
  fAQuiPronWord.partOfSpeech = PartOfSpeech::PRONOUN;

  fAQuiConjSWord.language = SemanticLanguageEnum::FRENCH;
  fAQuiConjSWord.lemma = "à qui";
  fAQuiConjSWord.partOfSpeech = PartOfSpeech::SUBORDINATING_CONJONCTION;
}


void VerbPrepExtractor::xProcessChunkLinkList
(const std::list<linguistics::ChunkLink>& pChunkLinks,
 const std::string& pText)
{
  for (const auto& currChkLk : pChunkLinks)
  {
    const linguistics::Chunk& chunk = *currChkLk.chunk;
    if (chunk.type == linguistics::ChunkType::VERB_CHUNK)
    {
      const linguistics::Chunk* questChunk = _getChildrenOfAChunk(chunk, linguistics::ChunkLinkType::QUESTIONWORD);
      if (questChunk != nullptr)
      {
        if (questChunk->head->inflWords.front().word == fAQuiPronWord)
        {
          const linguistics::Chunk* doChunkPtr = _getChildrenOfAChunk(chunk, linguistics::ChunkLinkType::DIRECTOBJECT);
          if (doChunkPtr != nullptr &&
              doChunkPtr->head->inflWords.front().word.partOfSpeech == PartOfSpeech::VERB)
          {
            xAddChunkHead(*doChunkPtr);
          }
          else
          {
            xAddChunkHead(chunk);
          }
        }
      }

      const linguistics::ChunkLink* subordClauseChunkLk =
          _getChildrenOfAChunkLink(chunk, linguistics::ChunkLinkType::SUBORDINATE_CLAUSE);
      if (subordClauseChunkLk != nullptr)
      {
        xAddSubVerbIfInterestingChunkLink(*subordClauseChunkLk);
      }
    }
    else
    {
      const linguistics::ChunkLink* subjectOfChunkLk =
          _getSubjectOfChunkLink(chunk);
      if (subjectOfChunkLk != nullptr)
      {
        xAddSubVerbIfInterestingChunkLink(*subjectOfChunkLk);
      }
    }

    xProcessChunkLinkList(chunk.children, pText);
  }
}


void VerbPrepExtractor::xAddSubVerbIfInterestingChunkLink
(const linguistics::ChunkLink& pChunkLink)
{
  for (auto it = getTheNextestToken(pChunkLink.tokRange.getItBegin(),
                                    pChunkLink.tokRange.getItEnd());
       it != pChunkLink.tokRange.getItEnd();
       it = getNextToken(it, pChunkLink.tokRange.getItEnd()))
  {
    if (it->inflWords.front().word == fAQuiConjSWord)
    {
      const linguistics::Chunk& subChunk = *pChunkLink.chunk;
      if (subChunk.type == linguistics::ChunkType::VERB_CHUNK)
      {
        xAddChunkHead(subChunk);
      }
    }
  }
}


void VerbPrepExtractor::xAddChunkHead
(const linguistics::Chunk& pChunk)
{
  const std::string& lemma = pChunk.head->inflWords.front().word.lemma;
  std::map<std::string, int>::iterator itVerbsToFreq = fVerbsToFreq.find(lemma);
  if (itVerbsToFreq != fVerbsToFreq.end())
  {
    ++itVerbsToFreq->second;
  }
  else
  {
    fVerbsToFreq[lemma] = 1;
  }
}


} // End of namespace onsem
