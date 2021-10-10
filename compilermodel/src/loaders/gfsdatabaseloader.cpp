#include "gfsdatabaseloader.hpp"
#include <boost/filesystem/fstream.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>

namespace onsem
{


void GFSDatabaseLoader::merge
(const boost::filesystem::path& pFilename,
 LinguisticIntermediaryDatabase& pLingdb)
{
  boost::filesystem::ifstream infile(pFilename, boost::filesystem::ifstream::in);
  if (!infile.is_open())
  {
    std::cerr << "Error: Can't open " << pFilename.string() << " file !" << std::endl;
    return;
  }

  std::string line;
  while (getline(infile, line))
  {
    if (line.empty())
    {
      continue;
    }

    if (line[0] == '#')
    {
      std::size_t endOfWord = line.find_first_of(":", 1);
      if (endOfWord == std::string::npos)
      {
        std::cerr << "Error: \":\" not found in \"" << line << "\"" << std::endl;
        continue;
      }
      std::string word = line.substr(1, endOfWord - 1);
      std::size_t beginOfGram = endOfWord + 1;
      std::size_t endOfGram = line.find_first_of(",", beginOfGram);
      if (endOfGram == std::string::npos)
      {
        std::cerr << "Error: \",\" not found in \"" << line << "\" (after the \"#\")" << std::endl;
        continue;
      }
      std::size_t beginOfLemma = endOfGram + 1;
      std::size_t endOfLemma = line.find_first_of("|", beginOfLemma);
      if (endOfLemma == std::string::npos)
      {
        std::cerr << "Error: \"|\" not found in \"" << line << "\" (after the \"#\")" << std::endl;
        continue;
      }
      while (endOfGram != std::string::npos && endOfLemma != std::string::npos)
      {
        LingdbDynamicTrieNode* node = pLingdb.getPointerToEndOfWord(word);
        if (node != nullptr)
        {
          PartOfSpeech gramType = partOfSpeech_fromStr(line.substr(beginOfGram, endOfGram - beginOfGram));
          if (gramType == PartOfSpeech::UNKNOWN)
          {
            std::cerr << "Error: gram type "
                      << line.substr(beginOfGram, endOfGram - beginOfGram)
                      << " is not found" << std::endl;
          }
          else
          {
            node->putAWordFormAtTheTopOfTheList(gramType,
                                                line.substr(beginOfLemma, endOfLemma - beginOfLemma));
          }
        }

        beginOfGram = endOfLemma + 1;
        endOfGram = line.find_first_of(",", beginOfGram);
        beginOfLemma = endOfGram + 1;
        endOfLemma = line.find_first_of("|", beginOfLemma);
      }
    }
    else
    {
      std::cerr << "Error: in " << pFilename
                << " a line don't begins with \"#\"" << std::endl;
    }
  }

  infile.close();
}



} // End of namespace onsem
