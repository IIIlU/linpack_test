#ifndef PROCESS_LIBRARY_H
#define PROCESS_LIBRARY_H

#include <string>
#include <vector>

struct ProcessLibData;
class ProcessLib
{
private:
    ProcessLibData *data;
public:
    static uint64_t systemAffinity;

    ProcessLib(const std::string &cmd, const std::vector<std::string> &env, uint64_t affinity);
    ProcessLib(const ProcessLib &rhs) = delete;
    ProcessLib(ProcessLib &&src);

    bool WriteToPipe(const std::string &input);
    std::string ReadFromPipe(void);
    
    bool changeAffinity(uint64_t affinity);
    bool is_running();
    bool terminate();
    ~ProcessLib();
};
void help_sleep(int ms);
#endif