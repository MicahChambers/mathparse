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

