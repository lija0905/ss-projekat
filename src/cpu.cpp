#include "../inc/cpu.hpp"

void CPU::setI(int i) {
  this->psw |= ((i & 0x01) << 15);
}

void CPU::setZ(int i){
  this->psw |= ((i & 0x01) << 0);
}

void CPU::setC(int i){
  this->psw |= ((i & 0x01) << 2);
}

void CPU::setN(int i){
  this->psw |= ((i & 0x01) << 3);
}

void CPU::setO(int i){
  this->psw |= ((i & 0x01) << 1);
}

short CPU::getI() {
  return (psw >> 15) & 0x01;
}

short CPU::getZ() {
  return (psw >> 0) & 0x01;
}

short CPU::getC() {
  return (psw >> 2) & 0x01;
}

short CPU::getN() {
  return (psw >> 3) & 0x01;
}

short CPU::getO() {
  return (psw >> 1) & 0x01;
}