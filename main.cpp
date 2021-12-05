#include <iostream>
#include <limits>
#include <time.h>
#include "core.hpp"
#include "LinX.hpp"
#include "cpu_info.h"

void printTime()
{
    time_t curTime = time(nullptr);
    struct tm *pLocal = localtime(&curTime);
    printf("(%04d-%02d-%02d %02d:%02d:%02d)",  
    pLocal->tm_year + 1900, pLocal->tm_mon + 1, pLocal->tm_mday,  
    pLocal->tm_hour, pLocal->tm_min, pLocal->tm_sec);
}

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
    if(diff > eps || diff < -eps) return false;
    return true;
}

bool LinXTest(Core c, int thread, int probSize, int iter, LinX::Instruction inst)
{
    LinX l{probSize, iter, thread, inst};
    Program<LinX> p{l};
    double residual, beforeResidual;
    auto r = c.run(p);
    for(int j = 0; j < iter; ++j)
    {
        while(true)
        {
            help_sleep(300);
            double time, gflop;
            int result = r.parseResult(time, gflop, residual);
            if(result == 1) break;
            if(result == -1)
            {
                return false;
            }
        }
        if(j == 0)
        {
            beforeResidual = residual;
            continue;
        }
        if(!isEqualEps(beforeResidual, residual))
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    if(CPU_Info::isHybridCPU())
    {
        cout << "Maybe You Use AlderLake\n";
    }

    auto affinities = CPU_Info::getCoreAffinities();
    int numCore = affinities.size();
    int numThread = 0;
    
    std::vector<Core> cores{};

    for(int i = 0; i < numCore; ++i)
    {
        Core c;
        c = affinities[i];
        c.setIndex(i);
        numThread += c.threadCount();
        cores.push_back(c);
    }

    cout << "Core: " << numCore << "\n";
    Core allCore{};

    int iter = getInt("Iteration per Core: ");
    int probSize = getInt("Problem Size: ");
    LinX::Instruction inst = getInst();

    int mode;
    cout << "Mode 1: Test All Core at Same Time\n";
    cout << "Mode 2: Test Thread Usage\n";
    cout << "Mode 3: Test All Core One by One\n";
    cout << "Mode 4: Test Specific Cores\n";
    do
    {
        cout << "(1 ~ 4): ";
        cin >> mode;
    } while (mode < 1 || mode > 4);
    
    int buff;
    std::vector<int> testedCore{};
    
    if(mode == 1)
    {
        buff = numThread;   
    }
    else if(mode == 2)
    {
        cout << "Thread Number: ";
        cin >> buff;
    }
    else if(mode == 3)
    {
        for(int i = 0; i < numCore; ++i)
        {
            testedCore.push_back(i);
        }
    }
    else if(mode == 4)
    {
        int t;
        cout << "How Much Cores Are You Testing?: ";
        cin >> t;
        cout << "Please Input Core Nums \n";
        for(int i = 0; i < t; ++i)
        {
            cin >> buff;
            testedCore.push_back(buff);
        }
    }

    if(mode <= 2)
    {
        if(LinXTest(allCore, buff, probSize, iter, inst))
        {
            cout << "Success!!!\n";
        }
        else
        {
            cout << "Fail!!!\n";
        }
    }
    else
    {
        int testTime = testedCore.size();
        for(int i = 0; i < testTime; ++i)
        {
            int idx = testedCore[i];
            printTime();
            cout << " Core " << idx << " Test\n";
            if(LinXTest(cores[idx], cores[idx].threadCount(), probSize, iter, inst))
            {
                cout << "Success!!!\n";
            }
            else
            {
                cout << "Fail!!!\n";
                break;
            }
        }
    }
    return 0;
}