#ifndef PROCESS_HPP
#define PROCESS_HPP
#include "process_library.h"

template <typename T>
class ProcessBase
{
protected:
    ProcessLib pData;
    T *data;
public:
    ProcessBase(const std::string &cmd, const std::vector<std::string> &env, T *data, uint64_t affinity = (uint64_t) -1) 
        : pData{cmd, env, affinity}, data{data} {}
    ProcessBase(ProcessLib &&src) : pData(std::move(src)) {}
    ProcessBase(ProcessBase<T> &src) = delete;
    ProcessBase(ProcessBase<T> &&src) : pData(std::move(src.pData)), data(src.data) 
    {
        src.data = nullptr;
    }
    
    bool WriteToPipe(const std::string &input)
    {
        return pData.WriteToPipe(input);
    }

    std::string ReadFromPipe(void)
    {
        return pData.ReadFromPipe();
    }

    bool is_running()
    {
        return pData.is_running();
    }

    bool terminate()
    {
        return pData.terminate();
    }

    ~ProcessBase()
    {
        delete data;
    }
};

template <typename T>
class Process : public ProcessBase<T>
{
public:
    Process<T>(ProcessLib &&src) : ProcessBase<T>(std::move(src)) {}
    Process<T>(const std::string &cmd, const std::vector<std::string> &env, T *data, uint64_t affinity = (uint64_t) -1) 
        : ProcessBase<T>{cmd, env, data, affinity} {}
    Process<T>(Process<T> &src) = delete;
    Process<T>(Process<T> &&src) : ProcessBase<T>(std::move(src)) {}
};

#endif