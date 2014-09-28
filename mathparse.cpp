#include <regex>
#include <string>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <functional>
#include <numeric>
#include <cstdlib>
#include <list>
#include <algorithm>

#include <stdexcept>

#define INVALID_ARGUMENT(EXP) \
std::invalid_argument(__PRETTY_FUNCTION__+std::string(" -> ")+std::string(EXP))

//unordered_map<string,function<double()> operations;

using namespace std;

double a = log(100);

void explainRegexError(std::regex_error& e)
{
    switch(e.code()) {
	case std::regex_constants::error_collate:
        cerr << "The expression contained an invalid collating element name."
            << endl;
		break;
	case std::regex_constants::error_ctype:
        cerr << "The expression contained an invalid character class name." 
            << endl;
		break;
	case std::regex_constants::error_escape:
        cerr << "The expression contained an invalid escaped character, or a "
            "trailing escape." << endl;
		break;
	case std::regex_constants::error_backref:
        cerr << "The expression contained an invalid back reference." << endl;
		break;
	case std::regex_constants::error_brack:
        cerr << "The expression contained mismatched brackets ([ and ])." 
            << endl;
		break;
	case std::regex_constants::error_paren:
        cerr << "The expression contained mismatched parentheses (( and ))." 
            << endl;
		break;
	case std::regex_constants::error_brace:
        cerr << "The expression contained mismatched braces ({ and })." 
            << endl;
		break;
	case std::regex_constants::error_badbrace:
        cerr << "The expression contained an invalid range between braces ({ "
            "and })." << endl;
		break;
	case std::regex_constants::error_range:
        cerr << "The expression contained an invalid character range." << endl;
		break;
	case std::regex_constants::error_space:
        cerr << "There was insufficient memory to convert the expression into "
            "a finite state machine." << endl;
		break;
	case std::regex_constants::error_badrepeat:
        cerr << "The expression contained a repeat specifier (one of *?+{) "
            "that was not preceded by a valid regular expression." << endl;
		break;
	case std::regex_constants::error_complexity:
        cerr << "The complexity of an attempted match against a regular "
            "expression exceeded a pre-set level." << endl;
		break;
	case std::regex_constants::error_stack:
        cerr << "There was insufficient memory to determine whether the "
            "regular expression could match the specified character sequence."
            << endl;
		break;
    }
}

//class UnaryMap : unordered_map<string, function<double(double)>> {
//    UnaryMap() 
//    {
//        pointer_to_unary_function<double,double> f(exp);
//        (*this)["exp"] = f;
//    }
//} UNARY;
unordered_map<string,int> priority({{"pow", 4}, {"exp", 4}, {"cos", 4}, {"sin", 4},
        {"tan", 4}, {"log", 4}, {"+", 3}, {"-",3}, {"*", 4},
        {"/", 4}, {"==", 2}, {"^", 6}});

unordered_map<string,function<double(double)>> UNARY({
        {"exp",pointer_to_unary_function<double,double>(exp)},
        {"cos",pointer_to_unary_function<double,double>(cos)},
        {"sin",pointer_to_unary_function<double,double>(sin)},
        {"tan",pointer_to_unary_function<double,double>(tan)},
        {"log",pointer_to_unary_function<double,double>(log)}});

unordered_map<string,function<double(double,double)>> BINARY({
        {"+",std::plus<double>()},
        {"-",std::minus<double>()},
        {"*",std::multiplies<double>()},
        {"/",std::divides<double>()},
        {"==",std::equal_to<double>()},
        {"^",pointer_to_binary_function<double,double,double>(std::pow<double>)}
        });

int main(int argc, char** argv)
{
    if(argc != 2) {
        cerr << "Need 1 argument: an equation" << endl;
        return -1;
    }

    string eq(argv[1]);
    cerr << "Equation: " << eq << endl;
    std::string ops;
    for(auto it = UNARY.begin(); it != UNARY.end(); ++it) 
        ops += "|" + it->first;
    for(auto it = BINARY.begin(); it != BINARY.end(); ++it) {
        if(it->first == "*" || it->first == "+")
            ops += "|\\*";
        else
            ops += "|" + it->first;
    }

    regex re;
    try {
        string regexstr = "\\s*(\\(|\\)"+ops+"|[0-9]*|[a-z]*)\\s*";
//        string regexstr = "\\s*(\\(|\\)|log|cos|sin|exp|tan|-|==|\\+|[0-9]*|[a-z]*)";
        cerr << "RE:\n" << regexstr << endl;
        re.assign(regexstr);
    } catch(regex_error& e) {
        explainRegexError(e);
        return -1;
    }
    regex_iterator<string::iterator> rit (eq.begin(), eq.end(), re,
            regex_constants::match_continuous);
    regex_iterator<string::iterator> rend;

    unordered_map<string, double> args;
    list<string> opstack;
    list<string> outqueue;
    for(; rit != rend && !rit->str().empty(); ++rit) {
        string tok = (*rit)[1];
        cerr << "tok: (" << tok << ")" << endl;
        if(tok == "(") {
            // Open Parenthetical
            opstack.push_front(tok);
        } else if(tok == ")") {
            // Close Parenthetical
            while(opstack.front() != "(") {
                outqueue.push_back(opstack.front());
                opstack.pop_front();
                if(opstack.empty()) {
                    throw INVALID_ARGUMENT("Error, closed paren was never "
                            "opened\n");
                }
            }
            opstack.pop_front();
        } else if(priority.count(tok) > 0) {
            // Add latest operator to stack until we find lower priority op
            while(!opstack.empty()) {
                // Go ahead and evaluate higher priority operators before 
                // the current
                if(priority[tok] <= priority[opstack.front()]) {
                    outqueue.push_back(opstack.front());
                    opstack.pop_front();
                } else {
                    break;
                }
            }
            opstack.push_front(tok);
        } else {
            // argument
            outqueue.push_back(tok);
        }
    }
	
    // Copy last operators to output queue
	while(!opstack.empty()) {
		if(opstack.front() == "(") 
            throw INVALID_ARGUMENT("Error, unmatched parentheses remaining");
		outqueue.push_back(opstack.front());
		opstack.pop_front();
	}

    /// HERE
	for(auto it = outqueue.begin(); it != outqueue.end(); it++) {

        cerr << "HERE: " << *it << endl;
    }

}

//Consider -
//    Stack S
//    Char  ch
//    Char  element
//while(Tokens are Available)
// {
//     ch = Read(Token);
//     if(ch is Operand)
//       {
//       Print ch ;
//       }
//     else
//       {
//       while(Priority(ch) <= Priority(Top Most Stack))
//            {
//            element = Pop(S);
//            Print(ele);
//            }
//       Push(S,ch);
//       }
//}
//while(!Empty(S))
//{
//element = Pop(S);
//Print(ele);
//}
