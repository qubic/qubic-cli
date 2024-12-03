#pragma once

#include <string>
#include <iostream>

void promptStdin(const char* desc, char* output, int _len)
{
    memset(output, 0, _len+1);
    std::string inp;
    std::cout << desc << ": ";
    std::getline (std::cin,inp);
    if (!inp.empty() && inp[inp.length()-1] == '\n')
    {
        inp.erase(inp.length()-1);
    }
    int len = std::min(int(inp.size()), _len);
    memcpy(output, inp.data(), len);
}