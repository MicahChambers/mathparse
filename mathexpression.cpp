#include <string>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <functional>
#include <numeric>
#include <cstdlib>
#include <list>
//#include <algorithm>
#include <cassert>
#include <random>
#include <memory>

#include <stdexcept>

#define INVALID_ARGUMENT(EXP) \
std::invalid_argument(__PRETTY_FUNCTION__+std::string(" -> ")+std::string(EXP))

std::default_random_engine rng;

using namespace std;


unordered_map<string,int> PRIORITY({{"exp", 5}, {"cos", 5}, {"sin", 5},
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

/**
 * @brief Constructor.
 *
 * @param eq String represntation of equation. Infix format unless rpn is
 * true 
 * @param rpn if true, then the equation is assumed to be
 * Reverse-Polish-Notation
 */
Equation::Equation(string eq, bool rpn)
{
    auto tokens = tokenize(eq);
    if(!rpn) 
        m_rpn = tokens;
    else
        m_rpn = infixreorder(tokens);

    args.clear();
    function<double()> lhs;
    function<double()> rhs;
    pair<unordered_map<string,shared_ptr<double>>::iterator, bool> inserted;

    list<function<double()>> stack;
    for(auto it = m_rpn.begin(); it != m_rpn.end(); it++) {
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
                    double lhsv = lhs();
                    double rhsv = rhs();
                    cerr << lhsv << tok << rhsv << endl;
                    return BINARY[tok](lhsv, rhsv); 
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
                    double lhsv = lhs();
                    cerr << tok << lhsv << endl;
                    return UNARY[tok](lhsv); 
                    });

        } else {
            function<double()> foo;
            char* end = NULL;
            double v = strtod(tok.c_str(), &end);
            if((end - tok.c_str()) == (int)tok.size()) {
                // number 
                cerr << "const tok=" << v << endl;
                foo = [v, tok]() 
                { 
                    cerr << "const tok " << tok << " " << v << endl;
                    return v; 
                };
            } else if(args.count(tok) > 0) {
                // bind this
                auto tmp = args[tok];
                cerr << "tok=" << tmp << endl;
                foo = [tmp, tok]() 
                { 
                    cerr << tok << " " << tmp << " " << *tmp << endl;
                    return *tmp; 
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

    executor = stack.back();
}
    
/**
 * @brief Sets variable (argument in the math equation
 *
 * @param arg Set an arg value
 * @param val to set arg to
 *
 * @return error if != 0
 */
int Equation::setarg(char arg, double val)
{
    string c = " ";
    c[0] = arg;
    auto it = args.find(c);
    if(it == args.end()) {
        cerr << arg << " not found in equation!" << endl;
        return -1;
    }
    *it->second = val;

    return 0;
}

/**
 * @brief return the current value of a named argument in val
 *
 * @param arg Input, argument to check the value of 
 * @param val Return value
 *
 * @return error if != 0
 */
int Equation::getarg(char arg, double& val)
{
    string c = " ";
    c[0] = arg;
    auto it = args.find(c);
    if(it == args.end()) {
        cerr << arg << " not found in equation!" << endl;
        return -1;
    }
    val = *it->second;

    return 0;
}

/**
 * @brief Performs the expression and returns the result
 *
 * @return 
 */
double Equation::exec()
{
    return executor();
}

/**
 * @brief Print the expression as polish notation (PN)
 */
void Equation::printPN()
{
    cerr << "PN:";
	for(auto it = m_rpn.rbegin(); it != m_rpn.rend(); it++) {
        cerr << " " << *it;
    }
    cerr << endl;
}

/**
 * @brief Print the expression as reverse-polish notation (RPN)
 */
void Equation::printRPN()
{
    cerr << "RPN:";
    for(auto it = m_rpn.begin(); it != m_rpn.end(); it++) {
        cerr << " " << *it;
    }
    cerr << endl;
}

/**
 * @brief Print the expression as infix
 */
void Equation::printInfix()
{
    list<string> stack;
    for(auto it = m_rpn.begin(); it != m_rpn.end(); it++) {
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
    cerr << "INFIX:" << stack.back() << endl;
}


/**
 * @brief Helper function, turns a raw string into tokens
 *
 * @param exp Expression to turn into tokens
 *
 * @return list of tokens
 */
list<string> Equation::tokenize(string exp)
{
    cerr << "Equation: " << exp << endl;
    bool restart = true; // restart loop
    list<string> out;
    string singlechar = " ";
    for(size_t ii=0; ii<exp.size();) {
        restart = false;
        while(ii < exp.length() && isspace(exp[ii]))
            ii++;

        if(exp[ii] == ')' || exp[ii] == '(') {
            singlechar[0] = exp[ii];
            out.push_back(singlechar);
            ii++;
            continue;
        }

        // check for operators
        for(auto& v : PRIORITY) {
            if(exp.compare(ii, v.first.length(), v.first) == 0) {
                out.push_back(v.first);
                ii += v.first.length();
                restart = true;
                break;
            }
        }
        if(restart) continue;

        // check for char
        if(isalpha(exp[ii])) {
            singlechar[0] = exp[ii];
            out.push_back(singlechar);
            ii++;
            continue;
        }

        // otherwise assume its a number
        char* end;
        strtod(&exp.c_str()[ii], &end);
        if(end == &exp.c_str()[ii]) {
            throw INVALID_ARGUMENT("Unknown character: "+exp[ii]);
        } else {
            size_t len = ((end-&exp.c_str()[ii]));
            out.push_back(exp.substr(ii, len));
            ii += len;
        }
    }

    return out;
}

/**
 * @brief Helper function that reorder tokens based on their priority
 * so that infix is turned into RPN.
 *
 * @param list of tokens
 *
 * @return list of tokens, now in RPN
 */
list<string> Equation::infixreorder(list<string> tokens) 
{
    list<string> opstack;
    list<string> outqueue;
    string prev = "";
    bool impliedmult = false;
    bool prevarg = false;
    for(auto it=tokens.begin(); it != tokens.end(); ++it) {
        string tok = *it;
        cerr << tok << endl;
        if(impliedmult && tok != ")" && (tok == "(" || BINARY.count(tok) == 0)) {
            while(!opstack.empty()) {
                // Go ahead and evaluate higher PRIORITY operators before 
                // the current
                if(PRIORITY["*"] <= PRIORITY[opstack.front()]) {
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
        } else if(PRIORITY.count(tok) > 0) {
            // check for prefix +-
            if(!prevarg && tok == "+") {
                // ignore + prefix 
            } else if(!prevarg && tok == "-") {
                // prefix negate has highest PRIORITY
                opstack.push_front("neg");
            } else {
                // Add latest operator to stack until we find lower PRIORITY op
                while(!opstack.empty()) {
                    // Go ahead and evaluate higher PRIORITY operators before 
                    // the current
                    if(PRIORITY[tok] <= PRIORITY[opstack.front()]) {
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

    return outqueue;
}

