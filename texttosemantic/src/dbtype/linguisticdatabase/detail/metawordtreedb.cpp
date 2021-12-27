#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/metawordtreedb.hpp>

namespace onsem
{

MetaWordTreeDb::MetaWordTreeDb()
  : VirtualSemBinaryDatabase(),
    fPtrPatriciaTrie(nullptr)
{
}


bool MetaWordTreeDb::isASeparator
(const std::string& pString,
 const std::size_t pPos)
{
  static const std::string apos = "’";
  static const std::size_t aposSize = apos.size();
  if (pPos < pString.size())
  {
    char follChar = pString[pPos];
    return follChar == ' ' || follChar == '\'' || follChar == '"' || follChar == '-' ||
        follChar == ',' ||  follChar == '.' || follChar == '?' || follChar == '!'  ||
        follChar == ';' || follChar == ':' || follChar == '=' || follChar == '(' ||
        follChar == ')' || follChar == '|' || follChar == '{' || follChar == '}' ||
        follChar == '\n' || follChar == '\t' ||
        pString.compare(pPos, aposSize, apos) == 0;
  }
  return true;
}


const signed char* MetaWordTreeDb::xSearchInPatriciaTrie
(std::size_t& pLongestWord,
 const std::string& pString,
 const std::size_t pBeginOfString, const std::size_t pSizeOfString,
 bool pOnlyWordWithWordFroms,
 SearchForLongestWordMode pLongWordMode) const
{
  bool begWithASeparator = false;
  if (pLongWordMode == SearchForLongestWordMode::ENBABLED_BUTWITHSEPARATORAFTER)
    begWithASeparator = isASeparator(pString, pBeginOfString);

  pLongestWord = 0;
  // current offset in "pString"
  std::size_t offWord = 0;
  bool findLetter = true;
  const signed char* currNode = fPtrPatriciaTrie;

  // Until we found the current letter in a child node
  while (findLetter)
  {
    findLetter = false;
    // Iterate through all child nodes
    unsigned char nbChildren = xNbChildren(currNode);
    if (nbChildren > 0)
    {
      const int* children = xGetFirstChild(currNode);
      for (unsigned char i = 0; i < nbChildren; ++i)
      {
        {
          signed char searchedLetter = pString[pBeginOfString + offWord];
          signed char databaseLetter = xGetCharAfterAlignedDec(children[i]);
          // If searched letter is before current database letter, then
          // the searched letter does not exist in the database
          // (because children are sorted alphabetically)
          if (searchedLetter < databaseLetter)
          {
            return nullptr;
          }
          // If searched letter is different from current database letter, then
          // go to the next database letter
          if (searchedLetter != databaseLetter)
          {
            continue;
          }
        }
        currNode = xAlignedDecToPtr(fPtrPatriciaTrie, children[i]);
        ++offWord;
        // letters already read + the number of letters in the node >
        // number of letters in the word we look for
        if (offWord + xNbLetters(currNode) > pSizeOfString)
        {
          return nullptr;
        }
        // Check all the letters of the node
        for (unsigned char j = 0; j < xNbLetters(currNode); ++j, ++offWord)
        {
          if (static_cast<signed char>(pString[pBeginOfString + offWord]) != xGetLetter(currNode, j))
          {
            return nullptr;
          }
        }
        // If the node correspond to the end of a word
        if (xIfEndOfAWord(currNode, pOnlyWordWithWordFroms))
        {
          // Save the actual length because it will maybe be the longest word that
          // match the begin of the string
          switch (pLongWordMode)
          {
          case SearchForLongestWordMode::ENABLED:
            pLongestWord = offWord;
            break;
          case SearchForLongestWordMode::ENBABLED_BUTWITHSEPARATORAFTER:
            if (begWithASeparator ||
                isASeparator(pString, pBeginOfString + offWord))
              pLongestWord = offWord;
            break;
          case SearchForLongestWordMode::DISABLED:
            break;
          }

          // If we are at the end of the string
          if (offWord == pSizeOfString)
          {
            return currNode;
          }
        }
        findLetter = true;
        break;
      }
    }
  }
  return nullptr;
}



void MetaWordTreeDb::xGetWordsThatBeginWith
(std::list<const signed char*>& pResWords,
 const std::string& pBeginOfWords) const
{
  // current offset in "pString"
  std::size_t offWord = 0;
  bool findLetter = true;
  const auto* currNode = fPtrPatriciaTrie;

  // Until we found the current letter in a child node
  while (findLetter)
  {
    if (offWord >= pBeginOfWords.size())
    {
      if (offWord > pBeginOfWords.size())
        xGetWordsFromANodeOfTheTree(pResWords, currNode, true);
      if (offWord == pBeginOfWords.size())
        xGetWordsFromANodeOfTheTree(pResWords, currNode, false);
    }

    findLetter = false;
    // Iterate through all child nodes
    unsigned char nbChildren = xNbChildren(currNode);
    if (nbChildren > 0)
    {
      const int* children = xGetFirstChild(currNode);
      for (unsigned char i = 0; i < nbChildren; ++i)
      {
        {
          signed char searchedLetter = pBeginOfWords[offWord];
          signed char databaseLetter = xGetCharAfterAlignedDec(children[i]);
          // If searched letter is before current database letter, then
          // the searched letter does not exist in the database
          // (because children are sorted alphabetically)
          if (searchedLetter < databaseLetter)
            return;
          // If searched letter is different from current database letter, then
          // go to the next database letter
          if (searchedLetter != databaseLetter)
            continue;
        }
        currNode = xAlignedDecToPtr(fPtrPatriciaTrie, children[i]);
        ++offWord;

        // Check all the letters of the node
        for (unsigned char j = 0; j < xNbLetters(currNode); ++j, ++offWord)
        {
          if (offWord == pBeginOfWords.size())
          {
            xGetWordsFromANodeOfTheTree(pResWords, currNode, true);
            return;
          }
          if (pBeginOfWords[offWord] != xGetLetter(currNode, j))
            return;
        }

        findLetter = true;
        break;
      }
    }
  }
}



void MetaWordTreeDb::xGetWord
(std::string& pWord,
 int pWordNode) const
{
  const auto* currNode = fPtrPatriciaTrie + pWordNode;
  std::list<char> resList;
  do
  {
    // get letters inside the node
    {
      unsigned char i = xNbLetters(currNode);
      while (i > 0)
      {
        resList.push_front(xGetLetter(currNode, --i));
      }
    }

    const auto* prevNode = currNode;
    currNode = xAlignedDecToPtr(fPtrPatriciaTrie,
                                *xGetFather(currNode));

    // get letter in the link of the father
    unsigned char nbChildren = xNbChildren(currNode);
    if (nbChildren > 0)
    {
      const int* children = xGetFirstChild(currNode);
      for (unsigned char i = 0; i < nbChildren; ++i)
      {
        if (xAlignedDecToPtr(fPtrPatriciaTrie, children[i]) == prevNode)
        {
          resList.push_front(xGetCharAfterAlignedDec(children[i]));
          break;
        }
      }
    }
  }
  while (currNode != fPtrPatriciaTrie);

  // write the word
  pWord.resize(resList.size(), ' ');
  std::size_t count = 0;
  for (std::list<char>::iterator it = resList.begin(); it != resList.end(); ++it)
  {
    pWord[count++] = *it;
  }
}



void MetaWordTreeDb::xGetWordsFromANodeOfTheTree
(std::list<const signed char*>& pResWords,
 const signed char* pCurrNode,
 bool pCanAddCurrNode) const
{
  if (pCanAddCurrNode &&
      xIfEndOfAWord(pCurrNode, false))
  {
    pResWords.push_back(pCurrNode);
  }

  // Iterate through all child nodes
  unsigned char nbChildren = xNbChildren(pCurrNode);
  if (nbChildren > 0)
  {
    const int* children = xGetFirstChild(pCurrNode);
    for (unsigned char i = 0; i < nbChildren; ++i)
    {
      const signed char* currNode = xAlignedDecToPtr(fPtrPatriciaTrie, children[i]);
      xGetWordsFromANodeOfTheTree(pResWords, currNode, true);
    }
  }
}

} // End of namespace onsem
