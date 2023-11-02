#ifndef _CPU_H_
#define _CPU_H_

#include <iostream> 
#include <string.h>

using namespace std;

class CPU {

  public: 
    unsigned short reg[8] = {};
    unsigned short psw;
    unsigned char *mem;
    int int_ev = 0;

    CPU() {
       this->mem = new unsigned char[65536];
       this->psw = (1 << 13);
    }

    ~CPU() {
      delete[] this->mem;
    }

    void setI(int i);
    void setZ(int i);
    void setC(int i);
    void setN(int i);
    void setO(int i);

    short getI();
    short getZ();
    short getC();
    short getN();
    short getO();

};


#endif