#include <regex>
#include <string>
#include <iostream>

//UnorderedMap<string,function<double()> operations;
//
//UnorderedMap<string,function<double(double)>> UNARY({"exp","cos","sin","tan","log"});
//UnorderedMap<string,function<double(double,double)>> LEFTBINARY({"+","-","*","/"});
//UnorderedMap<string,function<double(double,double)>> RIGHTBINARY({"^"});

using namespace std;

int main(int argc, char** argv)
{
    if(argc != 2) {
        cerr << "Need 1 argument: an equation" << endl;
        return -1;
    }

    string eq(argv[1]);
    cerr << "Equation: " << eq << endl;
    regex re("\\s*(exp|cos|sin|tan|log|\\+|-|\\*|/|^|[0-9]*|[a-z])");
    regex_iterator<string::iterator> rit (eq.begin(), eq.end(), re,
            regex_constants::match_continuous);
    regex_iterator<string::iterator> rend;
    for(; rit != rend; ++rit) {
        cout << (*rit)[1] << endl; 
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
