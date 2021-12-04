#ifndef LINX_HPP
#define LINX_HPP
#include "process.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

class LinX;

template <>
class Process<LinX> : public ProcessBase<LinX>
{
public:
    Process<LinX>(ProcessLib &&src) : ProcessBase<LinX>(std::move(src)) {}
    Process<LinX>(const std::string &cmd, const std::vector<std::string> &env, LinX *data, uint64_t affinity = (uint64_t) -1) 
        : ProcessBase<LinX>{cmd, env, data, affinity} {}
    Process<LinX>(Process<LinX> &src) = delete;
    Process<LinX>(Process<LinX> &&src) : ProcessBase<LinX>(std::move(src)) {}
    int parseResult(double &outTime, double &outGFlop, double &outNormResidual)
    {
        int size, LDA, Align;
        double residual;
        std::string Check;
        std::string result{ReadFromPipe()};
        std::stringstream sst;
        
        if(result.empty()) return 0;
        sst.str(result);
        sst >> size >> LDA >> Align >> outTime >> outGFlop >> residual >> outNormResidual >> Check  ;
        std::cout << std::left;
        std::cout << std::setw(7) << size;
        std::cout << std::setw(7) << LDA;
        std::cout << std::setw(7) << Align;
        std::cout << std::setw(11) << outTime;
        std::cout << std::setw(9) << outGFlop;
        std::cout << std::setw(13) << residual;
        std::cout << std::setw(15) << outNormResidual;
        std::cout << std::setw(7) << Check << "\n";
        return Check == "pass" ? 1 : -1;
    }
};

class LinX
{
public:
    enum Instruction
    {
        AVX512,
        AVX2,
        AVX,
        SSE4_2
    };
private:
    int nThread;
    std::string inst;
    int probSize;
    int trial;
public:
    LinX(int probSize, int trial, int nThread, Instruction tInst=AVX2) : nThread(nThread), probSize(probSize), trial(trial)
    {
        if(tInst == AVX512) inst = "AVX512";
        else if(tInst == AVX2) inst = "AVX2";
        else if(tInst == AVX) inst = "AVX";
        else inst = "SSE4_2";
    };

    LinX(const LinX &src) : nThread(src.nThread), inst(src.inst), probSize(src.probSize), trial(src.trial){}

    std::string getCmd()
    {
        return "library\\linpack_xeon64.exe";
    }

    std::vector<std::string> getEnvs()
    {
        return std::vector<std::string>{ "MKL_DYNAMIC=FALSE", "OMP_NUM_THREADS=" + std::to_string(nThread), 
        "MKL_ENABLE_INSTRUCTIONS=" + inst};
    }

    Process<LinX> runAfter(Process<LinX> &&p)
    {
        p.WriteToPipe("a\r\nTEST\r\n1\r\n" + std::to_string(probSize) + "\r\n" + std::to_string(probSize) + "\r\n" + std::to_string(trial) + "\r\n4\r\n");
        help_sleep(300);
        p.ReadFromPipe();
        std::cout << "Size   LDA    Align. Time(s)    GFlops   Residual     Residual(norm) Check\n";
        return std::move(p);
    }
};



#endif