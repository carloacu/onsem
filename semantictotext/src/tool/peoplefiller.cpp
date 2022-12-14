#include <onsem/semantictotext/tool/peoplefiller.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>

namespace onsem
{

namespace peopleFiller
{

// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)


void addPeople(SemanticMemoryBlock& pMemoryBlock,
               std::istream& pInputStream,
               SemanticLanguageEnum pLanguage,
               const linguistics::LinguisticDatabase& pLingDb)
{
  static const std::string peopleRootLabelStr = "peopleRoot";
  struct PersonDescription
  {
    std::vector<std::string> names;
  };
  std::map<std::string, PersonDescription> idToPeople;

  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(pInputStream, tree);
  childLoop(tree, currElt, peopleRootLabelStr)
  {
    std::string id = currElt.second.get("<xmlattr>.id", "");

    PersonDescription personDesc;
    auto namesTreeOpt = currElt.second.get_child_optional("names");
    if (namesTreeOpt)
    {
      for (const auto& currNameTree : *namesTreeOpt)
        personDesc.names.emplace_back(currNameTree.second.get("<xmlattr>.name", ""));
    }
    idToPeople.emplace(id, std::move(personDesc));
  }

  for (const auto& currPerson : idToPeople)
    pMemoryBlock.addASetOfEquivalentNames(currPerson.second.names, pLanguage, pLingDb);
}



} // End of namespace peopleFiller

} // End of namespace onsem
