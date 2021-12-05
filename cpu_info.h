#ifndef CPU_INFO_H
#define CPU_INFO_H
#include <vector>
#include <stdint.h>

namespace CPU_Info
{
    bool isHybridCPU();
    std::vector<uint64_t> getCoreAffinities();
};

#endif