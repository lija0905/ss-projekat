#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "../inc/cpu.hpp"

#include <cstdio>
#include <iostream>
#include <limits>
#include <string>

#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>

using namespace std;

#define term_in -254
#define term_out -256

class Terminal {

public: 
    static int terminal_descriptor;

    static void terminalDone();
    static void terminalSignal(int signum);

    Terminal(CPU *cpu);

    int terminalInit();
    void setRerminalRawMode();
    void setTerminalOriginalMode();
    void stdinput();

    ~Terminal();
private: 
    CPU *cpu;

};


#endif