#include "../firstnamesextractor.hpp"
#include <fstream>
#include <sstream>


namespace onsem
{
namespace firstnamesExtractor
{


inline static bool upperCaseOfFirstLetter
(std::string& pPLowerCaseSentence,
 std::size_t pPos)
{
  if (pPLowerCaseSentence[pPos] >= 'a' && pPLowerCaseSentence[pPos] <= 'z')
  {
    pPLowerCaseSentence[pPos] = static_cast<char>(pPLowerCaseSentence[pPos] + 'A' - 'a');
    return true;
  }
  if (pPLowerCaseSentence.compare(pPos, 2, "é") == 0)
  {
    pPLowerCaseSentence.replace(pPos, 2, "É");
    return true;
  }
  return false;
}


// Regenarete first names
void run(const std::string& pMyDataMiningPath,
         const std::string& pInputResourcesFolder)
{
  const std::string inFilename = pMyDataMiningPath + "/resources/common/firstnames/firstnames.txt";

  std::ifstream prenomsFile(inFilename, std::ifstream::in);
  if (!prenomsFile.is_open())
    throw std::runtime_error("Error: Can't open " + inFilename + " file !");

  std::ofstream outFile(pInputResourcesFolder.c_str());

  std::string line;
  getline(prenomsFile, line);
  while (getline(prenomsFile, line))
  {
    std::size_t endOfName = line.find('\t');
    if (endOfName != std::string::npos)
    {
      std::string name = line.substr(0, endOfName);

      if (name.size() < 2)
        continue;
      if (name[name.size() - 1] == ')')
      {
        if (name.substr(name.size() - 4, 4) == " (1)")
        {
          name = name.substr(0, name.size() - 4);
        }
        else
        {
          continue;
        }
      }

      std::size_t beginOfGender = endOfName + 1;
      std::size_t endOfGender = line.find('\t', beginOfGender);
      if (endOfGender != std::string::npos &&
          endOfGender > beginOfGender)
      {
        std::string gendersStr = line.substr(beginOfGender, endOfGender - beginOfGender);
        bool isMasc = gendersStr.find('m') != std::string::npos;
        bool isFem = gendersStr.find('f') != std::string::npos;
        outFile << "#" << name << ",";
        upperCaseOfFirstLetter(name, 0);
        outFile << name << ".PN+z2+Hum";

        std::stringstream ssFrexions;
        if (isMasc)
          ssFrexions << ":ms";
        if (isFem)
          ssFrexions << ":fs";
        ssFrexions <<  "\n";

        outFile << ssFrexions.str();
        outFile << "#" << name << ",.PN+Hum" << ssFrexions.str();
      }
    }
  }
  outFile.close();
  prenomsFile.close();
}


} // End of namespace firstnamesExtractor
} // End of namespace onsem
