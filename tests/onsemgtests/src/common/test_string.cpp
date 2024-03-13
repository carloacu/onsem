#include <gtest/gtest.h>
#include <onsem/common/utility/string.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;

namespace {

std::string _vectToStr(const std::vector<std::string>& pStrs) {
    std::string res;
    bool firstElt = true;
    for (auto& currStr : pStrs) {
        if (firstElt)
            firstElt = false;
        else
            res += ",";
        res += currStr;
    }
    return "[" + res + "]";
}

std::string _testSplitAnyOf(const std::string& pStr, const std::set<char>& pChars) {
    std::vector<std::string> strs;
    mystd::splitAnyOf(strs, pStr, pChars);
    return _vectToStr(strs);
}

}

TEST(StringUtil, split) {
    EXPECT_EQ("[,ff,fd ,vdd,,]", _testSplitAnyOf("+ff-fd +vdd--", {'+', '-'}));
    EXPECT_EQ("[a,ff,fd ,vdd,,u]", _testSplitAnyOf("a+ff-fd +vdd--u", {'+', '-'}));
    EXPECT_EQ("[+ff-fd +vdd--]", _testSplitAnyOf("+ff-fd +vdd--", {}));
    EXPECT_EQ("[]", _testSplitAnyOf("", {'+', '-'}));
}
