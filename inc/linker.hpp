#ifndef LINKER_H_
#define LINKER_H_

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include "../inc/types.hpp"

using namespace std;

class Linker {

  public: 
    Linker() {
      global_symbol_table = new list<sym_entry>();
      all_sections = new list<section_mapped_addresses>();
      memory = vector<list<memory_el>*>();
    }

    void readFile(const char* file, int num);
    void skipLines(ifstream& file, int num);
    void splitString(std::vector<std::string>& tokens, string line);
    void createContent(std::vector<std::string>& tokens, string section, int file);
    void createRelocation(std::vector<std::string>& tokens, string section, int file);
    void allocateSectionsDefault();
    void allocateSectionsWithPlaces(list<section_mapped_addresses>* places);
    void updateSymbolsAddresses();
    void updateContentAddresses();
    void resolveRelocations();
    void formMemory();
    int getStartOfSection(string section, int file);
    bool checkIfSymbolExists(string symbol, int global);
    bool sectionAddedToSymTable(string section, int section_size);
    bool checkIfSectionsAreOverlaping();
    void addRelocationRecord(reloc& reloc);
    int getSymbolAddress(string symbol);
    void makeHexFile(const char* file);
    void patchContent(string section, int file, int p, int content);
    void printTable();
    void printTables();
    void printSections();
    void printContent();
    void printRelocation();
    void printRelocations();
    void printMemory();



  private: 
    list<sym_entry> *global_symbol_table;
    list<section_mapped_addresses>* all_sections;
    vector<list<sym_entry>*> symbol_tables;
    vector<list<memory_el>*> memory;

};

#endif