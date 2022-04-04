#include <contextualplanner/arithmeticevaluator.hpp>
#include <string>
#include <sstream>

namespace cp
{

class ArithmeticEvaluator
{
public:
ArithmeticEvaluator(const std::string& pExpressionToParse,
                    std::size_t pBeginPos)
 : expressionToParse(pExpressionToParse),
   _currentPos(pBeginPos)
{
}

int expression()
{
  int result = term();
  while (peek() == '+' || peek() == '-')
  {
    if (get() == '+')
      result += term();
    else
      result -= term();
  }
  return result;
}


private:
const std::string& expressionToParse;
std::size_t _currentPos;

char peek()
{
  return expressionToParse[_currentPos];
}

char get()
{
  return expressionToParse[_currentPos++];
}

int number()
{
  int result = get() - '0';
  while (peek() >= '0' && peek() <= '9')
    result = (10 * result) + get() - '0';
  return result;
}

int factor()
{
  if (peek() >= '0' && peek() <= '9')
    return number();
  else if (peek() == '(')
  {
    get(); // '('
    int result = expression();
    get(); // ')'
    return result;
  }
  else if (peek() == '-')
  {
    get();
    return -factor();
  }
  throw std::runtime_error("Cannot evaluate this expression");
  return 0; // error
}


int term()
{
  int result = factor();
  while (peek() == '*' || peek() == '/')
    if (get() == '*')
      result *= factor();
    else
      result /= factor();
  return result;
}
};





int evalute(const std::string& pText,
            std::size_t pBeginPos)
{
  ArithmeticEvaluator evaluator(pText, pBeginPos);
  return evaluator.expression();
}


std::string evaluteToStr(const std::string& pText,
                         std::size_t pBeginPos)
{
  std::stringstream ss;
  try {
    ss << evalute(pText, pBeginPos);
    return ss.str();
  } catch (...) {}
  return "_";
}


}
