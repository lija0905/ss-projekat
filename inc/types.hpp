#ifndef TYPES_H_
#define TYPES_H_

#include <string.h>
#include <list>
#include <iostream>
using namespace std;


typedef struct patch_entry {
  int addr;
  string section;
  int current_loc_counter;
  int flag_pc_rel;
  int deleted;
} patch_entry;

typedef struct section_content {
  static int count_content;
  int offset;
  short content;
} section_content;

typedef struct reloc {
  string section;
  int offset;
  string type;
  string order;
  int addend;
} reloc;

typedef struct sym_entry
{
  /* data */
  static int count;
  string label;
  string section;
  int offset;
  int global;
  int order;
  int section_size;
  list<patch_entry> *patch_addr = nullptr;
  list<section_content>* content = nullptr;
  list<reloc>* relocation_record = nullptr;

  sym_entry(string label, string section, int offset, int global, int section_size) {
    this->label = label;
    this->section = section;
    this->offset = offset;
    this->global = global;
    this->section_size = section_size;
    this->order = ++count;
    this->patch_addr = new list<patch_entry>();
           this->relocation_record = new list<reloc>();

    if (section_size == -1 || section_size > 0) {
       this->content = new list<section_content>();
    }
  }
} sym_entry;

typedef struct {
  string symbol;
  int curr_loc_counter;
} dummy_symbol;

typedef struct sections_content_linker {
  string content;
  int offset;
} sections_content_linker;


typedef struct section_mapped_addresses {
  string section;
  int file;
  int size;
  int mapped_address;
  int processed;
  int allocated;
  list<reloc>* local_reloc_records = nullptr;
  list<sections_content_linker>* local_section_content = nullptr;

  section_mapped_addresses(string section, int mapped_address) {
    this->section = section;
    this->mapped_address = mapped_address;
  }

  section_mapped_addresses(string section, int file, int size) {
    this->section = section;
    this->file = file;
    this->mapped_address = 0;
    this->size = size;
    this->processed = 0;
    this->allocated = 0;
    local_reloc_records = new list<reloc>();
    local_section_content = new list<sections_content_linker>();
  }
  
} section_mapped_addresses;

typedef struct memory_el {
  int address;
  string content;
} memory_el;



#endif