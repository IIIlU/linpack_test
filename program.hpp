#ifndef PROGRAM_HPP
#define PROGRAM_HPP
#include <string>

template<typename T>
class Process;

template<typename T>
class ProgramBase
{
protected:
    T data;
public:
    ProgramBase(T &t): data(t) {};
    
    std::string getCmd()
    {
        return data.getCmd();
    }

    std::vector<std::string> getEnvs()
    {
        return data.getEnvs();
    }

    Process<T> runAfter(Process<T> &&p)
    {
        return data.runAfter(std::move(p));
    }

    T *getData()
    {
        return new T{data};
    }
};

template<typename T>
class Program : public ProgramBase<T>
{
public:
    Program<T>(T &t) : ProgramBase<T>(t) {}
};

#endif