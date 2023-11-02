#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include "types.hpp"
#include <iostream>
#include <fstream> 
#include <iomanip>  
 
using namespace std;

class Assembler {

 public: 
    Assembler() {
        symbol_table = new list<sym_entry>();
    }

    void addSymbol(string label, string section, int offset, int global, int section_size);
    void addSymbols(list<dummy_symbol> *symbols, string section, int offset, int global, int section_size);
    void setSectionSize(string section, int size);
    void resolveLabel(string symbol, string section, int offset);
    void setGlobal(string symbol);
    void addByteToSection(string section, short byte);
    void addBytesToSection(string section, list<dummy_symbol> *symbols);
    void addToPatchList(string symbol, string section, int address, int current_loc_counter, int flag_pc_rel = 0);
    short getLastByte(string section);
    int getAddressToPatch(string section);
    list<section_content>* findContent(string label, int flag);
    list<reloc>* getRelocRecordsList(string label);
    int getSymbolAddr(string label);
    void crateRelocationRecord();
    string symbolExists(string label);
    void makeOutputFile(const char* file);
    void printTable();
    void printContent();
    void printPatch(string symbol);
    void printRelocationRecords();

 private: 
    list<sym_entry> *symbol_table;



};



#endif