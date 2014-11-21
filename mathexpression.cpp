/******************************************************************************
 * Copyright 2014 Micah C Chambers (micahc.vt@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file mathexpression.cpp MathExpression library, for declaring and computing
 * arbitrary math at runtime.
 *
 *****************************************************************************/

#include "mathexpression.h"

#include <string>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <functional>
#include <iomanip>
#include <cstdlib>
#include <list>
#include <cassert>
#include <random>
#include <memory>

#include <stdexcept>

#define INVALID_ARGUMENT(EXP) \
std::invalid_argument(__PRETTY_FUNCTION__+std::string(" -> ")+std::string(EXP))

std::default_random_engine rng;

using namespace std;


unordered_map<string,int> PRIORITY({{"exp", 5}, {"cos", 5}, {"sin", 5}, {"tan",
        5}, {"log", 5}, {"+", 3}, {"-",3}, {"*", 4}, {"/", 4}, {"==", 2}, {"^",
        6}, {"neg", 7}, {"<=", 2}, {">=", 2}, {"<", 2}, {">", 2}, {"ceil", 5},
        {"abs", 5}, {"round", 5}, {"floor", 5}});

unordered_map<string,function<double(double)>> UNARY({
        {"exp",pointer_to_unary_function<double,double>(exp)},
        {"cos",pointer_to_unary_function<double,double>(cos)},
        {"sin",pointer_to_unary_function<double,double>(sin)},
        {"tan",pointer_to_unary_function<double,double>(tan)},
        {"neg",std::negate<double>()},
        {"abs",pointer_to_unary_function<double,double>(abs)},
        {"round",pointer_to_unary_function<double,double>(round)},
        {"floor",pointer_to_unary_function<double,double>(floor)},
        {"ceil",pointer_to_unary_function<double,double>(ceil)},
        {"log",pointer_to_unary_function<double,double>(log)}});

unordered_map<string,function<double(double,double)>> BINARY({
        {"+",std::plus<double>()},
        {"-",std::minus<double>()},
        {"*",std::multiplies<double>()},
        {"/",std::divides<double>()},
        {"==",std::equal_to<double>()},
        {"<", std::less<double>()},
        {">",std::greater<double>()},
        {"<=", std::less_equal<double>()},
        {">=",std::greater_equal<double>()},
        {"^",pointer_to_binary_function<double,double,double>(std::pow<double>)}
        });

void listops()
{
    cerr << '\t' << left << setw(6) << "Op" << setw(10) << "Priority" << endl;
    for(auto it=PRIORITY.begin(); it != PRIORITY.end(); ++it) {
        cerr << '\t' << left << setw(6) << it->first << setw(10) << it->second << endl;
    }
    cerr << endl;
}

/**
 * @brief Constructor.
 *
 * @param eq String represntation of equation. Infix format unless rpn is
 * true 
 * @param rpn if true, then the equation is assumed to be
 * Reverse-Polish-Notation
 */
MathExpression::MathExpression(string eq, bool rpn)
{
    auto tokens = tokenize(eq);
    if(rpn) 
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
#ifdef VERYDEBUG
                    cerr << lhsv << tok << rhsv << "=" << BINARY[tok](lhsv, rhsv) << endl;
#endif 
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
#ifdef VERYDEBUG
                    cerr << tok << lhsv << "=" << UNARY[tok](lhsv) << endl;
#endif
                    return UNARY[tok](lhsv); 
                    });

        } else {
            function<double()> foo;
            char* end = NULL;
            double v = strtod(tok.c_str(), &end);
            if((end - tok.c_str()) == (int)tok.size()) {
                // number 
#ifdef VERYDEBUG
                cerr << "const tok=" << v << endl;
#endif
                foo = [v, tok]() 
                { 
#ifdef VERYDEBUG
                    cerr << "const tok " << tok << " " << v << endl;
#endif
                    return v; 
                };
            } else if(args.count(tok) > 0) {
                // bind this
                auto tmp = args[tok];
#ifdef VERYDEBUG
                cerr << "tok=" << tmp << endl;
#endif
                foo = [tmp, tok]() 
                { 
#ifdef VERYDEBUG
                    cerr << tok << " " << tmp << " " << *tmp << endl;
#endif
                    return *tmp; 
                };
            } else {
                // need to create it
                args[tok].reset(new double);
                auto tmp = args[tok];
#ifdef VERYDEBUG
                cerr << "new tok=" << tmp << endl;
#endif
                foo = [tmp, tok]() 
                { 
#ifdef VERYDEBUG
                    cerr << tok << " " << tmp << " " << *tmp << endl;
#endif
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
int MathExpression::setarg(char arg, double val)
{
    string c = " ";
    c[0] = arg;
    auto it = args.find(c);
    if(it == args.end()) {
//        cerr << arg << " not found in equation!" << endl;
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
int MathExpression::getarg(char arg, double& val)
{
    string c = " ";
    c[0] = arg;
    auto it = args.find(c);
    if(it == args.end()) {
//        cerr << arg << " not found in equation!" << endl;
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
double MathExpression::exec()
{
    return executor();
}

void MathExpression::randomTest()
{
    cerr << "Equation: ";
    printInfix();
    cerr << "Using: \n";
    for(auto it=args.begin(); it != args.end(); ++it) {
        *it->second = rand()/(double)RAND_MAX-.5;
        cerr << it->first << "=" << *it->second << endl;
    }
    
    cerr << "Eval: " << exec() << endl;
}


/**
 * @brief Print the expression as polish notation (PN)
 */
void MathExpression::printPN()
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
void MathExpression::printRPN()
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
void MathExpression::printInfix()
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
list<string> MathExpression::tokenize(string exp)
{
    cerr << "MathExpression: " << exp << endl;
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
            throw INVALID_ARGUMENT("Unknown character: "+to_string(exp[ii]));
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
list<string> MathExpression::infixreorder(list<string> tokens) 
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

