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
 * @file main.cpp Function that tests the math expression function for 
 * a novel input argument.
 *
 *****************************************************************************/

#include <iostream>
#include "mathexpression.h"

using namespace std;

int main(int argc, char** argv)
{
    if(argc != 2) {
        cerr << "Need 1 argument: an equation" << endl;
        return -1;
    }
    MathExpression func(string(argv[1]), true);
    func.printInfix();
    func.printRPN();
    func.printPN();

    cerr << "Computing With: " << endl;
    for(auto it = func.begin(); it != func.end(); ++it) {
        it->second = rand()%20-10;
        cerr << it->first << "=" << it->second;
    }
    cerr << " => " << func.exec() << endl;
}


