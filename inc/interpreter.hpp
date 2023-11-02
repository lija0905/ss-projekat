#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "../inc/cpu.hpp"
#include "../inc/terminal.hpp"
#include <iostream>
#include <fstream>
#include <string.h>
#include <iomanip>
#include <map>

using namespace std;

enum Instructions {
    HALT, IRET, RET, CALL, JMP, JEQ, JNE, JGT, INT, NOT, XCHG, ADD, SUB, MUL, DIV, CMP, AND, 
    OR, XOR, TEST, SHL, SHR, LDR, STR, PUSH, POP
};

enum AddressType {
    IMM = 0, REGDIR = 1, REGDIRS = 2, REGINDIR = 3, REGINDIRS = 4, MEM = 5
};

class Interpreter {

  
  static map<unsigned char, Instructions> instructions;
  static int stop;


  public:
    Interpreter() {
      cpu = new CPU();
      terminal = new Terminal(cpu);
      pc_rel = 0;
    }

    ~Interpreter() {
      terminal->~Terminal();
      cpu->~CPU();
    }

    void loadMem(const char* file);
    void readFile();
    void emulating();
    void print();
        int resolveSizeOfInstruction(unsigned char instr);
    void execute(unsigned char instr, unsigned char reg, unsigned char type, unsigned char data_high, unsigned char data_low);
    void resolveAddr(unsigned char instr, int reg, int type);
    short calculateOperand(unsigned char instr, unsigned char reg, unsigned char type, unsigned char data_high, unsigned char data_low);

    //instructions
    void halt();
    void intInstr(unsigned char reg);
    void iret();
    void call(short operand, short type);
    void ret();
    void jmp(short operand, short type);
    void jeq(short operand, short type);
    void jne(short operand, short type);
    void jgt(short operand, short type);
    void push(char regD);
    void pop(char regD);
    void xchg(char regD, char regS);
    void add(char regD, char regS);
    void sub(char regD, char regS);
    void mul(char regD, char regS);
    void div(char regD, char regS);
    void cmp(char regD, char regS);
    void notInstr(char regD);
    void andInstr(char regD, char regS);
    void orInstr(char regD, char regS);
    void xorInstr(char regD, char regS);
    void testInstr(char regD, char regS);
    void shl(char regD, char regS);
    void shr(char regD, char regS);
    void ldr(char regD,short operand);//6 in regS indicates it is not ldr, str but push and pop
    void str(char regD, char regS, char type, short operand);
    void handleTermInterrupt();



  private: 
    CPU *cpu;
    Terminal *terminal;
    int pc_rel;


};



#endif