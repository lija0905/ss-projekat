#include "../inc/linker.hpp"

int sym_entry::count = 0;
int section_content::count_content;

void Linker::readFile(const char* name, int num) {

  char finput[128];
  //strcpy(finput, "tests/");
  strcpy(finput, name);

  ifstream file;
  string line, skip;
  file.open(finput);

  if (file.is_open())
  {//dodajem komentar
    while (getline (file,line))
    {
      if (line.find("Symbol table")!=string::npos) {
        skipLines(file, 3);
        getline(file, line);
  
         list<sym_entry> *loc_list = new list<sym_entry>();
          while (line.find("-----") == string::npos) {
    
            std::vector<std::string> tokens;
            splitString(tokens, line);

            //cout << line << endl;
            
            //put the symbol in global table and after that we will make right adresses
            sym_entry entry = sym_entry(tokens[0], tokens[1], stoi(tokens[2]), stoi(tokens[3]), stoi(tokens[5]));
            loc_list->push_back(entry);

            //if it is a section
            if (stoi(tokens[5]) > 0) {
              section_mapped_addresses sections = section_mapped_addresses(tokens[0], num, stoi(tokens[5]));
              all_sections->push_back(sections);
            }
              getline(file, line);

          }

          symbol_tables.push_back(loc_list);
      }
      if (line.find("Sections content")!=string::npos) {
          skipLines(file, 1);
          while (line.find("------") == string::npos) {
             getline(file, line);

             std::vector<std::string> tokens;
             splitString(tokens, line);
             createContent(tokens, tokens[0], num); 
             
          }
          //cout << "ended content " << endl;
      }

      if (line.find("Relocation Records")!=string::npos) {
        getline(file, line);
        while(1) {
          getline(file, line);
          if (line.find(".section")==string::npos) break;
             
          std::vector<std::string> tokens;
          splitString(tokens, line); 
          string section_name = tokens[1];

          skipLines(file, 3);
          getline(file, line);
            while(line.find("------") == string::npos) {

             std::vector<std::string> relocation;
             splitString(relocation, line);
             createRelocation(relocation, section_name, num);

             getline(file, line);

            }
        }
      }

      //cout << "end reloc" << endl;

    }

  /*  this->printSections();
  printRelocation();
    printContent();*/
    file.close();
  }

}

void Linker::allocateSectionsDefault(){

  list<section_mapped_addresses>::iterator iter;
  list<section_mapped_addresses>::iterator iter2;
  int start_address = 0;

  for (iter = all_sections->begin(); iter!=all_sections->end(); iter++) {
    if (iter->processed == 0) {
        iter->mapped_address = start_address;
        iter->processed = 1;
        start_address+=iter->size;

        for (iter2 = all_sections->begin(); iter2!=all_sections->end(); iter2++) {
            if (iter2->processed == 0 && iter2->section == iter->section) {
                 iter2->mapped_address = start_address;
                 iter2->processed = 1;
                 start_address+=iter2->size;
            }
        }
    }
  
  }

}

void Linker::allocateSectionsWithPlaces(list<section_mapped_addresses>* places) {

  list<section_mapped_addresses>::iterator iter;
  list<section_mapped_addresses>::iterator iter2;
  list<section_mapped_addresses>::iterator iter3;

  for (iter = places->begin(); iter!=places->end(); iter++) {

    int start_address = iter->mapped_address;

    for (iter2 = all_sections->begin(); iter2!=all_sections->end(); iter2++) {

    if (iter2->processed == 0 && iter->section == iter2->section) {
        iter2->mapped_address = start_address;
        iter2->processed = 1;
        start_address+=iter2->size;
        iter->size+=iter2->size;

        for (iter3 = all_sections->begin(); iter3!=all_sections->end(); iter3++) {
            if (iter3->processed == 0 && iter3->section == iter2->section) {
                 iter3->mapped_address = start_address;
                 iter3->processed = 1;
                 start_address+=iter3->size;
                 iter->size+=iter2->size;
            }
        }
    }
  
  }

  }

  //checking if sections are overlaping
  bool overlap = false;
  int max_address = places->begin()->mapped_address + places->begin()->size;
  for (iter = places->begin(); iter != places->end(); iter++) {
      for (iter2 = places->begin(); iter2 != places->end(); iter2++) {
          if (iter->section != iter2->section) {
            if ((iter->mapped_address <= iter2->mapped_address && iter->mapped_address + iter->size >= iter2->mapped_address) ||
            (iter->mapped_address >= iter2->mapped_address && iter2->mapped_address + iter2->size >= iter->mapped_address)) overlap = true;
            else {
              if (iter->mapped_address > iter2->mapped_address) max_address = iter->mapped_address + iter->size;
              else max_address = iter2->mapped_address + iter2->size;
            }
          }
      }
  }

  if (overlap) {
    cout << "Error! Sections are overlaping!" << endl; exit(-1);
  }

  //allocate other sections
  for (iter = all_sections->begin(); iter!=all_sections->end(); iter++) {
    if (iter->processed == 0) {
        iter->mapped_address = max_address;
        iter->processed = 1;
        max_address+=iter->size;

        for (iter2 = all_sections->begin(); iter2!=all_sections->end(); iter2++) {
            if (iter2->processed == 0 && iter2->section == iter->section) {
                 iter2->mapped_address = max_address;
                 iter2->processed = 1;
                 max_address+=iter2->size;
            }
        }
    }
  
  }

  //places->clear();
  
}

void Linker::updateSymbolsAddresses() {
  list<sym_entry>::iterator iter;
  vector<string> *undefined = new vector<string>();

  for (int i = 0; i < symbol_tables.size(); i++) {
    for (iter = symbol_tables[i]->begin(); iter!=symbol_tables[i]->end(); iter++){
      if (iter->section!="undefined") {
        //if it is a section check if already added into symbol table
        if (iter->section_size > 0 && sectionAddedToSymTable(iter->section, iter->section_size)) continue;
        else if (checkIfSymbolExists(iter->label, 0)) {
          cout << "Error! Symbol " << iter->label <<  " already defined." << endl; return;
        }
        sym_entry entry = sym_entry(iter->label, iter->section, iter->offset+getStartOfSection(iter->section, i+1), iter->global, iter->section_size);
        global_symbol_table->push_back(entry);
      } else {
        undefined->push_back(iter->label);
      }
    }
  }

  vector<string>::iterator vec;
  for (vec = undefined->begin(); vec!=undefined->end(); vec++) {
    if (checkIfSymbolExists(*vec, 1)) continue;
    cout << "Error! Symbol " << *vec << " not defined." << endl; exit(-1);
  }


}

int Linker::getStartOfSection(string section, int file) {
 list<section_mapped_addresses>::iterator iter;

  for (iter = all_sections->begin(); iter!=all_sections->end(); iter++) {
    //-1 indicates that we need first section;
    if (iter->section == section && file == -1) return iter->mapped_address;
    if (iter->section == section && iter->file == file) return iter->mapped_address;
  }

  return 0;
}

bool Linker::sectionAddedToSymTable(string section, int section_size) {
  list<sym_entry>::iterator iter;

  for (iter = global_symbol_table->begin(); iter!=global_symbol_table->end(); iter++) {
    if (iter->section == section && iter->section_size > 0) {
      iter->section_size += section_size;
      return true;
    }
  }
  return false;
}

bool Linker::checkIfSymbolExists(string symbol, int global) {
   list<sym_entry>::iterator iter;

  for (iter = global_symbol_table->begin(); iter!=global_symbol_table->end(); iter++) {
    if (global == 0 && iter->label == symbol) return true;
    else if (iter->label == symbol && iter->global == 1) return true;
    
  }
  return false;
}

void Linker::createContent(std::vector<std::string>& tokens, string section, int file) {

  list<section_mapped_addresses>::iterator iter;

    for (iter = this->all_sections->begin(); iter!=this->all_sections->end(); iter++) {
      if (iter->section == section && iter->file == file) {
        for (int i = 2; i < iter->size + 2; i++) {
          sections_content_linker content;
          content.content = tokens[i];
          content.offset = i - 2;
          iter->local_section_content->push_back(content);
        }
      }
    }


}

void Linker::createRelocation(std::vector<std::string>& tokens, string section, int file) {

   list<section_mapped_addresses>::iterator iter;

    for (iter = this->all_sections->begin(); iter!=this->all_sections->end(); iter++) {
      if (iter->section == section && iter->file == file) {
          reloc relocation;
          relocation.offset = stoi(tokens[0]);
          relocation.type = tokens[1];
          relocation.order = tokens[2];
          relocation.addend = stoi(tokens[3]);
          iter->local_reloc_records->push_back(relocation);
        
      }
    }
}

void Linker::updateContentAddresses() {

    list<section_mapped_addresses>::iterator iter;

    for (iter = this->all_sections->begin(); iter!=this->all_sections->end(); iter++) {
      list<sections_content_linker>::iterator iter2;
        for (iter2 = iter->local_section_content->begin(); iter2!=iter->local_section_content->end(); iter2++) {
          iter2->offset += iter->mapped_address;

        }

    }
}

void Linker::resolveRelocations() {


    list<section_mapped_addresses>::iterator iter;

    for (iter = this->all_sections->begin(); iter!=this->all_sections->end(); iter++) {
      list<reloc>::iterator iter2;
        for (iter2 = iter->local_reloc_records->begin(); iter2!=iter->local_reloc_records->end(); iter2++) {
            //pcREL patch = S+A-P P = sec + offset, S = addr(symb) A = addend
            //absolute = S+A 
              iter2->offset += iter->mapped_address;
              int P = iter2->offset;
              int S = getSymbolAddress(iter2->order);
              int A = iter2->addend; 
            
              if (iter2->type == "R_X86_64_PC32") {
                patchContent(iter->section, iter->file, P, S+A-P);
                //A + S3 - S1
                iter2->addend += iter->mapped_address - getStartOfSection(iter->section, -1);
              } else if (iter2->type == "R_X86_64_32") {
                patchContent(iter->section, iter->file, P, S+A);
              }

              reloc new_reloc;
              new_reloc.addend = iter2->addend;
              new_reloc.offset = iter2->offset;
              new_reloc.order = iter2->order;
              new_reloc.section = iter->section;
              new_reloc.type = iter2->type;
              addRelocationRecord(new_reloc);

        }

    }
}

void Linker::patchContent(string section, int file, int p, int content) {
 list<section_mapped_addresses>::iterator iter;

    for (iter = this->all_sections->begin(); iter!=this->all_sections->end(); iter++) {
      if (iter->section == section && iter->file == file) {
         list<sections_content_linker>::iterator iter2;
         //little endian format..ovde ispostuj moraces u asembleru da ispravljas
        
         for (iter2 = iter->local_section_content->begin(); iter2!=iter->local_section_content->end(); iter2++) {
          std::stringstream stream;
        
          if (iter2->offset == p){
          stream << std::hex << std::setfill('0') << std::setw(2) << (short)(content & 0xFF);
          std::string result(stream.str());
          iter2->content = result;
          } 
          if (iter2->offset == p + 1) {
            stream << std::hex << std::setfill('0') << std::setw(2) << (short)((content >> 8) & 0xFF);
            std::string result(stream.str());
            iter2->content = result;
          }
        }
      }
   
    }
}

void Linker::addRelocationRecord(reloc& reloc) {


   list<sym_entry>::iterator iter;

  for (iter = global_symbol_table->begin(); iter!=global_symbol_table->end(); iter++) {
    if (iter->label == reloc.section) {
      iter->relocation_record->push_back(reloc);
    }
    
  }

}

void Linker::formMemory() {

  list<section_mapped_addresses>::iterator iter;
  list<section_mapped_addresses>::iterator iter2;
  int start_address = 0;
 
  for (iter = all_sections->begin(); iter!=all_sections->end(); iter++) {
    if (iter->allocated == 0) {
        list<memory_el>* mem = new list<memory_el>();

        for (iter2 = all_sections->begin(); iter2!=all_sections->end(); iter2++) {
            if (iter2->allocated== 0 && iter2->section == iter->section) {
                 iter2->allocated = 1;
                 list<sections_content_linker>::iterator iter3;
                 for (iter3 = iter2->local_section_content->begin(); iter3!=iter2->local_section_content->end(); iter3++) {
                    memory_el el;
                    el.address = iter3->offset;
                    el.content = iter3->content;
                    mem->push_back(el);
                    //cout << el.address << " " << el.content << endl;
                   // memory->push_back(el);
                 }
            }
        }
        memory.push_back(mem);
        iter->allocated = 1;
    }
  
  }

}

void Linker::makeHexFile(const char* file) {

  char foutput[128];
  //strcpy(foutput, "tests/");
  strcpy(foutput, file);

  ofstream output;
  output.open(foutput, ios::trunc | ios::ate);
  

  output << "----------------------------------" << endl; 
  output << "Symbol table: " << endl;
  output << "----------------------------------" << endl;
  output <<  "Label: |  Section:  |   Value:  |  Local:   |  Order:  |  Section Size: " << endl;
  output << "----------------------------------" << endl;


  list<sym_entry>::iterator iter2;

  for (iter2 = this->global_symbol_table->begin(); iter2 != this->global_symbol_table->end(); iter2++) {
    output << iter2->label << " " << iter2->section << " " << iter2->offset << " " << iter2->global << " " << iter2->order << " " << iter2->section_size << endl;
  }


  output << "----------------------------------" << endl;
  output << "Relocation Records: " << endl;
  output << "----------------------------------" << endl;


    for (iter2 = this->global_symbol_table->begin(); iter2 != this->global_symbol_table->end(); iter2++) {

    if (iter2->section_size == -2) continue;

    output << ".section " << iter2->label  << endl;
    output << "------------------------------------------------" << endl;
    output <<  "  Offset:  | Type:  |  Symbol:  |   Addend:   " << endl;
    output << "------------------------------------------------" << endl;


     list<reloc>::iterator iter5; 
      for (iter5 = iter2->relocation_record->begin(); iter5 != iter2->relocation_record->end(); iter5++) {

    output << iter5->offset << " " << iter5->type << " " << iter5->order << "  " << iter5->addend <<  endl;
     
      }
      output << "--------------------------------" << endl;

    }

    int start_address = this->getSymbolAddress("my_start");
    output << " Start Address: " << std::hex << setw(4) << setfill('0') << start_address << endl;
    output << "----------------------------------" << endl;

  output << " Memory: " << endl;
  output << "--------------------------------" << endl;
  list<memory_el>::iterator iter;

 for (int cnt = 0; cnt < memory.size(); cnt++) {

  int i = 0;

  for (iter = memory[cnt]->begin(); iter!= memory[cnt]->end(); iter++, i++) {
    if (i % 8 == 0) {
      output << std::hex << std::setfill('0') << std::setw(4) << iter->address << ":";

    }
    output << iter->content << " ";
    if (i % 8 == 7) output << endl;
  }
  output << endl;
 }

  output.close();
}

int Linker::getSymbolAddress(string symbol) {

   list<sym_entry>::iterator iter;

  for (iter = global_symbol_table->begin(); iter!=global_symbol_table->end(); iter++) {
    if (iter->label == symbol) return iter->offset;
    
  }

  return 0;

}

void Linker::skipLines(ifstream& file, int num) {
  
  string idle;
  for (int i = 0; i < num; i++) getline(file, idle);
}

void Linker::splitString(std::vector<std::string>& tokens, string line) {

    std::string buf;                 // Have a buffer string
    std::stringstream ss(line);       // Insert the string into a stream


    while (ss >> buf) {
        tokens.push_back(buf);
    }

}

void Linker::printTables() {

  list<sym_entry>::iterator iter;

for(int i =0; i < symbol_tables.size(); i++) {
  for (iter = symbol_tables[i]->begin(); iter != symbol_tables[i]->end(); iter++) {
    cout << iter->label << ", " << iter->section << ", " << iter->offset << ", size: " << iter->section_size << " global: " << iter->global <<  endl;
  }
}


}

void Linker::printTable() {

  list<sym_entry>::iterator iter;


  for (iter = global_symbol_table->begin(); iter != global_symbol_table->end(); iter++) {
    cout << iter->label << ", " << iter->section << ", " << iter->offset << ", size: " << iter->section_size << " global: " << iter->global <<  endl;
  }


}


void Linker::printSections() {
    list<section_mapped_addresses>::iterator iter;

  for (iter = this->all_sections->begin(); iter != this->all_sections->end(); iter++) {
    cout << iter->section << ", " << iter->file << ", " << iter->mapped_address << endl;
  }
}

void Linker::printContent() {

  list<section_mapped_addresses>::iterator iter;

  for (iter = this->all_sections->begin(); iter != this->all_sections->end(); iter++) {

      cout << iter->section << ":" << iter->file << endl;

     list<sections_content_linker>::iterator iter2; 
      for (iter2 = iter->local_section_content->begin(); iter2 != iter->local_section_content->end(); iter2++) {

           cout << std::hex << std::setfill('0') << std::setw(4) << iter2->offset << ", " << std::setw(2) << iter2->content << endl;
  }

  }
}

void Linker::printRelocations() {
  list<section_mapped_addresses>::iterator iter;

  for (iter = this->all_sections->begin(); iter != this->all_sections->end(); iter++) {

      cout << std::dec << endl;
      cout << iter->section << ":" <<  iter->file << endl;

     list<reloc>::iterator iter2; 
      for (iter2 = iter->local_reloc_records->begin(); iter2 != iter->local_reloc_records->end(); iter2++) {

           cout << iter2->offset << " " << iter2->type  << " " << iter2->addend << " " << iter2->order << endl;
  }

  }
}

void Linker::printRelocation() {

   list<sym_entry>::iterator iter;

  for (iter = this->global_symbol_table->begin(); iter != this->global_symbol_table->end(); iter++) {

    if (iter->section_size > 0) {

      cout << std::dec << endl;
      cout << iter->section << ":" << endl;

     list<reloc>::iterator iter2; 
      for (iter2 = iter->relocation_record->begin(); iter2 != iter->relocation_record->end(); iter2++) {

           cout << iter2->offset << " " << iter2->type  << " " << iter2->addend << " " << iter2->order << endl;
    }

  }

  }

}

void Linker::printMemory() {

  /*list<memory_el>::iterator iter;

  for (iter = memory->begin(); iter!=memory->end(); iter++) {
    cout << std::hex << std::setfill('0') << std::setw(4) << iter->address << " " << iter->content << endl;
  }*/
}
int main (int argc, char* argv[]) {

  Linker *linker = new Linker();

  if (strcmp(argv[1], "-hex")!=0) {
      cout << "Invalid argument!" << endl;  exit(-1);
  }

  int i = 2;
  int numfile = 0;
  const char* output;

  list<section_mapped_addresses>* places = new list<section_mapped_addresses>();

  while(argv[i]!=NULL) {

    string str = argv[i];
 
    if (str.find("place") != string::npos) {

      string section = str.substr(0, str.find("@"));
      string address = str.substr(str.find("@") + 1, str.length());
      address = address.substr(address.find("x") + 1, address.length());
      section = section.substr(section.find("=") + 1 , section.length());
   
      int addr = stoi(address, nullptr, 16);
      section_mapped_addresses place = section_mapped_addresses(section, addr);
      places->push_back(place);
      
    } else if (str.find(".hex")!= string::npos) output = argv[i];
    else {
        linker->readFile(argv[i], numfile++);
    }
    i++;
  }
  //something is up with linker

  if (places->size() > 0) {
      linker->allocateSectionsWithPlaces(places);
  } else linker->allocateSectionsDefault();


  linker->updateSymbolsAddresses();
  linker->updateContentAddresses();
  linker->resolveRelocations();
  linker->formMemory();
  linker->makeHexFile(output);
 //linker->printSections();
 //linker->printTable();

 //linker->printMemory();
 // linker->printRelocations();

  return 0;

}
