#include <onsem/semanticdebugger/dotsaver.hpp>
#include <map>
#include <fstream>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>


namespace onsem
{
namespace linguistics
{
typedef std::map<std::shared_ptr<Chunk>, std::size_t> NodeToId;
const static std::string strToAvoidWarnings = " ";

namespace DotSaver
{
namespace
{

  std::string _replaceString
(std::string subject,
 const std::string& search,
 const std::string& replace)
{
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos)
  {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return subject;
}


std::string _stringToPrintByDot
(const std::string& subject)
{
  std::string res = _replaceString(subject, "\\", "\\\\");
  return _replaceString(res, "\"", "\\\"");
}



bool _writeAChunkNode
(std::ostream& pSs, NodeToId& pNodesWriten,
 std::size_t& pNodeId, std::size_t& pNewId,
 const ChunkLink& pNewNode)
{
  if (!pNewNode.chunk->hasOnlyOneReference)
  {
    NodeToId::iterator itNode = pNodesWriten.find(pNewNode.chunk);
    if (itNode != pNodesWriten.end())
    {
      pNodeId = itNode->second;
      return false;
    }
    pNodesWriten[pNewNode.chunk] = pNewId;
  }
  pNodeId = pNewId;

  if (pNewNode.chunk->type == ChunkType::AND_CHUNK)
  {
    pSs << pNewId << " [label=\"AND";
  }
  else if (pNewNode.chunk->type == ChunkType::OR_CHUNK)
  {
    pSs << pNewId << " [label=\"OR";
  }
  else if (pNewNode.chunk->type == ChunkType::THEN_CHUNK)
  {
    pSs << pNewId << " [label=\"THEN";
  }
  else if (pNewNode.chunk->type == ChunkType::THEN_REVERSED_CHUNK)
  {
    pSs << pNewId << " [label=\"THEN_REVERSED";
  }
  else if (pNewNode.chunk->type == ChunkType::TEACH_CHUNK)
  {
    pSs << pNewId << " [label=\"TEACH";
  }
  else
  {
    pSs << pNewId << " [label=\"" << strToAvoidWarnings;
    std::string tokRangeStr;
    pNewNode.chunk->tokRange.getStr(tokRangeStr);
    tokRangeStr = _stringToPrintByDot(tokRangeStr);
    pSs << tokRangeStr << "\\nhead: " << _stringToPrintByDot(pNewNode.chunk->head->str);
  }
  if (chunkTypeIsVerbal(pNewNode.chunk->type))
  {
    if (!pNewNode.chunk->positive)
    {
      pSs << "\\nnegative form";
    }
    if (pNewNode.chunk->isPassive)
    {
      pSs << "\\npassive form";
    }
  }
  if (!pNewNode.chunk->requests.empty() && pNewNode.chunk->type != ChunkType::AUX_CHUNK)
  {
    pSs << "\\n";
    bool firstIteration = true;
    for (const auto& currRequest : pNewNode.chunk->requests.types)
    {
      if (firstIteration)
        firstIteration = false;
      else
        pSs << " & ";
      pSs << semanticRequestType_toStr(currRequest);
    }
  }
  if (chunkTypeIsAList(pNewNode.chunk->type) || pNewNode.chunk->type == ChunkType::TEACH_CHUNK)
    pSs << "\"";
  else
    pSs << strToAvoidWarnings << "\" ";
  if (pNewNode.chunk->type == ChunkType::VERB_CHUNK ||
      pNewNode.chunk->type == ChunkType::AUX_CHUNK)
  {
    pSs << ",color=orange";
  }
  else if (pNewNode.chunk->type == ChunkType::SEPARATOR_CHUNK)
  {
    pSs << ",color=yellow";
  }
  else if (pNewNode.chunk->type == ChunkType::INFINITVE_VERB_CHUNK)
  {
    pSs << ",color=sandybrown";
  }

  pSs << "]" << std::endl;
  return true;
}



void _writeAChunk
(std::ostream& pSs, NodeToId& pNodesWriten,
 std::size_t pMotherId, std::size_t& pNewId,
 const ChunkLink& pNewNode)
{
  std::size_t nodeId;
  bool newNode = _writeAChunkNode(pSs, pNodesWriten, nodeId, pNewId, pNewNode);
  pSs << pMotherId << "->" << nodeId
      << " [label=\"" << strToAvoidWarnings << chunkLinkType_toStr(pNewNode.type);
  if (!pNewNode.tokRange.isEmpty())
  {
    pSs << "\\n\\\"";
    std::string tokRangeStr;
    pNewNode.tokRange.getStr(tokRangeStr);
    pSs << tokRangeStr << "\\\"";
  }
  const std::string chunkLinkColorStr = chunkLinkType_toColorStr(pNewNode.type);
  pSs << strToAvoidWarnings << "\" "
      << "fontcolor=\"" << chunkLinkColorStr << "\" "
      << "color=\"" << chunkLinkColorStr << "\""
      << "]" << std::endl;
  if (!newNode)
  {
    return;
  }
  pMotherId = nodeId;
  ++pNewId;
  for (const auto& currChunkLink : pNewNode.chunk->children)
    _writeAChunk(pSs, pNodesWriten, pMotherId, pNewId, currChunkLink);

}


int _generateDotFile
(const std::string& pInFilename,
 const std::string& pOutFilename)
{
  std::stringstream ss;
  ss << "dot " << pInFilename << " -Tpng -o " << pOutFilename;
  return system(ss.str().c_str());
}

}


void writeChunkLinks(std::ostream& pSs,
                     const std::list<ChunkLink>& pSyntTree)
{
  std::size_t nextNodeId = 1;
  std::size_t motherId = 0;
  NodeToId nodesWriten;
  for (const auto& currChunkLink : pSyntTree)
    _writeAChunk(pSs, nodesWriten, motherId, nextNodeId, currChunkLink);
}



void save(const std::string& pInFilename,
          const std::string& pOutFilename,
          const std::string& pDotContent)
{
  std::ofstream outfile(pInFilename);

  outfile << "digraph \"Syntactic Graph\" {" << std::endl;
  outfile << "bgcolor=\"transparent\"" << std::endl;
  outfile << "node [style=filled,color=lightgrey]" << std::endl;
  if (!pDotContent.empty())
  {
    outfile << "0 [color=greenyellow label=\"root\"]" << std::endl;
    outfile << pDotContent;
  }
  outfile << "}" << std::endl;
  outfile.close();
  _generateDotFile(pInFilename, pOutFilename);
}


} // End of namespace DotSaver

} // End of namespace linguistics
} // End of namespace onsem
