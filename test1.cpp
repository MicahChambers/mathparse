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
 * @file test1.cpp Test for the MathExpression library
 *
 *****************************************************************************/

#include <iostream>
#include <cmath>
#include "mathexpression.h"

using namespace std;

int main()
{
    {
        MathExpression func("3*3^32/1.e-3", true);
        func.printInfix();
        func.printRPN();
        func.printPN();

        if(func.exec() != 3*pow(3,32)/1.e-3) {
            cerr << "ERROR!" << endl;
            return -1;
        }
    }
    {
        MathExpression func("3*3^-32/1.e-3", true);
        func.printInfix();
        func.printRPN();
        func.printPN();

        if(func.exec() != 3*pow(3,-32)/1.e-3) {
            cerr << "ERROR!" << endl;
            return -1;
        }
    }
    {
        MathExpression func("3-3e5-1*5/1.e-3", true);
        func.printInfix();
        func.printRPN();
        func.printPN();

        if(func.exec() != 3-3e5-1*5/1.e-3) {
            cerr << "ERROR!" << endl;
            return -1;
        }
    }
    
    return 0;
}

