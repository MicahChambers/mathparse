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
#include <cassert>
#include <random>

#include <stdexcept>

#define INVALID_ARGUMENT(EXP) \
std::invalid_argument(__PRETTY_FUNCTION__+std::string(" -> ")+std::string(EXP))

std::default_random_engine rng;

//unordered_map<string,function<double()> operations;

using namespace std;

class Equation
{
public:
    Equation(std::list<string> rpn);
    unordered_map<string, shared_ptr<double>> args;
    double exec();

private:

    // function that has been bound to have no arguments
    function<double()> executor;
};

void explainRegexError(std::regex_error& e);
void printInfix(std::list<string> rpn);

//class UnaryMap : unordered_map<string, function<double(double)>> {
//    UnaryMap() 
//    {
//        pointer_to_unary_function<double,double> f(exp);
//        (*this)["exp"] = f;
//    }
//} UNARY;
unordered_map<string,int> priority({{"exp", 5}, {"cos", 5}, {"sin", 5},
        {"tan", 5}, {"log", 5}, {"+", 3}, {"-",3}, {"*", 4},
        {"/", 4}, {"==", 2}, {"^", 6}});

unordered_map<string,function<double(double)>> UNARY({
        {"exp",pointer_to_unary_function<double,double>(exp)},
        {"cos",pointer_to_unary_function<double,double>(cos)},
        {"sin",pointer_to_unary_function<double,double>(sin)},
        {"tan",pointer_to_unary_function<double,double>(tan)},
        {"neg",std::negate<double>()},
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
        if(it->first == "*" || it->first == "+" || it->first == "^")
            ops += "|\\" + it->first;
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
    string prev = "";
    bool impliedmult = false;
    bool prevarg = false;
    for(; rit != rend && !rit->str().empty(); ++rit) {
        string tok = (*rit)[1];

        if(impliedmult && tok != ")" && (tok == "(" || BINARY.count(tok) == 0)) {
            while(!opstack.empty()) {
                // Go ahead and evaluate higher priority operators before 
                // the current
                if(priority["*"] <= priority[opstack.front()]) {
                    outqueue.push_back(opstack.front());
                    opstack.pop_front();
                } else {
                    break;
                }
            }
            opstack.push_front("*");
        }

        if(tok == "(") {
            // Open Parenthetical
            opstack.push_front(tok);
            impliedmult = false;
            prevarg = false;
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
            impliedmult = true;
            prevarg = true;
        } else if(priority.count(tok) > 0) {
            // check for prefix +-
            if(!prevarg && tok == "+") {
                // ignore + prefix 
            } else if(!prevarg && tok == "-") {
                // prefix negate has highest priority
                opstack.push_front("neg");
            } else {
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
            }
            impliedmult = false;
            prevarg = false;
        } else {
            // argument
            outqueue.push_back(tok);

            impliedmult = true;
            prevarg = true;
        }
    }
	
    // Copy last operators to output queue
	while(!opstack.empty()) {
		if(opstack.front() == "(") 
            throw INVALID_ARGUMENT("Error, unmatched parentheses remaining");
		outqueue.push_back(opstack.front());
		opstack.pop_front();
	}
	
    cerr << "RPN:";
    for(auto it = outqueue.begin(); it != outqueue.end(); it++) {
        cerr << " " << *it;;
    }
    cerr << endl;
    printInfix(outqueue);
    Equation func(outqueue);
}

void printInfix(std::list<string> rpn)
{
    list<string> stack;
	for(auto it = rpn.begin(); it != rpn.end(); it++) {
        string tok = *it;
        if(BINARY.count(tok))  {
            string lhs, rhs;
            if(stack.size() < 2) 
                throw INVALID_ARGUMENT("Not Enough Arguments!");
            rhs = stack.back();
            stack.pop_back();
            lhs = stack.back();
            stack.pop_back();

            tok = "(" + lhs + tok + rhs + ")";
            stack.push_back(tok);
        } else if(UNARY.count(tok)) {
            if(stack.size() < 1)
                throw INVALID_ARGUMENT("Not Enough Arguments!");
            tok = tok + "(" + stack.back() + ")";
            stack.pop_back();
            stack.push_back(tok);
        } else {
            stack.push_back(tok);
        }
    }
    if(stack.size() != 1) 
        throw INVALID_ARGUMENT("Extra Arguments Left on Stack");
    cerr << "INF:" << stack.back() << endl;
}

string randstr() 
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    const int LEN = 32;
    std::uniform_int_distribution<int> dist(0, sizeof(alphanum)-1);

    string out(LEN, 'a');
    out[0] = 'k';
    for (int i = 1; i < out.size(); ++i) {
        out[i] = alphanum[dist(rng)];
    }
    return out;
}

bool isnumeric(string str)
{
    regex re("[+-]?\\d*(?:.\\d*)?(?:e[+-]?\\d*)?(?:.\\d*)?");
    cerr << "[+-]?\\d*(?:.\\d*)?(?:e[+-]?\\d*)?(?:.\\d*)?" << endl;
    cerr << str << endl;
    if(regex_match(str.begin(), str.end(), re)) {
        cerr << "matches" << endl;
        return true;
    }
    return false;
}

Equation::Equation(std::list<string> rpn)
{
	using namespace std::placeholders;
    
    args.clear();
    char* endptr;
    function<double()> lhs;
    function<double()> rhs;
    pair<unordered_map<string,shared_ptr<double>>::iterator, bool> inserted;

    list<function<double()>> stack;
	for(auto it = rpn.begin(); it != rpn.end(); it++) {
        string tok = *it;
        if(BINARY.count(tok))  {

            // pull out left and right hand sides
            if(stack.size() < 2) 
                throw INVALID_ARGUMENT("Not Enough Arguments!");
            
            // RHS 
            rhs = stack.back();
            stack.pop_back();
            
            // LHS
            lhs = stack.back();
            stack.pop_back();
            
            // push onto the stack so that we can get it later
            stack.push_back(
                        [lhs, rhs, tok](){
                            return BINARY[tok](lhs(), rhs()); 
                        });

        } else if(UNARY.count(tok)) {
            // pull out left and right hand sides
            if(stack.size() < 1) 
                throw INVALID_ARGUMENT("Not Enough Arguments!");
           
            // LHS
            auto lhs = stack.back();
            stack.pop_back();
            
            // push onto the stack so that we can get it later
            stack.push_back(
                        [lhs, tok](){
                            return UNARY[tok](lhs()); 
                        });
            
        } else {
            function<double()> foo;
            if(args.count(tok) > 0) {
                // bind this
                auto tmp = args[tok];
                cerr << "tok=" << tmp << endl;
                foo = [tmp, tok]() 
                { 
                    cerr << tok << " " << tmp << " " << *tmp << endl;
                    return *tmp; 
                };
            } else if(isnumeric(tok)) {
                // 
                double tmp = atof(tok.c_str());
                cerr << "const tok=" << tmp << endl;
                foo = [tmp, tok]() 
                { 
                    cerr << "const tok " << tok << " " << tmp << endl;
                    return tmp; 
                };
            } else {
                // need to create it
                args[tok].reset(new double);
                auto tmp = args[tok];
                cerr << "new tok=" << tmp << endl;
                foo = [tmp, tok]() 
                { 
                    cerr << tok << " " << tmp << " " << *tmp << endl;
                    return *tmp; 
                };
            }
            stack.push_back(foo);
        }
    }
	
    cerr << stack.size() << endl;
    cerr << stack.back()() << endl;
}

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

