%{
  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  
  #include <iostream>
  #include <fstream> 


  #include "../inc/assembler.hpp"
  #include "../inc/types.hpp"

  using namespace std;


  extern int yylex();
  extern int yyparse();
  Assembler assm = Assembler();
  extern FILE* yyin;

  int loc_counter = 0;
  short reg_dest = 0;
  short address = 0;
  short current_operand;
  string symbol_to_patch = "";
  string symbol_to_patch_section = "";
  int is_pc_relative = 0;
  int flag_pc_rel = 0;

  string current_section = "";
  list<dummy_symbol> *sym_list = new list<dummy_symbol>();



  void yyerror(const char* s);

%}

%union {
  int number;
  char* ident;
}


%token GLOBAL EXTERN SECTION WORD SKIP ASCII EQU END
%token HALT INT IRET CALL RET JMP JEQ JNE JGT PUSH POP XCHG 
%token ADD SUB MUL DIV CMP NOT OR AND XOR TEST SHR SHL LDR STR REG
%token PLUS MINUS STAR DIVISION DOLLAR PERCENT COMMA COLON LBRACKET RBRACKET LPAREN RPAREN QUOTE SLASH NEWL
%token <number> NUMBER
%token <ident> IDENT

%type<ident> symbol;
%type<number> literal;
%type<ident> string;


%start program

%%

program: line
         | program line
;

line: direct_instruct
      | label direct_instruct
;

direct_instruct: directive
                | instruction
                | NEWL;
;

label: IDENT COLON {
  //treba povezati global sa ovim svim, global zapravo samo postavlja da je globalna, slucaj kad je u tabeli i kad nije.
  assm.addSymbol($1, current_section, loc_counter, 0, -2);
}    
;

directive: GLOBAL symbol_list { 
            //-2 indicates that it's not a section, setting default to section_size instead having another flag
            assm.addSymbols(sym_list, "undefined", -1, 1, -2);
          }
          |
          EXTERN symbol_list {
            //same as global, just the offset and section are undifiend (-1)
            assm.addSymbols(sym_list, "undefined", -1, 1, -2);
          }
          |
          SECTION symbol { 
            //we need to update size of current section before reseting the loc counter and switching sections
            if (current_section != "") {
                assm.setSectionSize(current_section, loc_counter);
            }
            loc_counter = 0;
            //-1 indicates that this is section, and it's size should be updated latter.
            assm.addSymbol($2, $2, loc_counter, 0, -1);
            section_content::count_content = 0;

            current_section = $2;
          }
          |
          WORD symbol_list_lit {
          }
            //povezano sa referisanjem unapred
          |
          SKIP literal {
            loc_counter += $2;
            for (int i = 0; i < $2; i++) assm.addByteToSection(current_section, 0x00);
           // assm.printContent();
          }
          |
          ASCII string { 
            //God free me from this code...
            char* ch = $2;
            std::string str = std::string(ch);

            std::string::iterator it;
            for (it = str.begin(); it != str.end(); it++) {
                assm.addByteToSection(current_section, *it);
            }
        
            assm.addByteToSection(current_section, 0x5c);
            assm.addByteToSection(current_section, 0x30);
            //assm.printContent();
          }
          |
          END {
            if (current_section != "") {
                assm.setSectionSize(current_section, loc_counter);
            }
            loc_counter = 0;
          }
;

instruction: single_instructions
             |
             op_instructions operand_jmp {
              assm.addByteToSection(current_section, address);
              if (address != 0x01 && address != 0x02) { 

                if (symbol_to_patch!="") symbol_to_patch_section = assm.symbolExists(symbol_to_patch);
                else symbol_to_patch_section="";

                if ((current_operand < 0 && symbol_to_patch!="") || symbol_to_patch_section!=current_section){    
                  assm.addByteToSection(current_section, 0x00);
                  assm.addByteToSection(current_section, 0x00);         
                  assm.addToPatchList(symbol_to_patch, current_section, current_operand, loc_counter, flag_pc_rel);
                } else {
                 if (flag_pc_rel == 1) current_operand = current_operand - loc_counter;
                 assm.addByteToSection(current_section, (current_operand)& 0xFF);
                 assm.addByteToSection(current_section, ((current_operand) >> 8) & 0xFF);
                }
                symbol_to_patch =""; current_operand = 0; flag_pc_rel = 0;
              } 
             }
             |
             reg1_instructions REG { 
              assm.addByteToSection(current_section, (short)yylval.number << 4);
             }
             |
             reg2_instructions reg COMMA REG {  
                assm.addByteToSection(current_section, (reg_dest << 4) | (short)yylval.number);
             }
             |
             ldr_str COMMA operand_data { //ako imaju veze sa registrima
                 if (address == 0x01 || address == 0x02) {
                  short byte = assm.getLastByte(current_section);
                  assm.addByteToSection(current_section, ((byte & 0xF0) | (current_operand & 0x0F)));
                  assm.addByteToSection(current_section, address);
                 } else {
                  assm.addByteToSection(current_section, address);
                  //if label isn't defined yet, add address to patch
                  if (current_operand < 0 && symbol_to_patch!="") {
                    assm.addByteToSection(current_section, 0x00);
                    assm.addByteToSection(current_section, 0x00);
                    assm.addToPatchList(symbol_to_patch, current_section, current_operand, loc_counter, flag_pc_rel);
                 } else {
                    assm.addByteToSection(current_section, current_operand & 0xFF);
                    assm.addByteToSection(current_section, (current_operand >> 8) & 0xFF);
                 }
              }
 
                 address = 0; current_operand = 0; symbol_to_patch = ""; flag_pc_rel = 0;
             }
             |
             stack_instructions;
;

single_instructions: HALT { 
                      loc_counter += 1;
                      assm.addByteToSection(current_section, 0x00);
                    }
                    |
                    IRET { loc_counter += 1;
                        assm.addByteToSection(current_section, 0x20); }
                    |
                    RET { loc_counter += 1;
                        assm.addByteToSection(current_section, 0x40); }
;

op_instructions: CALL {
                  loc_counter += 1;
                  assm.addByteToSection(current_section, 0x30);
                 }
                 |
                 JMP {
                  loc_counter += 1; //ovde uvecavamo za 1, a posle u zavisnosti od adresiranja uvecavamo jos
                  assm.addByteToSection(current_section, 0x50);
                 }
                 |
                 JEQ {
                  loc_counter += 1; //ovde uvecavamo za 1, a posle u zavisnosti od adresiranja uvecavamo jos
                  assm.addByteToSection(current_section, 0x51);
                 }
                 |
                 JNE {
                  loc_counter += 1; //ovde uvecavamo za 1, a posle u zavisnosti od adresiranja uvecavamo jos
                  assm.addByteToSection(current_section, 0x52);
                 }
                 |
                 JGT {
                  loc_counter += 1; //ovde uvecavamo za 1, a posle u zavisnosti od adresiranja uvecavamo jos
                  assm.addByteToSection(current_section, 0x53);
                 }
;

reg1_instructions: INT {
                    loc_counter += 2;
                        assm.addByteToSection(current_section, 0x10);
                   }
                   |
                   NOT {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x80);
                   }
;

reg2_instructions: XCHG {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x60); //we will need or with REG number later
                   }
                   |
                   ADD {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x70);
                   }
                   |
                   SUB {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x71);
                   }
                   |
                   MUL {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x72);
                   }
                   |
                   DIV {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x73);
                   }
                   |
                   CMP {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x74);
                   }
                   |
                   AND {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x81);
                   }
                   |
                   OR {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x82);
                   }
                   |
                   XOR {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x83);
                   }
                   |
                   TEST {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x84);
                   }
                   |
                   SHL {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x90);
                   }
                   |
                   SHR {
                    loc_counter += 2;
                    assm.addByteToSection(current_section, 0x91);
                   }
;

ldr_str: LDR REG {
            loc_counter += 2;
            assm.addByteToSection(current_section, 0xA0);
            assm.addByteToSection(current_section, ((short)yylval.number << 4) | 0x00);
          }
         |
         STR REG {
          loc_counter += 2;
            assm.addByteToSection(current_section, 0xB0);
            assm.addByteToSection(current_section, ((short)yylval.number << 4) | 0x00);
         }
;

stack_instructions: PUSH REG{
                    loc_counter += 3;
                    assm.addByteToSection(current_section, 0xB0);
                    assm.addByteToSection(current_section, (short)(yylval.number << 4 | 0x06));
                    assm.addByteToSection(current_section, (0x10) | 0x01);
                   }
                   |
                   POP REG {
                    loc_counter += 3;
                    assm.addByteToSection(current_section, 0xA0);
                    assm.addByteToSection(current_section, (short)(yylval.number << 4 | 0x06));
                    assm.addByteToSection(current_section, (short)(0x40 | 0x01));
                   }
                   

symbol_list: symbol {   dummy_symbol sym;
                        sym.symbol = $1;
                        sym.curr_loc_counter = loc_counter;
                        sym_list->push_back(sym);
                    }
             |
             symbol_list COMMA symbol {
               dummy_symbol sym;
                        sym.symbol = $3;
                        sym.curr_loc_counter = loc_counter;
                        sym_list->push_back(sym);
               }
;

//this is used just for .word that's why it is processed here.
symbol_list_lit: symbol_list {
                    loc_counter += 2*(sym_list->size());
                    assm.addBytesToSection(current_section, sym_list);
                 }
                 |
                 literal {
                  loc_counter += 2;
                  short lit = $1;
                  assm.addByteToSection(current_section, lit & 0xFF);
                  assm.addByteToSection(current_section, lit >> 8);
                 }
;

string: QUOTE symbol SLASH NUMBER QUOTE { 
          if ($4 != 0) { yyerror("Error! String has to terminate with \0");} 
          $$ = $2; 
        }
;

symbol: IDENT
;

reg: REG { reg_dest = yylval.number; }
;

operand_data: DOLLAR literal {
              loc_counter += 3;
              address = 0x00;
              current_operand = $2;
              //vrednost literala da se cuva treba za data high i data low, short je max za imm 
             // assm.addByteToSection(current_section, 0x00);
         }
         |
         DOLLAR symbol {
              loc_counter += 3;
              address = 0x00;
              current_operand = assm.getSymbolAddr($2);
              symbol_to_patch = $2;
         }
         |
         literal {
            loc_counter += 3;
            address = 0x04;
            current_operand = $1;
         }
         |
         symbol {
          loc_counter += 3;
          address = 0x04;
          //a ovde je sta je na toj adresii..
          current_operand = assm.getSymbolAddr($1);
          symbol_to_patch = $1;
         }
         |
         PERCENT symbol {
           loc_counter += 3;
           address = 0x03;
           current_operand = assm.getSymbolAddr($2);
           symbol_to_patch = $2;
           flag_pc_rel = 1;
           short byte = assm.getLastByte(current_section);
           assm.addByteToSection(current_section, (byte | 0x07));
           //isto kao registarsko indirektno - razmisli
         }
         |
         REG {
          loc_counter += 1;
          address = 0x01;
          current_operand = yylval.number;
         }
         |
         LBRACKET REG RBRACKET {
          loc_counter += 1;
          address = 0x02;
          current_operand = yylval.number;
         }
         |
         LBRACKET reg PLUS literal RBRACKET {
          loc_counter += 3;
          address = 0x03;
          current_operand = $4; //nisam sigurna za ovo da li je bas njegova vrednost ili vrednost sa te adrese
          //ali mislim da je bas njegova vrednost
          short byte = assm.getLastByte(current_section);
          assm.addByteToSection(current_section, (byte | (reg_dest & 0x0F)));
         }
         |
         LBRACKET reg PLUS symbol RBRACKET {
          loc_counter += 3;
          address = 0x03;
          short byte = assm.getLastByte(current_section);
          assm.addByteToSection(current_section, (byte | (reg_dest & 0x0F)));
          current_operand = assm.getSymbolAddr($4);
          symbol_to_patch = $4;
         }
;

operand_jmp: literal {
              loc_counter += 4;
              address = 0x00;
              current_operand = $1;
              assm.addByteToSection(current_section, 0xFF);
            }
            |
            symbol {
              loc_counter += 4;
              address = 0x00;
              current_operand = assm.getSymbolAddr($1);
              symbol_to_patch = $1;
              flag_pc_rel = 1;
              assm.addByteToSection(current_section, 0xFF);
            }
            |
            PERCENT symbol {
              loc_counter += 4;
              address = 0x05;
              current_operand = assm.getSymbolAddr($2);
              symbol_to_patch = $2;
              flag_pc_rel = 1;
              assm.addByteToSection(current_section, 0x7F);
            }
            |
            STAR literal { 
              loc_counter += 4;
              address = 0x04;
              current_operand = $2;
              assm.addByteToSection(current_section, 0xFF);
            }
            |
            STAR symbol {
              loc_counter += 4;
              address = 0x04;
              current_operand = assm.getSymbolAddr($2);
              symbol_to_patch = $2;
              flag_pc_rel = 1;
              assm.addByteToSection(current_section, 0xFF);
            }
            |
            STAR REG {
              loc_counter += 2;
              address = 0x01;
              assm.addByteToSection(current_section, (short)yylval.number & 0xF0);
            }
            |
            STAR LBRACKET REG RBRACKET {
              loc_counter += 2;
              address = 0x02;
              assm.addByteToSection(current_section, (short)yylval.number & 0xF0);
            }
            |
            STAR LBRACKET reg PLUS literal RBRACKET {
              loc_counter += 4;
              address = 0x03;
              current_operand = $5;
              assm.addByteToSection(current_section, (short)reg_dest & 0xF0);
            }
            |
            STAR LBRACKET reg PLUS symbol RBRACKET {
              loc_counter += 4;
              address = 0x03;
              current_operand =  assm.getSymbolAddr($5);
              symbol_to_patch = $5;
              //flag_pc_rel = 1;
              assm.addByteToSection(current_section, (short)reg_dest & 0xF0);
            }
;

literal: NUMBER { $$ = $1; }
        |
        PLUS NUMBER { $$ = $2; }
        |
        MINUS NUMBER { $$ = -1*$2; }
  
;


%%

int main(int argc, char* argv[]) {

  /*if (!strcmp(argv[1], '-o')) {
    cout << "Invalid command " << argv[1] << endl;
    return -1;
  }*/

  char finput[128];
  //strcpy(finput, "tests/");
  strcpy(finput, argv[3]);


  FILE *file = fopen(finput, "r");

  /*ofstream file;
  file.open(argv[3], ios::out);*/


  if (!file) {
    cout << "Can't find file " << argv[3] << endl; return -1; 
  }

  	yyin = file;

	do {
		yyparse();
	} while(!feof(yyin));

  assm.crateRelocationRecord();
  assm.makeOutputFile(argv[2]);

  fclose(file);


	return 0;
}

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}
