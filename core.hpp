#ifndef CORE_HPP
#define CORE_HPP
#include "process_library.h"
#include "program.hpp"
#include "process.hpp"

class Core
{
private:
    uint64_t affinity;
public:
    Core() : affinity((uint64_t) -1) {};
    
    Core(const Core& src) : affinity(src.affinity) {};

    Core &operator=(const Core &src)
    {
        affinity = src.affinity;
        return *this;
    }

    Core &operator+=(const Core &rhs)
    {
        affinity |= rhs.affinity;
        return *this;
    }

    Core operator+(const Core &rhs)
    {
        Core lhs = *this;
        return lhs += rhs;
    }

    Core &operator*=(const Core &rhs)
    {
        affinity &= rhs.affinity;
        return *this;
    }

    Core operator*(const Core &rhs)
    {
        Core lhs = *this;
        return lhs *= rhs;
    }

    Core &operator-=(const Core &rhs)
    {
        affinity &= !rhs.affinity;
        return *this;
    }

    Core operator-(const Core &rhs)
    {
        Core lhs = *this;
        return lhs -= rhs;
    }
    
    template<typename T>
    Core(T t)
    {
        affinity = 1;
        affinity = affinity << t;
    }

    template<typename T, typename... types>
    Core(T t, types... vals)
    {
        affinity = 1;
        affinity = affinity << t;
        affinity |= Core{vals...}.affinity;
    }

    template<typename T>
    Process<T> run(Program<T> t)
    {
        Process<T> p{t.getCmd(), t.getEnvs(), t.getData(), affinity};
        return t.runAfter(std::move(p));
    }
};

#endif