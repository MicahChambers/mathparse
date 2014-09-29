
#include "mathexpression.h"

#include <iostream>

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

    func.setarg('a', 10);
    cerr << func.exec() << endl;
    
    func.setarg('a', 20);
    cerr << func.exec() << endl;
}


