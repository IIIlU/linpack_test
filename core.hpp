#ifndef CORE_HPP
#define CORE_HPP
#include "process_library.h"
#include "program.hpp"
#include "process.hpp"

class Core
{
private:
    uint64_t affinity;
    int index;
public:
    Core() : affinity((uint64_t) -1), index{-1} {};
    
    Core(const Core& src) : affinity(src.affinity), index{-1} {};

    void setIndex(int i)
    {
        index = i;
    }

    int getIndex()
    {
        return index;
    }

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
        index = -1;
        affinity = 1;
        affinity = affinity << t;
    }

    template<typename T, typename... types>
    Core(T t, types... vals)
    {
        index = -1;
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