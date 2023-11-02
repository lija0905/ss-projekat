#include "../inc/assembler.hpp"


int sym_entry::count = 0;
int section_content::count_content = 0;

void Assembler::addSymbol(string label, string section, int offset, int global, int section_size) {

    string sym_section = symbolExists(label);
     if(sym_section == "undefined"){
        //this->printPatch(label);
        this->resolveLabel(label, section, offset);
     } else if (sym_section!="nullptr" && global == 1) {
        this->setGlobal(section);
     } else {
      sym_entry entry = sym_entry(label, section, offset, global, section_size);
        symbol_table->push_back(entry);
     }

   // this->printTable();

}

void Assembler::addSymbols(list<dummy_symbol> *symbols, string section, int offset, int global, int section_size) {

  while (symbols->size() > 0) {
    this->addSymbol(symbols->front().symbol, section, offset, global, section_size);
    symbols->pop_front();
  }
}

void Assembler::setSectionSize(string section, int size) {
  
  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == section && iter->section_size == -1) iter->section_size = size;
  }

}

void Assembler::setGlobal(string symbol) {

   list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == symbol) iter->global = 1;
  }
}

void Assembler::resolveLabel(string symbol, string section, int offset) {
  
  list<sym_entry>::iterator iter;
  list<patch_entry>* patch_address;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == symbol && iter->section == "undefined") {
          iter->section = section;
          iter->offset = offset; 
          patch_address = iter->patch_addr;
    } 
  }

  list<patch_entry>::iterator iter1;
   for (iter1 = patch_address->begin(); iter1 != patch_address->end(); iter1++) {
      int addr = iter1->addr;

      //need to patch just if it is in the same section
      if (iter1->section == section && iter1->flag_pc_rel == 1) {
        list<section_content>* content = this->findContent(iter1->section, -1);
        list<section_content>::iterator iter2;
          for (iter2 = content->begin(); iter2!=content->end(); iter2++) {
            if (iter2->offset == addr) iter2->content = (short)((offset - iter1->current_loc_counter) & 0xFF);
            if (iter2->offset == addr + 1) iter2->content = (short)((offset - iter1->current_loc_counter) >> 8);
          }

        iter1->deleted = 1;
      }    

  }

}

void Assembler::addByteToSection(string section, short byte) {


  list<section_content>* content = findContent(section, -1);

  if (content) {
      section_content content_elem;
      content_elem.content = byte;
      content_elem.offset = section_content::count_content++;
              // cout << section << " " << content_elem.content << " " << content_elem.offset << endl;

   
      content->push_back(content_elem);
  } else { cout << "Error!" << endl; }

}



void Assembler::addBytesToSection(string section, list<dummy_symbol> *symbols) {

    while(symbols->size() > 0) {
      string symbol = symbols->front().symbol;
      int address = this->getSymbolAddr(symbol);
      this->addByteToSection(section, 0x00);
      this->addByteToSection(section, 0x00);
      if (address < 0) this->addToPatchList(symbol, section, address, symbols->front().curr_loc_counter);//if label isn't defined yet
      symbols->pop_front();
    }

}

void Assembler::addToPatchList(string symbol, string section, int address, int current_loc_counter, int flag_pc_rel) {

  if (address == -2) this->addSymbol(symbol, "undefined", -1, -1, -2);
  patch_entry entry;
  entry.addr = getAddressToPatch(section) - 1;
  entry.section = section;
  entry.current_loc_counter = current_loc_counter;
  entry.flag_pc_rel = flag_pc_rel;
  entry.deleted = 0;

  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == symbol) {
     iter->patch_addr->push_back(entry);
    }
  }

}

int Assembler::getSymbolAddr(string label) {
  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == label) return iter->offset;
  }

  return -2;
}

short Assembler::getLastByte(string section) {
  list<section_content>* content = findContent(section, -1);

  section_content byte = content->back();
  content->pop_back(); //need to remove it and that it will be pushed back
  section_content::count_content--;
 // cout << "kk: " << byte.offset << " " << byte.content << endl;
  return byte.content;
}

//we need place in content where should be putted
int Assembler::getAddressToPatch(string section) {
  list<section_content>* content = findContent(section, -1);

  section_content byte = content->back();
  return byte.offset;
}


list<section_content>* Assembler::findContent(string label, int flag) {

  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == label && (iter->section_size == flag || iter->section_size > 0)) return iter->content;
  }

  return nullptr;
}

list<reloc>* Assembler::getRelocRecordsList(string label) {

  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == label && (iter->section_size > 0)) return iter->relocation_record;
  }

  return nullptr;
  
}

string Assembler::symbolExists(string label) {

  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    if (iter->label == label) return iter->section;
  }

  return "nullptr";

}


void Assembler::crateRelocationRecord() {

    list<sym_entry>::iterator iter;

      for (iter = this->symbol_table->begin(); iter!=this->symbol_table->end(); iter++) {
        list<patch_entry>::iterator iter2;

          for (iter2 = iter->patch_addr->begin(); iter2!=iter->patch_addr->end(); iter2++){
             if (iter2->deleted == 1) continue; 
             reloc record;
             record.section = iter2->section;
             record.offset = iter2->addr;
             if (iter2->flag_pc_rel == 1) {
              record.type = "R_X86_64_PC32";
              if (iter->global == 1) record.addend = -2;
              else record.addend = iter->offset - 2;
             }
             else {
              record.type = "R_X86_64_32";
              if (iter->global == 1) record.addend = 0;
              else record.addend = iter->offset;
             }
             if (iter->global == 1) record.order = iter->label;
             else record.order = iter->section;

             list<reloc>* relocation_list = this->getRelocRecordsList(iter2->section);
              relocation_list->push_back(record);
         }

      }

}

void Assembler::printTable() {

  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    cout << iter->label << ", " << iter->section << ", " << iter->offset << ", size: " << iter->section_size << " global: " << iter->global <<  endl;
  }


}

void Assembler::printContent() {

  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {

    if (iter->section_size == -2) continue;

      cout << iter->label << ":" << endl;

     list<section_content>::iterator iter2; 
      for (iter2 = iter->content->begin(); iter2 != iter->content->end(); iter2++) {

           cout << iter2->offset << ", " << std::hex << iter2->content << endl;
  }

  }
}

void Assembler::printPatch(string symbol) {

  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {

    if (iter->label == symbol) {

      cout << iter->label << ":" << endl;

     list<patch_entry>::iterator iter2; 
      for (iter2 = iter->patch_addr->begin(); iter2 != iter->patch_addr->end(); iter2++) {

           cout << iter2->section << ", " << std::hex << iter2->addr << endl;
  }
    }
  }

}

void Assembler::makeOutputFile(const char* file) {

  char foutput[128];
  //strcpy(foutput, "tests/");
  strcpy(foutput, file);

  ofstream output;
  output.open(foutput, ios::trunc | ios::ate);

  output << "----------------------------------" << endl; 
  output << "Symbol table: " << endl;
  output << "----------------------------------" << endl;
  output <<  "Label: |  Section:  |   Offset:  |  Local:   |  Order:  |  Section Size: " << endl;
  output << "----------------------------------" << endl;


  list<sym_entry>::iterator iter;

  for (iter = this->symbol_table->begin(); iter != this->symbol_table->end(); iter++) {
    output << iter->label << " " << iter->section << " " << iter->offset << " " << iter->global << " " << iter->order << " " << iter->section_size << endl;
  }

  output << "----------------------------------" << endl;
  output << "Sections content: " << endl;
  output << "----------------------------------" << endl;
  
  list<sym_entry>::iterator iter2;

  for (iter2 = this->symbol_table->begin(); iter2 != this->symbol_table->end(); iter2++) {

    if (iter2->section_size == -2) continue;

/*cout << iter2->label << " ";
    cout << iter2->content->size() << " ";
    cout << iter2->section_size << endl;*/
     output << iter2->label << " -> ";

     list<section_content>::iterator iter3; 
      for (iter3 = iter2->content->begin(); iter3 != iter2->content->end(); iter3++) {

           output << std::hex << std::setfill('0') << std::setw(2) << iter3->content  << " ";
      }

      output << std::dec << endl;

  }

  output << "----------------------------------" << endl;
  output << "Relocation Records: " << endl;
  output << "----------------------------------" << endl;



    for (iter2 = this->symbol_table->begin(); iter2 != this->symbol_table->end(); iter2++) {

    if (iter2->section_size == -2) continue;

    output << ".section " << iter2->label  << endl;
    output << "------------------------------------------------" << endl;
    output <<  "  Offset:  | Type:  |  Symbol :  |   Addend:   " << endl;
    output << "------------------------------------------------" << endl;


     list<reloc>::iterator iter5; 
      for (iter5 = iter2->relocation_record->begin(); iter5 != iter2->relocation_record->end(); iter5++) {

    output << iter5->offset << " " << iter5->type << " " << iter5->order << "  " << iter5->addend <<  endl;
     
      }

    output << "------------------------------------------------" << endl;

  }

output.close();

}