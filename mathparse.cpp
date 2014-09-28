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
#include <memory>

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

void printInfix(std::list<string> rpn);
list<string> tokenize(string exp);
list<string> infixreorder(list<string> exp);

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


int main(int argc, char** argv)
{
    if(argc != 2) {
        cerr << "Need 1 argument: an equation" << endl;
        return -1;
    }
    auto tokens = tokenize(argv[1]);
    auto rpn = infixreorder(tokens);
    cerr << "RPN:";
    for(auto it = rpn.begin(); it != rpn.end(); it++) {
        cerr << " " << *it;;
    }
    cerr << endl;
    printInfix(rpn);
    Equation func(rpn);
}

list<string> infixreorder(list<string> tokens) 
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

Equation::Equation(std::list<string> rpn)
{
	using namespace std::placeholders;
    
    args.clear();
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
	
    cerr << stack.size() << endl;
    cerr << stack.back()() << endl;
}

list<string> tokenize(string exp)
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
