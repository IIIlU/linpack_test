#include "cpu_info.h"
#include <cpuid.h>
#include <algorithm>
#include "windows.h"

struct cpuids
{
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
};

cpuids getCpuId(unsigned int eax)
{
    cpuids c;
    c.eax = eax;
    __get_cpuid(0, &c.eax, &c.ebx, &c.ecx, &c.edx);
    return c;
}

bool CPU_Info::isHybridCPU()
{
    cpuids s = getCpuId(0x07);
    return (s.ecx >> 15) & 1 == 1;
}

std::vector<uint64_t> CPU_Info::getCoreAffinities()
{
    std::vector<uint64_t> v;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION (*ptr)[500] 
        = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION (*)[500])new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[500]{};
    long unsigned int length = (sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) * 500);
    GetLogicalProcessorInformation((SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) ptr, &length);
    length = length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for(int i = 0; i < length; ++i)
    {
        if((*ptr)[i].Relationship == RelationProcessorCore)
        {
            v.push_back((*ptr)[i].ProcessorMask);
        }
    }
    sort(v.begin(), v.end());
    delete[] ptr;
    return v;
}
