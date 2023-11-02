#include "../inc/interpreter.hpp"

#define psw_reg -1
#define pc 7
#define sp 6

int Interpreter::stop = 0;

map<unsigned char, Instructions> Interpreter::instructions {
  {0x00, HALT},
  {0x20, IRET},
  {0x40, RET},
  {0x30, CALL},
  {0x50, JMP},
  {0x51, JEQ},
  {0x52, JNE}, 
  {0x53, JGT},
  {0x10, INT},
  {0x80, NOT},
  {0x60, XCHG},
  {0x70, ADD},
  {0x71, SUB},
  {0x72, MUL},
  {0x73, DIV},
  {0x74, CMP},
  {0x81, ADD},
  {0x82, OR},
  {0x83, XOR},
  {0x84, TEST},
  {0x90, SHL},
  {0x91, SHR},
  {0xA0, LDR},
  {0xB0, STR},

};

void Interpreter::loadMem(const char* name) {

  char finput[128];
 // strcpy(finput, "tests/");
  strcpy(finput, name);

  ifstream file;
  string line;
  file.open(finput);

  if (file.is_open()) {

    while(getline(file, line)) {



      if (line.find("Start Address") != string::npos) cpu->reg[pc] = stoi(line.substr(16, 4), nullptr, 16);
      if (line.find("Memory") == string::npos) continue;
      else {
        getline(file, line); //skiping "-----" characters
        while(getline(file, line)) {
          if (line.size() > 0) {
              int address = stoi(line.substr(0, 4), nullptr, 16);
              int i = 5;//from position 5 in the  string starts the data of the memory

              //cout << std::hex << std::setw(4) << setfill('0');

              while(line.substr(i, 2)!="") {
                cpu->mem[address] = (unsigned char)(stoi(line.substr(i, 2), nullptr, 16));
                //cout << address << " " << (short)cpu->mem[address] << endl;
                address++;
                i += 3;
              }
          }
        }
      }
    }
  }
}

void Interpreter::emulating() {

  unsigned char instruction;
  unsigned char reg;
  unsigned char type_addr;
  unsigned char data_high;
  unsigned char data_low;

  while(!stop) {

    terminal->stdinput();
    this->handleTermInterrupt();

    instruction = cpu->mem[cpu->reg[pc]++];

    int size = this->resolveSizeOfInstruction(instruction);

    if (size == 1) execute(instruction, -1, -1, -1, -1);
    else if (size == 2) {
      reg = cpu->mem[cpu->reg[pc]++];
      execute(instruction, reg, -1, -1, -1);
    } else {
      reg = cpu->mem[cpu->reg[pc]++];
      type_addr = cpu->mem[cpu->reg[pc]++];
      //types that use payload
      if (type_addr == 4 || type_addr == 3 || type_addr == 0 || type_addr == 5) {
        //little endian
        //cout << cpu->reg[pc] << endl;
        data_low = cpu->mem[cpu->reg[pc]++];
        data_high = cpu->mem[cpu->reg[pc]++];
       // cout << "WTF: "<< (short)data_high << " " << (short)data_low << endl;
        execute(instruction, reg, type_addr, data_high, data_low);
      } else execute(instruction, reg, type_addr, -1, -1);

    }


   // this->print();
  }

}

int Interpreter::resolveSizeOfInstruction(unsigned char instr) {

  switch(instructions[instr]) {
    case HALT: case IRET: case RET: 
      return 1; break; 
    case INT: case NOT: case XCHG:  case ADD: case SUB: case MUL:
    case DIV: case CMP: case AND: case OR: case XOR: case TEST:
    case SHL: case SHR: 
      return 2; break;
    default:  return 3; break;

  }
}

void Interpreter::execute(unsigned char instr, unsigned char reg, unsigned char type, unsigned char data_high, unsigned char data_low) {

    short operand;
    
    char regD = (reg & 0xF0) >> 4;
    char regS = (reg & 0x0F); 


    switch(instructions[instr]) {

        case HALT:
          this->halt(); break;
        case INT:
          this->intInstr(reg); break;
        case IRET: 
          this->iret(); break;
        case CALL: 
          operand = this->calculateOperand(instr, regD, type, data_high, data_low);
          this->call(operand, type); break;
        case RET:
          this->ret(); break;
        case JMP:
          operand = this->calculateOperand(instr, regD, type, data_high, data_low);
          this->jmp(operand, type); break;
        case JEQ:
          operand = this->calculateOperand(instr, regD, type, data_high, data_low);
          this->jeq(operand, type); break;
        case JNE:
          operand = this->calculateOperand(instr, regD, type, data_high, data_low);
          this->jne(operand, type); break;
        case JGT:
          operand = this->calculateOperand(instr, regD, type, data_high, data_low);
          this->jgt(operand, type); break;
        case PUSH:
          this->push(regD); break;
        case POP:
          this->pop(regD); break;
        case XCHG: 
          this->xchg(regD, regS); break;
        case ADD: 
          this->add(regD, regS); break;
        case SUB:
          this->sub(regD, regS); break;
        case MUL: 
          this->mul(regD, regS); break;
        case DIV: 
          this->div(regD, regS); break;
        case CMP:
          this->cmp(regD, regS); break;
        case NOT: 
          this->notInstr(regD); break;
        case AND:
          this->andInstr(regD, regS); break;
        case OR:
          this->orInstr(regD, regS); break;
        case XOR: 
          this->xorInstr(regD, regS); break;
        case TEST:
          this->testInstr(regD, regS); break;
        case SHL:
          this->shl(regD, regS); break;
        case SHR: 
          this->shr(regD, regS); break;
        case LDR:
          operand = this->calculateOperand(instr, regS, type, data_high, data_low);
          if ((type >> 4) & 0x04) this->pop(regD);
          else this->ldr(regD, operand); break;
        case STR:
          operand = this->calculateOperand(instr, regS, type, data_high, data_low);
          if ((type >> 4) & 0x01) this->push(regD);
          else this->str(regD, regS, type, operand); break;
    }
}
//adresiranja ldr i str drugacije racunaju operand od jmp... ? pazi da ne zajebes proveri sutra...
short Interpreter::calculateOperand(unsigned char instr, unsigned char reg, unsigned char type, unsigned char data_high, unsigned char data_low){

      switch(type) {
        case 0: return ((data_high & 0xFF) << 8) | (data_low & 0xFF); break;
        case 1: return cpu->reg[reg]; break;
        case 2: {
          if (instructions[instr] == LDR){ 
            return (short)((cpu->mem[cpu->reg[reg]]) | (cpu->mem[cpu->reg[reg] + 1] << 8));
          } else if (instructions[instr] == STR) return cpu->reg[reg];
          else return (short)(cpu->mem[cpu->reg[reg]]); break;
        }
        case 3:
          if (instructions[instr] == LDR) {
            short addr = ((data_high & 0xFF) << 8) | (data_low & 0xFF);
            //cout << "addr: " << cpu->reg[reg] + addr << endl;
            return (((short)(cpu->mem[cpu->reg[reg] + addr])) | (((short)(cpu->mem[cpu->reg[reg] +addr +1])) << 8));
          } else if (instructions[instr] == STR) return (short)(cpu->reg[reg] + ((data_high & 0xFF) << 8) | (data_low & 0xFF));
          return ((short)(cpu->mem[cpu->reg[reg] + ((data_high & 0xFF) << 8) | (data_low & 0xFF)])); break;
        case 4: {
          if (instructions[instr] == LDR){ 
            short addr = ((data_high & 0xFF) << 8) | (data_low & 0xFF);
            //if (addr == term_in) printf("%c\n",(cpu->mem[addr] | (cpu->mem[addr + 1] << 8)));
            return (short)(cpu->mem[addr] | (cpu->mem[addr + 1] << 8));
          } else if (instructions[instr] == STR) return (((data_high & 0xFF) << 8) | (data_low & 0xFF));
          return (short)(cpu->mem[((data_high & 0xFF) << 8) | (data_low & 0xFF)]); break;
        }
        case 5:{
          //cout << "pcrel : " << cpu->reg[pc] << endl;

          return ((short)(((data_high & 0xFF) << 8) | (data_low & 0xFF))); break;

        } 
        
      }

      return 0;
}

void Interpreter::handleTermInterrupt() {

  if ((cpu->psw >> 15) & 1) return;

  if (((cpu->psw >> 14) == 0) && ((cpu->int_ev >> 3) & 0x01)) {

        cpu->int_ev &= ~(1 << 3);
        //instr
        push(pc);
        push(psw_reg);
        cpu->reg[pc] = cpu->mem[(3 % 8) * 2] | ((cpu->mem[(3 % 8)*2 + 1]) << 8);
  }
}

void Interpreter::halt() {
    //stop executing
    Interpreter::stop = 1;
}

//int->push(pc) ? 
void Interpreter::intInstr(unsigned char regD) {
  push(pc);
  push(psw_reg);
  cpu->reg[pc] = (cpu->mem[(cpu->reg[regD] % 8) * 2] & 0xFF) | ((cpu->mem[(cpu->reg[regD] % 8)*2 + 1] & 0xFF) << 8);
  //set psw i to disabled = 1; !
}

void Interpreter::iret(){
  pop(psw_reg);
  pop(pc);
  //cout << (short)cpu->mem[term_out] << endl;

}

void Interpreter::jmp(short operand, short type) {
    if (type == 1 || type == 2 || type == 3) cpu->reg[pc] = operand;
    else cpu->reg[pc] += operand;
}

void Interpreter::jne(short operand, short type) {
  if (!cpu->getZ()){
        if (type == 1 || type == 2 || type == 3) cpu->reg[pc] = operand;
    else cpu->reg[pc] += operand;
  }
}

void Interpreter::jeq(short operand, short type) {
  //jeq - compare with bit zero
  if (cpu->getZ()) {
       if (type == 1 || type == 2 || type == 3) cpu->reg[pc] = operand;
    else cpu->reg[pc] += operand;
  }
}
 
void Interpreter::jgt(short operand, short type) {
  //see condition - I can chack just N ?
  if (!cpu->getZ() && ((cpu->getC() && !cpu->getN()) || (!cpu->getC() && !cpu->getN() && !cpu->getO()))) {
    if (type == 1 || type == 2 || type == 3) cpu->reg[pc] = operand;
    else cpu->reg[pc] += operand;
  }

}

void Interpreter::call(short operand, short type) {
  push(pc);
  if (type == 1 || type == 2 || type == 3) cpu->reg[pc] = operand;
  else cpu->reg[pc] += operand;
}

void Interpreter::ret() {
  pop(pc);
}

void Interpreter::xchg(char regD, char regS) {
  short tmp = cpu->reg[regD];
  cpu->reg[regD] = cpu->reg[regS];
  cpu->reg[regS] = tmp;
}

void Interpreter::add(char regD, char regS) {
  cpu->reg[regD] += cpu->reg[regS];
}

void Interpreter::sub(char regD, char regS){ 
  cpu->reg[regD] -= cpu->reg[regS];
}

void Interpreter::mul(char regD, char regS) {
  cpu->reg[regD] *= cpu->reg[regS];
}

void Interpreter::div(char regD, char regS){
  cpu->reg[regD] /= cpu->reg[regS];
}

void Interpreter::cmp(char regD, char regS) {
  //CMP
  short temp = cpu->reg[regD] - cpu->reg[regS];
  if (temp == 0) cpu->setZ(1);
  if (temp < 0) {
    cpu->setZ(0); cpu->setN(1); 
    if (cpu->reg[regD] < 0 && cpu->reg[regS] > 0) cpu->setC(1);
    else cpu->setC(1);
  } else {
    cpu->setZ(0); cpu->setN(0);
    if (cpu->reg[regD] > 0 && cpu->reg[regS] < 0) cpu->setC(0);
    else cpu->setC(1);
  }
}

void Interpreter::notInstr(char regD){
  cpu->reg[regD] = ~cpu->reg[regD];
}

void Interpreter::andInstr(char regD, char regS) {
  cpu->reg[regD] &= cpu->reg[regS];
}

void Interpreter::orInstr(char regD, char regS) {
  cpu->reg[regD] |= cpu->reg[regS];
}

void Interpreter::xorInstr(char regD, char regS){
  cpu->reg[regD] ^= cpu->reg[regS];
}

void Interpreter::testInstr(char regD, char regS) {
  short tmp = cpu->reg[regD] & cpu->reg[regS];
  if (!tmp) cpu->setZ(1);
  if ((tmp >> 15) & 0x01) cpu->setN(1);
}

void Interpreter::shl(char regD, char regS) {
  int bit = cpu->reg[regD] & (1 << 16 - regS);
  cpu->reg[regD] <<= cpu->reg[regS];
  if (!cpu->reg[regD]) cpu->setZ(1);
  if ((cpu->reg[regD] >> 15) & 0x01) cpu->setN(1);
  if (bit) cpu->setC(1);
}

void Interpreter::shr(char regD, char regS) {
  int bit = cpu->reg[regD] & (1 << regS - 1);//arithmetic shift right?
  cpu->reg[regD] >>= cpu->reg[regS] | (cpu->reg[regD] & (1 << 15));
  if (!cpu->reg[regD]) cpu->setZ(1);
  if ((cpu->reg[regD] >> 15) & 0x01) cpu->setN(1);
  if (bit) cpu->setC(1);
}

void Interpreter::ldr(char regD, short operand) {
    cpu->reg[regD] = operand;
  
}

void Interpreter::str(char regD, char regS, char type, short operand){
      //cout << "store: " << operand << endl;

    if (type == 1) {
      cpu->reg[regD] = cpu->reg[regS];
    } else {

      //cout << operand << endl;
      
      if (operand == term_out) {
        printf("%c\n", (unsigned char)(cpu->reg[regD] & 0xFF));
      }

      cpu->mem[operand++] = cpu->reg[regD] & 0xFF;
      cpu->mem[operand] = (cpu->reg[regD] >> 8) & 0xFF;

      
    }

}
//push
void Interpreter::push(char num) {
  short val;
  if (num == psw_reg) val = cpu->psw;
  else val = cpu->reg[num];
  cpu->mem[--cpu->reg[6]] = (val >> 8) & 0xFF;
  cpu->mem[--cpu->reg[6]] = val & 0x00FF;
  //cout << "push: " << val << endl;
 
}

//pop
void Interpreter::pop(char num) {
  if(num == psw_reg) {
    cpu->psw = cpu->mem[cpu->reg[6]++] & 0xFF;
    cpu->psw |= (cpu->mem[cpu->reg[6]++] & 0xFF) << 8;
  } else {
    cpu->reg[num] = cpu->mem[cpu->reg[6]++] & 0xFF;
    cpu->reg[num] |= (cpu->mem[cpu->reg[6]++] & 0xFF) << 8;
    //cout << "pop: " << cpu->reg[num] << endl;
  }
}

void Interpreter::print() {
 
  cout << "---------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state: psw=" << std::hex << "0x" <<  std::setw(4) << std::setfill('0') << cpu->psw << endl;

  for (int i = 0; i < 8; i ++) {
    cout << std::dec << "reg" << i << "=";
    cout << std::hex << "0x" <<  std::setw(4) << std::setfill('0') << (this->cpu->reg[i] & 0xFFFF) << " ";
    if ((i + 1) % 4 == 0) cout << endl;
  }

  cout << endl;

}

int main(int argc, char* argv[]) {

  char* finput = argv[1];
  Interpreter interpreter =Interpreter();

  cout << "Started emulating" << endl;


  interpreter.loadMem(finput);
  interpreter.emulating();
  interpreter.print(); 

}