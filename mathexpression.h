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
 * @file mathexpression.h MathExpression library, for declaring and computing
 * arbitrary math at runtime.
 *
 *****************************************************************************/

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <list>

/**
 * @brief Class for parsing and evaluating math equations from text.
 */
class MathExpression
{
public:

    /**
     * @brief Constructor.
     *
     * @param eq String represntation of equation. Infix format unless rpn is
     * true 
     * @param rpn if true, then the equation is assumed to be
     * Reverse-Polish-Notation
     */
    MathExpression(std::string eq, bool rpn = false);

    /**
     * @brief Sets variable (argument in the math equation
     *
     * @param arg Set an arg value
     * @param val to set arg to
     *
     * @return error if != 0
     */
    int setarg(char arg, double val);


    /**
     * @brief return the current value of a named argument in val
     *
     * @param arg Input, argument to check the value of 
     * @param val Return value
     *
     * @return error if != 0
     */
    int getarg(char arg, double& val);

    /**
     * @brief Performs the expression and returns the result
     *
     * @return 
     */
    double exec();

    /**
     * @brief Print the expression as infix
     */
    void printInfix();

    /**
     * @brief Print the expression as reverse-polish notation (RPN)
     */
    void printRPN();

    /**
     * @brief Randomly assign each of the arguments, execute the function and
     * return the result.
     */
    void randomTest();
    
    /**
     * @brief Print the expression as polish notation (PN)
     */
    void printPN();

    /**
     * @brief Get an iterator for the the map of variables 
     *
     * @return Iterator that goes through the variables
     */
    std::unordered_map<std::string,std::shared_ptr<double>>::iterator begin()
    {
        return args.begin(); 
    };
    
    /**
     * @brief Get an iterator for the end of the the map of variables 
     *
     * @return Iterator, at end of variable map
     */
    std::unordered_map<std::string,std::shared_ptr<double>>::iterator end()
    {
        return args.end(); 
    };

    /**
     * @brief Get an iterator for the the map of variables 
     *
     * @return Iterator that goes through the variables
     */
    std::unordered_map<std::string,std::shared_ptr<double>>::const_iterator cbegin()
    {
        return args.cbegin(); 
    };
    
    /**
     * @brief Get an iterator for the end of the the map of variables 
     *
     * @return Iterator, at end of variable map
     */
    std::unordered_map<std::string,std::shared_ptr<double>>::const_iterator cend()
    {
        return args.cend(); 
    };

private:
    /**
     * @brief Storage for the variables, maps a string to its current value 
     */
    std::unordered_map<std::string, std::shared_ptr<double>> args;

    /**
     * @brief Helper function, turns a raw string into tokens
     *
     * @param exp Expression to turn into tokens
     *
     * @return list of tokens
     */
    std::list<std::string> tokenize(std::string exp);

    /**
     * @brief Helper function that reorder tokens based on their priority
     * so that infix is turned into RPN.
     *
     * @param list of tokens
     *
     * @return list of tokens, now in RPN
     */
    std::list<std::string> infixreorder(std::list<std::string> exp);

    /**
     * @brief Function that  calls math equations
     */
    std::function<double()> executor;

    /**
     * @brief MathExpression stored in RPN format. Mostly just for printing.
     */
    std::list<std::string> m_rpn;

};


/**
 * @brief Lists all the operations available.
 */
void listops();

