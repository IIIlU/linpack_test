#include <iostream>
#include <limits>
#include "core.hpp"
#include "LinX.hpp"

using std::cout, std::cin, std::string;

LinX::Instruction getInst()
{
    cout << "Instruction Set\n";
    cout << "1. SSE4_2\n";
    cout << "2. AVX(1)\n";
    cout << "3. AVX2\n";
    cout << "4. AVX512\n";


    int input;
    do
    {
        cout << "(1 ~ 4): ";
        cin >> input;
    } while (input < 1 || input > 4);
    if(input == 1) return LinX::SSE4_2;
    else if(input == 2) return LinX::AVX;
    else if(input == 3) return LinX::AVX2;
    return LinX::AVX512;
}

int getInt(string s)
{
    int i;
    cout << s;
    cin >> i;
    return i;
}

bool isEqualEps(double lhs, double rhs)
{
    const double eps = std::numeric_limits<double>::epsilon();
    const double diff = lhs - rhs;
    if(diff > eps || diff < eps) return false;
    return true;
}

int main(int argc, char **argv)
{
    int numCore = getInt("Core Count: ");
    int iter = getInt("Iteration per Core: ");
    int probSize = getInt("Problem Size: ");
    LinX::Instruction inst = getInst();

    int mode;
    cout << "Mode 1: Test All Core at Same Time\n";
    cout << "Mode 2: Test All Core One by One\n";
    cout << "Mode 3: Test Specific Cores\n";
    do
    {
        cout << "(1 ~ 3): ";
        cin >> mode;
    } while (mode < 1 || mode > 3);
    
    Core *cores;
    int coresLength;
    LinX l{probSize, iter, (mode == 1 ? numCore * 2 : 2), inst};
    Program<LinX> p{l};
    
    if(mode == 1)
    {
        cores = new Core[1]{};
        coresLength = 1;
    }
    else if(mode == 2)
    {
        cores = new Core[numCore];
        for(int i = 0; i < numCore; ++i) cores[i] = Core{2 * i, 2 * i + 1};
        coresLength = numCore;
    }
    else
    {
        cout << "How Much Cores Are You Testing?: ";
        cin >> coresLength;
        cout << "Please Input Core Nums \n";
        cores = new Core[coresLength]{};
        for(int i = 0; i < coresLength; ++i)
        {
            cin >> mode;
            cores[i] = Core{2 * mode, 2 * mode + 1};
        }
    }

    cout << "Start Test\n";
    mode = 0;
    int result;
    double time;
    double gflop;
    double residual;
    double beforeResidual;
    for(int i = 0; i < coresLength; ++i)
    {
        cout << "Test " << i << "\n";
        auto r = cores[i].run(p);
        for(int j = 0; j < iter; ++j)
        {
            while(true)
            {
                help_sleep(300);
                
                result = r.parseResult(time, gflop, residual);
                if(result == 1) break;
                if(result == -1)
                {
                    mode = -1;
                    break;
                }
            }
            if(mode == -1) break;
            if(j == 0)
            {
                beforeResidual = residual;
                continue;
            }
            if(!isEqualEps(beforeResidual, residual))
            {
                mode = -1;
                break;
            }
        }
        if(mode == -1) break;
    }
    
    if(mode == -1) cout << "ERROR!\n";
    delete[] cores;
    return 0;
}